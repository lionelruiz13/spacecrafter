/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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
#include "appModule/fps.hpp"
#include "tools/app_settings.hpp"
#include "appModule/save_screen_interface.hpp"
#include "appModule/space_date.hpp"
#include "appModule/screenFader.hpp"
#include "appModule/fontFactory.hpp"
#include "appModule/mkfifo.hpp"
#include "coreModule/callbacks.hpp"
#include "coreModule/core.hpp"
#include "coreModule/coreLink.hpp"
#include "executorModule/executor.hpp"
#include "eventModule/event_handler.hpp"
#include "eventModule/event_recorder.hpp"
#include "interfaceModule/app_command_interface.hpp"
#include "scriptModule/script_interface.hpp"
#include "scriptModule/script_mgr.hpp"
#include "mainModule/sdl_facade.hpp"
#include "mediaModule/media.hpp"
#include "tools/call_system.hpp"
#include "tools/io.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include "uiModule/ui.hpp"
#include "coreModule/time_mgr.hpp"

#include "eventModule/EventScriptHandler.hpp"
#include "eventModule/AppCommandHandler.hpp"
#include "eventModule/EventScreenFaderHandler.hpp"
#include "eventModule/EventSaveScreenHandler.hpp"
#include "eventModule/EventFpsHandler.hpp"
#include "eventModule/EventVideoHandler.hpp"
#include "eventModule/CoreHandler.hpp"

#include "vulkanModule/Vulkan.hpp"
#include "vulkanModule/VirtualSurface.hpp"
#include "vulkanModule/TextureMgr.hpp"
#include "vulkanModule/SetMgr.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/ResourceTracker.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/ThreadedCommandBuilder.hpp"

EventRecorder* EventRecorder::instance = nullptr;

