/*
 * SDLFacade.hpp
 * 
 * Copyright 2018 Olivier NIVOIX <olivier@orion>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <SDL2/SDL.h>
#include <string>

class SDLFacade
{
public:
	SDLFacade(int _width, int _height);
	~SDLFacade();
	bool init(const std::string &programName);
	void swapWindow();
	void cleanup();
private:
	bool SetOpenGLAttributes();
	void PrintSDL_GL_Attributes();
	void CheckSDLError(int line);
	SDL_Window *mainWindow=nullptr;
	SDL_GLContext mainContext;
	int width, height;
};
