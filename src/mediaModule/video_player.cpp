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
#include <sstream>

//#include "spacecrafter.hpp"
#include "mediaModule/video_player.hpp"
#include "tools/log.hpp"
#include "tools/s_texture.hpp"
#include "mediaModule/media.hpp"

#include "eventModule/event_recorder.hpp"
#include "eventModule/EventVideo.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "EntityCore/Tools/Tracer.hpp"

Tracer tracer{80, 24};


VideoPlayer::VideoPlayer(Media *media, InitParser &conf) : media(media)
{
	tracer.emplace(Trace::CUSTOM, this, "cache", &VideoPlayer::tracer_frameCache);
	tracer.emplace(Trace::CUSTOM, &decoding, "decoding", &VideoPlayer::tracer_atomic_bool);
	tracer.emplace(Trace::CUSTOM, &latency, "latency", &VideoPlayer::tracer_duration);
	tracer.emplace(Trace::ULONG, &currentFrame, "currentFrame");
	tracer.emplace(Trace::ULONG, &nbTotalFrame, "nbFrames");
	tracer.emplace(Trace::UCHAR, &m_isVideoPlayed, "played");
	tracer.emplace(Trace::UCHAR, &m_isVideoInPause, "paused");
	tracer.emplace(Trace::UCHAR, &m_isVideoSeeking, "seek");
	tracer.emplace(Trace::UCHAR, &skipFrame, "canskip");
	tracer.emplace(Trace::UCHAR, &adaptiveFramerate, "adaptive");
	tracer.emplace(Trace::INT, &videoRes.w, "width");
	tracer.emplace(Trace::INT, &videoRes.h, "height");
	m_isVideoPlayed = false;
	m_isVideoInPause = false;
	m_isVideoSeeking = false;
	skipFrame = conf.getBoolean(SCS_IO, SCK_VIDEO_FRAME_SKIP);
	if (conf.getBoolean(SCS_DEBUG, SCK_PRINT_VIDEO_INFO)) {
		if (conf.getBoolean(SCS_DEBUG, SCK_PRINT_LOG)) {
			cLog::get()->write(SCK_PRINT_VIDEO_INFO " can't be enabled while " SCK_PRINT_LOG " is active.", LOG_TYPE::L_ERROR);
		} else {
			debugMode = true;
		}
	}
	img_convert_ctx = NULL;
	std::string videoPlayerCodecThreadConfig = conf.getStr(SCS_IO, SCK_VIDEO_CODEC_THREADS);
	if (videoPlayerCodecThreadConfig.empty()) {
		cLog::get()->write("Videoplayer: missing '" SCK_VIDEO_CODEC_THREADS "' value, expected number or percentage of threads to use. Default to 50%", LOG_TYPE::L_WARNING);
		codecDecodeThreads = std::thread::hardware_concurrency() / 2;
	} else {
		try {
			codecDecodeThreads = std::stoi(videoPlayerCodecThreadConfig);
			if (videoPlayerCodecThreadConfig.back() == '%')
				codecDecodeThreads = codecDecodeThreads * std::thread::hardware_concurrency() / 100;
		} catch (...) {
			cLog::get()->write("Videoplayer: invalid '" SCK_VIDEO_CODEC_THREADS "' value '" + videoPlayerCodecThreadConfig + "', expected number or percentage of threads to use. Default to 50%", LOG_TYPE::L_WARNING);
			codecDecodeThreads = std::thread::hardware_concurrency() / 2;
		}
	}
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
	if (waitCacheFull)
		return;
	if (m_isVideoInPause) {
		m_isVideoInPause = false;
		currentTime = std::chrono::steady_clock::now();
		nextFrame = currentTime + deltaFrame;
		if (audio) {
			audio->musicJump(std::chrono::duration_cast<std::chrono::duration<double>>(currentFrame * deltaFrame).count());
			audio->musicResume();
		}
		latency = -deltaFrame;
	} else {
		m_isVideoInPause = true;
		nextFrame += std::chrono::hours(24);
		if (audio)
			audio->musicPause();
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
	if (audio)
		audio->musicRewind();
	threadPlay();
	if (result < 0) {
		printf("av_seek_frame forward failed. \n");
		return false;
	}

	currentFrame = 0;

	return true;
}


bool VideoPlayer::playNewVideo(const std::string& _fileName, Audio *_audio, bool paused, DecodePolicy policy)
{
	stopCurrentVideo(true);
	if (debugMode)
		tracer.start();
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
	if (pCodecCtx->width > 1024) // Only enforce threaded policy for lage videos, as smaller ones doesn't need it.
		policy = DecodePolicy::THREADED;
	switch (policy) {
		case DecodePolicy::THREADED:
			if (codecDecodeThreads)
				pCodecCtx->thread_count = codecDecodeThreads;
			break;
		case DecodePolicy::ASYNC:;
	}
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
	deltaFrame = std::chrono::steady_clock::duration(std::chrono::steady_clock::period::den * frame_rate.den / (std::chrono::steady_clock::period::num * frame_rate.num));
	nbTotalFrame = static_cast<int>((pFormatCtx->duration+1) * frameRate / AV_TIME_BASE);

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

	currentFrame = 0;
	frameCached = 0;
	frameUsed = 0;
	m_isVideoPlayed = true;
	audio = _audio;
	threadPlay();
	m_isVideoInPause = paused;
	if (audio) {
		audio->musicPlay();
		if (paused)
			audio->musicPause();
	}

	Event* event = new VideoEvent(VIDEO_ORDER::PLAY);
	EventRecorder::getInstance()->queue(event);
	return true;
}

void VideoPlayer::update()
{
	if (waitCacheFull && isVideoCacheFull()) {
		waitCacheFull = false;
		pauseCurrentVideo();
	}
}

bool VideoPlayer::getNextFrame()
{
	sTime = std::chrono::steady_clock::now();
	for (; av_read_frame(pFormatCtx, packet) >= 0; av_packet_unref(packet)) {
		if(packet->stream_index==videoindex) {
			auto now = std::chrono::steady_clock::now();
			sRead += now - sTime;
			sTime = now;
			int ret = avcodec_send_packet(pCodecCtx, packet);
			if(ret < 0) {
				cLog::get()->write("Decode Error", LOG_TYPE::L_ERROR);
				continue ;
			}
			now = std::chrono::steady_clock::now();
			sParse += now - sTime;
			sTime = now;
			ret = avcodec_receive_frame(pCodecCtx, pFrameIn);
			if(ret < 0 ) {
				cLog::get()->write("not got frame", LOG_TYPE::L_DEBUG);
				continue;
			}
			now = std::chrono::steady_clock::now();
			sDecode += now - sTime;
			sTime = now;
			if (m_isVideoSeeking) {
				if (pFrameIn->key_frame==1) {
					m_isVideoSeeking=false;
					currentFrame = (frameRate * (pFrameIn->pts) * video_st->time_base.num) / video_st->time_base.den + 0.5;
				} else {
					continue;
				}
			}
			av_packet_unref(packet);
			return true;
		}
	}
	decoding = false;
	return false;
}


void VideoPlayer::getNextVideoFrame()
{
	if (getNextFrame()) {
		for (int i = 0; i < 3; i++) {
			char *dst = reinterpret_cast<char*>(pImageBuffer[i][frameCached % MAX_CACHED_FRAMES]);
			char *src = reinterpret_cast<char*>(pFrameIn->data[i]);
			for (int j = 0; j < heights[i]; ++j) {
				memcpy(dst, src, widths[i]);
				src += pFrameIn->linesize[i];
				dst += widths[i];
			}
		}
		frameCached.fetch_add(1, std::memory_order_release);
		sWrite += std::chrono::steady_clock::now() - sTime;
	}
}


void VideoPlayer::stopCurrentVideo(bool newVideo)
{
	std::unique_lock<std::mutex> lock(videoTransitionMutex);
	if (!m_isVideoPlayed)
		return;
	tracer.stop();

	m_isVideoPlayed = false;
	if (!newVideo) {
		if (audio)
			audio->musicDrop();
		Event* event = new VideoEvent(VIDEO_ORDER::STOP);
		EventRecorder::getInstance()->queue(event);
		media->playerStopped();
	}
	threadTerminate(); // Don't overlap av_* calls

	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameOut);
	av_frame_free(&pFrameIn);
	avcodec_close(pCodecCtx);

	std::ostringstream oss;
	auto total = (sRead + sParse + sDecode + sWrite).count() / 100ULL;
	oss << "Video decode statistics : ";
	oss << "Read " << std::chrono::duration_cast<std::chrono::seconds>(sRead).count() << "s (" << sRead.count() / total << "%), ";
	oss << "Parse " << std::chrono::duration_cast<std::chrono::seconds>(sParse).count() << "s (" << sParse.count() / total << "%), ";
	oss << "Decode " << std::chrono::duration_cast<std::chrono::seconds>(sDecode).count() << "s (" << sDecode.count() / total << "%), ";
	oss << "Copy " << std::chrono::duration_cast<std::chrono::seconds>(sWrite).count() << "s (" << sWrite.count() / total << "%)";
	cLog::get()->write(oss.str(), LOG_TYPE::L_INFO);
	sRead = sParse = sDecode = sWrite = std::chrono::steady_clock::duration{};
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
			videoTexture.sync->syncOut->bufferBarrier(*stagingBuffer, VK_PIPELINE_STAGE_2_HOST_BIT_KHR, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_ACCESS_2_HOST_WRITE_BIT_KHR, VK_ACCESS_2_TRANSFER_READ_BIT_KHR);
			videoTexture.sync->syncIn->imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR);
		}
		videoTexture.sync->syncOut->build();
		videoTexture.sync->syncIn->build();
	}
}


