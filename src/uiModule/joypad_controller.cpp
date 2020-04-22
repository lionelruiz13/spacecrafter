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

#include <iostream>
#include "tools/log.hpp"
#include "uiModule/joypad_controller.hpp"
#include "uiModule/ui.hpp"


//splits a string in a vector
static void split(std::string str, std::string splitBy, std::vector<std::string>& tokens)
{
	tokens.push_back(str);
	size_t splitAt;
	size_t splitLen = splitBy.size();
	std::string frag;
	while (true) {
		frag = tokens.back();
		splitAt = frag.find(splitBy);
		if (splitAt == std::string::npos)
			break;
		tokens.back() = frag.substr(0, splitAt);
		tokens.push_back(frag.substr(splitAt + splitLen, frag.size() - (splitAt + splitLen)));
	}
}

JoypadController::JoypadController(UI * _ui) noexcept
{

	ui = _ui;

	joystick = SDL_JoystickOpen(0); // we only take joystick number 0

	SDL_JoystickUpdate();
	SDL_JoystickEventState(SDL_ENABLE);

	nbrAxis = SDL_JoystickNumAxes(joystick);
	nbrButtons = SDL_JoystickNumButtons(joystick);
	nbrHats = SDL_JoystickNumHats(joystick);

	axisValues = new double[nbrAxis];
	axisActions = new joy_axis_action[nbrAxis];
	axisSensitivity = new double[nbrAxis];
	axisDeadZone = new double[nbrAxis];
	axisIsStick = new bool[nbrAxis];


	for (int i = 0; i < nbrAxis; i++) {
		axisValues[i] = 0;
		axisActions[i].action = nullptr;
		axisSensitivity[i] = 0;
		axisDeadZone[i] = 0;
		axisIsStick[i] = false;
	}

	buttonActions = new joy_button_action[nbrButtons];

	for(int i = 0; i < nbrButtons; i++) {
		buttonActions[i].onPressAction = nullptr;
		buttonActions[i].onReleaseAction = nullptr;
	}


	//a remplacer un jour
	hatActions = new joy_button_action[nbrHats*4];

	for(int i = 0; i < nbrHats*4; i++) {
		hatActions[i].onPressAction = nullptr;
		hatActions[i].onReleaseAction = nullptr;
	}
	cLog::get()->write("Create JoypadController" , LOG_TYPE::L_INFO); 
}

JoypadController::~JoypadController()
{
	purge();
	handleDeal();
	
	cLog::get()->write("delete JoypadController", LOG_TYPE::L_INFO); 
	SDL_JoystickClose(joystick);
	joystick = nullptr;
	delete[] axisActions;
	delete[] buttonActions;
	delete[] axisValues;
	delete[] axisSensitivity;
	delete[] axisDeadZone;
	delete[] axisIsStick;

}

/*
 * Loads configuration from _configName (ini file)
 */
void JoypadController::init(const std::string &_configName) noexcept
{

	InitParser conf;
	//~ AppSettings::Instance()->loadAppSettings( &conf );
	conf.load(AppSettings::Instance()->getConfigDir() + _configName);

	std::string model = SDL_JoystickName(joystick);

	//~ cout << model << " connected" << endl;
	cLog::get()->write("Joypad: model name "+ model + " detected", LOG_TYPE::L_INFO);  

	if(!conf.findEntry(model)) {
		model = "joy_default";
		//~ cout << "no config detected switching to default" << endl;
		cLog::get()->write("Joypad: no config detected switching to default", LOG_TYPE::L_WARNING);  
	} else {
		//~ cout << "config detected"<<endl;
		cLog::get()->write("Joypad: config detected", LOG_TYPE::L_INFO);  
	}

	for (int i = 0; i < nbrAxis; i++) {
		std::string actionStr = conf.getStr(model,"AXIS" + std::to_string(i));
		axisActions[i] = getAxisActionFromString(actionStr);
		axisDeadZone[i] = conf.getDouble(model,"AXIS" + std::to_string(i) + "_DEADZONE");
		axisSensitivity[i] = conf.getDouble(model,"AXIS" + std::to_string(i) + "_SENSITIVITY");
		axisIsStick[i] = conf.getBoolean(model,"AXIS" + std::to_string(i) + "_IS_STICK");
	}

	for (int i = 0; i < nbrButtons; i++) {
		std::string actionStr = conf.getStr(model,"BUTTON" + std::to_string(i));
		buttonActions[i] = getButtonActionFromString(actionStr, i);
	}

	for(int i = 0; i < nbrHats; i++) {
		std::string actionStr = conf.getStr(model,"HAT" + std::to_string(i) + "_DOWN");
		hatActions[i + (int)(hat_event::hat_down)] = getButtonActionFromString(actionStr, -1);

		actionStr = conf.getStr(model,"HAT" + std::to_string(i) + "_UP");
		hatActions[i + (int)(hat_event::hat_up)] = getButtonActionFromString(actionStr, -1);

		actionStr = conf.getStr(model,"HAT" + std::to_string(i) + "_LEFT");
		hatActions[i + (int)(hat_event::hat_left)] = getButtonActionFromString(actionStr, -1);

		actionStr = conf.getStr(model,"HAT" + std::to_string(i) + "_RIGHT");
		hatActions[i + (int)(hat_event::hat_right)] = getButtonActionFromString(actionStr, -1);
	}

	purge();

}

