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

#include "spacecrafter.hpp"
#include "mediaModule/video_player.hpp"
#include "tools/log.hpp"
#include "tools/s_texture.hpp"
#include "mediaModule/media.hpp"

#include "eventModule/event_recorder.hpp"
#include "eventModule/EventVideo.hpp"


VideoPlayer::VideoPlayer(Media* _media)
{
	media = _media;
	m_isVideoPlayed = false;
	m_isVideoInPause = false;
	m_isVideoSeeking = false;
#ifndef WIN32
	img_convert_ctx = NULL;
#endif
}


VideoPlayer::~VideoPlayer()
{
	stopCurrentVideo();
}


void VideoPlayer::pauseCurrentVideo()
{
	if (m_isVideoInPause==false) {
		m_isVideoInPause = true;
		startPause = SDL_GetTicks();
	} else {
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
#ifndef WIN32
	#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	    av_register_all();
	#endif
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
#endif
}


bool VideoPlayer::restartCurrentVideo()
{
	if (!m_isVideoPlayed)
		return false;
#ifndef WIN32
	if(av_seek_frame(pFormatCtx, -1, 0, AVSEEK_FLAG_BACKWARD) < 0) {
		printf("av_seek_frame forward failed. \n");
		return false;
	}

	firstCount =  SDL_GetTicks();
	currentFrame = 0;

	return true;
#endif
}


int VideoPlayer::playNewVideo(const std::string& _fileName, bool convertToRBG)
{
#ifndef WIN32
	if (m_isVideoPlayed)
		stopCurrentVideo();
	std::ifstream fichier(_fileName.c_str());
	if (!fichier.fail()) { // verifie si le fichier vidéo existe
		cLog::get()->write("Videoplayer: reading file "+ _fileName, LOG_TYPE::L_INFO);
		fileName = _fileName;
	} else {
		cLog::get()->write("Videoplayer: error reading file "+ _fileName + " abording...", LOG_TYPE::L_ERROR);
		return -2;
	}

	init();

	//tests internes à ffmpeg
	if(avformat_open_input(&pFormatCtx,fileName.c_str(),NULL,NULL)!=0) {
		cLog::get()->write("Couldn't open input stream.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return -1;
	}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0) {
		cLog::get()->write("Couldn't find stream information.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return -1;
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
		return -1;
	}

	video_st = pFormatCtx->streams[videoindex];

	pCodecCtx= avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);

	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL) {
		cLog::get()->write("Unsupported pCodec for video file", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return -1;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0) {
		cLog::get()->write("Could not open codec.", LOG_TYPE::L_ERROR);
		avformat_close_input(&pFormatCtx);
		return -1;
	}

	videoRes.w = pCodecCtx->width;
	videoRes.h = pCodecCtx->height;

	AVRational frame_rate = av_guess_frame_rate(pFormatCtx, video_st, NULL);
	frameRate = frame_rate.num/(double)frame_rate.den;
	frameRateDuration = 1000*frame_rate.den/(double)frame_rate.num;
	nbTotalFrame = static_cast<int>(((pFormatCtx->duration)/1000)/frameRateDuration);

	isDisplayRVB = convertToRBG;
	initTexture();

	img_convert_ctx = NULL;
	unsigned char *out_buffer;

	if (isDisplayRVB) {
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
		if(img_convert_ctx==NULL) {
			cLog::get()->write("Unable to get a context for video file", LOG_TYPE::L_ERROR);
			return -1;
		}
		pFrameIn = av_frame_alloc();
		pFrameOut=av_frame_alloc();
		//std::cout << "video de taille: " << pCodecCtx->width << " " << pCodecCtx->height << std::endl;
		//std::cout << "taille d'une frame: "<< av_image_get_buffer_size(AV_PIX_FMT_RGB24,  pCodecCtx->width, pCodecCtx->height,1) << std::endl;
		out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24,  pCodecCtx->width, pCodecCtx->height,1));
		av_image_fill_arrays(pFrameIn->data, pFrameIn->linesize,out_buffer, AV_PIX_FMT_RGB24,pCodecCtx->width, pCodecCtx->height,1);
		av_image_fill_arrays(pFrameOut->data, pFrameOut->linesize,out_buffer, AV_PIX_FMT_RGB24,pCodecCtx->width, pCodecCtx->height,1);
	} else {
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
		if(img_convert_ctx==NULL) {
			cLog::get()->write("Unable to get a context for video file", LOG_TYPE::L_ERROR);
			return -1;
		}
		
		if(pCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P) {
			cLog::get()->write("Video codec isn't in AV_PIX_FMT_YUV420P format", LOG_TYPE::L_ERROR);
			return -1;
		}
		pFrameIn = av_frame_alloc();
		pFrameOut=av_frame_alloc();
		out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));
		av_image_fill_arrays(pFrameIn->data, pFrameIn->linesize,out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);
		av_image_fill_arrays(pFrameOut->data, pFrameOut->linesize,out_buffer, AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);
	}

	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	firstCount = SDL_GetTicks();
	lastCount = firstCount;
	d_lastCount = firstCount;
	currentFrame = 0;
	m_isVideoPlayed = true;
	this->getNextVideoFrame();

	Event* event = new VideoEvent(VIDEO_ORDER::PLAY);
	EventRecorder::getInstance()->queue(event);
	return 0;