App::App( SDLFacade* const sdl )
{
	mSdl = sdl;
	flagMasterput =false;
	mSdl->getResolution( &width, &height );

	settings = AppSettings::Instance();
	InitParser conf;
	settings->loadAppSettings( &conf );
	Pipeline::setDefaultLineWidth(conf.getDouble(SCS_RENDERING, SCK_LINE_WIDTH));

	int antialiasing = 1 << static_cast<int>(std::log2(conf.getInt(SCS_RENDERING, SCK_ANTIALIASING)|1));
	globalContext.vulkan = new Vulkan(APP_LOWER_NAME, nullptr, mSdl->getWindow(), 1, width, height, 256*1024*1024, cLog::get()->getDebug(), static_cast<VkSampleCountFlagBits>(antialiasing), settings->getUserDir());
	globalContext.tracker = new ResourceTracker();
	globalContext.textureMgr = new TextureMgr(globalContext.vulkan);
	context.global = &globalContext;
	context.surface = globalContext.vulkan->getVirtualSurface();
	context.setMgr = new SetMgr(context.surface, 512);
	context.commandMgr = new CommandMgr(context.surface, 64, true);
	context.commandMgrSingleUse = new CommandMgr(context.surface, 15, true, true, true);
	context.commandMgrSingleUseInterface = new ThreadedCommandBuilder(context.commandMgrSingleUse);
	context.commandMgrDynamic = new CommandMgr(context.surface, 8, true, false, true, true);
	commandIndexClear = context.commandMgr->getCommandIndex();
	context.commandMgr->init(commandIndexClear);
	context.commandMgr->beginRenderPass(renderPassType::CLEAR);
	context.commandMgr->compile();
	s_texture::setContext(&context);
	*getContext() = context;

	fontFactory = std::make_unique<FontFactory>();

	media = std::make_shared<Media>();
	saveScreenInterface = std::make_shared<SaveScreenInterface>(width, height, globalContext.vulkan);
	saveScreenInterface->setVideoBaseName(settings->getVframeDirectory() + APP_LOWER_NAME);
	saveScreenInterface->setSnapBaseName(settings->getScreenshotDirectory() + APP_LOWER_NAME);

	screenFader =  std::make_unique<ScreenFader>();

	observatory = std::make_shared<Observer>();
	core = std::make_shared<Core>(&context, width, height, media, fontFactory, mBoost::callback<void, std::string>(this, &App::recordCommand), observatory);
	coreLink = std::make_unique<CoreLink>(core);
	coreBackup = std::make_unique<CoreBackup>(core);

	screenFader->createSC_context(&context);

	ui = std::make_shared<UI>(core, coreLink.get(), this, mSdl, media);
	commander = std::make_shared<AppCommandInterface>(core, coreLink, coreBackup, std::shared_ptr<App>(this), ui, media, fontFactory);
	scriptMgr = std::make_shared<ScriptMgr>(commander, settings->getUserDir(), media);
	scriptInterface = std::make_shared<ScriptInterface>(scriptMgr);
	internalFPS = std::make_unique<Fps>();
	spaceDate = std::make_shared<SpaceDate>();

	executor = std::make_unique<Executor>(core, observatory.get());

	// fixation interface
	ui->initInterfaces(scriptInterface,spaceDate);
	commander->initInterfaces(scriptInterface, spaceDate, saveScreenInterface);

	EventRecorder::Init();
	eventRecorder = EventRecorder::getInstance();
	eventHandler = new EventHandler(eventRecorder);
	eventHandler-> add(new EventScriptHandler(scriptInterface.get()), Event::E_SCRIPT);
	eventHandler-> add(new EventCommandHandler(commander.get()), Event::E_COMMAND);
	eventHandler-> add(new EventFlagHandler(commander.get()), Event::E_FLAG);
	eventHandler-> add(new EventScreenFaderHandler(screenFader.get()), Event::E_SCREEN_FADER);
	eventHandler-> add(new EventScreenFaderInterludeHandler(screenFader.get()), Event::E_SCREEN_FADER_INTERLUDE);
	eventHandler-> add(new EventSaveScreenHandler(saveScreenInterface.get()), Event::E_SAVESCREEN);
	eventHandler-> add(new EventFpsHandler(internalFPS.get()), Event::E_FPS);
	eventHandler-> add(new EventAltitudeHandler(core), Event::E_CHANGE_ALTITUDE);
	eventHandler-> add(new EventObserverHandler(core), Event::E_CHANGE_OBSERVER);
	eventHandler-> add(new EventVideoHandler(ui.get(), scriptInterface.get()), Event::E_VIDEO);

	#if LINUX
	mkfifo = std::make_unique<Mkfifo>();
	#endif

	enable_mkfifo= false;
	enable_tcp= false;
	flagColorInverse= false;

	appDraw = std::make_unique<AppDraw>();
	appDraw->init(width, height);
}

App::~App()
{
	eventHandler->remove(Event::E_VIDEO);
	eventHandler->remove(Event::E_CHANGE_OBSERVER);
	eventHandler->remove(Event::E_CHANGE_ALTITUDE);
	eventHandler->remove(Event::E_FPS);
	eventHandler->remove(Event::E_SAVESCREEN);
	eventHandler->remove(Event::E_SCREEN_FADER_INTERLUDE);
	eventHandler->remove(Event::E_SCREEN_FADER);
	eventHandler->remove(Event::E_FLAG);
	eventHandler->remove(Event::E_COMMAND);
	eventHandler->remove(Event::E_SCRIPT);

	EventRecorder::End();

	context.commandMgrSingleUseInterface->terminate();
	context.commandMgrSingleUseInterface->waitIdle();
	globalContext.vulkan->waitIdle();
	appDraw.reset();
	if (enable_tcp)
		tcp.reset();
	#if LINUX
		mkfifo.reset();
	#endif
	ui.reset();
	scriptInterface.reset();
	scriptMgr.reset();
	media.reset();
	commander.reset();
	coreLink.reset();
	coreBackup.reset();
	core.reset();
	observatory.reset();
	saveScreenInterface.reset();
	internalFPS.reset();
	screenFader.reset();
	spaceDate.reset();
	fontFactory.reset();
	delete context.commandMgr;
	delete context.commandMgrSingleUseInterface;
	delete context.commandMgrSingleUse;
	delete context.commandMgrDynamic;
	delete globalContext.tracker;
	delete context.setMgr;
	s_texture::forceUnload();
	delete globalContext.textureMgr;
	delete globalContext.vulkan;
}
 
