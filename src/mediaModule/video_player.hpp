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


#ifndef _VIDEOPLAYER_HPP_
#define _VIDEOPLAYER_HPP_

#include <stdio.h>
#include <inttypes.h>

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <array>
#include "mediaModule/media_base.hpp"
#include "EntityCore/SubBuffer.hpp"

#include <thread>
#include <mutex>
#include "EntityCore/Tools/SafeQueue.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class s_texture;
class Media;
class SyncEvent;
class BufferMgr;

#define MAX_CACHED_FRAMES 31

/**
 * \class VideoPlayer
 * \brief Class that handles all the ffmpeg functions for the video player.
 *
 * Its goal is to make a pointer on texture(s) usable in the rest of the software
 * Two possibilities are offered to the user :
 * - a classic RBG24 texture (but slow to obtain because of conversions)
 * - 3 YUV420p textures directly (fast to obtain: directly from the file to the graphic card)
 *
 * The class manages by itself the good fps, that is to say that it looks via calls to SDL_GetTicks if it must update the frame.
 * If it is the case, then it updates the texture(s).
 *
 */
class VideoPlayer {
public:
	//! \fn VideoPlayer
	//! \brief Constructor: initializes the states of the ffmpeg
	VideoPlayer(Media* _media);

	//! Destructor, closes the states of the ffmpeg
	~VideoPlayer();

	//! Create video textures
	void createTextures();

	//! looks for the next frame
	void update();

	//! initializes the ffmpeg with the name of the file passed in argument
	bool playNewVideo(const std::string& fileName);

	//! ends the playback of a video in progress
	void stopCurrentVideo(bool newVideo);

	//! pauses the video
	void pauseCurrentVideo();

	//! Restarts the current video at the beginning
	bool restartCurrentVideo();

	bool invertVideoFlow(float &reallyDeltaTime);

	//! Allows to make a relative jump in the video stream
	//! \param seconde time to jump (in seconds)
	//! \param reallyDeltaTime : tells the Media class how far we have moved in the end.
	bool jumpInCurrentVideo(float seconde, float &reallyDeltaTime);

	//! Returns the state of the player
	//! @return true if a file is playing, false otherwise
	bool isVideoPlayed() const {
		return m_isVideoPlayed;
	}

	//! Returns the ID of the YUV textures in the GPU representing the frame read from the video file
	VideoTexture getYUV_VideoTexture() const {
		return videoTexture;
	}

	//! Record texture update to the transfer command executed in the graphic queue where the texture is used
	void recordUpdate(VkCommandBuffer cmd);
	//! Record event synchronization which can't be performed inside the renderPass
	void recordUpdateDependency(VkCommandBuffer cmd);
private:
	// returns the new video frame and converts it in the CG memory.
	void getNextVideoFrame(int frameIdx);
	// retrieves the new video frame before conversion
	void getNextFrame();
	// initialization of the class
	void init();
	// internal jump function in the video
	bool seekVideo(int64_t frameToSkeep, float &reallyDeltaTime);
	//! initialize a texture to the size of the video
	void initTexture();

	Media* media=nullptr;
	VideoTexture videoTexture;	//!< returns the texture indices for the classes requiring
	std::unique_ptr<BufferMgr> stagingBuffer;
	SubBuffer imageBuffers[3][MAX_CACHED_FRAMES];
	std::array<void *[MAX_CACHED_FRAMES], 3> pImageBuffer;

	std::string fileName; 	//!< video name
	Resolution videoRes;	//!< int video_w, video_h;	//!< size w,h of the vidéo

	bool m_isVideoPlayed;	//!< indicates if a video is playing
	bool m_isVideoInPause;	//!< indicates if the video is paused
	bool m_isVideoSeeking;	//!< indicates if a frame is being skipped

	//time management
	int64_t TickCount;
	int64_t firstCount;
	int64_t lastCount;
	double d_lastCount;

	//frameRate management
	int64_t currentFrame;	//!< number of the current frame
	int64_t nbTotalFrame;	//!< number of frames in the video
	double frameRate;
	double frameRateDuration;

	//pause management
	Uint32 startPause;
	Uint32 endPause;

	// avoid recalculating each time
	int widths[3];
	int heights[3];


	enum VideoThreadEvent {
		INTERRUPT = -2,
		RESUME = -1
	};
	//parameters related to ffmpeg
	AVFormatContext	*pFormatCtx;
	int				videoindex;
	AVCodecContext	*pCodecCtx;
	const AVCodec	*pCodec;
	AVFrame			*pFrameIn,*pFrameOut;
	AVStream		*video_st;
	AVPacket		*packet;
	struct SwsContext *img_convert_ctx;
	bool firstUse = true; // Tell if this texture is new and uninitialized yet
	bool decodeEnd = false; // Tell if the last frame of the video have been decoded
	bool wantInterrupt = false; // Tell if the worker thread should be interrupted
	std::atomic<int> needFrames = 0; // Tell how many frames are required now
	std::atomic<int> plannedFrames = 0; // Tell how many frame have been planned but not used
	int frameIdxSwap = 0; // To alternate between frameIdx
	void mainloop();
	// Interrupt video thread and drop every pending frames
	void threadInterrupt();
	// Resume video thread
	void threadResume();
	std::thread thread;
	PushQueue<int, MAX_CACHED_FRAMES> displayQueue; // Frames ready to display
	WorkQueue<int, MAX_CACHED_FRAMES> requestQueue; // Frames to compute
};

#endif // VIDEOPLAYER_HPP
