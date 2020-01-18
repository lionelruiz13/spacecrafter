/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2007 Digitalis Education Solutions, Inc.
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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

// manage an external viewer

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
//pipe
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <signal.h> //kill

#include "spacecrafter.hpp"
#include "mediaModule/external_mplayer.hpp"
#include "tools/app_settings.hpp"
#include "tools/call_system.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"

#define LEN 25



ExternalMplayer::ExternalMplayer(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}

void ExternalMplayer::init(const std::string &mplayerFileName, const std::string &mplayerMkfifoName, bool enable_mplayer)
{
	if (enable_mplayer) {
		mplayerMkfifo = mplayerMkfifoName;
		mplayerName = mplayerFileName;
		unlink(mplayerMkfifo.c_str()); //TODO verifier avant de supprimer
		cLog::get()->write("service ExternalMplayer est initialisé comme actif", LOG_TYPE::L_DEBUG);
	} else
		cLog::get()->write("service ExternalMplayer est initialisé comme inactif", LOG_TYPE::L_DEBUG);
	initMkfifo(enable_mplayer);

}

void ExternalMplayer::update(int delta_time)
{
    #ifdef LINUX
	if (!serviceAvariable)
		return ;
	if (kill(mplayerPid,0)!=0) {
		//~ printf("mplayer déconnecté\n");
		cLog::get()->write("video player not found reloading...", LOG_TYPE::L_INFO);
		launchMplayer();
	}
	#endif
}

void ExternalMplayer::initMkfifo(bool enable)
{
    #ifdef LINUX
	if (! enable) {
		cLog::get()->write("mplayer slave mode: system unavariable", LOG_TYPE::L_INFO);
		serviceAvariable = false;
		return ;
	}

	if (mkfifo(mplayerMkfifo.c_str(), S_IRWXU| S_IWGRP | S_IWOTH ) == -1) {
		cLog::get()->write("Error creating video mkFifo pipe"+Utility::intToString(errno), LOG_TYPE::L_ERROR);
		serviceAvariable = false;
	} else {
		cLog::get()->write("Creating video mkfifo pipe successfull", LOG_TYPE::L_INFO);
		char mode[] = "0777";
		int i = strtol(mode, 0, 8);
		chmod(mplayerMkfifo.c_str(),i);

		if((mkfifoFile = open( mplayerMkfifo.c_str(), O_RDWR )) == -1) {
			cLog::get()->write("Unable to open video mkfifo pipe, error code is "+Utility::intToString(errno), LOG_TYPE::L_ERROR);
			printf("error %i\n", errno);
			serviceAvariable = false;
		} else {
			serviceAvariable = true;
		}
	}
	launchMplayer();
	#endif
}


void ExternalMplayer::launchMplayer()
{
	std::string action;
	unsigned min_r = std::min(width, height);
	std::string motif = Utility::intToString(min_r) +"x"+Utility::intToString(min_r)+"+"+Utility::intToString((width-min_r)/2)+"+"+Utility::intToString((height-min_r)/2);
	action = "mplayer -slave -input file=" + mplayerMkfifo + " -idle -quiet -noborder -geometry "+ motif + " -osdlevel 0&";
	
	//printf("%s\n", action.c_str());
	if (CallSystem::useSystemCommand(action)) {
		cLog::get()->write("ExternalPlayer : launch Mplayer slave successfull", LOG_TYPE::L_DEBUG);
		serviceAvariable = true;
	} else {
		//~ printf("system: échec au lancement de Mplayer\n");
		cLog::get()->write("ExternalPlayer : failure to launch Mplayer slave", LOG_TYPE::L_ERROR);
		serviceAvariable = false;
	}
	if (!serviceAvariable)
		return ;
	//recuperer le pid de mplayer pour lui envoyer des kill 0 afin de tester sa présence
	char line[LEN];
	FILE *cmd = popen("pidof /usr/bin/mplayer", "r");
	fgets(line, LEN, cmd);
	pclose(cmd);
	mplayerPid = std::stoi(line);
}


int ExternalMplayer::play(const std::string &filename)
{
	if (serviceAvariable == false)
		return -1;

	stop();
	std::string extention=filename.substr(filename.length()-3,3);

	if (!(extention=="avi" || extention=="mov" || extention=="mpg" || extention=="mp4" || extention=="mp3" || extention=="ogg") ) {
		cLog::get()->write("unknown extention :  " + extention , LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
		return -2;
	}

	order.clear();
	order = "loadfile "+ filename+"\n";
	writeToMplayer(order);

	return 0;

}

ExternalMplayer::~ExternalMplayer()
{
	if (!serviceAvariable)
		return ;
	order.clear();
	order = "quit\n";
	writeToMplayer(order);

	unlink(mplayerMkfifo.c_str());
}

void ExternalMplayer::jumpAbsolute(int secondes)
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = "seek " + std::to_string(secondes) + " 0\n";
	writeToMplayer(order);
}

void ExternalMplayer::jumpRelative(int secondes)
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = "seek " +std::to_string(secondes) + " 1\n";
	writeToMplayer(order);
}

void ExternalMplayer::volume(int sound)
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = "volume " +std::to_string(sound) + "\n";
	writeToMplayer(order);
}


void ExternalMplayer::execute(const std::string &msg)
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = msg;
	writeToMplayer(order);
}

void ExternalMplayer::speed(int lvl)
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = "speed_set "+std::to_string(lvl) + "\n";
	writeToMplayer(order);
}

void ExternalMplayer::pause()
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = "pause\n";
	writeToMplayer(order);
}


void ExternalMplayer::stop()
{
	if (!serviceAvariable)
		return ;

	order.clear();
	order = "stop\n";
	writeToMplayer(order);
}

bool ExternalMplayer::writeToMplayer(const std::string &msg)
{
	if (!serviceAvariable)
		return false;

	int value = write(mkfifoFile, msg.c_str(), msg.size());
	if (value < 0) {
		printf("Mplayer write mkfifo erreur -1\n");
		cLog::get()->write("ExternalPlayer : error mkfifo writing", LOG_TYPE::L_ERROR);
		return false;
	} else {
		cLog::get()->write("ExternalPlayer : mkfifo write "+ msg, LOG_TYPE::L_DEBUG);
		return true;
	}
}