int App::getFpsClock() const {
 	return internalFPS->getFps();
}

void App::setLineWidth(float w) const {
	appDraw->setLineWidth(w);
}

float App::getLineWidth() const {
	return appDraw->getLineWidth();
}

float App::getFlagAntialiasLines() const{
	return appDraw->getFlagAntialiasLines();
}

void App::flag(APP_FLAG layerValue, bool _value) {
	switch(layerValue) {
		case APP_FLAG::VISIBLE :
				flagVisible = _value; break;
		case APP_FLAG::ALIVE :
				flagAlive = _value; break;
		case APP_FLAG::COLOR_INVERSE :
				flagColorInverse = _value; break;
		case APP_FLAG::ANTIALIAS :
				appDraw->setFlagAntialiasLines(_value); break;
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
		case APP_FLAG::COLOR_INVERSE :
				flagColorInverse = !flagColorInverse; break;
		case APP_FLAG::ANTIALIAS :
				appDraw->flipFlagAntialiasLines(); break;
		default: break;
	}
}


std::string App::getAppLanguage() {
	return Translator::globalTranslator.getLocaleName();
}


//! Load configuration from disk
void App::init()
{
	// Initialize video device and other sdl parameters
	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	appDraw->setFlagAntialiasLines(conf.getBoolean(SCS_RENDERING, SCK_FLAG_ANTIALIAS_LINES));

	internalFPS->setMaxFps(conf.getDouble (SCS_VIDEO,SCK_MAXIMUM_FPS));
	internalFPS->setVideoFps(conf.getDouble(SCS_VIDEO,SCK_REC_VIDEO_FPS));

	std::string appLocaleName = conf.getStr(SCS_LOCALIZATION, SCK_APP_LOCALE); //, "system");
	spaceDate->setTimeFormat(spaceDate->stringToSTimeFormat(conf.getStr(SCS_LOCALIZATION, SCK_TIME_DISPLAY_FORMAT)));
	spaceDate->setDateFormat(spaceDate->stringToSDateFormat(conf.getStr(SCS_LOCALIZATION, SCK_DATE_DISPLAY_FORMAT)));
	setAppLanguage(appLocaleName);

	// time_zone used to be in init_location section of config, so use that as fallback when reading config - Rob
	std::string tzstr = conf.getStr(SCS_LOCALIZATION, SCK_TIME_ZONE);
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
	PresetSkyTime 		= conf.getDouble (SCS_NAVIGATION, SCK_PRESET_SKY_TIME); //,2451545.);
	StartupTimeMode 	= conf.getStr(SCS_NAVIGATION, SCK_STARTUP_TIME_MODE);	// Can be "now" or "preset"
	DayKeyMode 			= conf.getStr(SCS_NAVIGATION, SCK_DAY_KEY_MODE); //,"calendar");  // calendar or sidereal
	cLog::get()->write("Read daykeymode as <" + DayKeyMode + ">", LOG_TYPE::L_INFO);

	if (StartupTimeMode=="preset" || StartupTimeMode=="Preset")
		coreLink->setJDay(PresetSkyTime - spaceDate->getGMTShift(PresetSkyTime) * JD_HOUR);
	else core->setTimeNow();

	// initialisation of the User Interface
	ui->init(conf);
	ui->localizeTui();

	//set all color
	core->setColorScheme(settings->getConfigFile(), SCS_COLOR);

	// play startup script
	scriptMgr->playStartupScript();
	// on sauvegarde ici l'état des composants du logiciel.
	coreBackup->saveGridState();
	coreBackup->saveDisplayState();
	coreBackup->saveLineState();

	fontFactory->reloadAllFont();
	commander->deleteVar();
}

