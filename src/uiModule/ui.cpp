/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class which handles the User Interface

#include <iostream>
#include <iomanip>
#include <algorithm>
#include "appModule/app.hpp"
#include "coreModule/core.hpp"
#include "coreModule/coreLink.hpp"
#include "eventModule/event_manager.hpp"
#include "eventModule/ScriptEvent.hpp"
#include "eventModule/CommandEvent.hpp"
#include "eventModule/FlagEvent.hpp"
#include "eventModule/ScreenFaderEvent.hpp"
#include "eventModule/SaveScreenEvent.hpp"
#include "eventModule/FpsEvent.hpp"
#include "interfaceModule/script_interface.hpp"
#include "mainModule/sdl_facade.hpp"
#include "mediaModule/media.hpp"
#include "tools/call_system.hpp"
#include "tools/log.hpp"
#include "uiModule/joypad_controller.hpp"
#include "uiModule/ui.hpp"

static const double CoeffMultAltitude = 0.02;
static const double DURATION_COMMAND = 0.1;


////////////////////////////////////////////////////////////////////////////////
//								CLASS FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

std::string default_landscape = "";
std::string current_landscape = "";

UI::UI(Core * _core, CoreLink * _coreLink, App * _app, SDLFacade *_m_sdl,  Media* _media) :
	tuiFont(nullptr),
	FlagShowTuiMenu(0),
	tui_root(nullptr),
	key_Modifier(NONE) ,
	KeyTimeLeft(0) ,
	deltaSpeed(DeltaSpeed::NO)
{
	if (!_core) {
		cLog::get()->write("UI.CPP CRITICAL : In stel_ui constructor, invalid core.",LOG_TYPE::L_ERROR);
		exit(-1);
	}
	core = _core;
	coreLink = _coreLink;
	media = _media;
	m_sdl= _m_sdl;
	app = _app;
	is_dragging = false;
}

/**********************************************************************************/
UI::~UI()
{
	delete tuiFont;
	tuiFont = nullptr;
	if (tui_root) delete tui_root;
	tui_root=nullptr;
}

////////////////////////////////////////////////////////////////////////////////
void UI::init(const InitParser& conf)
{
	// Ui section
	FlagShowFps			= conf.getBoolean("gui:flag_show_fps");
	FlagShowLatLon      = conf.getBoolean("gui:flag_show_latlon");
	FlagShowFov			= conf.getBoolean("gui:flag_show_fov");
	FlagNumberPrint		= conf.getInt("gui:flag_number_print");

	FontSizeGeneral		= conf.getDouble ("font","font_general_size");
	FontNameGeneral     = AppSettings::Instance()->getUserFontDir() +conf.getStr("font", "font_general_name");
	MouseCursorTimeout  = conf.getDouble("gui","mouse_cursor_timeout");
	PosDateTime			= conf.getInt("gui","datetime_display_position");
	PosObjectInfo		= conf.getInt("gui","object_info_display_position");	
	PosMenuM			= conf.getInt("gui","menu_display_position");
	FlagShowPlanetname	= conf.getBoolean("gui:flag_show_planetname");
	MouseZoom			= conf.getInt("navigation","mouse_zoom");

	// Text ui section
	FontSizeMenuTui   = conf.getDouble ("font","font_menutui_size");
	FlagEnableTuiMenu = conf.getBoolean("tui:flag_enable_tui_menu");
	FlagShowGravityUi = conf.getBoolean("tui:flag_show_gravity_ui");
	FlagShowTuiDateTime = conf.getBoolean("tui:flag_show_tui_datetime");
	FlagShowTuiShortObjInfo = conf.getBoolean("tui:flag_show_tui_short_obj_info");
	FlagMouseUsableInScript = conf.getBoolean("gui","flag_mouse_usable_in_script");
	FontNameMenu = AppSettings::Instance()->getUserFontDir()+conf.getStr("font", "font_menu_name");

	text_ui = Utility::strToVec3f(conf.getStr("tui","text_ui"));
	text_tui_root = Utility::strToVec3f(conf.getStr("tui","text_tui_root"));


	// set up mouse cursor timeout
	MouseTimeLeft = MouseCursorTimeout*1000;

	default_landscape = coreLink->observatoryGetLandscapeName();
	current_landscape = coreLink->observatoryGetLandscapeName();
	cLog::get()->write("Landscape : "+default_landscape ,LOG_TYPE::L_INFO);
}

void UI::initScriptInterface(ScriptInterface* _scriptInterface) {
	scriptInterface = _scriptInterface;
}

void UI::initSpaceDateInterface(SpaceDate* _spaceDate) {
	spaceDate = _spaceDate;
}

/*******************************************************************/
void UI::draw()
{
	if (FlagShowGravityUi) drawGravityUi();
	if (FlagShowTuiMenu) drawTui();
}

/*******************************************************************/
void UI::saveCurrentConfig(InitParser &conf)
{
	// gui section
	conf.setDouble("gui:mouse_cursor_timeout",MouseCursorTimeout);
	// Text ui section
	conf.setBoolean("tui:flag_show_gravity_ui", FlagShowGravityUi);
	conf.setBoolean("tui:flag_show_tui_datetime", FlagShowTuiDateTime);
	conf.setBoolean("tui:flag_show_tui_short_obj_info", FlagShowTuiShortObjInfo);
}

/*******************************************************************************/
int UI::handleMove(int x, int y)
{
	// core->setMouse(x,y);
	// Do not allow use of mouse while script is playing otherwise script can get confused
	if (scriptInterface->isScriptPlaying() && ! FlagMouseUsableInScript) return 0;

	// Show cursor
	SDL_ShowCursor(1);
	MouseTimeLeft = MouseCursorTimeout*1000;

	if (is_dragging) {
		if ((has_dragged || sqrtf((x-previous_x)*(x-previous_x)+(y-previous_y)*(y-previous_y))>4.)) {
			has_dragged = true;
			core->setFlagTracking(false);
			core->dragView(previous_x, previous_y, x, y);
			previous_x = x;
			previous_y = y;
			return 1;
		}
	}
	return 0;
}

/*******************************************************************************/
void UI::flag(UI_FLAG layerValue, bool _value) {
	switch(layerValue) {
		// case UI_FLAG::SHOW_FPS : 
		// 	FlagShowFps = _value;
		// 	break;
		case UI_FLAG::SHOW_LATLON : 
			FlagShowLatLon = _value;
			break;		
		case UI_FLAG::SHOW_TUISHORTOBJ_INFO : 
			FlagShowTuiShortObjInfo = _value;
			break;
		case UI_FLAG::SHOW_TUIDATETIME : 
			FlagShowTuiDateTime = _value;
			break;
		case UI_FLAG::HANDLE_KEY_ONVIDEO : 
			handleKeyOnVideo = _value;
			break;
		default: break;
	}
}

/*******************************************************************************/
void UI::toggle(UI_FLAG layerValue)
{
		switch(layerValue) {
		// case UI_FLAG::SHOW_FPS : FlagShowFps = !FlagShowFps;
		// 	break;
		case UI_FLAG::SHOW_LATLON : FlagShowLatLon = ! FlagShowLatLon;
			break;
		case UI_FLAG::SHOW_TUISHORTOBJ_INFO : FlagShowTuiShortObjInfo = ! FlagShowTuiShortObjInfo;
			break;		
		case UI_FLAG::SHOW_TUIDATETIME : FlagShowTuiDateTime = ! FlagShowTuiDateTime;
			break;	

		default: break;
	}
}
// SHOW_LATLON, SHOW_FOV, SHOW_PLANETNAME

