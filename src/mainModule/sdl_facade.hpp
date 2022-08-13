/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2021 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

/*
 * A wrapper around basic SDL functionality such as initialization, surface creation.
 */

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include "tools/no_copy.hpp"

class SDLFacade : public NoCopy{

public:
	static SDL_Cursor *create_cursor(const char *image[]);
	SDLFacade();
	virtual ~SDLFacade();

	// Must be called prior to any other SDL methods
	void initSDL();

	// Creates the rendering target. //Must be called prior to any OpenGL functions
	void createWindow(const std::string& appName, Uint16 w, Uint16 h, bool fullScreen, std::string iconFile); //, bool _debug);
	void createEmptyWindow(const std::string& appName, Uint16 w, Uint16 h);

	// Video mode queries
	void getResolution( Uint16* const w, Uint16* const h ) const;
	Uint16 getDisplayWidth() const {
		return windowW;
	}
	Uint16 getDisplayHeight() const {
		return windowH;
	}

	std::string getStrResolution() const {
		return std::to_string(windowW) +"x"+ std::to_string(windowH);
	}

	void warpMouseInWindow(float x, float y) const {
		SDL_WarpMouseInWindow( window, x , y);
	}

	void warpMouseInCenter() {
		SDL_WarpMouseInWindow( window, windowW /2, windowH /2);
	}

	SDL_Window *getWindow() {
		return isWindowEmpty ? nullptr : window;
	}
private:
	void getLogInfos(int w, int h);
	SDL_Window *window = nullptr;
	SDL_Cursor *Cursor = nullptr;
	Uint16 windowW;
	Uint16 windowH;
	bool isWindowEmpty = false;
};
