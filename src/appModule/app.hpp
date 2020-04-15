/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2006 Fabien Chereau
 * Copyright (C) 2009, 2010 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef APP_H
#define APP_H

#include <SDL2/SDL_thread.h>
#include <queue>

#include "appModule/appModule.hpp"
#include "appModule/fps.hpp"
#include "appModule/space_date.hpp"
#include "spacecrafter.hpp"
#include "tools/app_settings.hpp"


// mac seems to use KMOD_META instead of KMOD_CTRL
#ifdef MACOSX
#define COMPATIBLE_KMOD_CTRL KMOD_META
#else
#define COMPATIBLE_KMOD_CTRL KMOD_CTRL
#endif

// Predeclaration of some classes
class AppCommandInterface;
class ScriptMgr;
class ScriptInterface;
class SDLFacade;
class s_font;
class Media;
class Mkfifo;
class UI;
class Core;
class CoreLink;
class AppDraw;
class SaveScreenInterface;
class ServerSocket;
class ScreenFader;
class EventManager;
class EventHandler;



enum class APP_FLAG : char {NONE, VISIBLE, ALIVE, ON_VIDEO, COLOR_INVERSE};
 
/**
@author initial Fabien Chereau
*/
class App : public AppModule {

public:
	App( SDLFacade* const );
	~App();

	//! Initialize application and core
	void init();

	//! Update all object according to the delta time
	void update(int delta_time);

	//! Draw all
	void draw(int delta_time);

	//! Start the main loop until the end of the execution
	void startMainLoop() {
		start_main_loop();
	}

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

	int getFpsClock() const {
		return internalFPS->getFps();
	}

	// todo deprecated 
	void executeCommand(const std::string& _command);

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

	//! modifie un flag de App donnée par APP_FLAG
	void flag(APP_FLAG layerValue, bool _value);
	//! modifie un flag via un toggle
	void toggle(APP_FLAG layerValue);

private:
	//! Run the main program loop
	void start_main_loop();
	//! run any incoming command from shared memory interface
	void updateFromSharedData();
	//! mise à jour de l'accélération du temps
	void updateDeltaSpeed();
	//! Use by masterput, poor but he does his job
	void masterput();

	bool flagAlive; 				//!< indique si l'application doit s'arrêter ou pas
	bool flagVisible;				//!< say if your App Is Visible or not
	bool flagOnVideo; 				//!< indique si une video est en cours de visionnage
	bool flagColorInverse;			//!< indique si les couleurs de l'écran sont inversées
	bool flagAlwaysVisible;			//!< say if SC should always remain visible

	bool initialized;				//! has the init method been called yet?
	double PresetSkyTime;
	std::string StartupTimeMode;	//! Can be "now" or "preset"
	std::string DayKeyMode;			//! calendar or sidereal
	unsigned int deltaTime; 		//! représente la durée théorique d'une frame

	//communication with other processus
	bool enable_tcp;
	bool enable_mkfifo;
	bool flagMasterput;

	// Main elements of the stel_app
	AppSettings* settings = nullptr; 			//! base pour les constantes du logiciel
	AppCommandInterface * commander = nullptr;	//! interface to perform all UI and scripting actions
	ScriptMgr * scriptMgr = nullptr;			//! manage playing and recording scripts
	ScriptInterface* scriptInterface = nullptr;	//! interface for other composents
	Media* media = nullptr;						//!< media manager
	UI * ui = nullptr;							//! The main User Interface
	ScreenFader *screenFader = nullptr;			//! gestion des fondus

	//! The assicated Core instance
	Core* core = nullptr;
	CoreLink* coreLink = nullptr;
	SDLFacade* mSdl = nullptr;
	SaveScreenInterface* saveScreenInterface = nullptr;
	ServerSocket * tcp = nullptr;
	Fps* internalFPS = nullptr;				//! gestion fine du frameRate
	AppDraw* appDraw = nullptr;
	#if LINUX
	Mkfifo* mkfifo = nullptr;
	#endif

	SpaceDate * spaceDate = nullptr;			// Handles dates and conversions
	EventManager *eventManager = nullptr;
	EventHandler *eventHandler = nullptr;

	Uint16 width, height;  						//! Contient la résolution w et h de la fenetre SDL
	SDL_Event	E;
};
#endif