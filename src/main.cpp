/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2011 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014_2015 Association Sirius & LSS team
 * Spacecrafter astronomy simulation and visualization
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
#include <iostream>
#include <cstdlib>
#include <dirent.h>
#include <vector>
#include <SDL2/SDL.h>
#include <sys/stat.h>

#include "spacecrafter.hpp"
#include "appModule/app.hpp"
#include "mainModule/checkConfig.hpp"
#include "mainModule/sdl_facade.hpp"
#include "mainModule/signals.hpp"
#include "tools/call_system.hpp"
#include "tools/translator.hpp"
#include "tools/utility.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"

#ifdef LINUX
#include "mainModule/CPUInfo.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/utsname.h>
#endif


//! write in log file general information about spacecrafter
static void writeGeneralInfo(void)
{
	cLog::get()->mark();
	cLog::get()->write("Welcome to spacecrafter",  LOG_TYPE::L_INFO);
	std::string strMsg="Version : " + std::string(APP_NAME) + " - " + USER_EDITION;
	cLog::get()->write(strMsg, LOG_TYPE::L_INFO);
	strMsg.clear();
	strMsg +=" Copyright (c) 2014-2020 Association Sirius, LSS team et al.\n";
	strMsg +=" Copyright (c) 2012-2014 LSS team et al.\n";
	strMsg +=" Copyright (c) 2003-2011 Digitalis Education Solutions, Inc. et al.\n";
	strMsg +=" Copyright (c) 2000-2008 Fabien Chereau et al.\n";
	strMsg +=" http://lss-planetariums.info";
	cLog::get()->write(strMsg, LOG_TYPE::L_INFO);
	cLog::get()->mark();
}

//! write to log file information about his linux OS
static void getUnameInfo()
{
	#ifdef LINUX
	//get uname's info
	struct utsname buffer;
	errno = 0;
	if (uname(&buffer) != 0) {
		switch (errno) {
			case EFAULT:
				cLog::get()->write("call to uname() with an invalid pointer", LOG_TYPE::L_ERROR);
				break;
			default:
				cLog::get()->write("error calling uname()", LOG_TYPE::L_ERROR);
		}
		exit(EXIT_FAILURE);
	}
	std::string strMsg="system name = "+std::string(buffer.sysname);
	strMsg+="\nnode name   = "+std::string(buffer.nodename);
	strMsg+="\nrelease     = "+std::string(buffer.release);
	strMsg+="\nversion     = "+std::string(buffer.version);
	strMsg+="\nmachine     = "+std::string(buffer.machine);
	cLog::get()->write(strMsg, LOG_TYPE::L_INFO);
	cLog::get()->mark();
	#endif
}

// Display usage in the console
static void usage(char **argv)
{
	std::cout << APP_NAME << std::endl;
	std::cout << _("Usage: %s [OPTION] ...\n -v, --version \tOutput version information and exit.\n -h, --help \tDisplay this help and exit.\n");
}

// Check command line arguments
static void check_command_line(int argc, char **argv)
{
	if (argc == 2) {
		if (!(strcmp(argv[1],"--version") && strcmp(argv[1],"-v"))) {
			std::cout << APP_NAME << std::endl;
			exit(0);
		}
		if (!(strcmp(argv[1],"--help") && strcmp(argv[1],"-h"))) {
			usage(argv);
			exit(0);
		}
	}
	if (argc > 1) {
		std::cout << APP_NAME << std::endl;
		std::cout << argv[0] << " don't use command line argument(s)"<< std::endl;
		std::cout << "Try `"<< argv[0] << " --help' for more information."<< std::endl;
		//exit(1);
	}
}

#ifdef LINUX
//! create a lock file to avoid lanching more instance of spacecrafter
static void create_lock_file(std::string lock_file)
{
	std::ofstream file(lock_file.c_str(), std::ios::out);
	file << getpid() << std::endl;
	file.close();
}

//! check if a lock file already exist
static bool is_lock_file(std::string lock_file)
{
	std::ifstream file(lock_file.c_str(), std::ios::in);
	if (!file ) {
		return false;
	} else {
		std::string ch;
		file >> ch;
		file.close();
		cLog::get()->write("Saved getpid is : "+ ch, LOG_TYPE::L_INFO);

		std::string test="kill -0 "+ch +"\n";

		return CallSystem::useSystemCommand(test);
	}
	return false; //for compiler
}
#endif


