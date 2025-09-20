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
#include "EntityCore/Executor/AsyncLoaderMgr.hpp"

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
class FontFactory;

class AppCommandInterface: public NoCopy {

public:
	AppCommandInterface(std::shared_ptr<Core> core, std::shared_ptr<CoreLink> _coreLink, std::shared_ptr<CoreBackup> _coreBackup, App *_app, UI *_ui, std::shared_ptr<Media> _media, std::shared_ptr<FontFactory> _fontFactory);
	~AppCommandInterface();

	int terminateScript();
	void deleteVar();
	int executeCommand(const std::string &commandline);
	int executeCommand(const std::string &command, uint64_t &wait);

	void initInterfaces(std::shared_ptr<ScriptInterface> _scriptInterface, std::shared_ptr<SpaceDate> _spaceDate, std::shared_ptr<SaveScreenInterface> _saveScreenInterface);

	void setFlag(FLAG_NAMES flagName, FLAG_VALUES flag_value);
	void setTcp(ServerSocket* _tcp);
	bool isInterrupted();

protected:
	//all different command
	int commandAdd();
	int commandAudio();
	int commandModeJump();
	int commandBodyTrace();
	int commandBody();
	int commandCamera(uint64_t &wait);
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
	int commandDso3D();
	int commandDso2D();
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
	int commandDivide();
	int commandTangent();
	int commandTrunc();
	int commandSinus();
	int commandPersonal();
	int commandPersoneq();
	int commandPlanetScale();
	int commandPosition();
	int commandPrint();
	int commandRandom();
	int commandScript(uint64_t &wait);
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
	int commandTransition();
	int commandUncomment();
	int commandWait(uint64_t &wait);
	int commandZoom(uint64_t &wait);

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

	// external classes
	std::shared_ptr<Core> stcore;
	std::shared_ptr<CoreLink> coreLink;
	std::shared_ptr<CoreBackup> coreBackup;
	App *stapp;
	UI *ui;
	std::shared_ptr<Media> media;
	std::shared_ptr<ScriptInterface> scriptInterface;
	std::shared_ptr<SpaceDate> spaceDate;
	std::shared_ptr<SaveScreenInterface> saveScreenInterface;
	ServerSocket *tcp = nullptr;
	std::shared_ptr<FontFactory> fontFactory;

	std::unique_ptr<AppCommandInit> appInit;
	std::unique_ptr<AppCommandEval> appEval;

	std::string commandline;
	std::string command;
	stringHash_t args;
	int recordable;
	bool swapCommand;					// boolean which indicates if the instruction must be executed or not
	bool unskippable = false;			// set to true to force execution of the next command
	bool waitVideoCache = false;
	LoadPriority waitPriority = LoadPriority::NOW;
	std::unique_ptr<IfSwap> ifSwap; 	// management of multiple if statements
	std::string debug_message;			//!< for 'executeCommand' error details

	// transcription between the text and the associated command
	std::map<const std::string, SC_COMMAND> m_commands;
	std::map<SC_COMMAND, const std::string> m_commands_ToString;
	// transcription between the text and the associated flag
	std::map<const std::string, FLAG_NAMES> m_flags;
	std::map<FLAG_NAMES, const std::string> m_flags_ToString;
	// transcription between the text and the associated color
	std::map<const std::string, COLORCOMMAND_NAMES> m_color;
	std::map<COLORCOMMAND_NAMES, const std::string> m_color_ToString;
	// transcription between the text and the associated interface command
	std::map<const std::string, SCD_NAMES> m_set;
	std::map<SCD_NAMES, const std::string> m_set_ToString;
};

#endif // _APP_COMMAND_INTERFACE_H