#endif
}

void VideoPlayer::update()
{
#ifndef WIN32
	if (! m_isVideoPlayed)
		return;

	if (m_isVideoInPause) {
		return;
	}
	int timePassed = SDL_GetTicks()- lastCount;

	if ( timePassed > (int)frameRateDuration) {
		getNextVideoFrame();
		d_lastCount = firstCount + (int)(frameRateDuration*currentFrame);
		lastCount = (int)d_lastCount;
	}
#endif
}

void VideoPlayer::getNextFrame()
{
#ifndef WIN32
	bool getNextFrame= false;

	while(!getNextFrame) {

		if(av_read_frame(pFormatCtx, packet)<0) {
			cLog::get()->write("fin de fichier");
			m_isVideoSeeking = true;
			media->playerStop();
			return;
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
			if (m_isVideoSeeking && pFrameIn->key_frame==1) {
				m_isVideoSeeking=false;
			}

			getNextFrame = true;
			av_packet_unref(packet);
		}
	}
#endif
}


void VideoPlayer::getNextVideoFrame()
{
#ifndef WIN32
	this->getNextFrame();
	currentFrame ++;
	if (!m_isVideoSeeking) {
		if (isDisplayRVB) {
			sws_scale(img_convert_ctx, pFrameIn->data, pFrameIn->linesize, 0, pCodecCtx->height, pFrameOut->data, pFrameOut->linesize);
			glBindTexture(GL_TEXTURE_2D, RGBtexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx->width, pCodecCtx->height, GL_RGB, GL_UNSIGNED_BYTE, pFrameOut->data[0]);
		} else {
			const int widths[3]  = { videoRes.w, videoRes.w / 2, videoRes.w / 2 };  
			const int heights[3] = { videoRes.h, videoRes.h / 2, videoRes.h / 2 };
			sws_scale(img_convert_ctx, pFrameIn->data, pFrameIn->linesize, 0, pCodecCtx->height, pFrameOut->data, pFrameOut->linesize);
			for (int i = 0; i < 3; ++i) {  
    			glBindTexture(GL_TEXTURE_2D, YUVtexture[i]);  
    			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, widths[i], heights[i], GL_LUMINANCE, GL_UNSIGNED_BYTE, pFrameOut->data[i]);
			}
		}
	}
#endif
}


void VideoPlayer::stopCurrentVideo()
{
#ifndef WIN32
	if (m_isVideoPlayed==false)
		return;

	m_isVideoPlayed = false;
	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameOut);
	av_frame_free(&pFrameIn);
	avcodec_close(pCodecCtx);

	Event* event = new VideoEvent(VIDEO_ORDER::STOP);
	EventRecorder::getInstance()->queue(event);
	media->playerStop();
#endif
}


void VideoPlayer::initTexture()
{
	if (isDisplayRVB) {
		glGenTextures(1, &RGBtexture);
		glBindTexture(GL_TEXTURE_2D, RGBtexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, videoRes.w, videoRes.h , 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	} else {
		glGenTextures(3, YUVtexture);
		YUV_Texture.TexY = YUVtexture[0];
		YUV_Texture.TexU = YUVtexture[1];
		YUV_Texture.TexV = YUVtexture[2];
		const int widths[3]  = { videoRes.w, videoRes.w / 2, videoRes.w / 2 };  
		const int heights[3] = { videoRes.h, videoRes.h / 2, videoRes.h / 2 };
		for (int i = 0; i < 3; ++i) {  
    		glBindTexture(GL_TEXTURE_2D, YUVtexture[i]);  
    		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, widths[i],heights[i],0,GL_LUMINANCE,GL_UNSIGNED_BYTE, NULL);  
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
    		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
		}
	}
}


/* lets take a leap forward the video */
bool VideoPlayer::jumpInCurrentVideo(float deltaTime, float &reallyDeltaTime)
{
	if (m_isVideoPlayed==false)
		return false;
	if (m_isVideoInPause==true)
		this->pauseCurrentVideo();

#ifndef WIN32
	int64_t frameToSkeep = (1000.0*deltaTime) / frameRateDuration;
	return seekVideo(frameToSkeep, reallyDeltaTime);
#endif
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
#ifndef WIN32
	currentFrame = currentFrame + frameToSkeep;

	//saut avant le début de la vidéo
	if (currentFrame <= 0) {
		this->restartCurrentVideo();
		reallyDeltaTime=0.0;
		return true;
	}
	if(currentFrame < nbTotalFrame) { // on verifie qu'on ne saute pas hors vidéo
		if(avformat_seek_file(pFormatCtx, -1, INT64_MIN, currentFrame * frameRateDuration *1000 , INT64_MAX, AVSEEK_FLAG_ANY) < 0) {
			printf("av_seek_frame forward failed. \n");
			return false;
		}
		firstCount = firstCount - (int)( frameToSkeep * frameRateDuration);
	
		reallyDeltaTime= currentFrame * frameRateDuration/1000.0;
		m_isVideoSeeking = true;
		return true;
	}
	// fin de fichier ... vidéo s'arrête
	this->stopCurrentVideo();
	reallyDeltaTime= -1.0;
	return true;
#else
	return false;
#endif
}