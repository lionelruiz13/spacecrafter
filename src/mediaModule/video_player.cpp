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
#include <iomanip>

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

	// Top Subtitle
	textSubtitleTopParam.string = "";
	textSubtitleTopParam.altitude = 10.0f;
	textSubtitleTopParam.azimuth = 0.0f;
	// textSubtitleTopParam.fontSize;
	textSubtitleTopParam.textAlign = "CENTER";
	textSubtitleTopParam.color = Vec3f(1.0f, 1.0f, 1.0f);
	textSubtitleTopParam.useColor = true;
	textSubtitleTopParam.fader = false;

	// Bottom Subtitle
	textSubtitleBottomParam.string = "";
	textSubtitleBottomParam.altitude = 5.0f;
	textSubtitleBottomParam.azimuth = 0.0f;
	// textSubtitleBottomParam.fontSize;
	textSubtitleBottomParam.textAlign = "CENTER";
	textSubtitleBottomParam.color = Vec3f(1.0f, 1.0f, 1.0f);
	textSubtitleBottomParam.useColor = true;
	textSubtitleBottomParam.fader = false;

	m_isVideoPlayed = false;
	m_isVideoInPause = false;
	m_isVideoSeeking = false;
	hasAlphaChannel = false;
	targetFormat = AV_PIX_FMT_YUV420P;
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
	for (int i = 0; i < 4; i++)
		delete videoTexture.tex[i];
}

std::string VideoPlayer::formatTime(int seconds) const
{
	int hours = seconds / 3600;
	int minutes = (seconds - hours * 3600) / 60;
	int secs = seconds - hours * 3600 - minutes * 60;

	std::ostringstream oss;
	oss << std::setfill('0') << hours << ":"
		<< std::setw(2) << minutes << ":"
		<< std::setw(2) << secs;
	return oss.str();
}

std::string VideoPlayer::getTimeStatus() const
{
	int currentTimeSeconds = currentFrame / frameRate;
	int totalTimeSeconds = nbTotalFrame / frameRate;
	std::string currentTimeStr = formatTime(currentTimeSeconds);
	std::string totalTimeStr = formatTime(totalTimeSeconds);
	return currentTimeStr + " / " + totalTimeStr;
}

void VideoPlayer::createTextures()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	const uint32_t widthMax = 4096;
	const uint32_t heightMax = 2048;
	// Increase buffer to include alpha channel (2.0 instead of 1.5 for YUVA420P)
	stagingBuffer = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, widthMax*heightMax*2.0*MAX_CACHED_FRAMES, "Staging video buffer");
	for (int i = 0; i < 4; i++)
		videoTexture.tex[i] = new Texture(vkmgr, *stagingBuffer, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, "Video texture", VK_FORMAT_R8_UNORM);
}

