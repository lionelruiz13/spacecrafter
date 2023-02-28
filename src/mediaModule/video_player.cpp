/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - 2020 all rights reserved
*
*/


#include <fstream>
#include <SDL2/SDL.h>
#include <chrono>

//#include "spacecrafter.hpp"
#include "mediaModule/video_player.hpp"
#include "tools/log.hpp"
#include "tools/s_texture.hpp"
#include "mediaModule/media.hpp"

#include "eventModule/event_recorder.hpp"
#include "eventModule/EventVideo.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

VideoPlayer::VideoPlayer(Media* _media)
{
	media = _media;
	m_isVideoPlayed = false;
	m_isVideoInPause = false;
	m_isVideoSeeking = false;
	img_convert_ctx = NULL;
}


VideoPlayer::~VideoPlayer()
{
	media = nullptr;
	stopCurrentVideo(false);
	for (int i = 0; i < 3; i++)
		delete videoTexture.tex[i];
}

void VideoPlayer::createTextures()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	const uint32_t widthMax = 4096;
	const uint32_t heightMax = 2048;
	stagingBuffer = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, widthMax*heightMax*1.5*MAX_CACHED_FRAMES, "Staging video buffer");
	for (int i = 0; i < 3; i++)
		videoTexture.tex[i] = new Texture(vkmgr, *stagingBuffer, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, "Video texture", VK_FORMAT_R8_UNORM);
}

void VideoPlayer::pauseCurrentVideo()
{
	if (m_isVideoInPause==false) {
		m_isVideoInPause = true;
		startPause = SDL_GetTicks();
	}
	else {
		endPause = SDL_GetTicks();
		m_isVideoInPause = !m_isVideoInPause;
		firstCount = firstCount + (endPause - startPause);
		d_lastCount = d_lastCount + (endPause - startPause);
		lastCount = (int)d_lastCount;
	}

	Event* event = new VideoEvent(VIDEO_ORDER::PAUSE);
	EventRecorder::getInstance()->queue(event);
}


void VideoPlayer::init()
{
	m_isVideoPlayed = false;
	m_isVideoInPause= false;
	m_isVideoSeeking = false;
	#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	av_register_all();
	#endif
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
}


