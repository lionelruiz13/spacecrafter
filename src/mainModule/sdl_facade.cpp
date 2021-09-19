/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#include <string>
#include <SDL2/SDL_ttf.h>
#include "mainModule/sdl_facade.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"


SDLFacade::SDLFacade()
{}

SDLFacade::~SDLFacade()
{
	SDL_FreeCursor(Cursor);
	SDL_DestroyWindow(window);
}

void SDLFacade::getResolution( Uint16* const w, Uint16* const h ) const
{
	*w = windowW;
	*h = windowH;
}


void SDLFacade::createWindow(const std::string& appName, Uint16 w, Uint16 h, bool fullScreen, std::string iconFile) // , bool _debugGL)
{
	Uint32	Vflags;		// Our Video Flags
	windowW = w;
	windowH = h;

	// We want a hardware surface
	Vflags = SDL_WINDOW_VULKAN|SDL_WINDOW_SHOWN;

	// If fullscreen, set the Flag
	if (fullScreen) Vflags|=SDL_WINDOW_FULLSCREEN;

	// Create the SDL screen surface
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0"); //lost screen after mplayer
	window  = SDL_CreateWindow(appName.c_str(),SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,windowW,windowH,Vflags);
	if (window == nullptr) {
		cLog::get()->write("SDL Could not create window: "+ std::string(SDL_GetError()), LOG_TYPE::L_ERROR);
		exit(1);
	} else
		getLogInfos( w,  h);
	SDL_DisableScreenSaver();

	// set mouse cursor
	static const char *arrow[] = {
		/* width height num_colors chars_per_pixel */
		"    32    32        3            1",
		/* colors */
		"X c #000000",
		". c #ffffff",
		"  c None",
		/* pixels */
		"                                ",
		"                                ",
		"                                ",
		"                                ",
		"                                ",
		"              XXX               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              XXX               ",
		"                                ",
		"                                ",
		"                                ",
		"   XXXXXXXX         XXXXXXXX    ",
		"   X......X         X......X    ",
		"   XXXXXXXX         XXXXXXXX    ",
		"                                ",
		"                                ",
		"                                ",
		"              XXX               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              X.X               ",
		"              XXX               ",
		"                                ",
		"                                ",
		"15,17"
	};

	Cursor = create_cursor(arrow);
	SDL_SetCursor(Cursor);

	// Set the window icon
	SDL_Surface *icon = SDL_LoadBMP((iconFile).c_str());
	SDL_SetWindowIcon(window,icon);
	SDL_FreeSurface(icon);
	cLog::get()->mark();
}

void SDLFacade::initSDL()
{
	cLog::get()->mark();

	// Init the SDL library, the VIDEO subsystem
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE | SDL_INIT_TIMER| SDL_INIT_JOYSTICK)<0) {
		cLog::get()->write("Error: unable to open SDL : "+ std::string(SDL_GetError()), LOG_TYPE::L_ERROR );
		exit(-1);
	} else
		cLog::get()->write("SDL init oki !", LOG_TYPE::L_INFO );


	//Init the SDL_TTF library
	if(TTF_Init()==-1) {
		cLog::get()->write("Error: unable to open SDL_TTF : " + std::string(TTF_GetError()), LOG_TYPE::L_ERROR );
		exit(-1);
	}

	// Make sure that SDL_Quit will be called in case of exit()
	atexit(TTF_Quit);
	atexit(SDL_Quit);
	atexit(Mix_CloseAudio);
}

// from an sdl wiki
SDL_Cursor* SDLFacade::create_cursor(const char *image[])
{
	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;

	i = -1;
	for ( row=0; row<32; ++row ) {
		for ( col=0; col<32; ++col ) {
			if ( col % 8 ) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				++i;
				data[i] = mask[i] = 0;
			}
			switch (image[4+row][col]) {
				case 'X':
					data[i] |= 0x01;
					mask[i] |= 0x01;
					break;
				case '.':
					mask[i] |= 0x01;
					break;
				case ' ':
					break;
			}
		}
	}
	sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

void SDLFacade::getLogInfos(int w, int h)
{
	std::stringstream out;
	out << w << "x" << h << "px";
	cLog::get()->write("Windows size is "+out.str(), LOG_TYPE::L_INFO );
	cLog::get()->write("Video driver is  " + std::string(SDL_GetCurrentVideoDriver()) , LOG_TYPE::L_INFO);
	out.clear();
	static int display_in_use = 0; /* Only using first display */
	int i, display_mode_count;
	SDL_DisplayMode mode;
	Uint32 f;
	cLog::get()->write("SDL_GetNumVideoDisplays(): "+ std::to_string(SDL_GetNumVideoDisplays()), LOG_TYPE::L_INFO);

	display_mode_count = SDL_GetNumDisplayModes(display_in_use);
	if (display_mode_count < 1) {
		cLog::get()->write("SDL_GetNumDisplayModes failed: "+ std::string(SDL_GetError()), LOG_TYPE::L_ERROR);
	}
	cLog::get()->write("SDL_GetNumDisplayModes:  "+ std::to_string(display_mode_count), LOG_TYPE::L_INFO);

	for (i = 0; i < display_mode_count; ++i) {
		std::stringstream output;
		if (SDL_GetDisplayMode(display_in_use, i, &mode) != 0) {
			cLog::get()->write("SDL_GetDisplayMode failed: "+ std::string(SDL_GetError()),LOG_TYPE::L_ERROR);
		}
		f = mode.format;
		output << "Mode " << i << " bpp " << SDL_BITSPERPIXEL(f) << " " << SDL_GetPixelFormatName(f) << " " << mode.w << " " << mode.h;
		cLog::get()->write(output.str(), LOG_TYPE::L_INFO);
		output.clear();
	}
}