/* lets take a leap forward the video */
bool VideoPlayer::jumpInCurrentVideo(float deltaTime)
{
	return seekVideo(deltaTime * frameRate);
}


bool VideoPlayer::invertVideoFlow()
{
	return seekVideo(nbTotalFrame - 2*currentFrame);
}


bool VideoPlayer::seekVideo(int64_t frameToSkeep)
{
	if (!m_isVideoPlayed)
		return false;

	currentFrame = currentFrame + frameToSkeep;

	//jump before the beginning of the video
	if (currentFrame <= 0) {
		this->restartCurrentVideo();
		return true;
	}
	if(currentFrame < nbTotalFrame) { // we check that we don't jump out of the video
		threadInterrupt();
		if (avformat_seek_file(pFormatCtx, -1, INT64_MIN, static_cast<int64_t>(currentFrame / frameRate * AV_TIME_BASE), INT64_MAX, 0) < 0) {
			printf("av_seek_frame forward failed. \n");
			threadPlay();
			return false;
		}
		if (!m_isVideoInPause) {
			pauseCurrentVideo();
			waitCacheFull = true;
		}
		m_isVideoSeeking = true;
		threadPlay();
		return true;
	}
	// end of file ... video stops
	this->stopCurrentVideo(false);
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
	if (drawNextFrame) {
		VkBufferImageCopy region;
		region.bufferRowLength = region.bufferImageHeight = 0;
		region.imageSubresource = VkImageSubresourceLayers{videoTexture.tex[0]->getAspect(), 0, 0, 1};
		region.imageOffset = VkOffset3D{};
		region.imageExtent.depth = 1;
		auto frameIdx = frameUsed.fetch_add(1, std::memory_order_relaxed);
		for (int i = 0; i < 3; ++i) {
			region.bufferOffset = imageBuffers[i][frameIdx].offset;
			region.imageExtent.width = widths[i];
			region.imageExtent.height = heights[i];
			vkCmdCopyBufferToImage(cmd, stagingBuffer->getBuffer(), videoTexture.tex[i]->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}
		drawNextFrame = false;
	} else if (!m_isVideoInPause) {
		if (CoreLink::instance->predictibleRendering()) {
			currentTime += renderDeltaFrame;
			latency += renderDeltaFrame;
			while (decoding && frameUsed.load(std::memory_order_relaxed) == frameCached.load(std::memory_order_relaxed))
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
		} else {
			auto now = std::chrono::steady_clock::now();
			latency += now - currentTime;
			currentTime = now;
		}
		if (nextFrame <= currentTime) {
			if (auto nbFrames = frameCached.load(std::memory_order_acquire) - frameUsed.load(std::memory_order_relaxed)) {
				uint32_t frameIdx;
				do { // Determine how many frames to load
					++currentFrame;
					frameIdx = frameUsed.fetch_add(1, std::memory_order_relaxed);
					latency -= deltaFrame;
					if (adaptiveFramerate) {
						if (--nbFrames) {
							if (nbFrames < CACHE_STRESS || latency.count() > 0) {
								nextFrame += std::chrono::steady_clock::duration(static_cast<int64_t>(
									deltaFrame.count() * std::min(MIN_VIDEO_SPEED + nbFrames * SPEED_INCREMENT_PER_CACHED_FRAME, MAX_VIDEO_SPEED)
								));
							} else {
								nextFrame += deltaFrame;
							}
						} else {
							nextFrame = currentTime + deltaFrame;
						}
					} else {
						nextFrame += deltaFrame;
						break;
					}
				} while (nextFrame <= currentTime && skipFrame);
				cv.notify_all();
				frameIdx %= MAX_CACHED_FRAMES;
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
			} else if (m_isVideoPlayed && !decoding) {
				if (reloop) {
					restartCurrentVideo();
				} else {
					cLog::get()->write("end of file");
					stopCurrentVideo(false);
				}
			} else {
				pauseCurrentVideo();
				waitCacheFull = true;
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
		while (frameCached.load(std::memory_order_relaxed) - frameUsed.load(std::memory_order_relaxed) >= (MAX_CACHED_FRAMES-1) && decoding)
			cv.wait(ulock);
	}
}

void VideoPlayer::threadTerminate()
{
	if (thread.joinable()) {
		decoding = false;
		cv.notify_all();
		thread.join();
	}
	frameCached = 0;
	frameUsed = 0;
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
}

void VideoPlayer::threadPlay()
{
	currentTime = std::chrono::steady_clock::now();
	nextFrame = currentTime + deltaFrame;
	latency = -deltaFrame;
	drawNextFrame = true;
	this->getNextVideoFrame(); // The first valid frame must be ready
	if (decoding) {
		mtx.unlock();
		cv.notify_all();
	} else {
		decoding = true;
		thread = std::thread(&VideoPlayer::mainloop, this);
	}
}


//// Tracer facilities ////
static void traceNbr(uint32_t value, unsigned char *&buffer) {
	if (value > 9)
		traceNbr(value / 10, buffer);
	*(buffer++) = '0' + value % 10;
}

unsigned char *VideoPlayer::tracer_frameCache(void *data, unsigned char *buffer)
{
	uint32_t nbCached = reinterpret_cast<VideoPlayer*>(data)->frameCached.load(std::memory_order_relaxed) - reinterpret_cast<VideoPlayer*>(data)->frameUsed.load(std::memory_order_relaxed);
	traceNbr(nbCached, buffer);
	*(buffer++) = '/';
	traceNbr(MAX_CACHED_FRAMES, buffer);
	return buffer;
}

unsigned char *VideoPlayer::tracer_atomic_bool(void *data, unsigned char *buffer)
{
	if (reinterpret_cast<std::atomic<bool>*>(data)->load(std::memory_order_relaxed)) {
		memcpy(buffer, "true", 4);
		return buffer+4;
	} else {
		memcpy(buffer, "false", 5);
		return buffer+5;
	}
}

unsigned char *VideoPlayer::tracer_duration(void *data, unsigned char *buffer)
{
	int64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(*reinterpret_cast<std::chrono::steady_clock::duration*>(data)).count();
	memcpy(buffer, " 00m 00s 000ms", 14);
	if (time < 0) {
		*buffer = '-';
		time = -time;
	}
	buffer += 11;
	*buffer |= time % 10;
	time /= 10;
	*--buffer |= time % 10;
	time /= 10;
	*--buffer |= time % 10;
	time /= 10;
	buffer -= 3;
	*buffer |= time % 10;
	time /= 10;
	*--buffer |= time % 6;
	time /= 6;
	buffer -= 3;
	*buffer |= time % 10;
	time /= 10;
	*--buffer |= (time < 10) ? time : 15;
	return buffer + 13;
}
