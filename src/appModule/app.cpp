/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2017 of the LSS Team & Association Sirius
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

#include <exception>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sstream>

#include "spacecrafter.hpp"
#include "appModule/app.hpp"
#include "appModule/appDraw.hpp"
#include "appModule/save_screen_interface.hpp"
#include "appModule/screenFader.hpp"
#include "appModule/mkfifo.hpp"
#include "coreModule/callbacks.hpp"
#include "coreModule/core.hpp"
#include "eventModule/event_handler.hpp"
#include "eventModule/event_manager.hpp"
#include "interfaceModule/app_command_interface.hpp"
#include "interfaceModule/script_interface.hpp"
#include "interfaceModule/script_mgr.hpp"
#include "mainModule/sdl_facade.hpp"
#include "mediaModule/media.hpp"
#include "tools/call_system.hpp"
#include "tools/io.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include "uiModule/ui.hpp"

#include "eventModule/EventScriptHandler.hpp"
#include "eventModule/EventCommandHandler.hpp"
#include "eventModule/EventFlagHandler.hpp"
#include "eventModule/EventScreenFaderHandler.hpp"
#include "eventModule/EventSaveScreenHandler.hpp"
#include "eventModule/EventFpsHandler.hpp"

EventManager* EventManager::instance = nullptr;

App::App( SDLFacade* const sdl ) :
	initialized(false)
{
	mSdl = sdl;
	flagMasterput =false;
	mSdl->getResolution( &width, &height );

	settings = AppSettings::Instance();

	media = new Media( width, height );
	saveScreenInterface = new SaveScreenInterface(width, height);
	saveScreenInterface->setVideoBaseName(settings->getVframeDirectory() + APP_LOWER_NAME);
	saveScreenInterface->setSnapBaseName(settings->getScreenshotDirectory() + APP_LOWER_NAME);

	screenFader =  new ScreenFader();

	core = new Core(settings, width, height, media, mBoost::callback<void, std::string>(this, &App::recordCommand));

	screenFader->initShader();

	ui = new UI(core, this, mSdl, media);
	commander = new AppCommandInterface(core, this, ui, media);
	scriptMgr = new ScriptMgr(commander, settings->getUserDir(), media);
	scriptInterface = new ScriptInterface(scriptMgr);
	internalFPS = new Fps();
	spaceDate = new SpaceDate();

	// fixation interface 
	ui->initScriptInterface(scriptInterface);
	ui->initSpaceDateInterface(spaceDate);
	commander->initScriptInterface(scriptInterface);
	commander->initSpaceDateInterface(spaceDate);
	commander->initSaveScreenInterface(saveScreenInterface);

	EventManager::Init();
	eventManager = EventManager::getInstance();
	eventHandler = new EventHandler(eventManager);
	eventHandler-> add(new EventScriptHandler(scriptMgr), Event::E_SCRIPT);
	eventHandler-> add(new EventCommandHandler(commander), Event::E_COMMAND);
	eventHandler-> add(new EventFlagHandler(commander), Event::E_FLAG);
	eventHandler-> add(new EventScreenFaderHandler(screenFader), Event::E_SCREEN_FADER);
	eventHandler-> add(new EventScreenFaderInterludeHandler(screenFader), Event::E_SCREEN_FADER_INTERLUDE);
	eventHandler-> add(new EventSaveScreenHandler(saveScreenInterface), Event::E_SAVESCREEN);
	eventHandler-> add(new EventFpsHandler(internalFPS), Event::E_FPS);

	#if LINUX
	mkfifo= new Mkfifo();
	#endif

	enable_mkfifo= false;
	enable_tcp= false;
	flagColorInverse= false;
	flagOnVideo = false;

	appDraw = new AppDraw();
	appDraw->init(width, height);
}

App::~App()
{
	eventHandler->remove(Event::E_FPS);
	eventHandler->remove(Event::E_SAVESCREEN);
	eventHandler->remove(Event::E_SCREEN_FADER_INTERLUDE);
	eventHandler->remove(Event::E_SCREEN_FADER);
	eventHandler->remove(Event::E_FLAG);
	eventHandler->remove(Event::E_COMMAND);
	eventHandler->remove(Event::E_SCRIPT);

	EventManager::End();

	delete appDraw;
	if (enable_tcp)
		delete tcp;
	#if LINUX
	delete mkfifo;
	#endif
	delete ui;
	delete scriptInterface;
	delete scriptMgr;
	delete media;
	delete commander;
	delete core;
	delete saveScreenInterface;
	delete internalFPS;
	delete screenFader;
	delete spaceDate;
}