/*
 * Dispaches events to their respective handeling functions
 */
void JoypadController::handle(const SDL_Event &E) noexcept
{

	switch(E.type) {
		case SDL_JOYAXISMOTION :
			handleAxisEvent(E.jaxis);
			break;

		case SDL_JOYBUTTONUP:
			handleJoyButtonUp(E.jbutton);
			break;

		case SDL_JOYBUTTONDOWN:
			handleJoyButtonDown(E.jbutton);
			break;

		case SDL_JOYHATMOTION :
			handleJoyHat(E.jhat);
			break;
	}

}

/*
 * Performs actions for each axis if their values are outside of the deadzone
 */
void JoypadController::handleDeal() noexcept
{

	for (int i = 0; i < nbrAxis; i++) {

		if(axisIsStick[i]) {
			if (abs(axisValues[i])> axisDeadZone[i]) {
				joy_axis_action axis_action = axisActions[i];
				if(axis_action.action != nullptr)
					(ui->*axis_action.action)(((axisValues[i]>0)-(axisValues[i]<0))*(abs(axisValues[i])-axisDeadZone[i])/(axisSensitivity[i]-axisDeadZone[i]));
			}
		} else {
			if ((axisValues[i] + 32768.0) > axisDeadZone[i]) {
				joy_axis_action axis_action = axisActions[i];
				if(axis_action.action != nullptr)
					(ui->*axis_action.action)((axisValues[i] + 32768.0-axisDeadZone[i])/(axisSensitivity[i]-axisDeadZone[i]));
			}
		}
	}
}

void JoypadController::handleJoyButtonUp(const SDL_JoyButtonEvent &E) noexcept
{
	joy_button_action button_action = buttonActions[E.button];
	if(button_action.onReleaseAction != nullptr)
		(ui->*button_action.onReleaseAction)();
}

/*
 * Handles button presses
 */
void JoypadController::handleJoyButtonDown(const SDL_JoyButtonEvent &E) noexcept
{

	if(isCommand(E.button)) {
		for(std::string command : buttonCommand[E.button]) {
			std::cout << command << std::endl;
			ui->executeCommand(command);
		}
	} else {
		joy_button_action button_action = buttonActions[E.button];

		if(button_action.onPressAction != nullptr)
			(ui->*button_action.onPressAction)();
	}
}

void JoypadController::handleJoyHat(const SDL_JoyHatEvent &E) noexcept
{

	joy_button_action hat_action;

	switch(E.value) {
		case SDL_HAT_CENTERED:
			//we perform the onRelease action of every button for that hat
			for(int i = E.hat; i < E.hat + 4; i++) {
				hat_action = hatActions[i];
				if(hat_action.onReleaseAction != nullptr)
					(ui->*hat_action.onReleaseAction)();
			}
			break;

		case SDL_HAT_DOWN :
			hat_action = hatActions[E.hat + (int)(hat_event::hat_down)];
			if(hat_action.onPressAction != nullptr)
				(ui->*hat_action.onPressAction)();

			break;

		case SDL_HAT_LEFT :
			hat_action = hatActions[E.hat + (int)hat_event::hat_left];
			if(hat_action.onPressAction != nullptr)
				(ui->*hat_action.onPressAction)();

			break;
		case SDL_HAT_RIGHT :
			hat_action = hatActions[E.hat + (int)hat_event::hat_right];
			if(hat_action.onPressAction != nullptr)
				(ui->*hat_action.onPressAction)();

			break;
		case SDL_HAT_UP :
			hat_action = hatActions[E.hat + (int)hat_event::hat_up];
			if(hat_action.onPressAction != nullptr)
				(ui->*hat_action.onPressAction)();

			break;
	}
}

