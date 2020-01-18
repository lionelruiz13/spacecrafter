/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017  Association Sirius
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

#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>

#include "appModule/fps.hpp"


//! ajoute une frame
void Fps::addFrame() {
	numberFrames++;
	frame++;
}

void Fps::afterOneSecond()
{
	fps = frame;
	frame = 0; 
/* UNCOMMENT IF SAVE30FPS
	if (calculatedTime-timeBase > SECONDEDURATION) {
		fps=frame*SECONDEDURATIONF/(calculatedTime-timeBase);
		timeBase+=SECONDEDURATION;
		frame -= fps;
		int delay =int(effectiveCalculatedTime)-calculatedTime;
		if (delay>0) {
		calculatedTime += delay;
		}
		return true;
	} else {
		return false;
	} */
}

void Fps::wait()
{
	// std::cout << "Dt: " << tickCount - lastCount << " Fd: " << frameDuration;
	if (tickCount-lastCount < frameDuration) {
		int delay = frameDuration - (tickCount-lastCount) ;
		if (delay<0)
			delay = 0;
		SDL_Delay(delay);
	} else {
		SDL_Delay(1); // no full speed
	}
}


Uint32 Fps::callbackfunc(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = NULL;
    userevent.data2 = NULL;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}