// Main procedure
int main(int argc, char **argv)
{
	// fix all repertory
	#if LINUX
	std::string homeDir = getenv("HOME");
	std::string appDir = homeDir + "/." + APP_LOWER_NAME+"/";
	std::string dataRoot = std::string(CONFIG_DATA_DIR);
	#else //win32
	std::string homeDir = "";
	std::string appDir = "";
	std::string dataRoot="";
	std::string CONFIG_DATA_DIR="";
	std::string LOCALEDIR="";
	#endif
	// Check the command line
	check_command_line(argc, argv);

	//check if home Directory exist and if not try to create it.
	std::string dirResult;
	CallSystem::checkUserDirectory(appDir, dirResult);
	CallSystem::checkUserSubDirectory(appDir, dirResult);

	//-------------------------------------------
	//create log system
	//-------------------------------------------
	cLog* Log = cLog::get();

	#ifdef LINUX
		Log->setDirectory(getenv("HOME") + std::string("/.") + APP_LOWER_NAME +"/log/");
	#else // on windows
		Log->setDirectory("log\\");
	#endif

	// Open log files
	Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
	Log->openLog(LOG_FILE::SCRIPT, "script", true);
	Log->openLog(LOG_FILE::TCP, "tcp");

	// Write the console logo & Uname Information...
	writeGeneralInfo();
	getUnameInfo();
	//Information about memory capacity
	Log->write(CallSystem::getRamInfo());
	Log->mark();

	// write in logfile the result of repertory checking
	Log->write(dirResult, LOG_TYPE::L_INFO);
	dirResult.clear();
	CallSystem::checkIniFiles(appDir, dataRoot);
	//-------------------------------------------
	// end
	//-------------------------------------------

	// if lock_file is created, check if it's valid and avoid launch SC twice.
	#ifdef LINUX
	std::string lock_file = PATH_FILE_LOCK;
	Log->write("Lock file is "+ lock_file,LOG_TYPE::L_INFO);
	Log->write("My getpid() is "+ Utility::doubleToString(getpid()), LOG_TYPE::L_INFO);

	if (is_lock_file(lock_file)) {
		Log->write("There already has an instance of the running program. New instance aborded", LOG_TYPE::L_WARNING);
		return 0;
	}
	create_lock_file(lock_file);
	#endif

	// Used for getting system date formatting
	setlocale(LC_TIME, "");

	//save the environnement variable
	Log->write("CONFIG DIR: " + std::string(appDir), LOG_TYPE::L_INFO);
	Log->write("ROOT   DIR: " + std::string(dataRoot), LOG_TYPE::L_INFO);
	Log->write("LOCALE DIR: " + std::string(LOCALEDIR), LOG_TYPE::L_INFO);

	//test if config.ini is not to old.
	CheckConfig* configUptodate =  new CheckConfig();
	configUptodate->checkConfigIni(appDir+"config.ini", std::string(VERSION));
	delete configUptodate;

	//create AppSettings and InitParser
	AppSettings::Init(appDir, dataRoot, LOCALEDIR);
	InitParser conf;
	AppSettings* ini = AppSettings::Instance();
	ini->loadAppSettings( &conf );

	Log->setDebug(conf.getBoolean(SCS_MAIN, SCK_DEBUG));

	#ifdef LINUX
	CPUInfo *cpuInfo =  nullptr;
	if (conf.getBoolean(SCS_MAIN,SCK_CPU_INFO)) {
		cpuInfo = new CPUInfo();
		cpuInfo -> init(ini->getLogDir()+"CPUlog.csv",ini->getLogDir()+"GPUlog.csv");
		cpuInfo -> start();
		Log->write("CPUInfo actived",LOG_TYPE::L_DEBUG);
	}
	#endif

	//-------------------------------------------
	// create the SDL windows for display
	//-------------------------------------------
	SDLFacade* sdl = new SDLFacade();
	sdl->initSDL();

	// détermination de la résolution initiale
	bool autoscreen = conf.getBoolean(SCS_VIDEO, SCK_AUTOSCREEN);
	Uint16 curW, curH;
	bool fullscreen;
	int antialiasing;

	if (autoscreen) {
		SDL_DisplayMode dm;
		if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
			SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		curW = dm.w;
		curH = dm.h;
		fullscreen = true;
	} else {
		curW = conf.getInt(SCS_VIDEO, SCK_SCREEN_W);
		curH = conf.getInt(SCS_VIDEO, SCK_SCREEN_H);
		fullscreen = conf.getBoolean(SCS_VIDEO, SCK_FULLSCREEN);
	}
	antialiasing = conf.getInt(SCS_RENDERING, SCK_ANTIALIASING);
	sdl->createWindow(curW, curH, conf.getInt(SCS_VIDEO, SCK_BBP_MODE), antialiasing, fullscreen, dataRoot + "data/icon.bmp"); //, conf.getBoolean("main:debug_opengl"));

	//-------------------------------------------
	// create the main class for SC logical software
	//-------------------------------------------
	App* app = new App( sdl );

	// Register custom suspend and term signal handers
	ISignals* signalObj = ISignals::Create(app);
	signalObj->Register( SIGTSTP, ISignals::NSSigTSTP );
	signalObj->Register( SIGTERM, ISignals::NSSigTERM );
	signalObj->Register( SIGINT, ISignals::NSSigTERM );
	signalObj->Register( SIGQUIT, ISignals::NSSigTERM );

	// SC logical software start here
	app->firstInit();
	app->startMainLoop();
	//SC logical software end here

	// Close all
	#ifdef LINUX
	// remove correctly the lock file
	if( unlink(lock_file.c_str()) != 0 )
		Log->write("Error deleting file.lock",  LOG_TYPE::L_ERROR);
	else
		Log->write("File file.lock successfully deleted",  LOG_TYPE::L_INFO);
	#endif
	// close cpu information
	#ifdef LINUX
	if (cpuInfo != nullptr) {
		cpuInfo->stop();
	}
	#endif

	delete app;
	delete sdl;
	delete signalObj;

	// Log->write("EOF", LOG_TYPE::L_INFO);
	// Log->write("EOF", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
	Log->close();
	AppSettings::close();

	return 0;
}
