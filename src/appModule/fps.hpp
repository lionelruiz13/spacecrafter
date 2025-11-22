/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017  Association Sirius
 * Copyright (C) 2018-2020  Association Sirius & LSS Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef FPS_CLOCK_HPP
#define FPS_CLOCK_HPP

#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>
#include <thread>
#include <atomic>

#include "tools/no_copy.hpp"

/**
* \file fps.hpp
* \brief Framerate management
* \author Olivier NIVOIX
* \author Calvin RUIZ
* \version 2
*/

/*! @class Fps
* @brief class dealing with framerate and FPS
*
* @description
* The Fps class manages the framerate of the software.
*/
class Fps  : public NoCopy {
public:
	Fps();
	~Fps()
	{
		active = false;
		watchdog.join();
	}

	//! returns the duration of the current frame (in seconds)
	std::chrono::steady_clock::duration getPreciseDeltaTime() const noexcept {
		return recVideoMode ? frameDuration : currentFrameDuration;
	}

	//! indicates at what FPS the program should run in video capture mode
	void setVideoFps(double fps) noexcept {
		videoFPS = fps;
	}

	//! indicates at what FPS the program should run in normal mode
	void setMaxFps(double fps) noexcept {
		maxFPS = fps;
	}

	//! switches to video recording mode
	void selectVideoFps();

	//! switches to normal mode
	void selectMaxFps();

	//! Mark the beginning of a frame
	//! Return the delta time in milliseconds, rounding error are accumulated internally so that the sum is not off by more than 0.5ms
	uint32_t beginFrame();

	//! Mark the end of a frame
	//! Wait the time needed to stabilize at the target framerate
	void endFrame();

	//! indicates the current target FPS
	double getTargetFps() const {
		return recVideoMode ? videoFPS : maxFPS;
	}

	//! indicates the current FPS
	uint32_t getFps() const {
		return framerate;
	}

	void watchdogMainloop();

private:
	static void sigstacktrace(int);
	std::atomic<uint64_t> numberFrames=0;
	uint64_t pid=0U;
	double videoFPS=30.;
	double maxFPS=30.;
	std::chrono::steady_clock::duration frameDuration;
	std::chrono::steady_clock::duration currentFrameDuration;
	std::chrono::steady_clock::time_point nextFrameEnd{}; // Expected time for the next frame end
	std::chrono::steady_clock::duration durationRoundingError{std::chrono::microseconds{500}}; // Mitigate rounding error due to delta time being in milliseconds
	std::thread watchdog;
	uint32_t framerate;
	bool recVideoMode = false;
	bool active = true;
	bool suspended = true;
};

#endif