void App::flag(APP_FLAG layerValue, bool _value) {
	switch(layerValue) {
		case APP_FLAG::VISIBLE : 
						flagVisible = _value; break;
		case APP_FLAG::ALIVE :
						flagAlive = _value; break;
		case APP_FLAG::ON_VIDEO :
						flagOnVideo = _value; break;
		case APP_FLAG::COLOR_INVERSE : 
						flagColorInverse = _value; break;
		default: break;
	}
}

void App::toggle(APP_FLAG layerValue)
{
		switch(layerValue) {
		case APP_FLAG::VISIBLE : 
						flagVisible = !flagVisible; break;
		case APP_FLAG::ALIVE :
						flagAlive = !flagAlive; break;
		case APP_FLAG::ON_VIDEO :
						flagOnVideo = !flagOnVideo; break;
		case APP_FLAG::COLOR_INVERSE : 
						flagColorInverse = !flagColorInverse; break;
		default: break;
	}
}

// void App::warpMouseInWindow(float x, float y) {
// 	mSdl->warpMouseInWindow( x , y);
// }

std::string App::getAppLanguage() {
	return Translator::globalTranslator.getLocaleName();
}


//! Load configuration from disk
void App::init()
{
	// Clear screen, this fixes a strange artifact at loading time in the upper top corner.
	glClear(GL_COLOR_BUFFER_BIT);

	if (!initialized) {
		appDraw->initSplash();
		mSdl->glSwapWindow();	// And swap the buffers
		Translator::initSystemLanguage();
	}

	// Initialize video device and other sdl parameters
	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	internalFPS->setMaxFps(conf.getDouble ("video","maximum_fps"));
	internalFPS->setScriptFps(conf.getDouble ("video","script_fps"));

	std::string appLocaleName = conf.getStr("localization", "app_locale"); //, "system");
	spaceDate->setTimeFormat(spaceDate->stringToSTimeFormat(conf.getStr("localization:time_display_format")));
	spaceDate->setDateFormat(spaceDate->stringToSDateFormat(conf.getStr("localization:date_display_format")));
	setAppLanguage(appLocaleName);

	// time_zone used to be in init_location section of config, so use that as fallback when reading config - Rob
	std::string tzstr = conf.getStr("localization", "time_zone");
	#if LINUX
	if (tzstr == "system_default") {
		spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT);
		// Set the program global intern timezones variables from the system locale
		tzset();
	} else {
	#endif
		if (tzstr == "gmt+x") {
			spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT);
		} else {
			// We have a custom time zone name
			spaceDate->setTimeZoneMode(SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT);
			spaceDate->setCustomTzName(tzstr);
		}
	#if LINUX
	}
	#endif

	core->init(conf);

	// Navigation section
	PresetSkyTime 		= conf.getDouble ("navigation","preset_sky_time"); //,2451545.);
	StartupTimeMode 	= conf.getStr("navigation:startup_time_mode");	// Can be "now" or "preset"
	DayKeyMode 			= conf.getStr("navigation","day_key_mode"); //,"calendar");  // calendar or sidereal
	cLog::get()->write("Read daykeymode as <" + DayKeyMode + ">", LOG_TYPE::L_INFO);

	if (StartupTimeMode=="preset" || StartupTimeMode=="Preset")
		core->setJDay(PresetSkyTime - spaceDate->getGMTShift(PresetSkyTime) * JD_HOUR);
	else core->setTimeNow();

	// initialisation of the User Interface
	ui->init(conf);
	ui->localizeTui();
	if (!initialized) 
		ui->initTui(); // don't reinit tui since probably called from there
	else
		ui->localizeTui();  // update translations/fonts as needed
	//set all color
	core->setColorScheme(settings->getConfigFile(), "color");
	core->setFontScheme();

	if (! initialized) {

		appDraw->createShader();
		bool mplayerEnable =conf.getBoolean("io:enable_mplayer");
		std::string mplayerFileName =conf.getStr("io:mplayer_name");
		std::string mplayerMkfifoName = conf.getStr("io:mplayer_mkfifo_name");
		media->externalInit(mplayerFileName, mplayerMkfifoName,mplayerEnable);
		media->createViewPort();
		media->createVR360();
		media->createImageShader();

		flagMasterput=conf.getBoolean("main:flag_masterput");
		enable_tcp=conf.getBoolean("io","enable_tcp");
		enable_mkfifo=conf.getBoolean("io","enable_mkfifo");
		flagAlwaysVisible = conf.getBoolean("main:flag_always_visible");

		// std::stringstream oss;
		// cLog::get()->write(oss.str(), LOG_TYPE::L_INFO);

		if (enable_tcp) {
			int port = conf.getInt("io:tcp_port_in");
			int buffer_in_size=conf.getInt("io:tcp_buffer_in_size");
			cLog::get()->write("buffer TCP taille " + Utility::intToString(buffer_in_size));
			tcp = new ServerSocket(port, 16, buffer_in_size, IO_DEBUG_INFO, IO_DEBUG_ALL);
			tcp->open();
			core->tcpConfigure(tcp);
		}
		#if LINUX // special mkfifo
		if (enable_mkfifo) {
			std::string mplayerMkfifoName = conf.getStr("io:mplayer_mkfifo_name");
			std::string mkfifo_file_in = conf.getStr("io:mkfifo_file_in");
			int buffer_in_size=conf.getInt("io:mkfifo_buffer_in_size");
			cLog::get()->write("buffer MKFIFO taille "+ Utility::intToString(buffer_in_size));
			mkfifo->init(mkfifo_file_in, buffer_in_size);
		}
		#endif

		cLog::get()->write(CallSystem::getRamInfo());
		cLog::get()->mark();
		cLog::get()->write("End of loading SC");
		cLog::get()->write("End of loading SC",LOG_TYPE::L_INFO,LOG_FILE::SCRIPT);
		cLog::get()->mark();
	}
	// play startup script
	scriptMgr->playStartupScript();
	initialized = true;
}

