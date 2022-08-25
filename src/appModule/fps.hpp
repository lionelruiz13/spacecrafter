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

#include "tools/no_copy.hpp"

/**
* \file fps.hpp
* \brief Framerate management
* \author Olivier NIVOIX
* \version 2
*/

/*! @class Fps
* @brief class dealing with framerate and FPS
*
* @description
* The Fps class manages the framerate of the software. It uses two conditions for this
* the wait function : it takes care of the duration of a frame compared to another one. (local view, subject to the imperfection of ms integer rounding)
* the afterOneSecond function : it takes care of the duration of the frames over a period of one second in order to determine the FPS
* afterOneSecond is launched via an SDL trigger in App.hpp every 1000 ms.
*/
class Fps  : public NoCopy {
public:
	Fps(){};
	~Fps(){};

	//! Initializes the clock parameters
	void init() {
		initCount= SDL_GetTicks();
		lastCount= SDL_GetTicks();
	};

	//! returns the number of frames displayed since the launch of the software
	unsigned long int getElapsedFrame() const {
		return numberFrames;
	}

	//! adds a frame
	void addFrame();

	//! returns the duration of a loop 
	unsigned int getDeltaTime() const;


	//! indicates at what FPS the program should run in video capture mode
	void setVideoFps(float fps) {
		videoFPS = fps;
	}

	//! indicates at what FPS the program should run in normal mode
	void setMaxFps(float fps) {
		maxFPS = fps;
	}

	//! switches to video recording mode
	void selectVideoFps();

	//! switches to normal mode 
	void selectMaxFps();

	//! Takes a time measurement
	void setTickCount() {
		tickCount = SDL_GetTicks();
	}

	//! Changes the reference time of the clock
	void setLastCount() {
		lastCount = tickCount;
	}

	//! indicates the current FPS
	int getFps() const {
		return fps;
	}

	// Determines how long to wait between two frames to get the theoretical FPS
	void wait();

	//! Calculates the FPS per second and corrects the differences
	void afterOneSecond();

	//! callback function launched by SDL2
	static Uint32 callbackfunc(Uint32 interval, void *param);
private:
	uint64_t numberFrames=0;
	int frame = 0;
	int fps = 0;
	float videoFPS=1.f;
	float maxFPS=1.f;
	uint64_t lastCount = 0;
	uint64_t initCount = 0;
	uint64_t tickCount = 0;
	uint16_t frameDuration=0;
	bool recVideoMode = false;

	const float SECONDEDURATION=1000.0;
};

#endif
