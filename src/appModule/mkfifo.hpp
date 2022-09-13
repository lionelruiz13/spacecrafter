/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of Association Sirius
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

#ifndef MKFIFO_HPP
#define MKFIFO_HPP

#include <SDL2/SDL_thread.h>
#include <queue> //ServerSocket
#include "tools/no_copy.hpp"
//#include "tools/app_settings.hpp"
#include <string>
#include <SDL2/SDL_net.h> //ServerSocket
#include <iostream> //ServerSocket

#ifdef __linux__
//for pipe
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#endif

class Mkfifo : public NoCopy {
public:
	/*!
	 * \brief Initialize the MKFIFO communication
	 * \param _filename full name of the special file mkfifo
	 * \param _buffer_size size of the buffer concerning the strings
	 */
	void init(const std::string& _filename, int _buffer_size);
	//! constructor
	Mkfifo();
	//! destructor
	~Mkfifo();
	/*!
	 * \brief retrieve an information from the pipe
	 * \param string receiving the information
	 * \return true if message obtained, false otherwise
	 */
	bool update(std::string &output);
private:
	// indicates the state of the Mkfifo
	bool is_active = false;
	// size of the buffer
	int buffer_size;
	// queue containing all messages obtained from the outside
	std::queue<std::string> from_outside;
	//full name of the pipe file
	std::string filename;
	//mutex on the pipe IO readings
	SDL_mutex* lock = nullptr;
	// create thread for mkfifo
	SDL_Thread* threadMkfifoRead;
	// function thread qui gere la lecture des données de l'extérieur
	int thread();
	// function what call threadMkfifoRead
	static int thread_wrapper(void *Data);
};

#endif // IO_H