void VideoPlayer::pauseCurrentVideo()
{
	if (waitCacheFull)
		return;
	if (m_isVideoInPause) {
		m_isVideoInPause = false;
		currentTime = std::chrono::steady_clock::now();
		nextFrame = currentTime + std::chrono::steady_clock::duration(static_cast<int64_t>(deltaFrame.count() / playbackSpeedFactor.toDouble()));
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

void VideoPlayer::setPlaybackSpeed(FixedPointI16_2 factor)
{
	if (factor <= 0.0f) {
		cLog::get()->write("VideoPlayer: Invalid playback speed factor " + factor.toString() + ", must be > 0", LOG_TYPE::L_WARNING);
		return;
	}

	// Add reasonable limits to prevent performance issues
	static const FixedPointI16_2 MIN_SPEED(0.1f);  // 10x slower
	static const FixedPointI16_2 MAX_SPEED(10.0f); // 10x faster

	if (factor < MIN_SPEED) {
		cLog::get()->write("VideoPlayer: Speed factor " + factor.toString() + " too low, clamping to " + MIN_SPEED.toString(), LOG_TYPE::L_WARNING);
		factor = MIN_SPEED;
	} else if (factor > MAX_SPEED) {
		cLog::get()->write("VideoPlayer: Speed factor " + factor.toString() + " too high, clamping to " + MAX_SPEED.toString(), LOG_TYPE::L_WARNING);
		factor = MAX_SPEED;
	}

	playbackSpeedFactor = factor;
	cLog::get()->write("VideoPlayer: Playback speed set to " + factor.toString() + "x", LOG_TYPE::L_INFO);
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
	if (audio && playbackSpeedFactor == FixedPointI16_2::one()) // Only rewind audio if normal speed
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
	// For VP9 (WebM), use the specific decoder that can handle alpha
	if (pCodecCtx->codec_id == AV_CODEC_ID_VP9) {
		pCodec = avcodec_find_decoder_by_name("libvpx-vp9");
		if (pCodec) {
			cLog::get()->write("Using libvpx-vp9 decoder for VP9 video", LOG_TYPE::L_INFO);
		} else {
			cLog::get()->write("libvpx-vp9 decoder not found, using default VP9 decoder", LOG_TYPE::L_WARNING);
			pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		}
	} else {
		pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	}

	if(pCodec==NULL) {
		cLog::get()->write("Unsupported pCodec for video file", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return false;
	}
	// Special configuration for VP9 with alpha - will be configured later after alpha detection
	AVDictionary *opts = nullptr;

	if(avcodec_open2(pCodecCtx, pCodec, &opts)<0) {
		cLog::get()->write("Could not open codec.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		av_dict_free(&opts);
		return false;
	}
	av_dict_free(&opts);

	videoRes.w = pCodecCtx->width;
	videoRes.h = pCodecCtx->height;

	AVRational frame_rate = av_guess_frame_rate(pFormatCtx, video_st, NULL);
	frameRate = frame_rate.num/(double)frame_rate.den;
	deltaFrame = std::chrono::steady_clock::duration(std::chrono::steady_clock::period::den * frame_rate.den / (std::chrono::steady_clock::period::num * frame_rate.num));
	nbTotalFrame = static_cast<int>((pFormatCtx->duration+1) * frameRate / AV_TIME_BASE);

	img_convert_ctx = NULL;

	// Detect if the input format potentially has alpha
	bool sourceHasAlpha = pCodecCtx->pix_fmt == AV_PIX_FMT_YUVA420P;

	// For WebM VP9, check specific metadata after opening the codec
	if (!sourceHasAlpha && pCodecCtx->codec_id == AV_CODEC_ID_VP9) {

		// Method 1: Check stream metadata for alpha_mode or alpha
		if (video_st->metadata) {
			AVDictionaryEntry *alpha_entry = av_dict_get(video_st->metadata, "alpha_mode", NULL, 0);
			if (!alpha_entry) {
				alpha_entry = av_dict_get(video_st->metadata, "alpha", NULL, 0);
			}
			if (alpha_entry) {
				sourceHasAlpha = true;
				cLog::get()->write("VP9: Alpha detected in stream metadata: " + std::string(alpha_entry->key) + "=" + std::string(alpha_entry->value), LOG_TYPE::L_INFO);
			}
		}

		// Method 2: Check format/container metadata
		if (!sourceHasAlpha && pFormatCtx->metadata) {
			AVDictionaryEntry *alpha_entry = av_dict_get(pFormatCtx->metadata, "alpha_mode", NULL, 0);
			if (!alpha_entry) {
				alpha_entry = av_dict_get(pFormatCtx->metadata, "alpha", NULL, 0);
			}
			if (alpha_entry) {
				sourceHasAlpha = true;
				cLog::get()->write("VP9: Alpha detected in format metadata: " + std::string(alpha_entry->key) + "=" + std::string(alpha_entry->value), LOG_TYPE::L_INFO);
			}
		}
	}

	// If the source has alpha, use YUVA420P as target format
	if (sourceHasAlpha) {
		hasAlphaChannel = true;
		targetFormat = AV_PIX_FMT_YUVA420P;

		// For VP9 with alpha, reconfigure the decoder to force YUVA420P
		if (pCodecCtx->codec_id == AV_CODEC_ID_VP9) {
			avcodec_close(pCodecCtx);

			// Force output format to YUVA420P
			pCodecCtx->pix_fmt = AV_PIX_FMT_YUVA420P;

			// Reopen with special options for alpha
			AVDictionary *alpha_opts = nullptr;
			av_dict_set(&alpha_opts, "apply_cropping", "0", 0);

			if(avcodec_open2(pCodecCtx, pCodec, &alpha_opts) < 0) {
				cLog::get()->write("Could not reopen VP9 codec with alpha support.", LOG_TYPE::L_ERROR);
				av_dict_free(&alpha_opts);
				avformat_close_input(&pFormatCtx);
				return false;
			}
			av_dict_free(&alpha_opts);
		}
	} else {
		hasAlphaChannel = false;
		targetFormat = AV_PIX_FMT_YUV420P;
	}

	// Now initialize textures with the correct alpha detection
	initTexture();

	// For VP9 with alpha, do not create conversion context
	if (pCodecCtx->codec_id == AV_CODEC_ID_VP9 && hasAlphaChannel) {
		img_convert_ctx = nullptr; // No conversion needed for VP9 with alpha
	} else {
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		                                 pCodecCtx->width, pCodecCtx->height, targetFormat,
		                                 SWS_BICUBIC, NULL, NULL, NULL);
		if(img_convert_ctx==NULL) {
			cLog::get()->write("Unable to get a context for video file", LOG_TYPE::L_ERROR);
			return false;
		}
	}

	// check if the pixel format is supported
	if (pCodecCtx->pix_fmt != targetFormat) {
		cLog::get()->write("Unsupported pixel format for video file. Expected " +
		                   std::string(av_get_pix_fmt_name(targetFormat)) + ", got " +
		                   std::string(av_get_pix_fmt_name(pCodecCtx->pix_fmt)), LOG_TYPE::L_ERROR);
		return false;
	}

	if (hasAlphaChannel) {
		cLog::get()->write("Video has alpha channel, using YUVA420P", LOG_TYPE::L_INFO);
	} else {
		cLog::get()->write("Video has no alpha channel, using YUV420P", LOG_TYPE::L_INFO);
	}
	pFrameIn = av_frame_alloc();
	pFrameOut = av_frame_alloc();

	// For VP9 with alpha, no additional buffer needed
	if (pCodecCtx->codec_id == AV_CODEC_ID_VP9 && hasAlphaChannel) {
		// VP9 with alpha: use decoded data directly
		cLog::get()->write("VP9 with alpha: using direct decoded frames", LOG_TYPE::L_INFO);
	} else {
		av_image_fill_arrays(pFrameIn->data, pFrameIn->linesize, NULL, targetFormat, pCodecCtx->width, pCodecCtx->height, 1);

		// Initialize pFrameOut with its own data for conversion (only if conversion needed)
		if (pCodecCtx->pix_fmt != targetFormat) {
			if (av_image_alloc(pFrameOut->data, pFrameOut->linesize, pCodecCtx->width, pCodecCtx->height, targetFormat, 1) < 0) {
				cLog::get()->write("Could not allocate frame buffer for conversion", LOG_TYPE::L_ERROR);
				return false;
			}
		}
	}

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
	if (waitCacheFull && isVideoCachePrefilled()) {
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
		// Number of textures to copy (3 for YUV, 4 for YUVA)
		int numTextures = hasAlphaChannel ? 4 : 3;

		for (int i = 0; i < numTextures; i++) {
			char *dst = reinterpret_cast<char*>(pImageBuffer[i][frameCached % MAX_CACHED_FRAMES]);
			char *src = reinterpret_cast<char*>(pFrameIn->data[i]);
			if (src) { // Check if source data exists
				for (int j = 0; j < heights[i]; ++j) {
					memcpy(dst, src, widths[i]);
					src += pFrameIn->linesize[i];
					dst += widths[i];
				}
			} else {
				// If no source data, fill with default values
				if (i == 3) { // Alpha
					memset(dst, 0xFF, widths[i] * heights[i]); // Opaque
				} else {
					memset(dst, 0, widths[i] * heights[i]); // Black
				}
			}
		}

		// If no alpha, fill the fake alpha texture with 0xFF (opaque)
		if (!hasAlphaChannel) {
			char *dst = reinterpret_cast<char*>(pImageBuffer[3][frameCached % MAX_CACHED_FRAMES]);
			memset(dst, 0xFF, widths[3] * heights[3]); // 1x1 pixel at 255 (opaque)
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

	m_isVideoPlayed = false;
	media->textDel("video_subtitle1t");
	media->textDel("video_subtitle2t");
	media->textDel("video_subtitle3t");
	media->textDel("video_subtitle1b");
	media->textDel("video_subtitle2b");
	media->textDel("video_subtitle3b");
	if (!newVideo) {
		if (audio)
			audio->musicDrop();
	}
	threadTerminate(); // Don't overlap av_* calls

	if (img_convert_ctx) {
		sws_freeContext(img_convert_ctx);
		img_convert_ctx = nullptr;
	}

	// Free the allocated buffer for pFrameOut (only if it was allocated)
	if (pFrameOut && pFrameOut->data[0]) {
		// For VP9 with alpha, pFrameOut does not have allocated data
		if (!(pCodecCtx && pCodecCtx->codec_id == AV_CODEC_ID_VP9 && hasAlphaChannel)) {
			av_freep(&pFrameOut->data[0]);
		}
	}
	if (pFrameOut) {
		av_frame_free(&pFrameOut);
		pFrameOut = nullptr;
	}
	if (pFrameIn) {
		av_frame_free(&pFrameIn);
		pFrameIn = nullptr;
	}

	// Free the packet
	if (packet) {
		av_packet_free(&packet);
		packet = nullptr;
	}
	if (pCodecCtx) {
		avcodec_close(pCodecCtx);
	}

	// Free the format context
	if (pFormatCtx) {
		avformat_close_input(&pFormatCtx);
	}

	std::ostringstream oss;
	auto total = (sRead + sParse + sDecode + sWrite).count() / 100ULL;
	oss << "Video decode statistics : ";
	oss << "Read " << std::chrono::duration_cast<std::chrono::seconds>(sRead).count() << "s (" << sRead.count() / total << "%), ";
	oss << "Parse " << std::chrono::duration_cast<std::chrono::seconds>(sParse).count() << "s (" << sParse.count() / total << "%), ";
	oss << "Decode " << std::chrono::duration_cast<std::chrono::seconds>(sDecode).count() << "s (" << sDecode.count() / total << "%), ";
	oss << "Copy " << std::chrono::duration_cast<std::chrono::seconds>(sWrite).count() << "s (" << sWrite.count() / total << "%)";
	cLog::get()->write(oss.str(), LOG_TYPE::L_INFO);
	sRead = sParse = sDecode = sWrite = std::chrono::steady_clock::duration{};

	EventRecorder::getInstance()->queue(new VideoEvent(VIDEO_ORDER::STOP));
	media->playerStopped();
	tracer.stop();
}

void VideoPlayer::initTexture()
{
	// Define dimensions for YUV and potentially Alpha
	const int _widths[4]  = { videoRes.w, videoRes.w / 2, videoRes.w / 2, hasAlphaChannel ? videoRes.w : 1 };
	const int _heights[4] = { videoRes.h, videoRes.h / 2, videoRes.h / 2, hasAlphaChannel ? videoRes.h : 1 };

	// Always handle 4 textures, but the 4th can be a 1x1 dummy texture
	int numTextures = 4;

	for(int i=0; i<numTextures; i++) {
		widths[i] = _widths[i];
		heights[i] = _heights[i];
	}

	bool uninitialized = false;
	// First pass: check if any texture needs reinitialization
	for (int i = 0; i < numTextures; ++i) {
		if (videoTexture.tex[i]->isOnGPU()) {
			int width, height;
			videoTexture.tex[i]->getDimensions(width, height);
			if (width != widths[i] || height != heights[i]) {
				cLog::get()->write("Texture " + std::to_string(i) + " has wrong dimensions (" + std::to_string(width) + "x" + std::to_string(height) + " vs " + std::to_string(widths[i]) + "x" + std::to_string(heights[i]) + "), need reinit", LOG_TYPE::L_INFO);
				uninitialized = true;
			} else {
				cLog::get()->write("Texture " + std::to_string(i) + " already initialized with correct dimensions " + std::to_string(width) + "x" + std::to_string(height), LOG_TYPE::L_INFO);
			}
		} else {
			cLog::get()->write("Texture " + std::to_string(i) + " not on GPU, need reinit", LOG_TYPE::L_INFO);
			uninitialized = true;
		}
	}

	// Second pass: if any texture needs reinit, destroy ALL textures
	if (uninitialized) {
		cLog::get()->write("Reinitializing all textures due to dimension mismatch", LOG_TYPE::L_INFO);
		auto &context = *Context::instance;
		context.helper->waitFrame(context.frameIdx);
		vkQueueWaitIdle(context.graphicQueue);
		for (int i = 0; i < numTextures; ++i) {
			if (videoTexture.tex[i]->isOnGPU()) {
				cLog::get()->write("Destroying texture " + std::to_string(i), LOG_TYPE::L_INFO);
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

		// Always initialize 4 textures (the 4th can be a 1x1 dummy texture without alpha)
		int numTextures = 4;

		for (int i = 0; i < numTextures; ++i) {
			// Try to initialize the texture with the requested dimensions
			bool success = videoTexture.tex[i]->init(widths[i], heights[i], nullptr, false, 1);

			// If initialization fails, retry with minimal dimensions
			if (!success || !videoTexture.tex[i]->isOnGPU()) {
				cLog::get()->write("Warning: Failed to initialize texture " + std::to_string(i) + " with dimensions " + std::to_string(widths[i]) + "x" + std::to_string(heights[i]) + ", trying 1x1", LOG_TYPE::L_WARNING);
				videoTexture.tex[i]->init(1, 1, nullptr, false, 1);
				widths[i] = 1;
				heights[i] = 1;
			}

			// Now all textures should be valid
			if (videoTexture.tex[i]->isOnGPU()) {
				for (int j = 0; j < MAX_CACHED_FRAMES; ++j) {
					imageBuffers[i][j] = stagingBuffer->fastAcquireBuffer(widths[i] * heights[i]);
					pImageBuffer[i][j] = stagingBuffer->getPtr(imageBuffers[i][j]);
				}
				videoTexture.sync->syncOut->imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR);
				videoTexture.sync->syncOut->bufferBarrier(*stagingBuffer, VK_PIPELINE_STAGE_2_HOST_BIT_KHR, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_ACCESS_2_HOST_WRITE_BIT_KHR, VK_ACCESS_2_TRANSFER_READ_BIT_KHR);
				videoTexture.sync->syncIn->imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR);
			} else {
				cLog::get()->write("Error: Critical failure to initialize texture " + std::to_string(i), LOG_TYPE::L_ERROR);
				// In case of critical failure, initialize with default values to avoid crash
				widths[i] = 1;
				heights[i] = 1;
			}
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


bool VideoPlayer::seekVideo(int64_t framesToSkip)
{
	if (!m_isVideoPlayed)
		return false;

	currentFrame = currentFrame + framesToSkip;

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
	if (!videoTexture.sync || !videoTexture.sync->inUse || !m_isVideoPlayed)
		return;
	if (firstUse) {
		SyncEvent helper;
		// Transition layout for the 4 textures (YUV + Alpha) - all guaranteed valid
		for (int i = 0; i < 4; ++i) {
			helper.imageBarrier(*videoTexture.tex[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, 0, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR);
		}
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
		for (int i = 0; i < 4; ++i) {
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
			if (auto nbFrames = framesAvailable(frameCached, frameUsed)) {
				uint32_t frameIdx;
				do { // Determine how many frames to load
					// No more frames available (even if we have to skip frame stop here to prevent "frame jump" when waiting for cache)
					if (framesAvailable(frameCached, frameUsed) == 0)
						break;
					++currentFrame;
					frameIdx = frameUsed.fetch_add(1, std::memory_order_relaxed);
					latency -= deltaFrame;
					if (adaptiveFramerate) {
						if (--nbFrames) {
							if (nbFrames < CACHE_STRESS || latency.count() > 0) {
								// Apply playback speed to adaptive framerate calculation
								nextFrame += std::chrono::steady_clock::duration(static_cast<int64_t>(
									deltaFrame.count() * std::min(MIN_VIDEO_SPEED + nbFrames * SPEED_INCREMENT_PER_CACHED_FRAME, MAX_VIDEO_SPEED) / playbackSpeedFactor.toDouble()
								));
							} else {
								// Apply playback speed to normal frame timing
								nextFrame += std::chrono::steady_clock::duration(static_cast<int64_t>(deltaFrame.count() / playbackSpeedFactor.toDouble()));
							}
						} else {
							// Apply playback speed when resetting timing
							nextFrame = currentTime + std::chrono::steady_clock::duration(static_cast<int64_t>(deltaFrame.count() / playbackSpeedFactor.toDouble()));
						}
					} else {
						// Apply playback speed to non-adaptive mode
						nextFrame += std::chrono::steady_clock::duration(static_cast<int64_t>(deltaFrame.count() / playbackSpeedFactor.toDouble()));
						//! Don't break here, we want to skip as many frames as needed
						//! to catch up with currentTime if we allow skipping or fast playback
						// break;
					}
				} while (nextFrame <= currentTime && (skipFrame || playbackSpeedFactor.toDouble() > 1.0f));
				cv.notify_all();
				frameIdx %= MAX_CACHED_FRAMES;

				// Update subtitle
				if (showSubtitles) {
					static std::string subtitleContent = "";
					int currentTimeInMs = static_cast<int>(currentFrame * 1000.0 / frameRate);
					std::string tmpSubtitle = media->subtitleGetSubtitleAt(currentTimeInMs);

					// Prevent recreating the subtitle text object if the subtitle hasn't changed
					if (subtitleContent != tmpSubtitle) {
						subtitleContent = tmpSubtitle;

						// Split subtitle into two lines if too long
						if (subtitleContent.length() > 50) {
							auto splitPos = subtitleContent.rfind(' ', subtitleContent.length() / 2);
							textSubtitleTopParam.string = subtitleContent.substr(0, splitPos);
							textSubtitleBottomParam.string = subtitleContent.substr(splitPos + 1);
						} else {
							textSubtitleTopParam.string = "";
							textSubtitleBottomParam.string = subtitleContent;
						}

						// Top subtitles
						textSubtitleTopParam.azimuth = 0.0f;
						media->textAdd("video_subtitle1t", textSubtitleTopParam);
						media->textDisplay("video_subtitle1t", true);

						if (subtitleProject == IMG_PROJECT::TWICE) {
							textSubtitleTopParam.azimuth = 180.0f;
							media->textAdd("video_subtitle2t", textSubtitleTopParam);
							media->textDisplay("video_subtitle2t", true);
						}

						if (subtitleProject == IMG_PROJECT::THRICE) {
							textSubtitleTopParam.azimuth = 120.0f;
							media->textAdd("video_subtitle2t", textSubtitleTopParam);
							media->textDisplay("video_subtitle2t", true);

							textSubtitleTopParam.azimuth = 240.0f;
							media->textAdd("video_subtitle3t", textSubtitleTopParam);
							media->textDisplay("video_subtitle3t", true);
						}

						// Bottom subtitles
						textSubtitleBottomParam.azimuth = 0.0f;
						media->textAdd("video_subtitle1b", textSubtitleBottomParam);
						media->textDisplay("video_subtitle1b", true);

						if (subtitleProject == IMG_PROJECT::TWICE) {
							textSubtitleBottomParam.azimuth = 180.0f;
							media->textAdd("video_subtitle2b", textSubtitleBottomParam);
							media->textDisplay("video_subtitle2b", true);
						}

						if (subtitleProject == IMG_PROJECT::THRICE) {
							textSubtitleBottomParam.azimuth = 120.0f;
							media->textAdd("video_subtitle2b", textSubtitleBottomParam);
							media->textDisplay("video_subtitle2b", true);

							textSubtitleBottomParam.azimuth = 240.0f;
							media->textAdd("video_subtitle3b", textSubtitleBottomParam);
							media->textDisplay("video_subtitle3b", true);
						}
					}
				}

				if (currentFrame >= nbTotalFrame) {
					// Reached end of video
					if (reloop) {
						restartCurrentVideo();
					} else {
						cLog::get()->write("end of file");
						stopCurrentVideo(false);
					}
					videoTexture.sync->syncIn->placeBarrier(cmd);
					return;
				}

				VkBufferImageCopy region;
				region.bufferRowLength = region.bufferImageHeight = 0;
				region.imageSubresource = VkImageSubresourceLayers{videoTexture.tex[0]->getAspect(), 0, 0, 1};
				region.imageOffset = VkOffset3D{};
				region.imageExtent.depth = 1;
				for (int i = 0; i < 4; ++i) {
					region.bufferOffset = imageBuffers[i][frameIdx].offset;
					region.imageExtent.width = widths[i];
					region.imageExtent.height = heights[i];
					vkCmdCopyBufferToImage(cmd, stagingBuffer->getBuffer(), videoTexture.tex[i]->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
				}
			} else if (m_isVideoPlayed) {
				if (decoding) {
					pauseCurrentVideo();
					waitCacheFull = true;
				} else {
					if (reloop) {
						restartCurrentVideo();
					} else {
						cLog::get()->write("end of file");
						stopCurrentVideo(false);
					}
				}
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
		while (framesAvailable(frameCached, frameUsed) >= (MAX_CACHED_FRAMES-1) && decoding)
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
	nextFrame = currentTime + std::chrono::steady_clock::duration(static_cast<int64_t>(deltaFrame.count() / playbackSpeedFactor.toDouble()));
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
