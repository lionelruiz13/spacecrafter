/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

/* This class handles parsing of a simple command syntax for scripting,
   UI components, network commands, etc.
*/

#ifndef _APP_COMMAND_INTERFACE_H_
#define _APP_COMMAND_INTERFACE_H_

#include <memory>
#include "tools/utility.hpp"
#include "base_command_interface.hpp"
#include "tools/no_copy.hpp"

class Core;
class CoreLink;
class CoreBackup;
class App;
class AppCommandInit;
class AppCommandEval;
class IfSwap;
class UI;
class ScriptInterface;
class SpaceDate;
class Media;
class SaveScreenInterface;
class ServerSocket;

class AppCommandInterface: public NoCopy {

public:
	AppCommandInterface(Core * core, CoreLink *_coreLink, CoreBackup* _coreBackup, App * app, UI* _ui, Media* _media);
	~AppCommandInterface();

	int executeCommand(const std::string &commandline);
	int executeCommand(const std::string &command, unsigned long int &wait);

	void initScriptInterface(ScriptInterface* _scriptInterface);
	void initSpaceDateInterface(SpaceDate* _spaceDate);
	void initSaveScreenInterface(SaveScreenInterface* _saveScreenInterface);

	void setFlag(FLAG_NAMES flagName, FLAG_VALUES flag_value);
	void setTcp(ServerSocket* _tcp);

protected:
	//all different command
	int commandAdd();
	int commandAudio();
	int commandBodyTrace();
	int commandBody();
	int commandCamera(unsigned long int &wait);
	int commandClear();
	int commandColor();
	int commandComment();
	int commandConfiguration();
	int commandConstellation();
	int commandDate();
	int commandDefine();
	int commandDeselect();
	int commandDomemasters();
	int commandDso();
	//int commandExternalMplayer();
	int commandExternalViewer();
	int commandFont();
	int commandFlag();
	int commandGet();
	int commandHeading();
	int commandIlluminate();
	int commandImage();
	int commandLandscape();
	int commandScreenFader();
	int commandLook();
	int commandMedia();
	int commandMeteors();
	int commandMoveto();
	int commandMultiply();
	int commandPersonal();
	int commandPersoneq();
	int commandPlanetScale();
	int commandPosition();
	int commandPrint();
	int commandRandom();
	int commandScript(unsigned long int &wait);
	int commandSearch();
	int commandSelect();
	int commandSet();
	int commandShutdown();
	int commandSkyCulture();
	int commandStarLines();
	int commandStruct();
	int commandSub();
	int commandSuntrace();
	int commandText();
	int commandTimerate();
	int commandUncomment();
	int commandWait(unsigned long int &wait);
	int commandZoom(unsigned long int &wait);

private:
	FLAG_VALUES convertStrToFlagValues(const std::string &value);
	int parseCommand(const std::string &command_line, std::string &command, stringHash_t &arguments);
	int evalCommandSet(const std::string& setName, const std::string& setValue);
	SCD_NAMES parseCommandSet(const std::string& setName);
	int executeCommandStatus();

	bool setFlag(FLAG_NAMES flagName, FLAG_VALUES flag_value, bool &newval);
	bool setFlag(const std::string &name, const std::string &value, bool &newval);
	double evalDouble(const std::string &var);
	int evalInt (const std::string &var);
	std::string evalString (const std::string &var);

	Core * stcore = nullptr;
	CoreLink *coreLink = nullptr;
	CoreBackup* coreBackup = nullptr;
	App * stapp = nullptr;
	UI* ui = nullptr;
	Media* media = nullptr;
	ScriptInterface* scriptInterface = nullptr;
	SpaceDate* spaceDate = nullptr;
	SaveScreenInterface* saveScreenInterface=nullptr;
	ServerSocket *tcp = nullptr;
	AppCommandInit *appInit = nullptr;
	AppCommandEval *appEval = nullptr;

	std::string commandline;
	std::string command;
	stringHash_t args;
	int recordable;
	bool swapCommand;					// boolean qui indique si l'instruction doit etre exécutée ou pas
	std::unique_ptr<IfSwap> ifSwap; 	// gestionnnaire des if multiples
	std::string debug_message;			//!< for 'executeCommand' error details

	// transcription entre le texte et la commande associée
	std::map<const std::string, SC_COMMAND> m_commands;
	std::map<SC_COMMAND, const std::string> m_commands_ToString;
	// transcription entre le texte et le flag associé
	std::map<const std::string, FLAG_NAMES> m_flags;
	std::map<FLAG_NAMES, const std::string> m_flags_ToString;
	// transcription entre le texte et la couleur associée
	std::map<const std::string, COLORCOMMAND_NAMES> m_color;
	std::map<COLORCOMMAND_NAMES, const std::string> m_color_ToString;
	// transcription entre le texte et la commande interface associé
	std::map<const std::string, SCD_NAMES> m_set;
	std::map<SCD_NAMES, const std::string> m_set_ToString;
};

#endif // _APP_COMMAND_INTERFACE_H
