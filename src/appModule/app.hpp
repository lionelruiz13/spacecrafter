/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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

#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <queue>
#include <memory>

#include "tools/no_copy.hpp"
#include "vulkanModule/Context.hpp"

// Predeclaration of some classes
class AppSettings;
class AppCommandInterface;
class ScriptMgr;
class ScriptInterface;
class SDLFacade;
class s_font;
class FontFactory;
class Media;
class Mkfifo;
class UI;
class Core;
class CoreLink;
class CoreBackup;
class AppDraw;
class SaveScreenInterface;
class ServerSocket;
class ScreenFader;
class EventRecorder;
class EventHandler;
class Fps;
class SpaceDate;
class Executor;
class Observer;

enum class APP_FLAG : char {NONE, ANTIALIAS, VISIBLE, ALIVE, COLOR_INVERSE};

/**
@author initial Fabien Chereau
*/
class App : public NoCopy {

public:
	App( SDLFacade* const );
	~App();

	//! First initialization of the application
	void firstInit();

	//! Initialize application and core
	void init();

	//! Update all object according to the delta time
	void update(int delta_time);

	//! Draw all
	void draw(int delta_time);

	//! Start the main loop until the end of the execution
	void startMainLoop();

	//! @brief Set the application language
	//! This applies to GUI, console messages etc..
	//! This function has no permanent effect on the global locale
	//! @param newAppLocaleName The name of the language (e.g fr) to use for GUI, TUI and console messages etc..
	void setAppLanguage(const std::string& newAppLangName);

	std::string getAppLanguage();

	// for use by TUI
	void saveCurrentConfig(const std::string& confFile);

	//! Required because stelcore doesn't have access to the script manager anymore!
	//! Record a command if script recording is on
	void recordCommand(const std::string& commandline);

	int getFpsClock() const;

	void setPresetSkyTime(double _value) {
		PresetSkyTime = _value;
	}
	double getPresetSkyTime() {
		return PresetSkyTime;
	}

	std::string getStartupTimeMode() {
		return StartupTimeMode;
	}
	void setStartupTimeMode(const std::string& _value) {
		StartupTimeMode = _value;
	}

	std::string getDayKeyMode() {
		return DayKeyMode;
	}
	void setDayKeyMode(const std::string& _value) {
		std::string DayKeyMode= _value;
	}

	void setLineWidth(float w) const;
	float getLineWidth() const;
	float getFlagAntialiasLines() const;

	//! modifie un flag de App donnée par APP_FLAG
	void flag(APP_FLAG layerValue, bool _value);
	//! modifie un flag via un toggle
	void toggle(APP_FLAG layerValue);

	void switchMode(const std::string setValue);

private:
	//! run any incoming command from shared memory interface
	void updateFromSharedData();
	//! Use by masterput, poor but he does his job
	void masterput();

	bool flagAlive; 				//!< indique si l'application doit s'arrêter ou pas
	bool flagVisible;				//!< say if your App Is Visible or not
	bool flagColorInverse;			//!< indique si les couleurs de l'écran sont inversées
	bool flagAlwaysVisible;			//!< say if SC should always remain visible

	double PresetSkyTime;
	std::string StartupTimeMode;	//! Can be "now" or "preset"
	std::string DayKeyMode;			//! calendar or sidereal
	unsigned int deltaTime; 		//! représente la durée théorique d'une frame

	//communication with other processus
	bool enable_tcp;
	bool enable_mkfifo;
	bool flagMasterput;


	// External class
	SDLFacade* mSdl = nullptr;

	// Graphic context
	GlobalContext globalContext;
	ThreadContext context;
	int commandIndexClear;

	// Main elements of the stel_app
	AppSettings* settings = nullptr;		 			//! base pour les constantes du logiciel
	std::shared_ptr<AppCommandInterface> commander;		//! interface to perform all UI and scripting actions
	std::shared_ptr<ScriptMgr> scriptMgr;				//! manage playing and recording scripts
	std::shared_ptr<ScriptInterface> scriptInterface;	//! interface for other composents
	std::shared_ptr<Media> media;						//!< media manager
	std::shared_ptr<UI> ui;								//! The main User Interface
	std::shared_ptr<ScreenFader> screenFader;			//! gestion des fondus

	std::shared_ptr<Core> core;
	std::shared_ptr<CoreLink> coreLink;
	std::shared_ptr<CoreBackup> coreBackup;
	std::shared_ptr<FontFactory> fontFactory;
	std::shared_ptr<SaveScreenInterface> saveScreenInterface;
	std::unique_ptr<ServerSocket> tcp;
	std::unique_ptr<Fps> internalFPS;			//! gestion fine du frameRate
	std::unique_ptr<AppDraw> appDraw;
	std::shared_ptr<Observer> observatory;
	std::unique_ptr<Executor> executor;
	#if LINUX
	std::unique_ptr<Mkfifo> mkfifo;
	#endif

	std::shared_ptr<SpaceDate> spaceDate;		// Handles dates and conversions
	EventRecorder* eventRecorder;
	EventHandler* eventHandler;

	Uint16 width, height;  						//! Contient la résolution w et h de la fenetre SDL
	SDL_Event	E;
};
#endif