//! Load configuration from disk
void App::firstInit()
{
	appDraw->initSplash(&context);

	InitParser conf;
	AppSettings::Instance()->loadAppSettings( &conf );

	fontFactory->init(std::min(width,height), conf);
	fontFactory->initMediaFont(media.get());
	fontFactory->buildAllFont();

	ui->registerFont(fontFactory->registerFont(CLASSEFONT::CLASS_UI));

	core->init(conf);
	ui->init(conf);
	ui->localizeTui();
	ui->initTui();

	appDraw->createSC_context(&context);
	media->initVR360(&context);
	media->createSC_context(&context);

	enable_tcp=conf.getBoolean(SCS_IO, SCK_ENABLE_TCP);
	enable_mkfifo=conf.getBoolean(SCS_IO, SCK_ENABLE_MKFIFO);
	flagAlwaysVisible = conf.getBoolean(SCS_MAIN,SCK_FLAG_ALWAYS_VISIBLE);
	flagMasterput=conf.getBoolean(SCS_IO, SCK_FLAG_MASTERPUT);

	if (enable_tcp) {
		int port = conf.getInt(SCS_IO, SCK_TCP_PORT_IN);
		int buffer_in_size=conf.getInt(SCS_IO, SCK_TCP_BUFFER_IN_SIZE);
		cLog::get()->write("buffer TCP taille " + std::to_string(buffer_in_size));
		tcp = std::make_unique<ServerSocket>(port, 16, buffer_in_size);
		tcp->open();
		commander->setTcp(tcp.get());
	}

	#if LINUX // special mkfifo
	if (enable_mkfifo) {
		std::string mplayerMkfifoName = conf.getStr(SCS_IO, SCK_MPLAYER_MKFIFO_NAME);
		std::string mkfifo_file_in = conf.getStr(SCS_IO, SCK_MKFIFO_FILE_IN);
		int buffer_in_size=conf.getInt(SCS_IO, SCK_MKFIFO_BUFFER_IN_SIZE);
		cLog::get()->write("buffer MKFIFO taille "+ std::to_string(buffer_in_size));
		mkfifo->init(mkfifo_file_in, buffer_in_size);
	}
	#endif

	cLog::get()->write(CallSystem::getRamInfo());
	cLog::get()->mark();
	cLog::get()->write("End of loading SC");
	cLog::get()->write("End of loading SC",LOG_TYPE::L_INFO,LOG_FILE::SCRIPT);
	cLog::get()->mark();

	this->init();
	context.surface->finalize(false);
	globalContext.vulkan->finalize();
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


void App::update(int delta_time)
{
	internalFPS->addFrame();
	// change time rate if needed to fast forward scripts
	delta_time *= scriptMgr->getMuliplierRate();
	// run command from a running script
	scriptMgr->update(delta_time);
	if (!scriptMgr->isPaused() || !scriptMgr->isFaster() )	media->audioUpdate(delta_time);
	// run any incoming command from shared memory interface
	updateFromSharedData();

	ui->updateTimeouts(delta_time);
	ui->tuiUpdateWidgets();

	if (!scriptMgr->isPaused()) media->imageUpdate(delta_time);

	media->playerUpdate();
	screenFader->update(delta_time);
	media->faderUpdate(delta_time);

	executor->update(delta_time);
}


//! Main drawinf function called at each frame
void App::draw(int delta_time)
{
	context.surface->waitTransferQueueIdle();
	context.surface->waitGraphicQueueIdle();
	context.commandMgr->waitCompletion(0);
	context.commandMgr->waitCompletion(1);
	context.commandMgr->waitCompletion(2);
	context.surface->acquireNextFrame();

	context.commandMgrSingleUseInterface->reset();
	context.commandMgr->setSubmission(commandIndexClear, true);
	executor->draw(delta_time);
	// Draw the Graphical ui and the Text ui
	ui->draw();
	//inversion des couleurs pour un ciel blanc
	if (flagColorInverse)
		appDraw->drawColorInverse();

	//draw video frame to classical viewport
	media->drawViewPort();
	//draw text user
	media->textDraw();
	s_font::endPrint();

	// Fill with black around the circle
	appDraw->drawViewportShape();

	screenFader->draw();
	context.commandMgrSingleUseInterface->waitIdle();
	context.surface->submitFrame();
}

//! @brief Set the application locale. This apply to GUI, console messages etc..
void App::setAppLanguage(const std::string& newAppLocaleName)
{
	// Update the translator with new locale name
	Translator::globalTranslator = Translator(settings->getLanguageDir(), newAppLocaleName);
	cLog::get()->write("Application locale is " + Translator::globalTranslator.getLocaleName(), LOG_TYPE::L_INFO);
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
	conf.setStr	(SCS_MAIN,SCK_VERSION, VERSION);
	// localization section
	conf.setStr(SCS_LOCALIZATION, SCK_APP_LOCALE, getAppLanguage());
	conf.setStr(SCS_LOCALIZATION, SCK_TIME_DISPLAY_FORMAT, spaceDate->getTimeFormatStr());
	conf.setStr(SCS_LOCALIZATION, SCK_DATE_DISPLAY_FORMAT, spaceDate->getDateFormatStr());
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_CUSTOM) {
		conf.setStr(SCS_LOCALIZATION, SCK_TIME_ZONE, spaceDate->getCustomTzName());
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_SYSTEM_DEFAULT) {
		conf.setStr(SCS_LOCALIZATION, SCK_TIME_ZONE, "system_default");
	}
	if (spaceDate->getTzFormat() == SpaceDate::S_TZ_FORMAT::S_TZ_GMT_SHIFT) {
		conf.setStr(SCS_LOCALIZATION, SCK_TIME_ZONE, "gmt+x");
	}
	conf.setDouble (SCS_NAVIGATION, SCK_PRESET_SKY_TIME, PresetSkyTime);
	conf.setStr	(SCS_NAVIGATION, SCK_STARTUP_TIME_MODE, StartupTimeMode);
	conf.setStr	(SCS_NAVIGATION, SCK_DAY_KEY_MODE, DayKeyMode);
	conf.setDouble(SCS_RENDERING, SCK_LINE_WIDTH, appDraw->getLineWidth());
	conf.setBoolean(SCS_RENDERING, SCK_FLAG_ANTIALIAS_LINES, appDraw->getFlagAntialiasLines());

	ui->saveCurrentConfig(conf);
	core->saveCurrentConfig(conf);

	// Get landscape and other observatory info
	coreLink->observerSetConf(conf, SCS_INIT_LOCATION);
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
		//cLog::get()->write("MASTERPUT is in action", LOG_TYPE::L_INFO);
		unlink(action.c_str());
		scriptMgr->playScript(settings->getFtpDir()+"pub/script.sts");
	}
}

