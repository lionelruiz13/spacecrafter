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

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <SDL2/SDL.h>
#include <chrono>
#include <thread>

#include "appModule/fps.hpp"
#include "tools/log.hpp"

//! switches to video recording mode
void Fps::selectVideoFps() {
	recVideoMode = true;
	frameDuration = std::chrono::steady_clock::duration(static_cast<uint64_t>(std::chrono::steady_clock::period::den / (std::chrono::steady_clock::period::num * static_cast<long double>(videoFPS))));
}

//! switches to normal mode
void Fps::selectMaxFps() {
	recVideoMode = false;
	frameDuration = std::chrono::steady_clock::duration(static_cast<uint64_t>(std::chrono::steady_clock::period::den / (std::chrono::steady_clock::period::num * static_cast<long double>(maxFPS))));
}

uint32_t Fps::beginFrame()
{
	currentFrameDuration = frameDuration;
	if (!(suspended | recVideoMode)) {
		auto now = std::chrono::steady_clock::now();
		if (now > nextFrameEnd) { // Skip any latency beyond the current frame's duration
			currentFrameDuration += now - nextFrameEnd;
			nextFrameEnd = now;
		}
	}
	static_assert(std::chrono::steady_clock::period::den % (std::chrono::steady_clock::period::num * 1000ULL) == 0U, "Can't accurately convert to milliseconds with the current implementation.");
	auto deltaTime = std::lldiv((durationRoundingError + currentFrameDuration).count(), std::chrono::steady_clock::period::den / (std::chrono::steady_clock::period::num * 1000ULL));
	durationRoundingError = std::chrono::steady_clock::duration(deltaTime.rem);
	return deltaTime.quot;
}

void Fps::endFrame()
{
	numberFrames.fetch_add(1, std::memory_order_relaxed);
	if (suspended) {
		suspended = false;
		nextFrameEnd = std::chrono::steady_clock::now() + frameDuration;
	} else {
		std::this_thread::sleep_until(nextFrameEnd);
		nextFrameEnd += frameDuration;
	}
}

void Fps::watchdogMainloop()
{
	uint64_t lastFrame = 0;
	uint32_t nbSameFrame = 0;
	std::array<uint64_t, 20> lastFrameHistory{};
	uint64_t i = 0;
	auto lastCheck = std::chrono::steady_clock::now();
	while (active) {
		std::this_thread::sleep_until(lastCheck += std::chrono::milliseconds(50));
		const uint64_t currentFrame = numberFrames.load(std::memory_order_relaxed);
		framerate = currentFrame - lastFrameHistory[++i % 20U];
		lastFrameHistory[i % 20U] = currentFrame;
		if (lastFrame != currentFrame) {
			lastFrame = currentFrame;
			nbSameFrame = 0;
		} else if (!suspended) {
			switch (++nbSameFrame) {
				case 2:
					cLog::get()->write("Frame stall detected", LOG_TYPE::L_WARNING);
					break;
				case 20:
					cLog::get()->write("This frame stall is very long", LOG_TYPE::L_WARNING);
					break;
			}
		}
	}
}
