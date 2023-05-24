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
#include <vector>
#include <SDL2/SDL.h>
#include <memory>
#include <filesystem>
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
#include "EntityCore/Core/VulkanMgr.hpp"
#include "tools/s_texture.hpp"
#include "mainModule/CPUInfo.hpp"
#include "EntityCore/Tools/LinuxExecutor.hpp"
#include "EntityCore/Executor/AsyncLoaderMgr.hpp"

#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
// Maybe not recommended, but it doesn't compile otherwise
#undef main
#endif

#define GETV(offset) ((VERSION[offset] - '0') * 10 + VERSION[offset + 1] - '0')

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
	#ifdef __linux__
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
static void usage(const char **argv)
{
	std::cout << APP_NAME << std::endl;
	std::cout << _("Usage: %s [OPTION] ...\n -v, --version \tOutput version information and exit.\n -h, --help \tDisplay this help and exit.\n");
}

// Check command line arguments
static void check_command_line(int argc, const char **argv)
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

//! create a lock file to avoid lanching more instance of spacecrafter
static void create_lock_file(std::string lock_file)
{
	std::ofstream file(lock_file, std::ios::out);
	file << getpid() << std::endl;
	file.close();
}

//! check if a lock file already exist
static bool is_lock_file(std::string lock_file)
{
	std::ifstream file(lock_file, std::ios::in);
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

static void remove_lock_file(std::string lock_file)
{
	std::error_code ec;
	if (std::filesystem::remove(lock_file, ec) && !ec)
		cLog::get()->write("File file.lock successfully deleted",  LOG_TYPE::L_INFO);
	else
		cLog::get()->write("Error deleting file.lock",  LOG_TYPE::L_ERROR);
}

// Main procedure
int main(int argc, const char *argv[])
{
	// fix all repertory
	#ifdef __linux__
	const std::string homeDir = getenv("HOME");
	#else
	const std::string homeDir = getenv("USERPROFILE");
	#endif
	const std::string appDir = homeDir + "/." + APP_LOWER_NAME+"/";
	const std::string dataRoot = std::string(CONFIG_DATA_DIR);
	// Check the command line
	check_command_line(argc, argv);

	LinuxExecutor executor(0, 0);
	executor.start(false);

	//check if home Directory exist and if not try to create it.
	std::string dirResult;
	CallSystem::checkUserDirectory(appDir, dirResult);
	CallSystem::checkUserSubDirectory(appDir, dirResult);

	//-------------------------------------------
	//create log system
	//-------------------------------------------
	cLog* Log = cLog::get();

	Log->setDirectory(appDir + "log/");

	// Open log files
	Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
	Log->openLog(LOG_FILE::SCRIPT, "script", true);
	Log->openLog(LOG_FILE::TCP, "tcp");
	Log->openLog(LOG_FILE::SHADER,"shader");
	Log->openLog(LOG_FILE::VULKAN,"vulkan");

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
	std::string lock_file = (std::filesystem::temp_directory_path()/PATH_FILE_LOCK).string();
	Log->write("Lock file is "+ lock_file,LOG_TYPE::L_INFO);
	Log->write("My getpid() is "+ std::to_string(getpid()), LOG_TYPE::L_INFO);

	if (is_lock_file(lock_file)) {
		Log->write("There already has an instance of the running program. New instance aborded", LOG_TYPE::L_WARNING);
		return 0;
	}
	create_lock_file(lock_file);

	// Used for getting system date formatting
	setlocale(LC_TIME, "");

	//save the environnement variable
	Log->write("CONFIG DIR: " + std::string(appDir), LOG_TYPE::L_INFO);
	Log->write("ROOT   DIR: " + std::string(dataRoot), LOG_TYPE::L_INFO);
	Log->write("LOCALE DIR: " + std::string(LOCALEDIR), LOG_TYPE::L_INFO);

	//test if config.ini is not to old.
	{
		std::unique_ptr<CheckConfig> configUptodate =  std::make_unique<CheckConfig>();
	configUptodate->checkConfigIni(appDir+"config.ini", std::string(VERSION));
		//delete configUptodate;
	}

	//create AppSettings and InitParser
	AppSettings::Init(appDir, dataRoot, LOCALEDIR);
	InitParser conf;
	AppSettings* ini = AppSettings::Instance();
	ini->loadAppSettings( &conf );

	Log->setDebug(conf.getBoolean(SCS_MAIN, SCK_DEBUG));
	Log->setWriteLog(conf.getBoolean(SCS_MAIN, SCK_LOG));

	std::unique_ptr<CPUInfo> cpuInfo =  nullptr;
	if (conf.getBoolean(SCS_MAIN,SCK_CPU_INFO)) {
		cpuInfo = std::make_unique<CPUInfo>();
		cpuInfo -> init(ini->getLogDir()+"CPUlog.csv",ini->getLogDir()+"GPUlog.csv");
		cpuInfo -> start();
		Log->write("CPUInfo actived",LOG_TYPE::L_DEBUG);
	}

	//-------------------------------------------
	// create the SDL windows for display
	//-------------------------------------------
	std::unique_ptr<SDLFacade> sdl = std::make_unique<SDLFacade>();
	sdl->initSDL();

	// determination of the initial resolution
	bool autoscreen = conf.getBoolean(SCS_VIDEO, SCK_AUTOSCREEN);
	bool remote_display = conf.getBoolean(SCS_VIDEO, SCK_REMOTE_DISPLAY);
	bool keep_empty_window = conf.getBoolean(SCS_VIDEO, SCK_KEEP_EMPTY_WINDOW);
	Uint16 curW, curH;
	bool fullscreen;
	//int antialiasing;

	if (autoscreen && !remote_display) {
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
	// Only use if we want a window, otherwise...
	if (!remote_display)
		sdl->createWindow(APP_NAME, curW, curH, fullscreen, dataRoot + "data/icon.bmp");
	else if (keep_empty_window)
		sdl->createEmptyWindow(APP_NAME, curW, curH);

	//-------------------------------------------
	// create the main class for SC logical software
	//-------------------------------------------
	auto curMin = std::min(curW, curH);
	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphore {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR, nullptr, VK_TRUE};
	// For windowless usage (like NDI), don't create sdl window, sdl->getWindow() must then return nullptr.
	VulkanMgrCreateInfo vkmgrInfo {.AppName=APP_LOWER_NAME, .appVersion=VK_MAKE_API_VERSION(0, GETV(0), GETV(3), GETV(6)),
		.window=sdl->getWindow(), .vulkanVersion=VK_API_VERSION_1_1, .width=curMin, .height=-curMin, .queueRequest={2, 0, 0, 1, 1},
		.requiredExtensions={"VK_KHR_timeline_semaphore"},
		.redirectLog=cLog::writeECLog, .cachePath=ini->getUserDir()+"cache/", .logPath=appDir+"log/",
		.swapchainUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, .chunkSize=256, .forceSwapchainCount=3,
		.enableDebugLayers=conf.getBoolean(SCS_MAIN, SCK_DEBUG_LAYER), .drawLogs=conf.getBoolean(SCS_MAIN, SCK_DEBUG),
		.saveLogs=conf.getBoolean(SCS_MAIN, SCK_LOG), .preserveCrashLogs = true,
		.preferIntegrated=false, .allowOverrides=true, .customReleaseMemory=&s_texture::releaseUnusedMemory
	};
	vkmgrInfo.requiredFeatures.features.geometryShader = VK_TRUE;
	vkmgrInfo.requiredFeatures.features.tessellationShader = VK_TRUE;
	vkmgrInfo.requiredFeatures.features.sampleRateShading = VK_TRUE;
	vkmgrInfo.requiredFeatures.features.wideLines = VK_TRUE;
	vkmgrInfo.preferedFeatures.features.shaderFloat64 = VK_TRUE;
	vkmgrInfo.preferedFeatures.features.samplerAnisotropy = VK_TRUE;
	vkmgrInfo.preferedFeatures.pNext = &timelineSemaphore;

	AsyncLoaderMgr loader(appDir, ini->getUserDir()+"cache/");
	loader.minPriority = LoadPriority::LOADING;
	loader.startBuilders(conf.getInt(SCS_MAIN, SCK_BUILDER_THREADS));
	std::unique_ptr<VulkanMgr> vulkan = std::make_unique<VulkanMgr>(vkmgrInfo);
	std::unique_ptr<App> app = std::make_unique<App>(sdl.get());

	// Register custom suspend and term signal handers
	ISignals* signalObj = ISignals::Create(app.get());
	signalObj->Register( SIGTSTP, ISignals::NSSigTSTP );
	signalObj->Register( SIGTERM, ISignals::NSSigTERM );
	signalObj->Register( SIGINT, ISignals::NSSigTERM );
	signalObj->Register( SIGQUIT, ISignals::NSSigTERM );

	// SC logical software start here
	app->firstInit();
	loader.startLoader();
	app->startMainLoop();
	loader.stop();
	//SC logical software end here

	// Close all
	executor.close();
	remove_lock_file(lock_file);
	// close cpu information
	if (cpuInfo != nullptr) {
		cpuInfo->stop();
	}

	app.reset();
	delete signalObj;

	// Log->write("EOF", LOG_TYPE::L_INFO);
	// Log->write("EOF", LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
	s_texture::forceUnload();
	vulkan.reset();
	Log->close();
	AppSettings::close();

	int timeout = 30; // Let up to 3 seconds for the LinuxExecutor to close
	while (!executor.closed() && timeout--)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	return 0;
}
