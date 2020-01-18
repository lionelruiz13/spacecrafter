/*
 * SDLFacade.cpp
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

#include "SDLFacade.hpp"
#include <iostream>
#include <GL/glew.h>

SDLFacade::SDLFacade(int _width, int _height)
{
	width = _width;
	height = _height;
}

SDLFacade::~SDLFacade()
{}


bool SDLFacade::init(const std::string &programName)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to init SDL\n";
		return false;
	}
	mainWindow = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,width, height, SDL_WINDOW_OPENGL);
	if (!mainWindow) {
		std::cout << "Unable to create window\n";
		CheckSDLError(__LINE__);
		return false;
	}
	mainContext = SDL_GL_CreateContext(mainWindow);
	SetOpenGLAttributes();
	PrintSDL_GL_Attributes();
	SDL_GL_SetSwapInterval(1);

	glewInit();
	return true;
}

bool SDLFacade::SetOpenGLAttributes()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	return true;
}


void SDLFacade::cleanup()
{
	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

void SDLFacade::CheckSDLError(int line = -1)
{
	std::string error = SDL_GetError();
	if (error != ""){
		std::cout << "SLD Error : " << error << std::endl;
		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;
		SDL_ClearError();
	}
}

void SDLFacade::PrintSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}

void SDLFacade::swapWindow()
{
	SDL_GL_SwapWindow(mainWindow);
}
