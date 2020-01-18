/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the Association Sirius
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

#include <fstream>
#include <iostream> //ServerSocket
#include <sstream>
#include <string> //ServerSocket

#include "spacecrafter.hpp"
#include "appModule/mkfifo.hpp" //ServerSocket
#include "tools/log.hpp"
#include "tools/utility.hpp"

#ifdef LINUX
//for pipe
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif


#if LINUX // special mkfifo


//mkfifo -----------------------
Mkfifo::Mkfifo()
{
	is_active=false;
	lock=NULL;
}

Mkfifo::~Mkfifo()
{
	is_active=false;
}

void Mkfifo::init(const std::string& _filename, int _buffer_size)
{
	filename=_filename;
	buffer_size= _buffer_size;
	is_active= true;
	lock=SDL_CreateMutex();
	threadMkfifoRead = SDL_CreateThread(thread_wrapper,"ThreadRead", this);
}

int Mkfifo::thread_wrapper(void *Data)
{
	return ((Mkfifo *)Data)->thread();
}

int Mkfifo::thread()
{
	cLog::get()->write("Thread MKFIFO, buffer_in_size is "+Utility::intToString(buffer_size), LOG_TYPE::L_INFO, LOG_FILE::TCP);
	int fdtr;
	char *in;
	in = (char*) calloc (buffer_size,sizeof(char));
	if (in ==NULL) return 1;

	cLog::get()->write("Pipe named  " + filename, LOG_TYPE::L_INFO);
	unlink(filename.c_str());
	if (mkfifo((filename.c_str()), S_IRWXU| S_IWGRP | S_IWOTH ) == -1) { //TODO why result has no g+o=w mode ?
		cLog::get()->write("Error creating MkFifo pipe thread in_thread "+Utility::intToString(errno), LOG_TYPE::L_ERROR, LOG_FILE::TCP);
		return 2;
	} else {
		cLog::get()->write("Creating Mkfifo pipe successfull", LOG_TYPE::L_INFO);
		char mode[] = "0777";
		int i = strtol(mode, 0, 8);
		chmod(filename.c_str(),i);
	}
	if((fdtr = open (filename.c_str(), O_RDONLY )) == -1) {
		cLog::get()->write("Unable to open named mkfifo pipe, error code is "+Utility::intToString(errno), LOG_TYPE::L_ERROR, LOG_FILE::TCP);
		return 2;
	}

	while (is_active) {
		memset(in,'\0',buffer_size);

		if (read(fdtr, in, buffer_size) == -1) {
			cLog::get()->write("Mkfifo read() error "+Utility::intToString(errno), LOG_TYPE::L_ERROR, LOG_FILE::TCP);
		}

		if (in[0] !='\0') {
			in[buffer_size-1]='\0';
			SDL_LockMutex( lock );
			from_outside.push(in);
			SDL_UnlockMutex( lock );
		} else
			sleep(1);
	}
	cLog::get()->write("Closing Mkfifo pipe thread", LOG_TYPE::L_INFO, LOG_FILE::TCP);
	close(fdtr);
	free(in);
	unlink(filename.c_str());
	return 0;
}

bool Mkfifo::update(std::string &output)
{
	if ( (is_active ==true)) {
		if (! from_outside.empty() ) {
			SDL_LockMutex(lock);
			output= from_outside.front();
			from_outside.pop();
			SDL_UnlockMutex(lock);
			cLog::get()->write("Mkfifo : I get " + output, LOG_TYPE::L_INFO, LOG_FILE::TCP);
			return true;
		} else
			return false;
	} else
		return false;
}

#endif