bool VideoPlayer::restartCurrentVideo()
{
	if (!m_isVideoPlayed)
		return false;
	threadInterrupt();
	auto result = av_seek_frame(pFormatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
	threadPlay();
	if (result < 0) {
		printf("av_seek_frame forward failed. \n");
		return false;
	}

	firstCount =  SDL_GetTicks();
	currentFrame = 0;

	return true;
}


bool VideoPlayer::playNewVideo(const std::string& _fileName)
{
	if (m_isVideoPlayed)
		stopCurrentVideo(true);
	std::ifstream fichier(_fileName.c_str());
	if (!fichier.fail()) { // check if the video file exists
		cLog::get()->write("Videoplayer: reading file "+ _fileName, LOG_TYPE::L_INFO);
		fileName = _fileName;
	}
	else {
		cLog::get()->write("Videoplayer: error reading file "+ _fileName + " abording...", LOG_TYPE::L_ERROR);
		return false;
	}

	init();

	//internal tests at ffmpeg
	if(avformat_open_input(&pFormatCtx,fileName.c_str(),NULL,NULL)!=0) {
		cLog::get()->write("Couldn't open input stream.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return false;
	}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0) {
		cLog::get()->write("Couldn't find stream information.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return false;
	}
	videoindex=-1;
	for(unsigned int i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex=i;
			break;
		}
	if(videoindex==-1) {
		cLog::get()->write("Didn't find a video stream.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return false;
	}

	video_st = pFormatCtx->streams[videoindex];

	pCodecCtx= avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);

	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL) {
		cLog::get()->write("Unsupported pCodec for video file", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return false;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0) {
		cLog::get()->write("Could not open codec.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return false;
	}

	videoRes.w = pCodecCtx->width;
	videoRes.h = pCodecCtx->height;

	AVRational frame_rate = av_guess_frame_rate(pFormatCtx, video_st, NULL);
	frameRate = frame_rate.num/(double)frame_rate.den;
	frameRateDuration = 1000*frame_rate.den/(double)frame_rate.num;
	nbTotalFrame = static_cast<int>(((pFormatCtx->duration)/1000)/frameRateDuration);

	initTexture();

	img_convert_ctx = NULL;
	unsigned char *out_buffer;

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	if(img_convert_ctx==NULL) {
		cLog::get()->write("Unable to get a context for video file", LOG_TYPE::L_ERROR);
		return false;
	}

	if(pCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P) {
		cLog::get()->write("Video codec isn't in AV_PIX_FMT_YUV420P format", LOG_TYPE::L_ERROR);
		return false;
	}
	pFrameIn = av_frame_alloc();
	pFrameOut=av_frame_alloc();
	out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));
	av_image_fill_arrays(pFrameIn->data, pFrameIn->linesize,out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);
	av_image_fill_arrays(pFrameOut->data, pFrameOut->linesize,out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);

	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	firstCount = SDL_GetTicks();
	lastCount = firstCount;
	d_lastCount = firstCount;
	currentFrame = 0;
	frameCached = 0;
	frameUsed = 0;
	needFrames = 0;
	m_isVideoPlayed = true;
	this->getNextVideoFrame(); // The first frame must be ready on time
	threadPlay();

	Event* event = new VideoEvent(VIDEO_ORDER::PLAY);
	EventRecorder::getInstance()->queue(event);
	return true;
}

void VideoPlayer::update()
{
	if (! m_isVideoPlayed)
		return;

	if (m_isVideoInPause) {
		return;
	}
	int timePassed = SDL_GetTicks()- lastCount;

	// Tell how many frames we need now
	if ( timePassed > 0) {
		++needFrames;
		d_lastCount = firstCount + (int)(frameRateDuration*++currentFrame);
		lastCount = (int)d_lastCount;
	}
}

bool VideoPlayer::getNextFrame()
{
	while (true) {
		if (av_read_frame(pFormatCtx, packet)<0) {
			decoding = false;
			return false;
		}

		if(packet->stream_index==videoindex) {
			int ret = avcodec_send_packet(pCodecCtx, packet);
			if(ret < 0) {
				cLog::get()->write("Decode Error", LOG_TYPE::L_ERROR);
				continue ;
			}
			ret = avcodec_receive_frame(pCodecCtx, pFrameIn);
			if(ret < 0 ) {
				cLog::get()->write("not got frame", LOG_TYPE::L_DEBUG);
				continue;
			}
			if (m_isVideoSeeking) {
				if (pFrameIn->key_frame==1) {
					m_isVideoSeeking=false;
				} else {
					--needFrames;
					av_packet_unref(packet);
					return false;
				}
			}
			av_packet_unref(packet);
			return true;
		}
		av_packet_unref(packet);
	}
}


void VideoPlayer::getNextVideoFrame()
{
	if (getNextFrame()) {
		sws_scale(img_convert_ctx, pFrameIn->data, pFrameIn->linesize, 0, pCodecCtx->height, pFrameOut->data, pFrameOut->linesize);
		for (int i = 0; i < 3; i++) {
			memcpy(pImageBuffer[i][frameCached % MAX_CACHED_FRAMES], pFrameOut->data[i], widths[i] * heights[i]);
		}
		++frameCached;
	}
}


void VideoPlayer::stopCurrentVideo(bool newVideo)
{
	if (m_isVideoPlayed==false) {
		if (thread.joinable())
			thread.join();
		return;
	}

	m_isVideoPlayed = false;
	threadTerminate(); // Don't overlap av_* calls

	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameOut);
	av_frame_free(&pFrameIn);
	avcodec_close(pCodecCtx);

	if (media) {
		Event* event = new VideoEvent(VIDEO_ORDER::STOP);
		EventRecorder::getInstance()->queue(event);
		media->playerStop(newVideo);
	}
}

void VideoPlayer::initTexture()
{
	const int _widths[3]  = { videoRes.w, videoRes.w / 2, videoRes.w / 2 };
	const int _heights[3] = { videoRes.h, videoRes.h / 2, videoRes.h / 2 };
	for(int i=0; i<3; i++) {
		widths[i] = _widths[i];
		heights[i] = _heights[i];
	}

	bool uninitialized = true;
	for (int i = 0; i < 3; ++i) {
		if (videoTexture.tex[i]->isOnGPU()) {
			int width, height;
			videoTexture.tex[i]->getDimensions(width, height);
			if (width == widths[i] && height == heights[i]) {
				uninitialized = false;
			} else {
				// The texture mustn't be in use while processing.
				auto &context = *Context::instance;
				context.helper->waitFrame(context.frameIdx);
				vkQueueWaitIdle(context.graphicQueue);
				videoTexture.tex[i]->unuse();
			}
		}
	}
	if (uninitialized) {
		firstUse = true;
		stagingBuffer->reset();
		videoTexture.sync = std::make_shared<VideoSync>();
		videoTexture.sync->syncOut = std::make_unique<SyncEvent>();
		videoTexture.sync->syncIn = std::make_unique<SyncEvent>();
		for (int i = 0; i < 3; ++i) {
			videoTexture.tex[i]->init(widths[i], heights[i], nullptr, false, 1);
			for (int j = 0; j < MAX_CACHED_FRAMES; ++j) {
				imageBuffers[i][j] = stagingBuffer->fastAcquireBuffer(widths[i] * heights[i]);
				pImageBuffer[i][j] = stagingBuffer->getPtr(imageBuffers[i][j]);
			}
			videoTexture.sync->syncOut->imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR);
			videoTexture.sync->syncIn->imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR);
		}
		videoTexture.sync->syncOut->build();
		videoTexture.sync->syncIn->build();
	}
}


