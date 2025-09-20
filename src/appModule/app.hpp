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
#include <string>

#include "tools/no_copy.hpp"
#include "tools/context.hpp"

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
class FrameSender;
class InitParser;

enum class APP_FLAG : char {NONE, ANTIALIAS, VISIBLE, ALIVE, COLOR_INVERSE, SUBTITLE, DSO_PICK, STAR_PICK, BODY_PICK};

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
	int getTargetFps() const;

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

	//! modifies a flag given by APP_FLAG
	void flag(APP_FLAG layerValue, bool _value);
	//! modify a flag via a toggle
	void toggle(APP_FLAG layerValue);

	void switchMode(const std::string setValue);

private:
	//! run any incoming command from shared memory interface
	void updateFromSharedData();
	//! Use by masterput, poor but he does his job
	void masterput();
	//! Submit a frame
	static void submitFrame(App *self, int id);
	//! Initialize vulkan resources needed for the splash screen
	void initVulkan(InitParser &conf);
	//! Initialize vulkan resources needed outside of the splash screen
	void finalizeInitVulkan(InitParser &conf);
	//! Initialize vulkan resources used for the shadow
	void initShadowSystem(InitParser &conf);

	bool flagAlive; 				//!< indicates if the application should stop or not
	bool flagVisible;				//!< say if your App Is Visible or not
	bool flagColorInverse;			//!< indicates if the colors on the screen are inverted*.
	bool flagSubtitle;				//!< active text on top of videos
	bool flagAlwaysVisible;			//!< say if SC should always remain visible

	double PresetSkyTime;
	double baseHeading;
	std::string StartupTimeMode;	//! Can be "now" or "preset"
	std::string DayKeyMode;			//! calendar or sidereal
	unsigned int deltaTime; 		//! represents the theoretical duration of a frame

	//communication with other processus
	bool enable_tcp;
	bool enable_mkfifo;
	bool flagMasterput;


	// External class
	SDLFacade* mSdl = nullptr;

	// Graphic context
	Context context;
	std::unique_ptr<FrameSender> sender;
	std::vector<std::unique_ptr<Texture>> offscreenImage; // If using sender instead of swapchain
	VkSampleCountFlagBits sampleCount;
	std::vector<std::unique_ptr<Texture>> multisampleImage;
	std::unique_ptr<Texture> depthBuffer;
	bool flushFrames = false; // Flush every frames, reduce framerates but potentially remove some graphical glitches
	bool initialized = false;

	// Main elements of the stel_app
	AppSettings* settings = nullptr;		 			//! base for the software constants
	std::shared_ptr<AppCommandInterface> commander;		//! interface to perform all UI and scripting actions
	std::shared_ptr<ScriptMgr> scriptMgr;				//! manage playing and recording scripts
	std::shared_ptr<ScriptInterface> scriptInterface;	//! interface for other composents
	std::shared_ptr<Media> media;						//!< media manager
	std::shared_ptr<UI> ui;								//! The main User Interface
	std::shared_ptr<ScreenFader> screenFader;			//! fade management

	std::shared_ptr<Core> core;
	std::shared_ptr<CoreLink> coreLink;
	std::shared_ptr<CoreBackup> coreBackup;
	std::shared_ptr<FontFactory> fontFactory;
	std::shared_ptr<SaveScreenInterface> saveScreenInterface;
	std::unique_ptr<ServerSocket> tcp;
	std::unique_ptr<Fps> internalFPS;			//! fine management of the frameRate
	std::unique_ptr<AppDraw> appDraw;
	std::shared_ptr<Observer> observatory;
	std::unique_ptr<Executor> executor;
	std::unique_ptr<Mkfifo> mkfifo;

	std::shared_ptr<SpaceDate> spaceDate;		// Handles dates and conversions
	EventRecorder* eventRecorder;
	EventHandler* eventHandler;

	uint16_t renderSize = 0; //! Render size, if render surface is not the screen surface
	Uint16 width, height;  						//! Contains the resolution w and h of the SDL window
	int colorID, depthID, multiColorID, shadowID;
	SDL_Event	E;
};
#endif