void App::updateFromSharedData()
{
	if (enable_mkfifo) {
		std::string out;
		#if LINUX
		if (mkfifo->update(out)) {
			cLog::get()->write("get mkfifo: " + out);
			commander->executeCommand(out);
		}
		#endif
	}
	if (enable_tcp) {
		std::string out;
		do {
			out = tcp->getInput();
			if (!out.empty()) {
				cLog::get()->write("get tcp : " + out);
				commander->executeCommand(out);
			}
		} while (!out.empty());
	}
	if (flagMasterput==true) 
		masterput();
}


// todo deprecated 
void App::executeCommand(const std::string& _command) {
	commander->executeCommand(_command);
}

void App::update(int delta_time)
{
	// std::cout << "delta_time " << delta_time << std::endl;
	internalFPS->addFrame();
	internalFPS->addCalculatedTime(delta_time);

	// change time rate if needed to fast forward scripts
	delta_time *= core->timeGetMultiplier();
	// run command from a running script
	scriptMgr->update(delta_time);
	if (!scriptMgr->isPaused() || !scriptMgr->isFaster() )	media->audioUpdate(delta_time);
	// run any incoming command from shared memory interface
	updateFromSharedData();

	ui->updateTimeouts(delta_time);
	ui->tuiUpdateWidgets();

	if (!scriptMgr->isPaused()) media->imageUpdate(delta_time);

	media->playerUpdate();
	media->faderUpdate(delta_time);

	core->updateMode();
	core->update(delta_time);
}


//! Main drawinf function called at each frame
void App::draw(int delta_time)
{
	//draw the first layer que si le mode starsTrace n'est pas actif
	appDraw->drawFirstLayer();

	core->draw(delta_time);

	// Draw the Graphical ui and the Text ui
	ui->draw();
	//inversion des couleurs pour un ciel blanc
	if (flagColorInverse)
		appDraw->drawColorInverse();

	//draw video frame to classical viewport
	media->drawViewPort();

	// Fill with black around the circle
	appDraw->drawViewportShape();

	screenFader->draw();
}

//! @brief Set the application locale. This apply to GUI, console messages etc..
void App::setAppLanguage(const std::string& newAppLocaleName)
{
	// Update the translator with new locale name
	Translator::globalTranslator = Translator(PACKAGE, settings->getLocaleDir(), newAppLocaleName);
	cLog::get()->write("Application locale is " + Translator::globalTranslator.getLocaleName(), LOG_TYPE::L_INFO);
	if (initialized)
		ui->localizeTui();
}