/* lets take a leap forward the video */
bool VideoPlayer::jumpInCurrentVideo(float deltaTime, float &reallyDeltaTime)
{
	if (m_isVideoPlayed==false)
		return false;
	if (m_isVideoInPause==true)
		this->pauseCurrentVideo();

	int64_t frameToSkeep = (1000.0*deltaTime) / frameRateDuration;
	return seekVideo(frameToSkeep, reallyDeltaTime);
}


bool VideoPlayer::invertVideoFlow(float &reallyDeltaTime)
{
	if (m_isVideoPlayed==false)
		return false;
	if (m_isVideoInPause==true)
		this->pauseCurrentVideo();

	return seekVideo(nbTotalFrame - 2*currentFrame, reallyDeltaTime);
}


bool VideoPlayer::seekVideo(int64_t frameToSkeep, float &reallyDeltaTime)
{
	currentFrame = currentFrame + frameToSkeep;

	//jump before the beginning of the video
	if (currentFrame <= 0) {
		this->restartCurrentVideo();
		reallyDeltaTime=0.0;
		return true;
	}
	if(currentFrame < nbTotalFrame) { // we check that we don't jump out of the video
		threadInterrupt();
		if (avformat_seek_file(pFormatCtx, -1, INT64_MIN, currentFrame * frameRateDuration *1000, INT64_MAX, AVSEEK_FLAG_ANY) < 0) {
			printf("av_seek_frame forward failed. \n");
			threadPlay();
			return false;
		}
		firstCount = firstCount - (int)( frameToSkeep * frameRateDuration);

		reallyDeltaTime= currentFrame * frameRateDuration/1000.0;
		m_isVideoSeeking = true;
		threadPlay();
		return true;
	}
	// end of file ... video stops
	this->stopCurrentVideo(false);
	reallyDeltaTime= -1.0;
	return true;
}

void VideoPlayer::recordUpdate(VkCommandBuffer cmd)
{
	if (!videoTexture.sync || !videoTexture.sync->inUse)
		return;
	if (firstUse) {
		SyncEvent helper;
		for (int i = 0; i < 3; ++i)
			helper.imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, 0, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR);
		helper.build();
		helper.placeBarrier(cmd);
		firstUse = false;
	} else {
		videoTexture.sync->syncOut->placeBarrier(cmd);
		Context::instance->waitFrameSync[1].stageMask |= VK_PIPELINE_STAGE_2_COPY_BIT_KHR;
	}
	if (needFrames > 0) {
		if (frameUsed != frameCached) {
			--needFrames;
			int frameIdx = (++frameUsed) % MAX_CACHED_FRAMES;
			VkBufferImageCopy region;
			region.bufferRowLength = region.bufferImageHeight = 0;
			region.imageSubresource = VkImageSubresourceLayers{videoTexture.tex[0]->getAspect(), 0, 0, 1};
			region.imageOffset = VkOffset3D{};
			region.imageExtent.depth = 1;
			for (int i = 0; i < 3; ++i) {
				region.bufferOffset = imageBuffers[i][frameIdx].offset;
				region.imageExtent.width = widths[i];
				region.imageExtent.height = heights[i];
				vkCmdCopyBufferToImage(cmd, stagingBuffer->getBuffer(), videoTexture.tex[i]->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			}
			cv.notify_one();
		} else if (!decoding) {
			if (media->getLoop()) {
				media->playerRestart();
			} else {
				cLog::get()->write("end of file");
				stopCurrentVideo(false);
			}
		}
	}
	videoTexture.sync->syncIn->placeBarrier(cmd);
}

void VideoPlayer::recordUpdateDependency(VkCommandBuffer cmd)
{
	if (!videoTexture.sync || !videoTexture.sync->inUse)
		return;
	videoTexture.sync->inUse = false;
	// videoTexture.sync->syncOut->srcDependency(cmd);
	Context::instance->signalFrameSync[1].stageMask |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
}

void VideoPlayer::mainloop()
{
	std::unique_lock<std::mutex> ulock(mtx);
	while (decoding) {
		getNextVideoFrame();
		while (frameCached - frameUsed >= MAX_CACHED_FRAMES)
			cv.wait(ulock);
	}
}

void VideoPlayer::threadTerminate()
{
	if (thread.joinable()) {
		decoding = false;
		mtx.lock();
		frameCached.store(frameUsed);
		mtx.unlock();
		cv.notify_all();
		thread.join();
	}
	frameCached = 0;
	frameUsed = 0;
	needFrames = 0;
}

void VideoPlayer::threadInterrupt()
{
	if (decoding) {
		frameCached += MAX_CACHED_FRAMES;
		mtx.lock();
	} else if (thread.joinable()) {
		thread.join();
	}
	frameCached = 0;
	frameUsed = 0;
	needFrames = 0;
}

void VideoPlayer::threadPlay()
{
	if (decoding) {
		mtx.unlock();
		cv.notify_all();
	} else {
		decoding = true;
		thread = std::thread(&VideoPlayer::mainloop, this);
	}
}
