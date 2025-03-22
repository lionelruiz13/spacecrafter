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
#include <chrono>
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
class InitParser;

// Maximal number of frames to load in advance (no longer need to be a power-of-two)
#define MAX_CACHED_FRAMES 80
// Minimal number of cached frames below which frames will be delivered with some latency
#define CACHE_STRESS 16
// Maximal speed at which the video is played when there is more than CACHE_STRESS cached frames and the video stream is behind
#define MAX_VIDEO_SPEED 1.5
// Minimal speed at which the video is played when below CACHE_STRESS
#define MIN_VIDEO_SPEED 0.6

constexpr double SPEED_INCREMENT_PER_CACHED_FRAME = (1 - MIN_VIDEO_SPEED) / CACHE_STRESS;

enum class DecodePolicy {
	ASYNC, // Asynchronously decode using a single thread
	THREADED, // Require the codec context to use multiple threads
};

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
	VideoPlayer(Media* _media, InitParser &conf);

	//! Destructor, closes the states of the ffmpeg
	~VideoPlayer();

	//! Create video textures
	void createTextures();

	//! looks for the next frame
	void update();

	//! initializes the ffmpeg with the name of the file passed in argument
	bool playNewVideo(const std::string& fileName, bool paused = false, DecodePolicy policy = DecodePolicy::ASYNC);

	void setRenderFramerate(int framerate) {
		renderDeltaFrame = std::chrono::steady_clock::duration(std::chrono::steady_clock::period::den / (std::chrono::steady_clock::period::num * framerate));
	}

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

	bool isVideoCacheFull() const {
		return (frameCached.load(std::memory_order_relaxed) - frameUsed.load(std::memory_order_relaxed) >= (MAX_CACHED_FRAMES-1)) || !decoding;
	}

	//! Returns the ID of the YUV textures in the GPU representing the frame read from the video file
	VideoTexture getYUV_VideoTexture() const {
		return videoTexture;
	}

	//! Record texture update to the transfer command executed in the graphic queue where the texture is used
	void recordUpdate(VkCommandBuffer cmd);
	//! Record event synchronization which can't be performed inside the renderPass
	void recordUpdateDependency(VkCommandBuffer cmd);
	void setAdaptiveFramerate(bool enable) {
		adaptiveFramerate = enable;
	}
private:
	// returns the new video frame and converts it in the CG memory.
	void getNextVideoFrame();
	// retrieves the new video frame before conversion
	bool getNextFrame();
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
	std::chrono::steady_clock::time_point nextFrame; // Time at which the next video frame should be rendered
	std::chrono::steady_clock::time_point currentTime; // Time at which the last frame was rendered, regardless of the video frame used

	//frameRate management
	int64_t currentFrame;	//!< number of the current frame
	int64_t nbTotalFrame;	//!< number of frames in the video
	double frameRate;
	std::chrono::steady_clock::duration latency; // Time behind the video which need to be reclaimed
	std::chrono::steady_clock::duration deltaFrame; // Time between two frames
	std::chrono::steady_clock::duration renderDeltaFrame; // Time between two rendered frames

	//performance query
	std::chrono::steady_clock::time_point sTime;
	std::chrono::steady_clock::duration sRead{};
	std::chrono::steady_clock::duration sParse{};
	std::chrono::steady_clock::duration sDecode{};
	std::chrono::steady_clock::duration sWrite{};

	// avoid recalculating each time
	int widths[3];
	int heights[3];

	std::atomic<uint32_t> frameCached = 0; // Index of the last cached frame
	std::atomic<bool> decoding = false; // Tell if the video have not been fully decoded yet

	//parameters related to ffmpeg
	AVFormatContext	*pFormatCtx;
	int				videoindex;
	AVCodecContext	*pCodecCtx;
	const AVCodec	*pCodec;
	AVFrame			*pFrameIn,*pFrameOut;
	AVStream		*video_st;
	AVPacket		*packet;
	struct SwsContext *img_convert_ctx;

	std::atomic<uint32_t> frameUsed = 0; // Index of the last rendered frame
	int frameIdxSwap = 0;
	uint8_t codecDecodeThreads = 0;
	bool firstUse = true; // Tell if this texture is new and uninitialized yet
	bool skipFrame = false; // Tell if frame could be skipped when playing a video
	bool adaptiveFramerate = false;
	void mainloop();
	// Stop video thread and drop every pending frames
	void threadTerminate();
	// Interrupt video thread and drop every pending frames
	void threadInterrupt();
	// Start or resume video thread
	void threadPlay();
	std::thread thread;
	std::mutex mtx;
	std::condition_variable cv;
};

#endif // VIDEOPLAYER_HPP