//! For use by TUI - saves all current settings
void App::saveCurrentConfig(const std::string& confFile)
{
	// No longer resaves everything, just settings user can change through UI
	cLog::get()->write("Saving configuration file " + confFile + " ...", LOG_TYPE::L_INFO);
	InitParser conf;
	conf.load(confFile);

	// Main section
	conf.setStr	("main:version", std::string(VERSION));
	// localization section
	conf.setStr("localization:app_locale", getAppLanguage());
	conf.setStr("localization:time_display_format", spaceDate->getTimeFormatStr());
	conf.setStr("localization:date_display_format", spaceDate->getDateFormatStr());
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_CUSTOM) {
		conf.setStr("localization:time_zone", spaceDate->getCustomTzName());
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT) {
		conf.setStr("localization:time_zone", "system_default");
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT) {
		conf.setStr("localization:time_zone", "gmt+x");
	}
	conf.setDouble ("navigation:preset_sky_time", PresetSkyTime);
	conf.setStr	("navigation:startup_time_mode", StartupTimeMode);
	conf.setStr	("navigation:day_key_mode", DayKeyMode);

	ui->saveCurrentConfig(conf);
	core->saveCurrentConfig(conf);

	// Get landscape and other observatory info
	(core->getObservatory())->setConf(conf, "init_location");
	conf.save(confFile);
}

void App::recordCommand(const std::string& commandline)
{
	scriptMgr->recordCommand(commandline);
}

//! Masterput script launch
void App::masterput()
{
	std::string action = settings->getFtpDir()+"pub/masterput.launch";
	FILE * tempFile = fopen(action.c_str(),"r");
	if (tempFile) {
		fclose(tempFile);
		cLog::get()->write("MASTERPUT is in action", LOG_TYPE::L_INFO);
		unlink(action.c_str());
		scriptMgr->playScript(settings->getFtpDir()+"pub/script.sts");
	}
}

void App::start_main_loop()
{
	flagVisible = true;		// At The Beginning, Our App Is Visible
	flagAlive = true; 		// au debut, on veut que l'application ne s'arrete pas :)

	//center mouse in middle screen
	mSdl->warpMouseInCenter();

	internalFPS->init();
	internalFPS->fixMaxFps();

	SDL_TimerID my_timer_id = SDL_AddTimer(1000, internalFPS->callbackfunc, nullptr);

	// Start the main loop
	while (flagAlive) {
		if (flagOnVideo != media->playerGetAlive()) {
			if (media->playerGetAlive() == false) {
				//std::cout << "vidéo arretée" << std::endl;
				media->playerStop();
				ui->flag(UI_FLAG::HANDLE_KEY_ONVIDEO, false);
			} else {
				ui->flag(UI_FLAG::HANDLE_KEY_ONVIDEO, true);
				//std::cout << "vidéo lancée" << std::endl;
			}

			flagOnVideo = !flagOnVideo;
		}

		while (SDL_PollEvent(&E)) {	// Fetch all Event Of The Queue
			ui->handleInputs(E);
		}

		//analyse le joystick au cas ou des events ont été accumulés pour le joystick
		ui->handleDeal();

		// on applique toutes les modifications faites dans ui etc
		eventHandler->handleEvents();

		// If the application is not visible
		if (!flagVisible && !flagAlwaysVisible) {
			// Leave the CPU alone, don't waste time, simply wait for an event
			SDL_WaitEvent(NULL);
		} else {
			internalFPS->setTickCount();
			// Wait a while if drawing a frame right now would exceed our preferred framerate.
			internalFPS->wait();
			internalFPS->setTickCount();

			deltaTime = internalFPS->getDeltaTime();

			// UNCOMMENT IF SAVE30FPS
			//if (deltaTime > internalFPS->getFrameDuration()) deltaTime = internalFPS->getFrameDuration();

			this->update(deltaTime);		// And update the motions and data
			this->draw(deltaTime);			// Do the drawings!
			mSdl->glSwapWindow();  			// And swap the buffers

			internalFPS->setLastCount();

			saveScreenInterface->readScreenShot();
		}
	}

	SDL_RemoveTimer(my_timer_id);
	CallSystem::killAllPidFromVLC();
}