/*
* resets all controlls to their deault values
* default is 0 for axis and -32768 for triggers
*/
void JoypadController::purge() noexcept{
	for (int i = 0; i < nbrAxis; i++){
		axisValues[i] = axisIsStick[i] ? 0.0 : -32768.0;
	}
	
}

/*
 * Returns an axis action from a string
 * Used to parse the config.ini file
 */
joy_axis_action JoypadController::getAxisActionFromString(const std::string &actionStr) noexcept
{

	joy_axis_action axis_action;
	axis_action.action = nullptr;

	if(actionStr == "mouse_alt") {
		axis_action.action = &UI::moveMouseAlt;
		return axis_action;
	}

	if(actionStr == "mouse_az") {
		axis_action.action = &UI::moveMouseAz;
		return axis_action;
	}

	if(actionStr == "move_lat") {
		axis_action.action = &UI::moveLat;
		return axis_action;
	}

	if(actionStr == "move_lon") {
		axis_action.action = &UI::moveLon;
		return axis_action;
	}

	if(actionStr == "raise_height") {
		axis_action.action = &UI::raiseHeight;
		return axis_action;
	}

	if(actionStr == "lower_height") {
		axis_action.action = &UI::lowerHeight;
		return axis_action;
	}

	return axis_action;

}

/*
 * Returns a joy_button_action from a string
 * Used to parse the config.ini file
 * If the action is a command or a list of command it get added in a dictionnairy with its button number
 */
joy_button_action JoypadController::getButtonActionFromString(std::string &actionStr, int buttonNumber) noexcept
{

	joy_button_action action;

	action.onPressAction = nullptr;
	action.onReleaseAction = nullptr;

	if(actionStr[actionStr.size()-1] == ')') { // It's a command

		if(buttonNumber == -1)
			return action;

		actionStr = actionStr.substr(8);
		actionStr.pop_back();
		std::vector<std::string> commands_raw;
		split(actionStr, ",", commands_raw);

		std::vector<std::string> commands;

		for(std::string s : commands_raw) {
			s = s.substr(1);
			s.pop_back();
			commands.push_back(s);
		}

		buttonCommand.insert(std::pair<Uint8, std::vector<std::string>>(buttonNumber, commands));	//add the commands to call in dictionnary

		return action;
	}

	if(actionStr == "zoom_in") {
		action.onPressAction = &UI::zoomIn;
		action.onReleaseAction = &UI::stopZoomIn;
		return action;
	}

	if(actionStr == "mouse_rclick") {
		action.onPressAction = &UI::rightClick;
		return action;
	}

	if(actionStr == "mouse_lclick") {
		action.onPressAction = &UI::leftClick;
		return action;
	}

	if(actionStr == "zoom_out") {
		action.onPressAction = &UI::zoomOut;
		action.onReleaseAction = &UI::stopZoomOut;
		return action;
	}

	if(actionStr == "lower_height") {
		action.onPressAction = &UI::lowerHeight;
		action.onReleaseAction = &UI::stopLowerHeight;
		return action;
	}

	if(actionStr == "raise_height") {
		action.onPressAction = &UI::raiseHeight;
		action.onReleaseAction = &UI::stopRaiseHeight;
		return action;
	}

	if(actionStr == "speed_decrease") {
		action.onPressAction = &UI::speedDecrease;
		action.onReleaseAction = &UI::stopSpeedDecrease;
		return action;
	}

	if(actionStr == "speed_increase") {
		action.onPressAction = &UI::speedIncrease;
		action.onReleaseAction = &UI::stopSpeedIncrease;
		return action;
	}

	if(actionStr == "script_pause") {
		action.onPressAction = &UI::pauseScriptOrTimeRate;
		return action;
	}

	if(actionStr == "center_mouse") {
		action.onPressAction = &UI::centerMouse;
		return action;
	}

	if(actionStr == "turn_up") {
		action.onPressAction = &UI::turnUp;
		action.onReleaseAction = &UI::stopTurnUp;
		return action;
	}

	if(actionStr == "turn_down") {
		action.onPressAction = &UI::turnDown;
		action.onReleaseAction = &UI::stopTurnDown;
		return action;
	}

	if(actionStr == "turn_left") {
		action.onPressAction = &UI::turnLeft;
		action.onReleaseAction = &UI::stopTurnLeft;
		return action;
	}

	if(actionStr == "turn_right") {
		action.onPressAction = &UI::turnRight;
		action.onReleaseAction = &UI::stopTurnRight;
		return action;
	}

	return action;
}
