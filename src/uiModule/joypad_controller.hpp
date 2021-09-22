
/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/
//! \file joypad_controller.hpp
//! \brief Wrapper class for joypad. Loads key configuration from ini file and handles inputs
//! \author Julien LAFILLE
//! \date april 2018

/*
 * The goal of this class is to link spacecrafter's methods to joystick events.
 * It also handle sensitivity and deadzones for the axes
 * 
 * This class handles three types of events :
 *  - axis movement
 *  - button presses
 *  - hat presses
 * 
 * Each of these is linked to a function of the UI class
 * The functions are parsed from the joypad.ini file
 * 
 * axis movement are linked to a void function which takes a double as input
 * button and hat presses are linked to two functions : press and release. both are void functions that take no parameters
 *  
 * if you wish to add a new function to pick from :
 * 	- implement the needed functions in the UI class
 *  - add a new case to getAxisActionFromString or getButtonActionFromString
 *
 */

#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <vector>
// #include "tools/app_settings.hpp"

#ifndef _JOYPAD_CONTROLLER_HPP_
#define _JOYPAD_CONTROLLER_HPP_

class UI;

/*
 * Contains both the actions to perform when a button is pressed and realeased
 * Each action is a function pointer to a method from the UI class
 * They must have the following declaration : void UI::myFunc(void);
 */
typedef struct joy_button_action {
	void (UI::*onPressAction)();
	void (UI::*onReleaseAction)();
} joy_button_action;

/*
 * Contains the action to perform when an axis is out of the deadzone
 * The action is a method from the UI class
 * It must have the following declaration : void UI::myFunc(double);
 */
typedef struct joy_axis_action {
	void (UI::*action)(double x);
} joy_axis_action;

class JoypadController {

public:

	enum hat_event { hat_up = 0, hat_down = 1, hat_left = 2, hat_right = 3 };

	JoypadController() = delete;

	JoypadController(UI * ui) noexcept;

	~JoypadController();


	/*
	 * Loads configuration from config.ini
	 */
	void init(const std::string &_configName) noexcept;

	/*
	 * Dispaches events to their respective handeling functions
	 */
	void handle(const SDL_Event &E) noexcept;

	/*
	 * Performs actions for each axis if their values are outside of the deadzone
	 */
	void handleDeal() noexcept;

private:
	
	/*
	 * resets all controlls to their deault values
	 */
	void purge() noexcept;
	
	/*
	 * Handles value changes from the joypad axis
	 */
	void handleAxisEvent(const SDL_JoyAxisEvent &E) noexcept{
		axisValues[E.axis]= E.value;
	};

	void handleJoyButtonUp(const SDL_JoyButtonEvent &E) noexcept;


	/*
	 * Handles button presses
	 */
	void handleJoyButtonDown(const SDL_JoyButtonEvent &E) noexcept;

	void handleJoyHat(const SDL_JoyHatEvent &E) noexcept;

	/*
	 * Returns an axis action from a string
	 * Used to parse the config.ini file
	 */
	joy_axis_action getAxisActionFromString(const std::string &actionStr) noexcept;

	/*
	 * Returns a joy_button_action from a string
	 * Used to parse the config.ini file
	 * If the action is a command or a list of command it get added in a dictionnairy with its button number
	 */
	joy_button_action getButtonActionFromString(std::string &actionStr, int buttonNumber) noexcept;

	/*
	 * Returns true if the button is associated with a command
	 */
	bool isCommand(Uint8 buttonNumber) noexcept {
		return buttonCommand.count(buttonNumber);
	};

	UI * ui;

	SDL_Joystick* joystick=nullptr;

	int nbrAxis;
	int nbrButtons;
	int nbrHats;

	joy_axis_action* axisActions;					//actions to perform for each axis
	joy_button_action* buttonActions; 				//actions to perform for each button
	std::map<Uint8, std::vector<std::string>> buttonCommand; 	//commands associated with their button code
	joy_button_action* hatActions;					//actions to perform for each hat

	double *axisValues; 							//values of the controller's axises
	double *axisSensitivity;
	double *axisDeadZone;
	bool * axisIsStick;
};
#endif //_JOYPAD_CONTROLLER_HPP_