/*******************************************************************************/
int UI::handleClic(Uint16 x, Uint16 y, s_gui::S_GUI_VALUE button, s_gui::S_GUI_VALUE state)
{
	// Do not allow use of mouse while script is playing otherwise script can get confused
	if (scriptInterface->isScriptPlaying() && ! FlagMouseUsableInScript) return 0;

	// Make sure object pointer is turned on (script may have turned off)
	core->setFlagSelectedObjectPointer(true);

	// Show cursor
	SDL_ShowCursor(1);
	MouseTimeLeft = MouseCursorTimeout*1000;

	switch (button) {
		case s_gui::S_GUI_MOUSE_RIGHT :
			break;
		case s_gui::S_GUI_MOUSE_LEFT :
			if (state==s_gui::S_GUI_PRESSED) {
				is_dragging = true;
				has_dragged = false;
				previous_x = x;
				previous_y = y;
			} else {
				is_dragging = false;
			}
			break;
		case s_gui::S_GUI_MOUSE_MIDDLE :
			break;
		case s_gui::S_GUI_MOUSE_WHEELUP :
			coreLink->zoomTo(coreLink->getAimFov()-MouseZoom*coreLink->getAimFov()/60., 0.2);
			return 1;
		case s_gui::S_GUI_MOUSE_WHEELDOWN :
			coreLink->zoomTo(coreLink->getAimFov()+MouseZoom*coreLink->getAimFov()/60., 0.2);
			return 1;
		default:
			break;
	}

	// Manage the event for the main window
	{
		// Deselect the selected object
		if (button==s_gui::S_GUI_MOUSE_RIGHT && state==s_gui::S_GUI_RELEASED) {
			switch(key_Modifier) {
				case NONE:
			                this->executeCommand("select");
					break;

				case KWIN:
					this->executeCommand("flag mouse_coordinates toggle");
					//EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			return 1;
		}
		if (button==s_gui::S_GUI_MOUSE_MIDDLE && state==s_gui::S_GUI_RELEASED) {
			if (core->getFlagHasSelected()) {
				core->gotoSelectedObject();
				core->setFlagTracking(true);
			}
		}
		if (button==s_gui::S_GUI_MOUSE_LEFT && state==s_gui::S_GUI_RELEASED && !has_dragged) {
			// CTRL + left clic = right clic for 1 button mouse
			if (SDL_GetModState() & KMOD_CTRL) {
				this->executeCommand("select");
				return 1;
			}
			// Try to select object at that position
			core->findAndSelect(x, y);
		}
	}
	return 0;
}

// Update changing values
void UI::updateTimeouts(int delta_time)
{
//	 handle mouse cursor timeout
	if (MouseCursorTimeout > 0) {
		if (MouseTimeLeft > delta_time) MouseTimeLeft -= delta_time;
		else {
			// hide cursor
			MouseTimeLeft = 0;
			SDL_ShowCursor(0);
		}
	}

//	 handle key_Modifier cursor timeout
	if (key_Modifier != NONE) {
		if (KeyTimeLeft > delta_time) KeyTimeLeft -= delta_time;
		else {
			key_Modifier = NONE;
		}
	}
}


void UI::setFlagShowTuiMenu(const bool flag)
{
	if (flag && !FlagShowTuiMenu) {
		tuiUpdateIndependentWidgets();
	}
	FlagShowTuiMenu = flag;
}

void UI::handleJoyHat(SDL_JoyHatEvent E)
{
	// Mouvement d'un chapeau Nous devons donc utiliser le champ jhat
	//printf("Mouvement du chapeau %d du joystick %d\n",E.jhat.hat,E.jhat.which);
	switch (E.value) {
		case SDL_HAT_CENTERED:
			core->turnUp(0);
			core->turnRight(0);
			core->turnDown(0);
			core->turnLeft(0);
			break;
		case SDL_HAT_DOWN :
			core->turnDown(1);
			break;
		case SDL_HAT_LEFT :
			core->turnLeft(1);
			break;
		case SDL_HAT_RIGHT :
			core->turnRight(1);
			break;
		case SDL_HAT_UP :
			core->turnUp(1);
			break;
		default:
			break;
	}
}

void UI::moveMouseAlt(double x)
{
	int distZ=sqrt((posMouseX-m_sdl->getDisplayWidth()/2)*(posMouseX-m_sdl->getDisplayWidth()/2)+(posMouseY-m_sdl->getDisplayHeight()/2)*(posMouseY-m_sdl->getDisplayHeight()/2));
	if (distZ<1)
		distZ=1;
		
	if(abs(x) < 1){
		x = x>0 ? 1 : -1;
	}

	posMouseX = posMouseX+x*(posMouseY-m_sdl->getDisplayHeight()/2)/distZ/2;
	posMouseY = posMouseY-x*(posMouseX-m_sdl->getDisplayWidth()/2)/distZ/2;
	m_sdl->warpMouseInWindow(posMouseX, posMouseY);
	handleMove(posMouseX , posMouseY);
}

void UI::moveMouseAz(double x)
{
	int distZ=sqrt((posMouseX-m_sdl->getDisplayWidth()/2)*(posMouseX-m_sdl->getDisplayWidth()/2)+(posMouseY-m_sdl->getDisplayHeight()/2)*(posMouseY-m_sdl->getDisplayHeight()/2));
	if (distZ<1)
		distZ=1;
	posMouseX = posMouseX+x*(posMouseX-m_sdl->getDisplayWidth()/2-1)/distZ/2; //-1 put to avoid no movement if already centered
	posMouseY = posMouseY+x*(posMouseY-m_sdl->getDisplayHeight()/2-1)/distZ/2; //-1 put to avoid no movement if already centered
	m_sdl->warpMouseInWindow(posMouseX, posMouseY);
	handleMove(posMouseX , posMouseY);
}

void UI::moveLat(double x)
{
	coreLink->observerMoveRelLon(x,DURATION_COMMAND);
}

void UI::moveLon(double x)
{
	if (core->getSelectedPlanetEnglishName()==core->getHomePlanetEnglishName())
		coreLink->observerMoveRelLat(-x,DURATION_COMMAND);
	else
		coreLink->observerMoveRelLat(x,DURATION_COMMAND);
}

void UI::lowerHeight(double x)
{
	double latimem = coreLink->observatoryGetAltitude();
	latimem = -latimem*(CoeffMultAltitude*x);
	coreLink->observerMoveRelAlt(latimem, DURATION_COMMAND);
	this->executeCommand("add r 1");
}

void UI::raiseHeight(double x)
{
	double latimem = coreLink->observatoryGetAltitude();
	latimem = latimem*(CoeffMultAltitude*x);
	coreLink->observerMoveRelAlt(latimem, DURATION_COMMAND);
	this->executeCommand("add r -1");
}

void UI::handleJoyAddStick()
{
	joypadController = new JoypadController(this);
	joypadController->init("joypad.ini");
}

void UI::handleJoyRemoveStick()
{
	delete joypadController;
	joypadController = nullptr;
}

void UI::raiseHeight()
{
	core->raiseHeight(1);
}

void UI::stopZoomIn()
{
	core->zoomIn(0);
}

void UI::stopZoomOut()
{
	core->zoomOut(0);
}

void UI::stopLowerHeight()
{
	core->lowerHeight(0);
}

void UI::stopRaiseHeight()
{
	core->raiseHeight(0);
}

void UI::stopSpeedDecrease()
{
	deltaSpeed = DeltaSpeed::NO;
}

void UI::stopSpeedIncrease()
{
	deltaSpeed = DeltaSpeed::NO;
}

void UI::centerMouse()
{
	m_sdl->warpMouseInCenter();
}

void UI::stopTurnLeft()
{
	core->turnLeft(0);
}

void UI::stopTurnRight()
{
	core->turnRight(0);
}

void UI::stopTurnUp()
{
	core->turnUp(0);
}

void UI::stopTurnDown()
{
	core->turnDown(0);
}

void UI::zoomIn()
{
	core->zoomIn(1);
}

void UI::zoomOut()
{
	core->zoomOut(1);
}

void UI::lowerHeight()
{
	lowerHeight(1);
}

void UI::speedDecrease()
{
	if(scriptInterface->isScriptPlaying())
		scriptInterface->slowerSpeed();
	else
		deltaSpeed = DeltaSpeed::DOWN;
}

void UI::speedIncrease()
{
	if(scriptInterface->isScriptPlaying())
		scriptInterface->fasterSpeed();
	else
		deltaSpeed = DeltaSpeed::UP;
}

void UI::turnLeft()
{
	core->turnLeft(1);
}

void UI::turnRight()
{
	core->turnRight(1);
}

void UI::turnUp()
{
	core->turnUp(1);
}

void UI::turnDown()
{
	core->turnDown(1);
}

void UI::leftClick()
{
	handleClic(posMouseX, posMouseY, s_gui::S_GUI_MOUSE_LEFT, s_gui::S_GUI_PRESSED);
	handleClic(posMouseX, posMouseY, s_gui::S_GUI_MOUSE_LEFT, s_gui::S_GUI_RELEASED);
}

void UI::rightClick()
{
	handleClic(posMouseX, posMouseY, s_gui::S_GUI_MOUSE_RIGHT, s_gui::S_GUI_PRESSED);
	handleClic(posMouseX, posMouseY, s_gui::S_GUI_MOUSE_RIGHT, s_gui::S_GUI_RELEASED);
}

void UI::executeCommand(const std::string& command)
{
	//app->executeCommand(command);
	Event* event = new CommandEvent(command);
	EventManager::getInstance()->queue(event);	
}

void UI::pauseScript()
{
	if ( scriptInterface->isScriptPlaying() ) {
		this->executeCommand("script action pause");
		// coreLink->timeResetMultiplier();
	} else
		this->executeCommand("timerate action pause");;
}

void UI::handleInputs(SDL_Event E)
{

	enum s_gui::S_GUI_VALUE bt;

	switch (E.type) {		// And Processing It

		case SDL_QUIT:
			app->flag(APP_FLAG::ALIVE, false);
			break;

        case SDL_USEREVENT: {
            /* and now we can call the function we wanted to call in the timer but couldn't because of the multithreading problems */
			media->externalUpdate(0); // @TODO  cette valeur ne sert à rien
			Event* event = new FpsEvent(FPS_ORDER::AFTER_ONE_SECOND);
			EventManager::getInstance()->queue(event);	
            break;
        }

		case SDL_JOYDEVICEADDED:
			handleJoyAddStick();
			break;

		case SDL_JOYDEVICEREMOVED:
			handleJoyRemoveStick();
			break;

		case SDL_JOYAXISMOTION :
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYHATMOTION :
			joypadController->handle(E);
			break;

		case SDL_WINDOWEVENT:
			switch(E.window.event) {
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					app->flag(APP_FLAG::VISIBLE, true);
					break;

				case SDL_WINDOWEVENT_FOCUS_LOST:
					app->flag(APP_FLAG::VISIBLE, false);
					break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			// Convert the name from GLU to my GUI
			switch (E.button.button) {
				case SDL_BUTTON_RIGHT :
					bt=s_gui::S_GUI_MOUSE_RIGHT;
					break;
				case SDL_BUTTON_LEFT :
					bt=s_gui::S_GUI_MOUSE_LEFT;
					break;
				case SDL_BUTTON_MIDDLE :
					bt=s_gui::S_GUI_MOUSE_MIDDLE;
					break;
				default :
					bt=s_gui::S_GUI_MOUSE_LEFT;
			}
			handleClic(E.button.x,E.button.y,bt,s_gui::S_GUI_PRESSED);
			break;

		case SDL_MOUSEWHEEL:
			if(E.wheel.y>0)
				bt=s_gui::S_GUI_MOUSE_WHEELUP;
			else
				bt=s_gui::S_GUI_MOUSE_WHEELDOWN;
			handleClic(E.button.x,E.button.y,bt,s_gui::S_GUI_PRESSED);
			break;

		case SDL_MOUSEBUTTONUP:
			// Convert the name from GLU to my GUI
			switch (E.button.button) {
				case SDL_BUTTON_RIGHT :
					bt=s_gui::S_GUI_MOUSE_RIGHT;
					break;
				case SDL_BUTTON_LEFT :
					bt=s_gui::S_GUI_MOUSE_LEFT;
					break;
				case SDL_BUTTON_MIDDLE :
					bt=s_gui::S_GUI_MOUSE_MIDDLE;
					break;
				default :
					bt=s_gui::S_GUI_MOUSE_LEFT;
			}
			handleClic(E.button.x,E.button.y,bt,s_gui::S_GUI_RELEASED);
			break;

		case SDL_MOUSEMOTION:
			posMouseX=E.motion.x;
			posMouseY=E.motion.y;
			handleMove(E.motion.x,E.motion.y);
			break;

		case SDL_KEYDOWN:
			// Rescue escape in case of lock : CTRL + ESC forces brutal quit
			if (E.key.keysym.scancode==SDL_SCANCODE_ESCAPE && (SDL_GetModState() & KMOD_CTRL)) {
				app->flag(APP_FLAG::ALIVE, false);
				break;
			}
			// Send the event to the gui and stop if it has been intercepted
			handleKeys(E.key.keysym.scancode,E.key.keysym.mod,E.key.keysym.sym,s_gui::S_GUI_PRESSED);
			break;

		case SDL_KEYUP:
			handleKeys(E.key.keysym.scancode,E.key.keysym.mod,E.key.keysym.sym,s_gui::S_GUI_RELEASED);
			break;
	}
}

void UI::handleDeltaSpeed() noexcept
{
	if (deltaSpeed!=DeltaSpeed::NO) {
	 	if (deltaSpeed==DeltaSpeed::UP)
 			executeCommand("timerate action sincrement");
	 	else
	 		executeCommand("timerate action sdecrement");
	}
}

void UI::handleDeal()
{
	if(joypadController!=nullptr)
		joypadController->handleDeal();

	handleDeltaSpeed();
}

/*******************************************************************************/
// LSS HANDLE KEYS
// TODO replace this with flexible keymapping feature
// odd extension to prevent compilation from makefile but inclusion in make dist
int flag_compass = 0;
int flag_triangle = 0;
int flag_creu = 0;
int flag_f9 = 0;

bool antipodes = false;

int UI::handleKeysOnVideo(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
{

	int retVal = 0;

	switch(key) {
		case SDL_SCANCODE_SPACE :
			media->playerPause();
			break;
		case SDL_SCANCODE_ESCAPE :
			handleKeyOnVideo = false;
			media->playerStop();
			break;
		case SDL_SCANCODE_A :
		case SDL_SCANCODE_G :
			handleKeyOnVideo = false;
			media->playerStop();
			break;
		case SDL_SCANCODE_J :
			media->playerInvertflow();
			break;
		case SDL_SCANCODE_K :
			if ( scriptInterface->isScriptPlaying() ) {
				this->executeCommand("script action resume");
				// coreLink->timeResetMultiplier();
			} else
				media->playerPause();
			break;
		case SDL_SCANCODE_LEFT :
			media->playerJump(-10.0);
			break;
		case SDL_SCANCODE_RIGHT :
			media->playerJump(10.0);
			break;
		case SDL_SCANCODE_UP :
			media->playerJump(600.0);
			break;
		case SDL_SCANCODE_DOWN :
			// JUMP BEGINNING
			media->playerRestart(); //bug : crash software
			break;
		case SDL_SCANCODE_KP_MULTIPLY :
			this->executeCommand("audio volume increment");
			break;
		case SDL_SCANCODE_KP_DIVIDE :
			this->executeCommand("audio volume decrement");
			break;
		case SDL_SCANCODE_TAB :
			handleKeyOnVideo = false;
			break;
		default:
			retVal = 1;
			break;
	}

	return retVal;

}

int UI::handlKkeysOnTui(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
{
	s_tui::S_TUI_VALUE tuiv;
	if (state == s_gui::S_GUI_PRESSED)
		tuiv = s_tui::S_TUI_PRESSED;
	else
		tuiv = s_tui::S_TUI_RELEASED;

	if (state==s_gui::S_GUI_PRESSED && key==SDL_SCANCODE_SEMICOLON) {
		// leave tui menu
		FlagShowTuiMenu = false;

		// If selected a script in tui, run that now
		if (scriptInterface->getSelectedScript()!="") {
			event = new ScriptEvent( scriptInterface->getSelectedScriptDirectory()+scriptInterface->getSelectedScript());
			EventManager::getInstance()->queue(event);
		}
		// clear out now
		scriptInterface->setSelectedScriptDirectory("");
		scriptInterface->setSelectedScript("");
		return 1;
	}

	return handleKeysTui(key, tuiv);
}

int UI::handleKeyPressed(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
{

	int retVal = 0;

	std::ostringstream oss;

	std::string IDIR = AppSettings::Instance()->getScriptDir();
	std::string SDIR = AppSettings::Instance()->getScriptDir();
	std::string ADIR = AppSettings::Instance()->getAudioDir();
	std::string VDIR = AppSettings::Instance()->getVideoDir();
	if (core->getFlagNav()) { // Change scripts, audio and video folders on the fly in case of navigation mode
		SDIR = SDIR + "navigation/";
		ADIR = ADIR + "navigation/";
		VDIR = VDIR + "navigation/";
	}

	if (key == SDL_SCANCODE_A && (mod & COMPATIBLE_KMOD_CTRL)) {
		app->flag(APP_FLAG::ALIVE, false);
	}

	// if(!scriptInterface->isScriptPlaying())
	// 	coreLink->timeResetMultiplier();  // if no script in progress always real time

	switch (key) {

		case SDL_SCANCODE_LEFT :
			core->turnLeft(1);
			break;

		case SDL_SCANCODE_RIGHT :
			core->turnRight(1);
			break;

		case SDL_SCANCODE_UP :
			if (mod & KMOD_CTRL)
				core->zoomIn(1);
			else
				core->turnUp(1);
			break;

		case SDL_SCANCODE_DOWN :
			if (mod & KMOD_CTRL)
				core->zoomOut(1);
			else
				core->turnDown(1);
			break;

		case SDL_SCANCODE_PAGEUP :
			core->zoomIn(1);
			break;

		case SDL_SCANCODE_PAGEDOWN :
			core->zoomOut(1);
			break;

		case SDL_SCANCODE_RALT :
			key_Modifier=ALT;
			KeyTimeLeft = 120*1000;
			break;

		case SDL_SCANCODE_GRAVE :
			if (key_Modifier != SUPER) {
				key_Modifier=SUPER;
				KeyTimeLeft = 3*1000;
			} else key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RGUI :
		case SDL_SCANCODE_LGUI :
			if (key_Modifier != KWIN) {
				key_Modifier=KWIN;
				KeyTimeLeft = 120*1000;
			} else key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RSHIFT :
		case SDL_SCANCODE_LSHIFT :
			if (key_Modifier != SHIFT) {
				key_Modifier=SHIFT;
				KeyTimeLeft = 120*1000;
			} else key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RCTRL :
		case SDL_SCANCODE_LCTRL :
			key_Modifier=CTRL;
			KeyTimeLeft = 120*1000; // 2 min
			break;

		case SDL_SCANCODE_BACKSLASH :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( IDIR+"internal/clear_mess.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_NEBULA_NAMES , FLAG_VALUES::FV_TOGGLE,"flag nebula_names toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_SEMICOLON :
			switch(key_Modifier) {
				case NONE:
					if (FlagEnableTuiMenu) setFlagShowTuiMenu(true);
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_COMMA :
			switch(key_Modifier) {
				case NONE:
					event = new CommandEvent("external_viewer filename "+ADIR+"02.mp3 action play");
					EventManager::getInstance()->queue(event);					
					break;
				case SUPER:
					event = new CommandEvent("external_viewer filename "+ADIR+"06.mp3 action play");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new CommandEvent("external_viewer filename "+ADIR+"14.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new CommandEvent("external_viewer filename "+ADIR+"18.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL:
					event = new CommandEvent("external_viewer filename "+ADIR+"10.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_PERIOD :
			switch(key_Modifier) {
				case NONE:
					event = new CommandEvent("external_viewer filename "+ADIR+"03.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new CommandEvent("external_viewer filename "+ADIR+"07.mp3 action play");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new CommandEvent("external_viewer filename "+ADIR+"15.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new CommandEvent("external_viewer filename "+ADIR+"19.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL:
					event = new CommandEvent("external_viewer filename "+ADIR+"11.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_SLASH :
			switch(key_Modifier) {
				case NONE:
					event = new CommandEvent("external_viewer filename "+ADIR+"04.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new CommandEvent("external_viewer filename "+ADIR+"08.mp3 action play");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new CommandEvent("external_viewer filename "+ADIR+"16.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new CommandEvent("external_viewer filename "+ADIR+"20.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL:
					event = new CommandEvent("external_viewer filename "+ADIR+"12.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case  SDL_SCANCODE_A:
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_CARDINAL_POINTS , FLAG_VALUES::FV_TOGGLE,"flag cardinal_points toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					if (flag_creu != 1) {
						event = new ScriptEvent( IDIR+"internal/windrose.sts");
						EventManager::getInstance()->queue(event);
					} 
					else 
						core->setLandscape(current_landscape);
					flag_creu = (flag_creu+1)%2;
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					app->flag(APP_FLAG::ALIVE, false);
					break;
				case SHIFT:
					if (core->getFlagNav()) {
						if (!scriptInterface->isScriptPlaying()) {
							std::string s;
							std::stringstream out;
							out << flag_compass+1;
							s = out.str();
							if (flag_compass == 4) {
								core->setLandscape(current_landscape);
							} else {
								event = new ScriptEvent( SDIR+"fscripts/windrose/0"+s+".sts");
								EventManager::getInstance()->queue(event);
							}
							flag_compass = (flag_compass+1)%5;
							flag_triangle = 0;
							flag_f9 = 0;
						}
					}
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_B:
			switch(key_Modifier) {
				case NONE:
					if (coreLink->getMeteorsRate()==10) this->executeCommand("meteors zhr 10000");
					else this->executeCommand("meteors zhr 10");
					break;
				case SUPER:
					if (coreLink->getMeteorsRate()<=10000) this->executeCommand("meteors zhr 150000");
					else this->executeCommand("meteors zhr 10");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_C:
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_EQUATORIAL_GRID , FLAG_VALUES::FV_TOGGLE,"flag equatorial_grid toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE , FLAG_VALUES::FV_TOGGLE,"flag circumpolar_circle toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new FlagEvent( FLAG_NAMES::FN_ARIES_LINE , FLAG_VALUES::FV_TOGGLE,"flag aries_line toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_GREENWICH_LINE , FLAG_VALUES::FV_TOGGLE,"flag greenwich_line toggle");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new FlagEvent( FLAG_NAMES::FN_VERNAL_POINTS , FLAG_VALUES::FV_TOGGLE,"flag vernal_points toggle");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_D:
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_EQUATOR_LINE , FLAG_VALUES::FV_TOGGLE,"flag equator_line toggle");
					EventManager::getInstance()->queue(event);					
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_TROPIC_LINES , FLAG_VALUES::FV_TOGGLE,"flag tropic_lines toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( IDIR+"internal/dm_record.sts");
					//event = new CommandEvent("domemasters action record");
					EventManager::getInstance()->queue(event);
					//event = new SaveScreenEvent(SAVESCREEN_ORDER::TOGGLE_VIDEO);
					//EventManager::getInstance()->queue(event);
					//key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_SATELLITES_ORBITS , FLAG_VALUES::FV_TOGGLE,"flag satellites_orbits toggle");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/equator_poles.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_E:
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_CONSTELLATION_ART , FLAG_VALUES::FV_TOGGLE,"flag constellation_art toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					core->selectZodiac();
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_CONSTELLATION_PICK , FLAG_VALUES::FV_TOGGLE,"flag constellation_pick toggle");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					break;
				case CTRL:
					event = new ScriptEvent( IDIR+"internal/sky_culture3.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_F  :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_MOON_SCALED , FLAG_VALUES::FV_TOGGLE,"flag moon_scaled toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_SUN_SCALED , FLAG_VALUES::FV_TOGGLE,"flag sun_scaled toggle");
					EventManager::getInstance()->queue(event);
					event = new ScriptEvent( IDIR+"internal/big_planets.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new ScriptEvent( IDIR+"internal/bodies-asteroids-501ex.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( IDIR+"internal/bodies-kuiper.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL :
					event = new FlagEvent( FLAG_NAMES::FN_OORT , FLAG_VALUES::FV_TOGGLE,"flag oort toggle");
					EventManager::getInstance()->queue(event);
					event = new ScriptEvent( IDIR+"internal/comet.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_G :
			switch(key_Modifier) {
				case NONE:
					if ( scriptInterface->isScriptPlaying() ) {
						this->executeCommand("script action end");
						// coreLink->timeResetMultiplier();
					} else
						this->executeCommand("timerate rate 0");
					break;
				case SUPER:
					event = new CommandEvent("flag galactic_center toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new CommandEvent("flag galactic_line toggle");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new CommandEvent("flag galactic_pole toggle");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new CommandEvent("flag galactic_grid toggle");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_H :
			switch(key_Modifier) {
				case NONE:
					if ( scriptInterface->isScriptPlaying() ) {
						this->executeCommand("script action pause");
						// coreLink->timeResetMultiplier();
					} else
						this->executeCommand("timerate action pause");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case SHIFT:
					if (core->getFlagNav()) {
					  event = new CommandEvent("flag nautical_ra toggle");
					  EventManager::getInstance()->queue(event);
					}
					break;
				case KWIN:
					if (core->getFlagNav()) {
					  event = new CommandEvent("flag nautical_alt toggle");
					  EventManager::getInstance()->queue(event);
					}
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/personal.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_I :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("date sidereal -1");
					break;
				case SUPER:
   					if (core->getFlagNav()) {
					  key_Modifier= NONE;
					  event = new FlagEvent( FLAG_NAMES::FN_ORTHODROMY , FLAG_VALUES::FV_TOGGLE,"flag orthodromy toggle");
					  EventManager::getInstance()->queue(event);
					}
					  break;
				case SHIFT:
					this->executeCommand("date relative_month -1");
					break;
				case KWIN:
					event = new ScreenFaderEvent(ScreenFaderEvent::DOWN, 0.05);
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					this->executeCommand("date relative -1");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_J :
			switch(key_Modifier) {
				case NONE:
					if(scriptInterface->isScriptPlaying())
						scriptInterface->slowerSpeed();
					else {
						event = new CommandEvent("timerate action decrement");
						EventManager::getInstance()->queue(event);
					}
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/proper_demotion.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new CommandEvent("date sun rise");
					EventManager::getInstance()->queue(event);
					event = new ScriptEvent( IDIR+"internal/date_shift_minus.sts");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					this->executeCommand("moveto delta_alt -50000 duration 1");
					break;
				case CTRL:
					this->executeCommand("date relative_year -20");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_K :
			switch(key_Modifier) {
				case NONE:
					if ( scriptInterface->isScriptPlaying() ) {
						event = new CommandEvent("script action resume");
						EventManager::getInstance()->queue(event);
						// coreLink->timeResetMultiplier();
					} else {
						event = new CommandEvent("timerate rate 1");
						EventManager::getInstance()->queue(event);
					}
					break;
				case SUPER:
					event = new CommandEvent("timerate rate 1");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL :
					event = new CommandEvent("date sun midnight");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN :
					break;
				case SHIFT:
					event = new CommandEvent("date sun meridian");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_L :
			switch(key_Modifier) {
				case NONE:
					if(scriptInterface->isScriptPlaying())
						scriptInterface->fasterSpeed();
					else {
						event = new CommandEvent("timerate action increment");
						EventManager::getInstance()->queue(event);
					}
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/proper_motion.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					this->executeCommand("moveto delta_alt 50000 duration 1");
					break;
				case SHIFT:
					event = new CommandEvent("date sun set");
					EventManager::getInstance()->queue(event);
					event = new ScriptEvent( IDIR+"internal/date_shift_plus.sts");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					this->executeCommand("date relative_year 20");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_M :
			switch(key_Modifier) {
				case NONE:
					event = new CommandEvent("external_viewer filename "+ADIR+"01.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new CommandEvent("external_viewer filename "+ADIR+"05.mp3 action play");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new CommandEvent("external_viewer filename "+ADIR+"13.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new CommandEvent("external_viewer filename "+ADIR+"17.mp3 action play");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new CommandEvent("external_viewer filename "+ADIR+"09.mp3 action play");
					EventManager::getInstance()->queue(event);
				default:
					break;
			}
			break;

		case SDL_SCANCODE_N :
			switch(key_Modifier) {
				case NONE:
					CallSystem::killAllPidFromVLC();
					media->audioMusicDrop();
					// Mix_CloseAudio();
					// cLog::get()->write("Close Audio", LOG_TYPE::L_DEBUG );
					// if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
					// 	cLog::get()->write("Error while reopening Mix_OpenAudio: "+ std::string(Mix_GetError()), LOG_TYPE::L_ERROR );
					// } else
					// cLog::get()->write("SDL Sound re loaded once again", LOG_TYPE::L_INFO);
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/chut.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( IDIR+"internal/navigation.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					// media->audioDebug();
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_O :
			switch(key_Modifier) {
				case NONE:
					event = new CommandEvent("date sidereal 1");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new CommandEvent("flag angular_distance toggle");
					EventManager::getInstance()->queue(event);
				    key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScreenFaderEvent(ScreenFaderEvent::UP, 0.05);
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT:
					this->executeCommand("date relative_month 1");
					break;
				case CTRL :
					event = new CommandEvent("date relative 1");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_P :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("date sidereal 7");
					break;
				case SUPER:
					key_Modifier= NONE;
					this->executeCommand("flag polar_point toggle");
					break;
				case SHIFT:
					this->executeCommand("date relative_year 1");
					break;
				case KWIN:
					this->executeCommand("flag polar_circle toggle");
					break;
				case CTRL :
					this->executeCommand("date relative 7");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_Q :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("flag constellation_drawing toggle");
					//event = new FlagEvent( FLAG_NAMES::FN_CONSTELLATION_DRAWING , FLAG_VALUES::FV_TOGGLE,"flag constellation_drawing toggle");
					//EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					this->executeCommand("set sky_culture western-asterisms");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("flag star_lines toggle");
					//event = new FlagEvent( FLAG_NAMES::FN_STAR_LINES , FLAG_VALUES::FV_TOGGLE,"flag star_lines toggle");
					//EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					break;
				case CTRL :
					event = new ScriptEvent(IDIR+"internal/sky_culture1.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_R:
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES , FLAG_VALUES::FV_TOGGLE,"flag constellation_boundaries toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_ZODIAC , FLAG_VALUES::FV_TOGGLE,"flag zodiac toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_ATMOSPHERIC_REFRACTION , FLAG_VALUES::FV_TOGGLE,"flag atmospheric_refraction toggle");
					EventManager::getInstance()->queue(event);					
					break;
				case KWIN:
					if (scriptInterface->isScriptRecording()) {
						this->executeCommand("script action cancelrecord");
					} else {
						this->executeCommand("script action record");
					}
					key_Modifier= NONE;
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/sky_culture4.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_S:
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_ECLIPTIC_LINE , FLAG_VALUES::FV_TOGGLE,"flag ecliptic_line toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_PRECESSION_CIRCLE , FLAG_VALUES::FV_TOGGLE,"flag precession_circle toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new SaveScreenEvent(SAVESCREEN_ORDER::TAKE_SCREENSHOT);
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_PLANET_ORBITS , FLAG_VALUES::FV_TOGGLE,"flag planets_orbits toggle");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/ecliptic_poles.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_T :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_OBJECT_TRAILS , FLAG_VALUES::FV_TOGGLE,"flag object_trails toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_BODY_TRACE , FLAG_VALUES::FV_TOGGLE,"flag body_trace toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					this->executeCommand("body_trace action clear");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("body_trace pen toggle");
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/trace.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_U :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("date sidereal -7");
					key_Modifier= NONE;
					break;
					break;
				case SUPER:
   					if (core->getFlagNav()) {
					  key_Modifier= NONE;
					  event = new FlagEvent( FLAG_NAMES::FN_LOXODROMY , FLAG_VALUES::FV_TOGGLE,"flag loxodromy toggle");
					  EventManager::getInstance()->queue(event);
					}
					break;
				case SHIFT:
					this->executeCommand("date relative_year -1");
					break;
				case KWIN:
					break;
				case CTRL :
					this->executeCommand("date relative -7");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_V :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_SHOW_TUI_DATETIME , FLAG_VALUES::FV_TOGGLE,"flag show_tui_datetime toggle");
					EventManager::getInstance()->queue(event);					
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO , FLAG_VALUES::FV_TOGGLE,"flag show_tui_short_obj_info toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL :
					event = new FlagEvent( FLAG_NAMES::FN_SHOW_LATLON , FLAG_VALUES::FV_TOGGLE,"flag show_latlon toggle");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new CommandEvent("flag object_coordinates toggle");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_W :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_CONSTELLATION_NAMES , FLAG_VALUES::FV_TOGGLE,"flag constellation_names toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_ZENITH_LINE , FLAG_VALUES::FV_TOGGLE,"flag zenith_line toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new FlagEvent( FLAG_NAMES::FN_ZODIAC_LIGHT , FLAG_VALUES::FV_TOGGLE,"flag zodiacal_light toggle");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/sky_culture2.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_X :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_MERIDIAN_LINE , FLAG_VALUES::FV_TOGGLE,"flag meridian_line toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_AZIMUTHAL_GRID , FLAG_VALUES::FV_TOGGLE,"flag azimuthal_grid toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					if (core->getFlagNav()) {
						event = new FlagEvent( FLAG_NAMES::FN_VERTICAL_LINE , FLAG_VALUES::FV_TOGGLE,"flag vertical_line toggle");
						EventManager::getInstance()->queue(event);
					}
					key_Modifier= NONE;
					break;
				case SHIFT :
					event = new FlagEvent( FLAG_NAMES::FN_PLANETS_AXIS , FLAG_VALUES::FV_TOGGLE,"flag planets_axis toggle");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					event = new ScriptEvent( IDIR+"internal/mire.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_Y :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_ANALEMMA_LINE , FLAG_VALUES::FV_TOGGLE,"flag analemma_line toggle");
					EventManager::getInstance()->queue(event);					
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_GALACTIC_CENTER , FLAG_VALUES::FV_TOGGLE,"flag galactic_center toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_ANALEMMA , FLAG_VALUES::FV_TOGGLE,"flag analemma toggle");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					this->executeCommand(std::string("body_trace target selected pen on"));
					break;
				case CTRL :
					this->executeCommand("select planet home_planet pointer off");
					event = new FlagEvent( FLAG_NAMES::FN_TRACK_OBJECT , FLAG_VALUES::FV_ON,"flag track_object toggle");
					EventManager::getInstance()->queue(event);					
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_Z :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_ATMOSPHERE , FLAG_VALUES::FV_TOGGLE,"flag atmosphere toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_LANDSCAPE , FLAG_VALUES::FV_TOGGLE,"flag landscape toggle");
					EventManager::getInstance()->queue(event);					
					key_Modifier= NONE;
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/panorama2.sts");
					EventManager::getInstance()->queue(event);
					current_landscape = coreLink->observatoryGetLandscapeName();
					break;
				case KWIN:
					this->executeCommand(std::string("body name selected skin_use toggle"));				
					break;
				case CTRL :
					event = new FlagEvent( FLAG_NAMES::FN_CLOUDS , FLAG_VALUES::FV_TOGGLE,"flag clouds toggle");
					EventManager::getInstance()->queue(event);
					event = new ScriptEvent( SDIR+"fscripts/panorama4.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_0 :
			switch(key_Modifier) {
				case NONE:
					coreLink->observerMoveRelLat(45,7000);  //latitude , duration
					break;
				case SUPER:
					this->executeCommand("moveto lat 90 duration 5");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K0.sts");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					coreLink->observerMoveRelLat(30,5000);  //latitude , duration
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_1 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_STAR_NAMES , FLAG_VALUES::FV_TOGGLE,"flag star_names toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/white_room.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W13.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new ScriptEvent( SDIR+"fscripts/13.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K1.sts");
					EventManager::getInstance()->queue(event);

					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_2 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_PLANET_NAMES , FLAG_VALUES::FV_TOGGLE,"flag planet_names toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new FlagEvent( FLAG_NAMES::FN_PLANET_ORBITS , FLAG_VALUES::FV_TOGGLE,"flag planet_orbits toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W14.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new ScriptEvent( SDIR+"fscripts/14.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K2.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_3 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_NEBULA_HINTS , FLAG_VALUES::FV_TOGGLE,"flag nebula_hints toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/deepsky_drawings.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W15.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new ScriptEvent( SDIR+"fscripts/15.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K3.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_4 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_FOG , FLAG_VALUES::FV_TOGGLE,"flag fog toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/orange_fog.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W16.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new ScriptEvent( SDIR+"fscripts/16.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K4.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_5 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_PLANETS , FLAG_VALUES::FV_TOGGLE,"flag planets toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					this->executeCommand("body action clear");
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W17.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new ScriptEvent( SDIR+"fscripts/17.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K5.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_6 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_STARS , FLAG_VALUES::FV_TOGGLE,"flag stars toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					this->executeCommand("deselect");
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W18.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new ScriptEvent( SDIR+"fscripts/18.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K6.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_7 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_MILKY_WAY , FLAG_VALUES::FV_TOGGLE,"flag milky_way toggle");
					EventManager::getInstance()->queue(event);					
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/milkyway.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new FlagEvent( FLAG_NAMES::FN_COLOR_INVERSE , FLAG_VALUES::FV_TOGGLE,"flag color_inverse toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new FlagEvent( FLAG_NAMES::FN_STARS_TRACE , FLAG_VALUES::FV_TOGGLE,"flag stars_trace toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K7.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_8 :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_NEBULAE , FLAG_VALUES::FV_TOGGLE,"flag nebulae toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					this->executeCommand("nebula action clear");
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new FlagEvent( FLAG_NAMES::FN_DSO_PICTOGRAMS , FLAG_VALUES::FV_TOGGLE,"flag dso_pictograms toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					event = new FlagEvent( FLAG_NAMES::FN_NEBULA_NAMES , FLAG_VALUES::FV_TOGGLE,"flag nebula_names toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT:
					event = new ScriptEvent( SDIR+"fscripts/K8.sts");
					EventManager::getInstance()->queue(event);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_9 :
			switch(key_Modifier) {
				case NONE:
					coreLink->observerMoveRelLat(-45,7000);  //latitude , duration
					break;
				case SUPER:
					this->executeCommand("moveto lat -90 duration 5");
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( IDIR+"internal/takeoff.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/K9.sts");
					EventManager::getInstance()->queue(event);
					break;
				case CTRL :
					coreLink->observerMoveRelLat(-30,5000);  //latitude , duration
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_MINUS :
			switch(key_Modifier) {
				case NONE:
					if (core->getFlagManualAutoZoom()) this->executeCommand("zoom auto out manual 1");
					else this->executeCommand("zoom auto initial");
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/zoom.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					break;
				case CTRL :
					this->executeCommand("zoom auto out");
					this->executeCommand("zoom fov 60 duration 5");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_EQUALS :
			switch(key_Modifier) {
				case NONE:
					if(core->getFlagManualAutoZoom()) this->executeCommand("zoom auto in manual 1");
					else this->executeCommand("zoom auto in");
					break;
				case SUPER:
					this->executeCommand("zoom auto out");
					this->executeCommand("zoom fov 10 duration 5");
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					break;
				case CTRL :
					this->executeCommand("zoom auto in");
					this->executeCommand("zoom fov 1 duration 5");
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_SPACE :
			switch(key_Modifier) {
				case NONE:
					if ( scriptInterface->isScriptPlaying() ) {
						this->executeCommand("script action pause");
						// coreLink->timeResetMultiplier();
					} else
						this->executeCommand("timerate action pause");
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL :
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_NONUSBACKSLASH :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_LANDSCAPE , FLAG_VALUES::FV_TOGGLE,"flag landscape toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/panorama1.sts");
					EventManager::getInstance()->queue(event);
					current_landscape = coreLink->observatoryGetLandscapeName();
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case SHIFT :
					event = new ScriptEvent( SDIR+"fscripts/panorama3.sts");
					EventManager::getInstance()->queue(event);
					current_landscape = coreLink->observatoryGetLandscapeName();
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/panorama5.sts");
					EventManager::getInstance()->queue(event);
					current_landscape = coreLink->observatoryGetLandscapeName();
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_LEFTBRACKET :
			switch(key_Modifier) {
				case NONE:
					event = new FlagEvent( FLAG_NAMES::FN_TRACK_OBJECT , FLAG_VALUES::FV_TOGGLE,"flag track_object toggle");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/takeoff.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN: {
					event = new ScriptEvent( IDIR+"internal/fly_to_selected.sts");
					EventManager::getInstance()->queue(event);
					} break;
				case SHIFT:
					break;
				case CTRL : 
					this->executeCommand("set home_planet selected");
					break;
				default:
					break;
			}
			break;


		case  SDL_SCANCODE_RIGHTBRACKET :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("date load preset");
					break;
				case SUPER:
					app->init();
					event = new ScriptEvent( IDIR+"internal/initial.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("date load current");
					this->executeCommand("date sun set");
					this->executeCommand("date relative 0.07");
					break;
				case KWIN:
					break;
				case CTRL:
					this->executeCommand("date load keep_time");
					break;
				default:
					break;
			}
			break;

		case  SDL_SCANCODE_APOSTROPHE :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("flag lock_sky_position toggle");
					break;
				case SHIFT:
					this->executeCommand("position action save");
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/clear_mess.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new FlagEvent( FLAG_NAMES::FN_TRACK_OBJECT , FLAG_VALUES::FV_TOGGLE,"flag track_object toggle");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL: 
					this->executeCommand(std::string("set home_planet selected"));
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_ESCAPE:
			break;

		case SDL_SCANCODE_INSERT:
			this->executeCommand("position action save");
			break;
		case SDL_SCANCODE_DELETE:
			this->executeCommand("position action load");
			break;
		case SDL_SCANCODE_HOME:
			app->init();
			event = new ScriptEvent( IDIR+"internal/initial.sts");
			EventManager::getInstance()->queue(event);
			break;
		case SDL_SCANCODE_END:
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( IDIR+"internal/initial_night.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT:
					break;
				case SUPER:
					event = new ScriptEvent( IDIR+"internal/initial_dawn.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;
			
		case SDL_SCANCODE_RETURN:
			this->executeCommand("deselect");
			break;
			
		case SDL_SCANCODE_KP_1 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M01.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M11.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					coreLink->moveHeadingRelative(-0.2);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S01.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					coreLink->moveHeadingRelative(-1);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_2 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M02.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M12.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S02.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("moveto delta_lat -0.5 duration 0.1");
					break;
				case CTRL:
					this->executeCommand("add r -1");
					break;
				case ALT:
					coreLink->cameraMoveRelativeXYZ(0.,-1.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_3 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M03.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M13.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("moveto multiply_alt 0.2 duration 1");
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S03.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				case ALT:
					coreLink->cameraMoveRelativeXYZ(0.,0.0,-1.0);
					break;

				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_4 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M04.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M14.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("moveto delta_lon -0.5 duration 0.1");
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S04.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case CTRL:
					this->executeCommand("add s +1");
					break;
				case ALT:
					coreLink->cameraMoveRelativeXYZ(-1.,0.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_5 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M05.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M15.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S05.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT: {
					std::string newplanet = core->getSelectedPlanetEnglishName();
					if (newplanet!="") this->executeCommand(std::string("set home_planet ") + newplanet);
				}
				break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_6 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M06.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M16.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("moveto delta_lon 0.5 duration 0.1");
					//core->moveRelLonObserver(0.5,DURATION_COMMAND);  //longitude , duration
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S06.sts");
					EventManager::getInstance()->queue(event);	
					key_Modifier= NONE;
					break;
				case CTRL:
					this->executeCommand("add s -1");
					break;
				case ALT:
					coreLink->cameraMoveRelativeXYZ(1.,0.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_7 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M07.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M17.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					coreLink->moveHeadingRelative(0.2);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S07.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				case CTRL :
					coreLink->moveHeadingRelative(1);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_8 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M08.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M18.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("moveto delta_lat 0.5 duration 0.1");
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S08.sts");
					EventManager::getInstance()->queue(event);	
					key_Modifier= NONE;
					break;
				case CTRL :
					this->executeCommand("add r 1");
					break;
				case ALT:
					coreLink->cameraMoveRelativeXYZ(0.,1.0,0.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_9 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M09.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M19.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("moveto multiply_alt 5.0 duration 1");
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S09.sts");
					EventManager::getInstance()->queue(event);	
					key_Modifier= NONE;
					break;
				case CTRL :
					event = new FlagEvent( FLAG_NAMES::FN_GALACTIC_GRID , FLAG_VALUES::FV_TOGGLE,"flag galactic_grid toggle");
					EventManager::getInstance()->queue(event);					
					break;
				case ALT:
					coreLink->cameraMoveRelativeXYZ(0.,0.0,1.0);
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_0 :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M00.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M10.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT:
					event = new FlagEvent( FLAG_NAMES::FN_TRACK_OBJECT , FLAG_VALUES::FV_TOGGLE,"flag track_object toggle");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S10.sts");
					EventManager::getInstance()->queue(event);	
					key_Modifier= NONE;
					break;
				case CTRL:
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_TAB :
			switch(key_Modifier) {
				case NONE:
					handleKeyOnVideo = true;
					break;
				case SUPER:
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_PERIOD :
			switch(key_Modifier) {
				case NONE:
					event = new ScriptEvent( SDIR+"fscripts/M20.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SUPER:
					event = new ScriptEvent( SDIR+"fscripts/M21.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				case SHIFT: //{
					//std::string coreplanet = core->getSelectedPlanetEnglishName();
					//if (coreplanet!="") {
					//	this->executeCommand(std::string("set home_planet ") + coreplanet);
					//	this->executeCommand(std::string("select planet ')+coreplanet+string(' pointer off"));
					//	this->executeCommand(std::string("wait duration 0.01"));
					//	this->executeCommand(std::string("body name _ action drop"));
					//	this->executeCommand(std::string("body action load name _ parent Sun radius 1 oblateness 0.0 albedo 0.0 lighting false halo false color 1.0,1.0,1.0 rot_periode 1000000000 tex_map none.png coord_func ')+coreplanet+string('_special"));
					//	this->executeCommand(std::string("set home_planet _ duration 0"));
					//	this->executeCommand(std::string("select planet _ pointer off"));
					//	event = new FlagEvent( FLAG_NAMES::FN_TRACK_OBJECT , FLAG_VALUES::FV_ON,"flag track_object on");
					//	EventManager::getInstance()->queue(event);						
					//}
				//}
				break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S11.sts");
					EventManager::getInstance()->queue(event);					
					key_Modifier= NONE;
					break;
				case CTRL :
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_PLUS :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand(std::string("set home_planet Earth"));
					this->executeCommand(std::string("moveto lat default lon default alt default"));
					this->executeCommand(std::string("zoom auto out manual 1"));
					break;
				case SUPER:
					this->executeCommand("moveto lat inverse lon inverse");
					if (!antipodes) {
						event = new ScriptEvent( IDIR+"internal/antipodes.sts");
						EventManager::getInstance()->queue(event);
					} else {
						core->setLandscape(current_landscape);
					}
					antipodes = !antipodes;
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S12.sts");
					EventManager::getInstance()->queue(event);					
					key_Modifier= NONE;
					break;
				case CTRL :
					this->executeCommand("moveto lat inverse");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_MINUS :
			switch(key_Modifier) {
				case NONE:
					SDL_ShowCursor(1);
					m_sdl->warpMouseInCenter();
					break;
				case SUPER:
					SDL_ShowCursor(0);
					m_sdl->warpMouseInWindow( m_sdl->getDisplayWidth()/2 , m_sdl->getDisplayHeight() );
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S13.sts");
					EventManager::getInstance()->queue(event);						
					key_Modifier= NONE;
					break;
				case CTRL :

					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_MULTIPLY :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("audio volume increment");
					break;
				case SUPER:
					this->executeCommand("audio volume 100");
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S14.sts");
					EventManager::getInstance()->queue(event);					
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("set ambient_light increment");
					break;
				case CTRL:
					this->executeCommand("define a 1");
					this->executeCommand("script action resume");
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_KP_DIVIDE :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("audio volume decrement");
					break;
				case SUPER:
					this->executeCommand("audio volume 0");
					key_Modifier= NONE;
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/S15.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("set ambient_light decrement");
					break;
				case CTRL :
					this->executeCommand("define a 0");
					this->executeCommand("script action resume");
					break;
				default:
					break;
			}
			break;


		case  SDL_SCANCODE_F1:
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"01.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"13.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"25.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/01.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W01.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;


		case SDL_SCANCODE_F2 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"02.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"14.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"26.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/02.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W02.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F3 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"03.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"15.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"27.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/03.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W03.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F4:
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"04.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"16.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"28.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/04.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W04.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F5:
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"05.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"17.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"29.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/05.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W05.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F6 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"06.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"18.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"30.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/06.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W06.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F7 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"07.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"19.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"31.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/07.sts");
					EventManager::getInstance()->queue(event);				
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W07.sts");
					EventManager::getInstance()->queue(event);				
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F8 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"08.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"20.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"32.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/08.sts");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W08.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F9 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"09.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"21.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"33.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/09.sts");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W09.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F10 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"10.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"22.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"34.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/10.sts");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W10.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F11 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"11.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"23.avi action play");
					key_Modifier= NONE;
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/11.sts");
					EventManager::getInstance()->queue(event);
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"35.avi action play");
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W11.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

		case SDL_SCANCODE_F12 :
			switch(key_Modifier) {
				case NONE:
					this->executeCommand("external_viewer filename "+VDIR+"12.avi action play");
					break;
				case SUPER:
					this->executeCommand("external_viewer filename "+VDIR+"24.avi action play");
					key_Modifier= NONE;
					break;
				case SHIFT:
					this->executeCommand("external_viewer filename "+VDIR+"36.avi action play");
					break;
				case CTRL :
					event = new ScriptEvent( SDIR+"fscripts/12.sts");
					EventManager::getInstance()->queue(event);
					break;
				case KWIN:
					event = new ScriptEvent( SDIR+"fscripts/W12.sts");
					EventManager::getInstance()->queue(event);
					key_Modifier= NONE;
					break;
				default:
					break;
			}
			break;

			retVal=1;

		default:
			break;
	}  // end S_GUI_PRESSED

	return retVal;
}

int UI::handleKeysReleased(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
{

	int retVal = 0;

	switch (key) {

		case SDL_SCANCODE_LEFT :
			core->turnLeft(0);
			break;

		case SDL_SCANCODE_RIGHT :
			core->turnRight(0);
			break;

		case SDL_SCANCODE_UP :
			if (mod & KMOD_CTRL)
				core->zoomIn(0);
			else
				core->turnUp(0);
			break;

		case SDL_SCANCODE_DOWN :
			if(mod & KMOD_CTRL)
				core->zoomOut(0);
			else
				core->turnDown(0);
			break;

		case SDL_SCANCODE_PAGEUP :
			core->zoomIn(0);
			break;

		case SDL_SCANCODE_PAGEDOWN :
			core->zoomOut(0);
			break;

		case SDL_SCANCODE_RALT:
			KeyTimeLeft=0;
			key_Modifier = NONE;
			break;

		case SDL_SCANCODE_RCTRL :
		case SDL_SCANCODE_LCTRL :
			KeyTimeLeft=0;
			key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RSHIFT :
		case SDL_SCANCODE_LSHIFT :
			KeyTimeLeft=0;
			key_Modifier=NONE;
			break;

		case SDL_SCANCODE_RGUI :
		case SDL_SCANCODE_LGUI :
			KeyTimeLeft=0;
			key_Modifier=NONE;
			break;
		default:
			retVal = 1;
			break;
	}

	return retVal;
}

int UI::handleKeys(SDL_Scancode key, Uint16 mod, Uint16 unicode, s_gui::S_GUI_VALUE state)
{

	if (FlagShowTuiMenu) {
		return handlKkeysOnTui(key, mod, unicode, state);
	}

	//gestion touches video
	if (handleKeyOnVideo && state==s_gui::S_GUI_PRESSED) { //isOnVideo
		return handleKeysOnVideo(key,mod,unicode,state);
	}

	if (state==s_gui::S_GUI_PRESSED) {
		return handleKeyPressed(key, mod, unicode, state);
	}

	if (state==s_gui::S_GUI_RELEASED) {
		switch (key) {

			case SDL_SCANCODE_LEFT :
				core->turnLeft(0);
				break;

			case SDL_SCANCODE_RIGHT :
				core->turnRight(0);
				break;

			case SDL_SCANCODE_UP :
				if (mod & KMOD_CTRL)
					core->zoomIn(0);
				else
					core->turnUp(0);
				break;

			case SDL_SCANCODE_DOWN :
				if(mod & KMOD_CTRL)
					core->zoomOut(0);
				else
					core->turnDown(0);
				break;

			case SDL_SCANCODE_PAGEUP :
				core->zoomIn(0);
				break;

			case SDL_SCANCODE_PAGEDOWN :
				core->zoomOut(0);
				break;

		case SDL_SCANCODE_RALT:
			KeyTimeLeft=0;
			key_Modifier = NONE;
			break;

			case SDL_SCANCODE_RCTRL :
			case SDL_SCANCODE_LCTRL :
				KeyTimeLeft=0;
				key_Modifier=NONE;
				break;

			case SDL_SCANCODE_RSHIFT :
			case SDL_SCANCODE_LSHIFT :
				KeyTimeLeft=0;
				key_Modifier=NONE;
				break;

			case SDL_SCANCODE_RGUI :
			case SDL_SCANCODE_LGUI :
				KeyTimeLeft=0;
				key_Modifier=NONE;
				break;
			default:
				break;
		}
	}
	return 1;
}