void App::startMainLoop()
{
	flagVisible = true;		// At The Beginning, Our App Is Visible
	flagAlive = true; 		// au debut, on veut que l'application ne s'arrete pas :)

	//center mouse in middle screen
	mSdl->warpMouseInCenter();

	internalFPS->init();
	internalFPS->selectMaxFps();

	SDL_TimerID my_timer_id = SDL_AddTimer(1000, internalFPS->callbackfunc, nullptr);

	// Start the main loop
	while (flagAlive) {
		while (SDL_PollEvent(&E)) {	// Fetch all Event Of The Queue
			ui->handleInputs(E);
		}

		//analyse le joystick au cas ou des events ont été accumulés pour le joystick
		ui->handleDeal();

		// on applique toutes les modifications faites dans ui etc
		eventHandler->handleEvents(executor.get());

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

			this->update(deltaTime);		// And update the motions and data
			this->draw(deltaTime);			// Do the drawings!
			saveScreenInterface->readScreenShot();
			globalContext.vulkan->sendFrame(); // Send submission to the presentation engine

			internalFPS->setLastCount();
		}
	}

	SDL_RemoveTimer(my_timer_id);
	CallSystem::killAllPidFrom("vlc");
	CallSystem::killAllPidFrom("mplayer");
}

void App::switchMode(const std::string setValue) {
		executor->switchMode(setValue);
}