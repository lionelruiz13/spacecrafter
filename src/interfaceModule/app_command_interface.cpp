/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005-2006 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>

#include "app_command_color.hpp"
#include "appModule/app.hpp"
#include "appModule/save_screen_interface.hpp"
#include "coreModule/core.hpp"
#include "coreModule/coreLink.hpp"
#include "coreModule/backup_mgr.hpp"
#include "eventModule/event_recorder.hpp"
#include "eventModule/EventScreenFader.hpp"
#include "interfaceModule/app_command_interface.hpp"
#include "interfaceModule/app_command_init.hpp"
#include "interfaceModule/app_command_eval.hpp"
#include "interfaceModule/script_interface.hpp"
#include "mediaModule/media.hpp"
#include "tools/app_settings.hpp"
#include "tools/call_system.hpp"
#include "tools/file_path.hpp"
#include "tools/io.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include "tools/utility.hpp"
#include "uiModule/ui.hpp"


AppCommandInterface::AppCommandInterface(Core * core, CoreLink *_coreLink, CoreBackup* _coreBackup, App * app, UI* _ui,  Media* _media)
{
	stcore = core;
	coreLink = _coreLink;
	coreBackup = _coreBackup;
	stapp = app;
	media = _media;
	ui = _ui;
	swapCommand = false;
	swapIfCommand = false;
	appEval = new AppCommandEval(coreLink);
	appInit = new AppCommandInit();
	appInit->initialiseCommandsName(m_commands, m_commands_ToString);
	appInit->initialiseFlagsName(m_flags, m_flags_ToString);
	appInit->initialiseColorCommand(m_color, m_color_ToString);
	appInit->initialiseSetCommand(m_set, m_set_ToString);
}

void AppCommandInterface::initScriptInterface(ScriptInterface* _scriptInterface) {
	scriptInterface = _scriptInterface;
}

void AppCommandInterface::initSpaceDateInterface(SpaceDate* _spaceDate) {
	spaceDate = _spaceDate;
}

void AppCommandInterface::initSaveScreenInterface(SaveScreenInterface* _saveScreenInterface) {
	saveScreenInterface = _saveScreenInterface;
}

AppCommandInterface::~AppCommandInterface()
{
	m_commands.clear();
	m_flags.clear();
	m_color.clear();
	m_set.clear();
}

bool AppCommandInterface::isBoolean(const std::string &a)
{
	if ( isTrue(a) || isFalse(a) )
		return true;
	else
		return false;
}

bool AppCommandInterface::isTrue(const std::string &a)
{
	if (a==W_TRUE || a =="1" || a==W_ON )
		return true;
	else
		return false;
}

bool AppCommandInterface::isFalse(const std::string &a)
{
	if (a==W_FALSE || a =="0" || a==W_OFF )
		return true;
	else
		return false;
}

void AppCommandInterface::setTcp(ServerSocket* _tcp)
{
	tcp=_tcp;
}


int AppCommandInterface::parseCommand(const std::string &command_line, std::string &command, stringHash_t &arguments)
{
  	std::string str = command_line;

	// transformation des débuts de chaines de caratères en supprimant espaces et tabulations en début de chaine
	while (str[0]==' ' || str[0]=='\t') {
        str.erase(0,1); 
		//std::cout << str << std::endl;
	}

	// transformation des chaines utilisateurs de la forme " text" en "text"
  	std::size_t found = str.find(" \" ");
  	while(found!=std::string::npos) {
		//std::cout << "Modify: " << str << '\n';
  		str.erase(found+2,1);
  		//std::cout << "To    : " << str << '\n';
		found = str.find(" \" ");
  	}
  
	std::istringstream commandstr( str );
	std::string key, value;
	char nextc;

	commandstr >> command;
	transform(command.begin(), command.end(), command.begin(), ::tolower);

	while (commandstr >> key >> value ) {
		if (value[0] == '"') {
			// pull in all text inside quotes
			if (value[value.length()-1] == '"') {
				// one word in quotes
				value = value.substr(1, value.length() -2 );
			} else {
				// multiple words in quotes
				value = value.substr(1, value.length() -1 );

				while (1) {
					nextc = commandstr.get();
					if ( nextc == '"' || !commandstr.good()) break;
					value.push_back( nextc );
				}
			}
		}
		transform(key.begin(), key.end(), key.begin(), ::tolower);
		arguments[key] = value;
	}

	#ifdef PARSE_DEBUG
	cLog::get()->write("Command: " + command + "Argument hash:", LOG_TYPE::L_DEBUG);
	for ( stringHashIter_t iter = arguments.begin(); iter != arguments.end(); ++iter ) {
		cLog::get()->write("\t" + iter->first + " : " + iter->second, LOG_TYPE::L_DEBUG);
	}
	#endif
	return 1;  // no error checking yet
}


int AppCommandInterface::executeCommand(const std::string &commandline )
{
	unsigned long int delay;
	return executeCommand(commandline, delay);
}

//! @brief called by script executors and transform a std::string to instruction
int AppCommandInterface::executeCommand(const std::string &_commandline, unsigned long int &wait)
{
	recordable = 1;  // true if command should be recorded (if recording)
	debug_message.clear(); // initialise to empty
	wait = 0;  // default, no wait between commands
	commandline = _commandline;

	command.clear(); // = ""; //vide l'ancienne valeur de args
	args.clear(); //vide les anciennes valeurs de args //TODO A VERIFIER

	// on découpe toute la ligne en CMD {ARG1,VALUE1} {ARG2,VALUE2} ...
	parseCommand(commandline, command, args);

	FilePath::fixScriptPath(scriptInterface->getScriptPath());

	// If command is empty then don't bother checking all these cases
	if( command.length() < 1  && command == "")
		return 0;

	cLog::get()->write("Execute_command " + commandline, LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);

	//                                                 //
	// application specific logic to run each command  //
	//                                                 //
	if (command =="comment"  )
		return commandComment();

	if (command =="uncomment")
		return commandUncomment();

	if (command =="struct")
		return commandStruct();

	if (swapCommand== true || swapIfCommand==true) {	 // on n'execute pas les commandes qui suivent
		cLog::get()->write("cette commande n'a pas été exécutée " + commandline, LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);  //A traduire
		return 1;
	}

	auto m_commands_it = m_commands.find(command);
	if (m_commands_it == m_commands.end()) {
		//~ cout <<"error command "<< command << endl;
		debug_message = _("Unrecognized or malformed command name");
		cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
		appInit->searchSimilarCommand(command);
		return 0;
	}

	switch(m_commands_it->second) {
		case SC_COMMAND::SC_ADD : 	return commandAdd(); break;
		case SC_COMMAND::SC_AUDIO : 	return commandAudio(); break;
		case SC_COMMAND::SC_BODY_TRACE :	return commandBodyTrace();  break;
		case SC_COMMAND::SC_BODY :	return commandBody(); break;
		case SC_COMMAND::SC_CAMERA :	return commandCamera(wait); break;
		case SC_COMMAND::SC_CLEAR :	return commandClear(); break;
		case SC_COMMAND::SC_COLOR :	return commandColor(); break;
		case SC_COMMAND::SC_CONFIGURATION :	return commandConfiguration(); break;
		case SC_COMMAND::SC_CONSTELLATION :	return commandConstellation(); break;
		case SC_COMMAND::SC_DATE :	return commandDate(); break;
		case SC_COMMAND::SC_DEFINE :	return commandDefine(); break;
		case SC_COMMAND::SC_DESELECT :	return commandDeselect(); break;
		case SC_COMMAND::SC_DOMEMASTERS :	return commandDomemasters(); break;
		case SC_COMMAND::SC_DSO :	return commandDso(); break;
	//	case SC_COMMAND::SC_EXERNASC_MPLAYER :	return commandExternalMplayer(); break;
		case SC_COMMAND::SC_EXTERNASC_VIEWER :	return commandExternalViewer(); break;
		case SC_COMMAND::SC_FLAG :	return commandFlag(); break;
		case SC_COMMAND::SC_FONT :	return commandFont(); break;
		case SC_COMMAND::SC_GET :	return commandGet(); break;
		case SC_COMMAND::SC_HEADING :	return commandHeading(); break;
		case SC_COMMAND::SC_ILLUMINATE :	return commandIlluminate(); break;
		case SC_COMMAND::SC_IMAGE :	return commandImage();  break;
		case SC_COMMAND::SC_LANDSCAPE :	return commandLandscape(); break;
		case SC_COMMAND::SC_LOOK :	return commandLook(); break;
		case SC_COMMAND::SC_MEDIA :	return commandMedia(); break;
		case SC_COMMAND::SC_METEORS :	return commandMeteors(); break;
		case SC_COMMAND::SC_MOVETO :	return commandMoveto(); break;
		case SC_COMMAND::SC_MULTIPLY :	return commandMultiply(); break;
		case SC_COMMAND::SC_PERSONAL :	return commandPersonal(); break;
		case SC_COMMAND::SC_PERSONEQ :	return commandPersoneq(); break;
		case SC_COMMAND::SC_PLANET_SCALE :	return commandPlanetScale(); break;
		case SC_COMMAND::SC_POSITION :	return commandPosition(); break;
		case SC_COMMAND::SC_PRINT :	return commandPrint(); break;
		case SC_COMMAND::SC_RANDOM :	return commandRandom(); break;
		case SC_COMMAND::SC_SCRIPT :	return commandScript(wait); break;
		case SC_COMMAND::SC_SEARCH :	return commandSearch(); break;
		case SC_COMMAND::SC_SELECT :	return commandSelect(); break;
		case SC_COMMAND::SC_SET :	return commandSet(); break;
		case SC_COMMAND::SC_SHUTDOWN :	return commandShutdown(); break;
		case SC_COMMAND::SC_SKY_CULTURE :	return commandSkyCulture(); break;
		case SC_COMMAND::SC_STAR_LINES :	return commandStarLines(); break;
		case SC_COMMAND::SC_SUB : 	return commandSub(); break;
		case SC_COMMAND::SC_SUNTRACE :	return commandSuntrace(); break;
		case SC_COMMAND::SC_TEXT :	return commandText(); break;
		case SC_COMMAND::SC_TIMERATE :	return commandTimerate(); break;
		case SC_COMMAND::SC_WAIT :	return commandWait(wait); break;
		case SC_COMMAND::SC_ZOOMR :	return commandZoom(wait); break;
		default:	break;
	}
	return 1;
}

//! set flags
//! @param newval is new value of flag changed
bool AppCommandInterface::setFlag(const std::string &name, const std::string &value, bool &newval)
{
	//test name if exist and get his value
	auto m_flag_it = m_flags.find(name);
	if (m_flag_it == m_flags.end()) {
		//~ cout <<"error command "<< command << endl;
		//debug_message = _("Unrecognized or malformed flag name");
		cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
		appInit->searchSimilarFlag(name);
		return false;
	}

	//fix newval dans les cas ou il vaut on ou off, toggle sera fixé après si besoin
	FLAG_VALUES flag_value = convertStrToFlagValues(value);

	return this->setFlag(m_flag_it->second, flag_value, newval);
}

bool AppCommandInterface::setFlag(FLAG_NAMES flagName, FLAG_VALUES flag_value)
{
	bool val;
	if (setFlag( flagName, flag_value, val) == false) {
		debug_message = _("Unrecognized or malformed flag argument");
		return false;
	}

	// @TODO reconstruct commandline to avoid passing std::string _commandline
	auto m_flags_ToString_it = m_flags_ToString.find(flagName);
	if (m_flags_ToString_it != m_flags_ToString.end()) {
		commandline = "set " + m_flags_ToString_it->second + " " + std::to_string(val);
	}

	return true;
}

bool AppCommandInterface::setFlag(FLAG_NAMES flagName, FLAG_VALUES flag_value, bool &newval)
{
	if (flag_value==FLAG_VALUES::FV_ON)
		newval = true;
	else if (flag_value==FLAG_VALUES::FV_OFF)
		newval= false;

	switch(flagName) {
		case FLAG_NAMES::FN_ANTIALIAS_LINES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				stapp->toggle(APP_FLAG::ANTIALIAS);
			else
				stapp->flag(APP_FLAG::ANTIALIAS,newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_DRAWING :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->constellationGetFlagLines();

			coreLink->constellationSetFlagLines(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->constellationGetFlagNames();

			coreLink->constellationSetFlagNames(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_ART :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->constellationGetFlagArt();

			coreLink->constellationSetFlagArt(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->constellationGetFlagBoundaries();

			coreLink->constellationSetFlagBoundaries(newval);
			break;

		case FLAG_NAMES::FN_CONSTELLATION_PICK :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->constellationGetFlagIsolateSelected();

			coreLink->constellationSetFlagIsolateSelected(newval);
			break;

		case FLAG_NAMES::FN_STAR_TWINKLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starGetFlagTwinkle();

			coreLink->starSetFlagTwinkle(newval);
			break;

		case FLAG_NAMES::FN_SHOW_TUI_DATETIME :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				ui->toggle(UI_FLAG::SHOW_TUIDATETIME);
			else
				ui->flag(UI_FLAG::SHOW_TUIDATETIME, newval);
			break;

		case FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				ui->toggle(UI_FLAG::SHOW_TUISHORTOBJ_INFO);
			else
				ui->flag(UI_FLAG::SHOW_TUISHORTOBJ_INFO, newval);
			break;

		case FLAG_NAMES::FN_MANUAL_ZOOM :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagManualAutoZoom();

			stcore->setFlagManualAutoZoom(newval);
			break;

		case FLAG_NAMES::FN_NAVIGATION :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagNav();

			stcore->setFlagNav(newval);
			break;

		case FLAG_NAMES::FN_LIGHT_TRAVEL_TIME :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->getFlagLightTravelTime();

			coreLink->setFlagLightTravelTime(newval);
			break;

		case FLAG_NAMES::FN_FOG :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->fogGetFlag();

			coreLink->fogSetFlag(newval);
			break;

		case FLAG_NAMES::FN_ATMOSPHERE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->atmosphereGetFlag();

			if (!newval) coreLink->fogSetFlag(false); // turn off fog with atmosphere
			coreLink->starSetFlagTwinkle(newval); // twinkle stars depending on atmosphere activated
			coreLink->atmosphereSetFlag(newval);
			break;

		case FLAG_NAMES::FN_AZIMUTHAL_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyGridMgrGetFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL);
				coreLink->skyGridMgrFlipFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL);
			} else
				coreLink->skyGridMgrSetFlagShow(SKYGRID_TYPE::GRID_ALTAZIMUTAL, newval);
			break;

		case FLAG_NAMES::FN_EQUATORIAL_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyGridMgrGetFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL);
				coreLink->skyGridMgrFlipFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL);
			} else
				coreLink->skyGridMgrSetFlagShow(SKYGRID_TYPE::GRID_EQUATORIAL, newval);
			break;

		case FLAG_NAMES::FN_ECLIPTIC_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyGridMgrGetFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC);
				coreLink->skyGridMgrFlipFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC);
			} else
				coreLink->skyGridMgrSetFlagShow(SKYGRID_TYPE::GRID_ECLIPTIC, newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_GRID :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyGridMgrGetFlagShow(SKYGRID_TYPE::GRID_GALACTIC);
				coreLink->skyGridMgrFlipFlagShow(SKYGRID_TYPE::GRID_GALACTIC);
			} else
				coreLink->skyGridMgrSetFlagShow(SKYGRID_TYPE::GRID_GALACTIC, newval);
			break;

		case FLAG_NAMES::FN_EQUATOR_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_EQUATOR);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_EQUATOR);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_EQUATOR, newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, newval);
			break;

		case FLAG_NAMES::FN_ECLIPTIC_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC, newval);
			break;

		case FLAG_NAMES::FN_PRECESSION_CIRCLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_PRECESSION);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_PRECESSION);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_PRECESSION, newval);
			break;

		case FLAG_NAMES::FN_TROPIC_LINES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_TROPIC);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_TROPIC);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_TROPIC, newval);
			break;

		case FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_CIRCUMPOLAR, newval);
			break;

		case FLAG_NAMES::FN_MERIDIAN_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_MERIDIAN);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_MERIDIAN);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_MERIDIAN, newval);
			break;

		case FLAG_NAMES::FN_ZENITH_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ZENITH);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ZENITH);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ZENITH, newval);
			break;

		case FLAG_NAMES::FN_POLAR_CIRCLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_CIRCLE_POLAR, newval);
			break;

		case FLAG_NAMES::FN_POLAR_POINT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_POINT_POLAR, newval);
			break;

		case FLAG_NAMES::FN_ECLIPTIC_CENTER :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ECLIPTIC_POLE, newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_POLE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_GALACTIC_POLE, newval);
			break;

		case FLAG_NAMES::FN_GALACTIC_CENTER :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_GALACTIC_CENTER, newval);
			break;

		case FLAG_NAMES::FN_VERNAL_POINTS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_VERNAL);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_VERNAL);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_VERNAL, newval);
			break;

		case FLAG_NAMES::FN_ANALEMMA_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ANALEMMALINE, newval);
			break;

		case FLAG_NAMES::FN_ANALEMMA :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ANALEMMA);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ANALEMMA);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ANALEMMA, newval);
			break;

		case FLAG_NAMES::FN_ARIES_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ARIES);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ARIES);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ARIES, newval);
			break;

		case FLAG_NAMES::FN_ZODIAC :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_ZODIAC);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_ZODIAC);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_ZODIAC, newval);
			break;

		case FLAG_NAMES::FN_GREENWICH_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_GREENWICH);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_GREENWICH);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_GREENWICH, newval);
			break;

		case FLAG_NAMES::FN_VERTICAL_LINE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = coreLink->skyLineMgrGetFlagShow(SKYLINE_TYPE::LINE_VERTICAL);
				coreLink->skyLineMgrFlipFlagShow(SKYLINE_TYPE::LINE_VERTICAL);
			} else
				coreLink->skyLineMgrSetFlagShow(SKYLINE_TYPE::LINE_VERTICAL, newval);
			break;

		case FLAG_NAMES::FN_PERSONAL :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_PERSONAL);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_PERSONAL);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_PERSONAL, newval);
			break;

		case FLAG_NAMES::FN_PERSONEQ :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_PERSONEQ);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_PERSONEQ);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_PERSONEQ, newval);
			break;

		case FLAG_NAMES::FN_NAUTICAL :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_NAUTICAL);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_NAUTICAL);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_NAUTICAL, newval);
			break;

		case FLAG_NAMES::FN_NAUTICEQ :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_NAUTICEQ);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_NAUTICEQ);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_NAUTICEQ, newval);
			break;
			
		case FLAG_NAMES::FN_OBJCOORD :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_OBJCOORDS);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_OBJCOORDS);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_OBJCOORDS, newval);
			break;

		case FLAG_NAMES::FN_MOUSECOORD :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_MOUSECOORDS);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_MOUSECOORDS);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_MOUSECOORDS, newval);
			break;
		
		case FLAG_NAMES::FN_ANG_DIST :
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_ANGDIST);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_ANGDIST);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_ANGDIST, newval);
			break;

		case FLAG_NAMES::FN_LOXODROMY:
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_LOXODROMY);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_LOXODROMY);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_LOXODROMY, newval);
			break;

		case FLAG_NAMES::FN_ORTHODROMY:
			if (flag_value==FLAG_VALUES::FV_TOGGLE) {
				newval = !coreLink->skyDisplayMgrGetFlag(SKYDISPLAY_NAME::SKY_ORTHODROMY);
				coreLink->skyDisplayMgrFlipFlag(SKYDISPLAY_NAME::SKY_ORTHODROMY);
			} else
				coreLink->skyDisplayMgrSetFlag(SKYDISPLAY_NAME::SKY_ORTHODROMY, newval);
			break;

		case FLAG_NAMES::FN_CARDINAL_POINTS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->cardinalsPointsGetFlag();

			coreLink->cardinalsPointsSetFlag(newval);
			break;

		case FLAG_NAMES::FN_CLOUDS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->getFlagClouds();

			coreLink->setFlagClouds(newval);
			break;

		case FLAG_NAMES::FN_MOON_SCALED :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->getFlagMoonScaled();

			coreLink->setFlagMoonScaled(newval);
			break;

		case FLAG_NAMES::FN_SUN_SCALED :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->getFlagSunScaled();

			coreLink->setFlagSunScaled(newval);
			break;

		case FLAG_NAMES::FN_LANDSCAPE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->landscapeGetFlag();

			coreLink->landscapeSetFlag(newval);
			break;

		case FLAG_NAMES::FN_STARS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starGetFlag();

			coreLink->starSetFlag(newval);
			break;

		case FLAG_NAMES::FN_STAR_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starGetFlagName();

			coreLink->starSetFlagName(newval);
			break;

		case FLAG_NAMES::FN_STAR_PICK :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starGetFlagIsolateSelected();

			coreLink->starSetFlagIsolateSelected(newval);
			break;

		case FLAG_NAMES::FN_PLANETS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlag();

			coreLink->planetsSetFlag(newval);
			break;

		case FLAG_NAMES::FN_PLANET_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlagHints();

			coreLink->planetsSetFlagHints(newval);
			if (coreLink->planetsGetFlagHints()) coreLink->planetsSetFlag(true); // for safety if script turns planets off
			break;

		case FLAG_NAMES::FN_PLANET_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlagOrbits() && !coreLink->satellitesGetFlagOrbits();

			coreLink->planetsSetFlagOrbits(newval);
			coreLink->satellitesSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_PLANETS_AXIS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlagAxis();

			coreLink->planetsSetFlagAxis(newval);
			break;

		case FLAG_NAMES::FN_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlagOrbits() && !coreLink->satellitesGetFlagOrbits();

			coreLink->planetsSetFlagOrbits(newval);
			coreLink->satellitesSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_PLANETS_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlagOrbits();

			coreLink->planetsSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_SATELLITES_ORBITS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->satellitesGetFlagOrbits();

			coreLink->satellitesSetFlagOrbits(newval);
			break;

		case FLAG_NAMES::FN_NEBULAE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->nebulaGetFlag();

			coreLink->nebulaSetFlag(newval);
			break;

		case FLAG_NAMES::FN_NEBULA_HINTS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->nebulaGetFlagHints();

			coreLink->nebulaSetFlagHints(newval);
			break;

		case FLAG_NAMES::FN_DSO_PICTOGRAMS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getDsoPictograms();

			stcore->setDsoPictograms(newval);
			break;

		case FLAG_NAMES::FN_NEBULA_NAMES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->nebulaGetFlagNames();

			if (newval) coreLink->nebulaSetFlagNames(true); // make sure visible
			coreLink->nebulaSetFlagNames(newval);
			break;

		case FLAG_NAMES::FN_MILKY_WAY :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->milkyWayGetFlag();

			coreLink->milkyWaySetFlag(newval);
			break;

		case FLAG_NAMES::FN_ZODIAC_LIGHT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->milkyWayGetFlagZodiacal();

			coreLink->milkyWaySetFlagZodiacal(newval);
			break;

		case FLAG_NAMES::FN_BRIGHT_NEBULAE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->nebulaGetFlagBright();

			coreLink->nebulaSetFlagBright(newval);
			break;

		case FLAG_NAMES::FN_OBJECT_TRAILS :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->planetsGetFlagTrails();

			coreLink->planetsSetFlagTrails(newval);
			break;

		case FLAG_NAMES::FN_TRACK_OBJECT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagTracking();

			stcore->setFlagTracking(newval);
			break;

		case FLAG_NAMES::FN_SCRIPT_GUI_DEBUG :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !cLog::get()->getDebug();

			cLog::get()->setDebug(newval);
			break;

		case FLAG_NAMES::FN_LOCK_SKY_POSITION :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !stcore->getFlagLockSkyPosition();

			stcore->setFlagLockSkyPosition(newval);
			break;

		case FLAG_NAMES::FN_SHOW_LATLON :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				ui->toggle(UI_FLAG::SHOW_LATLON);
			else
				ui->flag(UI_FLAG::SHOW_LATLON, newval);
			break;

		case FLAG_NAMES::FN_OORT :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->oortGetFlagShow();

			coreLink->oortSetFlagShow(newval);
			break;

		case FLAG_NAMES::FN_TULLY :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->tullyGetFlagShow();

			coreLink->tullySetFlagShow(newval);
			break;

		case FLAG_NAMES::FN_BODY_TRACE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->bodyTraceGetFlag();

			coreLink->bodyTraceSetFlag(newval);
			break;

		case FLAG_NAMES::FN_COLOR_INVERSE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				stapp->toggle(APP_FLAG::COLOR_INVERSE);
			else
				stapp->flag(APP_FLAG::COLOR_INVERSE, newval);
			break;

		case FLAG_NAMES::FN_STARS_TRACE :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starGetTraceFlag();

			coreLink->starSetTraceFlag(newval);
			break;

		case FLAG_NAMES::FN_STAR_LINES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starLinesGetFlag();

			coreLink->starLinesSetFlag(newval);
			break;

		case FLAG_NAMES::FN_STAR_LINES_SELECTED :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->starLinesSelectedGetFlag();

			coreLink->starLinesSelectedSetFlag(newval);
			break;

		case FLAG_NAMES::FN_SATELLITES :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->hideSatellitesFlag();

			coreLink->setHideSatellites(newval);
			break;
		case FLAG_NAMES::FN_ATMOSPHERIC_REFRACTION :
			if (flag_value==FLAG_VALUES::FV_TOGGLE)
				newval = !coreLink->atmosphericRefractionGetFlag();

			coreLink->atmosphericRefractionSetFlag(newval);
			break;
		
		default:
			cLog::get()->write("no effect with unknown case ",LOG_TYPE::L_DEBUG);
			break;
	}
	return true; // flag was found and updated
}

FLAG_VALUES AppCommandInterface::convertStrToFlagValues(const std::string &value)
{
	if (value == W_TOGGLE) return FLAG_VALUES::FV_TOGGLE;
	else if (isTrue(value)) return FLAG_VALUES::FV_ON;
	else
		return FLAG_VALUES::FV_OFF;
}


int AppCommandInterface::executeCommandStatus()
{
	if (debug_message.empty()) {
		// if recording commands, do that now
		if (recordable) scriptInterface->recordCommand(commandline);
		//    cout << commandline << endl;
		//cLog::get()->write( "have execute: " + commandline ,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
		return true;
	} else {
		//cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );

		//std::stringstream oss;
		//oss << "Could not execute: " << commandline /*<< std::endl*/ << debug_message;
		cLog::get()->write( "Could not execute: " + commandline ,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
		cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
		//std::cerr << oss.str() << std::endl;
		return false;
	}
}


int AppCommandInterface::commandFlag()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {
		bool val;
		if (setFlag( args.begin()->first, args.begin()->second, val) == false)
			debug_message = _("Unrecognized or malformed flag argument");

		// rewrite command for recording so that actual state is known (rather than W_TOGGLE)
		if (args.begin()->second == W_TOGGLE) {
			std::ostringstream oss;
			oss << command << " " << args.begin()->first << " " << val;
			commandline = oss.str();
		}
	} else
		debug_message = _("Unrecognized or malformed command_flag behaviour");

	return executeCommandStatus();
}


int AppCommandInterface::commandGet()
{
	std::string argStatus = args[W_STATUS];
	if (!tcp) {
		cLog::get()->write("No tcp : i can't send ", LOG_TYPE::L_WARNING);
		return executeCommandStatus();
	}

	if (!argStatus.empty()) {
		if (argStatus == W_POSITION) {
			tcp->setOutput(coreLink->tcpGetPosition());
		} else if (argStatus == W_PLANET_P) {
			std::string tmp = coreLink->getPlanetsPosition();
			if (tmp.empty())
				tmp = "NPF";	
			tcp->setOutput(tmp);
		} else if (argStatus == W_CONSTELLATION) {
			tcp->setOutput(coreLink->getConstellationSelectedShortName());
		} else if (argStatus == W_OBJECT) {
			std::string tmp = stcore->getSelectedObjectInfo();
			if (tmp.empty())
				tmp = "EOL";
			tcp->setOutput(tmp);	
		} else
			debug_message = _("command 'get': unknown status value");
		return executeCommandStatus();
	} else
		debug_message = _("command 'get': unknown argument");

	return executeCommandStatus();
}

int AppCommandInterface::commandSearch()
{
	std::string argName = args[W_NAME];
	std::string argMaxObject = args[W_MAX_OBJECT];
	if (!argName.empty()) {
		std::string toSend;
		if (!argMaxObject.empty()) {
			toSend=stcore->getListMatchingObjects(argName, evalInt(argMaxObject));
		} else {
			toSend=stcore->getListMatchingObjects(argName);
		}
		if (toSend.empty())
			toSend="NOF";
		if (tcp)
			tcp->setOutput(toSend);
	} else
		debug_message = _("command 'search' : missing name argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandPlanetScale()
{
	std::string argName = args[W_NAME];
	std::string argScale = args[W_SCALE];
	if (!argName.empty() && !argScale.empty()) {
		coreLink->planetSetSizeScale(argName, evalDouble(argScale));
	} else
		debug_message = _("command 'planet_scale' : missing name or scale argument");

	return executeCommandStatus();
}

int AppCommandInterface::commandWait(unsigned long int &wait)
{
	if ( args[W_DURATION]!="") {
		float fdelay = evalDouble(args[W_DURATION]);
		if (fdelay > 0) wait = (int)(fdelay*1000);
	} else {
		debug_message = _("command_'wait' : unrecognized or malformed argument name.");
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandPersonal()
{
	std::string argAction = args[W_ACTION];
	if (!argAction.empty()) {
		if (argAction == W_LOAD) {
			std::string fileName=args[W_FILENAME];
			if (!fileName.empty())
				fileName = "personal.txt";
			if ( !Utility::isAbsolute(fileName))
				fileName = scriptInterface->getScriptPath() + fileName;
			coreLink->skyDisplayMgrLoadData(SKYDISPLAY_NAME::SKY_PERSONAL, fileName);
			return executeCommandStatus();
		}
		if (argAction ==  ACP_CN_CLEAR) {
			coreLink->skyDisplayMgrClear(SKYDISPLAY_NAME::SKY_PERSONAL);
			return executeCommandStatus();
		}
		debug_message = "command_personal: Unknown 'action' value";
		return executeCommandStatus();
	}
	if (!args[W_XY].empty()) {
		coreLink->skyDisplayMgrLoadString(SKYDISPLAY_NAME::SKY_PERSONAL, args[W_XY]);
		return executeCommandStatus();
	}
	debug_message = "command_'personal' : unrecognized or malformed argument";

	return executeCommandStatus();
}

int AppCommandInterface::commandDso()
{
	std::string argAction = args[W_ACTION];
	std::string argPath = args[W_PATH];
	std::string argName = args[W_NAME];

	if (!argAction.empty()) {
		if (argAction== W_LOAD) {
			std::string path;
			if (!argPath.empty())
				path = argPath;
			else
				path = scriptInterface->getScriptPath() + argPath;

			bool status = stcore->loadNebula(evalDouble(args[W_RA]), evalDouble(args[W_DE]), evalDouble(args[W_MAGNITUDE]),
			                                evalDouble(args[W_ANGULAR_S]), evalDouble(args[W_ROTATION]), argName,
			                                path + args[W_FILENAME], args[W_CREDIT], evalDouble(args[W_TEXTURE]),
			                                evalDouble(args[W_DISTANCE]),args[W_CONSTELLATION], args[W_TYPE]);
			if (status==false)
				debug_message = "Error loading nebula.";
			return executeCommandStatus();
		}

		if (argAction == W_DROP && !argName.empty() ) {
			// Delete an existing nebulae, but only if was added by a script!
			stcore->removeNebula(argName);
			return executeCommandStatus();
		}

		if (argAction ==   ACP_CN_CLEAR) {
			// drop all nebulae that are not in the original config file
			stcore->removeSupplementalNebulae();
			return executeCommandStatus();
		}

		debug_message = _("Command 'dso': unknown action value");
		return executeCommandStatus();
	}

	std::string argHidden = args[W_HIDDEN];
	if ( !argHidden.empty() ) {
		std::string argType = args[W_TYPE];
		if (!argType.empty() ) {
			if (argType == W_ALL)
				if (isTrue(argHidden)) coreLink->dsoHideAll();
				else
					coreLink->dsoShowAll();
			else
				coreLink->dsoSelectType(isTrue(argHidden),argType);

			return executeCommandStatus();
		}

		std::string argConstellation = args[W_CONSTELLATION];
		if (!argConstellation.empty()) {
			if (argConstellation == W_ALL)
				if (isTrue(argHidden)) coreLink->dsoHideAll();
				else
					coreLink->dsoShowAll();
			else
				coreLink->dsoSelectConstellation(isTrue(argHidden),argConstellation);
			return executeCommandStatus();
		}

		if ( !argName.empty()  ) {
			coreLink->dsoSelectName(argName, isTrue(argHidden));
			return executeCommandStatus();
		}

		debug_message = _("Command 'dso': case hidden unknown argument");
		return executeCommandStatus();
	}

	debug_message = _("command 'dso' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandPersoneq()
{
	std::string argAction = args[W_ACTION];
	if ( !argAction.empty()) {
		if (argAction== W_LOAD) {
			std::string fileName=args[W_FILENAME];
			if (fileName.empty())
				fileName = "personeq.txt";
			if ( !Utility::isAbsolute(fileName))
				fileName = scriptInterface->getScriptPath() + fileName;
			coreLink->skyDisplayMgrLoadData(SKYDISPLAY_NAME::SKY_PERSONEQ, fileName);
			return executeCommandStatus();
		}
		if (argAction==  ACP_CN_CLEAR) {
			coreLink->skyDisplayMgrClear(SKYDISPLAY_NAME::SKY_PERSONEQ);
			return executeCommandStatus();
		}
		debug_message = "command_personeq: Unknown 'action' value";
		return executeCommandStatus();
	}
	if (!args[ W_XY].empty()) {
		coreLink->skyDisplayMgrLoadString(SKYDISPLAY_NAME::SKY_PERSONEQ, args[ W_XY]);
		return executeCommandStatus();
	}
	debug_message = "command_'personeq' : unrecognized or malformed argument";
	return executeCommandStatus();
}

// int AppCommandInterface::commandMovetocity()
// {
// 	std::string argName = args[W_NAME];
// 	std::string argCountry = args["country"];
// 	if (!argName.empty() || !argCountry.empty()) {
// 		double lon=0.0, lat=0.0;
// 		int alt=0.0;
// 		coreLink->getCoordonateemCityCore(argName,argCountry, lon, lat, alt);
// 		//cout << lon << ":" << lat << ":" << alt << endl;
// 		if (!((lon==0.0) & (lat ==0.0) & (alt ==-100.0))) {//there is nothing in (0,0,-100) it the magic number to say NO CITY
// 			int delay = (int)(1000.*evalDouble(args[W_DURATION]));
// 			coreLink->observerMoveTo(lat,lon,alt,delay );

// 		}
// 	} else
// 		debug_message = "command_'movetocity' : unknown argument";
// 	return executeCommandStatus();
// }

int AppCommandInterface::commandBodyTrace()
{
	std::string argPen = args[W_PEN];
	if (!argPen.empty()) {
		if (args[W_TARGET]!="") {
			coreLink->bodyTraceBodyChange(args[W_TARGET]);
		}

		if (isTrue(argPen)) {
			coreLink->bodyPenDown();
			return executeCommandStatus();
		}
		else {
			if (isFalse(argPen)) {
				coreLink->bodyPenUp();
				return executeCommandStatus();	
			} else {
				if (argPen == W_TOGGLE) {
					coreLink->bodyPenToggle();
					return executeCommandStatus();
				}
				else{
					debug_message= _("Command 'body_trace': unknown pen value");
					return executeCommandStatus();
				}
			}
		}
	}
	if (args[W_ACTION] == ACP_CN_CLEAR) {
		coreLink->bodyTraceClear();
		return executeCommandStatus();
	}
	if (args[W_TARGET]!="") {
		coreLink->bodyTraceBodyChange(args[W_TARGET]);
		return executeCommandStatus();
	}
	if (args[W_HIDE]!="") {
		coreLink->bodyTraceHide(args[W_HIDE]);
		return executeCommandStatus();
	}
	debug_message = _("command 'body_trace' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandSuntrace()
{
	std::string argPen = args[W_PEN];
	if (!argPen.empty()) {
		coreLink->bodyTraceBodyChange(args[W_SUN]);
		if (isTrue(argPen)) {
			coreLink->bodyPenDown();
			return executeCommandStatus();
		} else if (isFalse(argPen)) {
			coreLink->bodyPenUp();
			return executeCommandStatus();
		} else if (argPen ==W_TOGGLE) {
			coreLink->bodyPenToggle();
			return executeCommandStatus();
		}
	}
	if (args[W_ACTION]==  ACP_CN_CLEAR) {
		coreLink->bodyTraceBodyChange(W_SUN);
		coreLink->bodyTraceClear();
	}
	if (args[W_HIDE]!="") {
		coreLink->bodyTraceBodyChange(W_SUN);
		coreLink->bodyTraceHide(args[W_HIDE]);
	}
	return executeCommandStatus();
}


int AppCommandInterface::commandColor() 
{
	//gestion de la couleur
	Vec3f Vcolor;
	std::string argValue = args[W_VALUE];
	std::string argR= args[W_R];
	std::string argG= args[W_G];
	std::string argB= args[W_B];
	AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG, argB);
	if (!testColor)
		return executeCommandStatus();

	std::string argProperty = args[W_PROPERTY];
	if (argProperty.empty()) {
		debug_message = _("Command 'color': unknown expected argument 'property'");
		return executeCommandStatus();
	}
	auto m_color_it = m_color.find(argProperty);

	if (m_color_it ==m_color.end()) {
			debug_message = _("Command 'color': unknown property");
			appInit->searchSimilarColor(argProperty);
			return executeCommandStatus();
	}

	switch(m_color_it->second) {
		case COLORCOMMAND_NAMES::CC_CONSTELLATION_LINES:	coreLink->constellationSetColorLine( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_CONSTELLATION_NAMES:	coreLink->constellationSetColorNames( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_CONSTELLATION_ART: 		coreLink->constellationSetColorArt( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_CONSTELLATION_BOUNDARIES:	coreLink->constellationSetColorBoundaries( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_CARDINAL_POINTS:		coreLink->cardinalsPointsSetColor( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_PLANET_ORBITS:			coreLink->planetSetDefaultColor(W_ORBIT, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_PLANET_NAMES:			coreLink->planetSetDefaultColor(W_LABEL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_PLANET_TRAILS:			coreLink->planetSetDefaultColor(W_TRAIL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_AZIMUTHAL_GRID:			coreLink->skyGridMgrSetColor(SKYGRID_TYPE::GRID_ALTAZIMUTAL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_EQUATOR_GRID:			coreLink->skyGridMgrSetColor(SKYGRID_TYPE::GRID_EQUATORIAL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ECLIPTIC_GRID:			coreLink->skyGridMgrSetColor(SKYGRID_TYPE::GRID_ECLIPTIC, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_GALACTIC_GRID:			coreLink->skyGridMgrSetColor(SKYGRID_TYPE::GRID_GALACTIC, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_EQUATOR_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_EQUATOR, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_GALACTIC_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_GALACTIC_EQUATOR, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ECLIPTIC_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ECLIPTIC, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_MERIDIAN_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_MERIDIAN, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ZENITH_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ZENITH, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_POLAR_POINT:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_POINT_POLAR, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_POLAR_CIRCLE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_CIRCLE_POLAR, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ECLIPTIC_CENTER:		coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ECLIPTIC_POLE, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_GALACTIC_POLE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_GALACTIC_POLE, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_GALACTIC_CENTER:		coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_GALACTIC_CENTER, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_VERNAL_POINTS:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_VERNAL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ANALEMMA:				coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ANALEMMA, Vcolor );	break;
		case COLORCOMMAND_NAMES::CC_ANALEMMA_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ANALEMMALINE, Vcolor );	break;
		case COLORCOMMAND_NAMES::CC_GREENWICH_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_GREENWICH, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ARIES_LINE:				coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ARIES, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ZODIAC:					coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_ZODIAC, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_PERSONAL:				coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_PERSONAL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_PERSONEQ:				coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_PERSONEQ, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_NAUTICAL_ALT:			coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_NAUTICAL, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_NAUTICAL_RA:			coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_NAUTICEQ, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_OBJECT_COORDINATES:		coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_OBJCOORDS, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_MOUSE_COORDINATES:		coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_MOUSECOORDS, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_ANGULAR_DISTANCE:		coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_ANGDIST, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_LOXODROMY:				coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_LOXODROMY, Vcolor ); break; 
		case COLORCOMMAND_NAMES::CC_ORTHODROMY:				coreLink->skyDisplayMgrSetColor(SKYDISPLAY_NAME::SKY_ORTHODROMY, Vcolor );	break;
		case COLORCOMMAND_NAMES::CC_VERTICAL_LINE:			coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_VERTICAL, Vcolor );	break;
		case COLORCOMMAND_NAMES::CC_NEBULA_NAMES:			coreLink->nebulaSetColorLabels( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_NEBULA_CIRCLE: 			coreLink->nebulaSetColorCircle( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_PRECESSION_CIRCLE: 		coreLink->skyLineMgrSetColor(SKYLINE_TYPE::LINE_PRECESSION, Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_TEXT_USR_COLOR: 		coreLink->textSetDefaultColor( Vcolor ); break;
		case COLORCOMMAND_NAMES::CC_STAR_TABLE:				coreLink->starSetColorTable(evalInt(args[W_INDEX]), Vcolor ); break;
		default: 
		break;
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandIlluminate()
{
	std::string argHP = args[W_HP];
	std::string argDisplay = args[W_DISPLAY];
	std::string argConstellation = args[W_CONSTELLATION];

	double ang_size = evalDouble(args[W_SIZE]);
	float rotation = evalDouble(args[W_ROTATION]);

	if (argDisplay=="all_constellation_on") {
		coreLink->illuminateLoadAllConstellation(ang_size,rotation);
	}
	if (argDisplay=="all_constellation_off") {
		coreLink->illuminateRemoveAllConstellation();
	}

	//gestion de la couleur
	Vec3f Vcolor;
	std::string argValue = args[W_COLOR_VALUE];
	std::string argR= args[W_R];
	std::string argG= args[W_G];
	std::string argB= args[W_B];
	std::string errorColor;
	AppCommandColor testColor(Vcolor, errorColor, argValue, argR,argG, argB);

	if (!argConstellation.empty() && isTrue(argDisplay)) {
		if (!testColor)
			coreLink->illuminateLoadConstellation(argConstellation, ang_size, rotation);
		else
			coreLink->illuminateLoadConstellation(argConstellation, Vcolor, ang_size, rotation);
		return executeCommandStatus();
	}

	if (!argConstellation.empty() && isFalse(argDisplay)) {
		coreLink->illuminateRemoveConstellation(argConstellation);
		return executeCommandStatus();
	}

	if (!argHP.empty() && isTrue(argDisplay)) {

		if (!testColor)
			coreLink->illuminateLoad(evalInt(argHP), ang_size, rotation);
		else 		// here we have color
			coreLink->illuminateLoad(evalInt(argHP), Vcolor, ang_size, rotation);

		return executeCommandStatus();
	}

	if (!argHP.empty() && isFalse(argDisplay)) {
		coreLink->illuminateRemove( evalInt(argHP));
		return executeCommandStatus();
	}

	if (args[W_ACTION]==  ACP_CN_CLEAR) {
		coreLink->illuminateRemoveTex();
		coreLink->illuminateRemoveAll();
		return executeCommandStatus();
	}

	if (args[W_ACTION]=="clear_texture") {
		coreLink->illuminateRemoveTex();
		return executeCommandStatus();
	}

	std::string argFileName = args[W_FILENAME];
	if (!argFileName.empty()) {
		FilePath myFile  = FilePath(argFileName, FilePath::TFP::IMAGE);
		if (!myFile.exist()) {
			debug_message = _("command 'illuminate': filename not found");
			return executeCommandStatus();
		}
		coreLink->illuminateChangeTex(myFile.toString());
		return executeCommandStatus();
	}

	debug_message = _("command 'illuminate': argument unknown");
	return executeCommandStatus();
}

int AppCommandInterface::commandPrint()
{
	if (args.begin() == args.end()) {
		debug_message = "command_'print': malformed command";
		return executeCommandStatus();
	}
	for(const auto& i : args) {
		std::stringstream oss;
		oss << "[" << i.first <<"] " << i.second;
		//std::cout << oss.str() << std::endl;
		cLog::get()->write(oss.str(),  LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
		cLog::get()->write(oss.str(),  LOG_TYPE::L_WARNING);
	}
	return executeCommandStatus();
}


int AppCommandInterface::commandSet()
{
	// cas ou l'on tappe juste set
	if (args.begin() == args.end()) {
		debug_message = "command_'set': malformed command";
		//cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
		return executeCommandStatus();
	}
	bool returnValue =true;
	//debug 
	for (const auto&i : args ) {
		// std::cout << i.first << "->" << i.second;
		// std::cout << std::endl;
		returnValue *= evalCommandSet(i.first , i.second);
	}
	return returnValue;
}


SCD_NAMES AppCommandInterface::parseCommandSet(const std::string& setName) 
{
//	std::cout << "size "<< m_appcommand.size() << " m_appcommand" << std::endl;
	for(auto it = m_set.begin(); it != m_set.end(); it++) {
		// if (!args[it->first].empty())
		// 	return it->second;
		if (it->first == setName) {
			//std::cout << setName << " found" << std::endl;
			return it->second;
		}
	}
	//std::cout << setName << " not found" << std::endl;
	return SCD_NAMES::APP_FLAG_NONE;
}


int AppCommandInterface::evalCommandSet(const std::string& setName, const std::string& setValue)
{
	/*
	 *	the set command format is : set SET_NAME SET_VALUE
	*/
	//std::cout << setName << " : " << setValue << std::endl;
	// parse the SET_NAME
	SCD_NAMES parserSet = SCD_NAMES::APP_FLAG_NONE;
	//std::cout << "parserSet-> " << static_cast<std::underlying_type<SCD_NAMES>::type>(parserSet) << std::endl;
	parserSet = parseCommandSet(setName);
	// eval SET_NAME
	//std::cout << "avant switch" << std::endl;
	//std::cout << "parserSet " << static_cast<std::underlying_type<SCD_NAMES>::type>(parserSet) << std::endl;

	switch(parserSet) {
		case SCD_NAMES::APP_ATMOSPHERE_FADE_DURATION : coreLink->atmosphereSetFadeDuration(evalDouble(setValue)); break;
		case SCD_NAMES::APP_AUTO_MOVE_DURATION : stcore->setAutoMoveDuration(evalDouble(setValue)); break;
		case SCD_NAMES::APP_CONSTELLATION_ART_FADE_DURATION: coreLink->constellationSetArtFadeDuration(evalDouble(setValue)); break;
		case SCD_NAMES::APP_CONSTELLATION_ART_INTENSITY: coreLink->constellationSetArtIntensity(evalDouble(setValue)); break;
		case SCD_NAMES::APP_LIGHT_POLLUTION_LIMITING_MAGNITUDE:	stcore->setLightPollutionLimitingMagnitude(evalDouble(setValue)); break;
		case SCD_NAMES::APP_HEADING: 
						if (setValue==W_DEFAULT) coreLink->setDefaultHeading(); else coreLink->setHeading(evalDouble(setValue)); break;
		case SCD_NAMES::APP_HOME_PLANET:
						if (setValue==W_DEFAULT) stcore->setHomePlanet("Earth"); else stcore->setHomePlanet(setValue); break;
		case SCD_NAMES::APP_LANDSCAPE_NAME: 
						if (setValue==W_DEFAULT) stcore->setInitialLandscapeName(); else stcore->setLandscape(setValue); break;
		case SCD_NAMES::APP_LINE_WIDTH:	stapp->setLineWidth(evalDouble(setValue)); break;
		case SCD_NAMES::APP_MAX_MAG_NEBULA_NAME: coreLink->nebulaSetMaxMagHints(evalDouble(setValue)); break;
		case SCD_NAMES::APP_MAX_MAG_STAR_NAME: coreLink->starSetMaxMagName(evalDouble(setValue)); break;
		case SCD_NAMES::APP_MOON_SCALE: coreLink->setMoonScale(evalDouble(setValue)); break;
		case SCD_NAMES::APP_SUN_SCALE: coreLink->setSunScale(evalDouble(setValue)); break;
		case SCD_NAMES::APP_MILKY_WAY_FADER_DURATION: coreLink->milkyWaySetDuration(evalDouble(setValue)); break;
		case SCD_NAMES::APP_MILKY_WAY_INTENSITY:
						if (setValue==W_DEFAULT) coreLink->milkyWayRestoreIntensity(); else coreLink->milkyWaySetIntensity(evalDouble(setValue));
						if (coreLink->milkyWayGetIntensity()) coreLink->milkyWaySetFlag(true);
						break;
		case SCD_NAMES::APP_MILKY_WAY_TEXTURE: 
						if(setValue==W_DEFAULT) coreLink->milkyWayRestoreDefault();	else
							coreLink->milkyWayChangeStateWithoutIntensity(scriptInterface->getScriptPath() + setValue);
						break;
		case SCD_NAMES::APP_SKY_CULTURE: if (setValue==W_DEFAULT) stcore->setInitialSkyCulture(); else stcore->setSkyCultureDir(setValue); break;
		case SCD_NAMES::APP_SKY_LOCALE:  if ( setValue==W_DEFAULT) stcore->setInitialSkyLocale(); else stcore->setSkyLanguage(setValue); break;
		case SCD_NAMES::APP_UI_LOCALE: stapp->setAppLanguage(setValue); break;
		case SCD_NAMES::APP_STAR_MAG_SCALE: coreLink->starSetMagScale(evalDouble(setValue)); break;
		case SCD_NAMES::APP_STAR_SIZE_LIMIT: coreLink->starSetSizeLimit(evalDouble(setValue)); break;
		case SCD_NAMES::APP_PLANET_SIZE_LIMIT: stcore->setPlanetsSizeLimit(evalDouble(setValue)); break;
		case SCD_NAMES::APP_STAR_SCALE: 
							coreLink->starSetScale(evalDouble(setValue));
							coreLink->planetsSetScale(evalDouble(setValue));
						break;
		case SCD_NAMES::APP_STAR_TWINKLE_AMOUNT: coreLink->starSetTwinkleAmount(evalDouble(setValue)); break;
		case SCD_NAMES::APP_STAR_FADER_DURATION: coreLink->starSetDuration(evalDouble(setValue)); break;
		case SCD_NAMES::APP_STAR_LIMITING_MAG: coreLink->starSetLimitingMag(evalDouble(setValue)); break;
		case SCD_NAMES::APP_TIME_ZONE: spaceDate->setCustomTimezone(setValue); break;
		case SCD_NAMES::APP_AMBIENT_LIGHT: 
						if (setValue==W_INCREMENT)
							coreLink->uboSetAmbientLight(coreLink->uboGetAmbientLight()+0.01);
						else if (setValue==W_DECREMENT)
							coreLink->uboSetAmbientLight(coreLink->uboGetAmbientLight()-0.01);
						else
							coreLink->uboSetAmbientLight(evalDouble(setValue));
						break;	
		case SCD_NAMES::APP_TEXT_FADING_DURATION: coreLink-> textFadingDuration(Utility::strToInt(setValue)); break;
		case SCD_NAMES::APP_ZOOM_OFFSET: stcore->setViewOffset(evalDouble(setValue)); break;
		case SCD_NAMES::APP_STARTUP_TIME_MODE: stapp->setStartupTimeMode(setValue); break;
		case SCD_NAMES::APP_DATE_DISPLAY_FORMAT: spaceDate->setDateFormatStr(setValue); break;
		case SCD_NAMES::APP_TIME_DISPLAY_FORMAT: spaceDate->setTimeFormatStr(setValue); break;
		case SCD_NAMES::APP_MODE: stcore->switchMode(setValue); break;
		case SCD_NAMES::APP_SCREEN_FADER: 
						{	Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, evalDouble(setValue));
							EventRecorder::getInstance()->queue(event);
						} break;
		case SCD_NAMES::APP_STALL_RADIUS_UNIT: coreLink->cameraSetRotationMultiplierCondition(evalDouble(setValue)); break;
		case SCD_NAMES::APP_TULLY_COLOR_MODE: coreLink->tullySetColor(setValue); break;
		case SCD_NAMES::APP_DATETIME_DISPLAY_POSITION: ui->setDateTimePosition(evalInt(setValue)); break;
		case SCD_NAMES::APP_DATETIME_DISPLAY_NUMBER: ui->setDateDisplayNumber(evalInt(setValue)); break;
		default:
			debug_message = "command_'set': unknown argument";
			//for (const auto&i : args )
			//	std::cout << i.first << "->" << i.second << std::endl;
			appInit->searchSimilarSet(args.begin()->first);
			//cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
			return executeCommandStatus();
		break;
	}
	//std::cout << setName << " fin analyse" << std::endl;
//	std::cout << "apres switch" << std::endl;
	return executeCommandStatus();
}

int AppCommandInterface::commandShutdown()
{
	if (args[W_ACTION] == W_NOW)	{
		stapp->flag(APP_FLAG::ALIVE, false);
	} else
		debug_message = "Bad shutdown request.";

	return executeCommandStatus();
}

int AppCommandInterface::commandConfiguration()
{
	std::string argAction = args[W_ACTION];
	if (!argAction.empty()) {
		if(argAction== W_LOAD) {
			stapp->init();
			return executeCommandStatus();
		} else if(argAction==W_SAVE) {
			stapp->saveCurrentConfig(AppSettings::Instance()->getConfigFile());
			return executeCommandStatus();
		} else
			debug_message = "command 'configuration': unknown action value";
	}

	std::string argModule = args[W_MODULE];

	if (!argModule.empty()){
		if (argModule == ACP_CN_STAR_LINES){

			if (argAction ==   ACP_CN_CLEAR) {
				coreLink->starLinesClear();
				return executeCommandStatus();
			}

			std::string argName = args[W_NAME];
			if (argName.empty()){
				debug_message = "command 'star_lines' missing name parameter";
				return executeCommandStatus();
			}
			bool binaryMode = Utility::strToBool(args[W_BINARY],false);

			if (argAction ==  W_LOAD) {
				if (binaryMode)
					coreLink->starLinesLoadBinCat(argName);
				else
					coreLink->starLinesLoadCat(argName);
				return executeCommandStatus();
			} else
				debug_message = "command 'configuration': unknown starNavigator action argument";
		} else
		if (argModule=="star_navigator"){

			if (argAction ==   ACP_CN_CLEAR) {
				coreLink->starNavigatorClear();
				return executeCommandStatus();
			}

			std::string argName = args[W_NAME];
			if (argName.empty()){
				debug_message = "command 'configuration', star_navigator missing name argument";
				return executeCommandStatus();
			}

			bool binaryMode = Utility::strToBool(args[W_BINARY],false);

			if (argAction ==  W_LOAD) {
				std::string argMode = args[ACP_SC_MODE];

				if (argMode.empty()){
					debug_message = "command 'configuration', star_navigator missing mode argument";
					return executeCommandStatus();
				} else {
					if (argMode == W_RAW) {
						coreLink->starNavigatorLoadRaw(argName);
						return executeCommandStatus();
					} else 
					if (argMode == W_SC) {
						coreLink->starNavigatorLoad(argName, binaryMode);
						return executeCommandStatus();
					} else 
					if (argMode == W_OTHER) {
						coreLink->starNavigatorLoadOther(argName);
						return executeCommandStatus();
					} else {
						debug_message = "command 'configuration': unknown starNavigator mode parameter";
						return executeCommandStatus();
					}
				}
			} else
			if (argAction == W_SAVE) {
				coreLink->starNavigatorSave(argName, binaryMode);
			} else
				debug_message = "command 'configuration': unknown starNavigator action argument";
		} else
			debug_message = "command 'configuration': unknown module argument";
	}
	debug_message = "command 'configuration': unknown argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandConstellation()
{
	std::string argName = args[W_NAME];
	transform(argName.begin(),argName.end(),argName.begin(), ::toupper);
	if (argName.empty()) {
		debug_message = "command 'constellation': missing name";
		return executeCommandStatus();
	}

	std::string argIntensity = args[W_INTENSITY];
	if (!argIntensity.empty()) {
		coreLink->constellationSetArtIntensity(argName, evalDouble(argIntensity));
		return executeCommandStatus();
	}

	std::string type = args[W_TYPE];
	if (type.empty()) {
		debug_message = "command 'constellation': missing type";
		return executeCommandStatus();
	}

	Vec3f Vcolor;
	std::string argColor =  args[W_COLOR_VALUE];
	std::string argR= args[W_R];
	std::string argG= args[W_G];
	std::string argB= args[W_B];
	AppCommandColor testColor(Vcolor, debug_message, argColor, argR,argG, argB);
	if (!testColor) {
		return executeCommandStatus();
	} else
		debug_message.clear();

	if (type == W_LINE) {
		coreLink->constellationSetLineColor(argName, Vcolor);
		return executeCommandStatus();
	} else if (type == W_LABEL) {
		coreLink->constellationSetColorNames(argName, Vcolor);
		return executeCommandStatus();
	} else {
		debug_message = "command 'constellation': unknown type";
		return executeCommandStatus();
	}
}

// int AppCommandInterface::commandExternalMplayer()
// {
	// std::string argAction = args[W_ACTION];
	// if (!argAction.empty()) {
		// if (argAction==W_PLAY && args[W_FILENAME]!="") {
			// if (Utility::isAbsolute(args[W_FILENAME]))
				// media->externalPlay(args[W_FILENAME]);
			// else
				// media->externalPlay(scriptInterface->getScriptPath()+args[W_FILENAME]);
			// return executeCommandStatus();
		// }
		// if (argAction==W_STOP) {
			// media->externalStop();
			// return executeCommandStatus();
		// }
		// if (argAction==W_PAUSE) {
			// media->externalPause();
			// return executeCommandStatus();
		// }
		// if (argAction=="reset") {
			// media->externalReset();
			// return executeCommandStatus();
		// }
		// debug_message = _("Command 'externalMplayer': unknown action value");
		// return executeCommandStatus();
	// }
// 
	// std::string argJumpRelative=args["jump_relative"];
	// if (!argJumpRelative.empty()) {
		// media->externalJumpRelative(evalDouble(argJumpRelative));
		// return executeCommandStatus();
	// }
// 
	// if (args["jump_absolute"]!="") {
		// media->externalJumpAbsolute(evalDouble(args["jump_absolute"]));
		// return executeCommandStatus();
	// }
	// if (args[W_SPEED]!="") {
		// media->externalSpeed(evalDouble(args[W_SPEED]));
		// return executeCommandStatus();
	// }
	// if (args[W_VOLUME]!="") {
		// media->externalVolume(evalDouble(args[W_VOLUME]));
		// return executeCommandStatus();
	// }
	// if (args["execute"]!="") {
		// media->externalExecute(args["execute"]);
		// return executeCommandStatus();
	// }
	// debug_message= _("command 'externalmplayer' : unknown argument");
	// return executeCommandStatus();
// }

int AppCommandInterface::commandExternalViewer()
{
	std::string argAction = args[W_ACTION];
	std::string argFileName = args[W_FILENAME];

	if (argAction==W_PLAY && !argFileName.empty()) {
		if (argFileName.size()<5) {
			debug_message = _("command 'externalviewer' : fileName too short");
			return executeCommandStatus();
		}

		std::string action1=W_NONE;;
		std::string extention=argFileName.substr(argFileName.length()-3,3);

		if (extention == W_AVI || extention == W_MOV || extention == W_MPG || extention == W_MP4) {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::VIDEO);
			if (myFile)
				action1="mplayer -fs -osdlevel 0 "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer media fileName not found";
		} else if (extention == W_MP3 || extention == W_OGG) {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::AUDIO);
			if (myFile)
				action1="cvlc "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer audio fileName not found";
		} else if (extention == W_SH) {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::DATA);
			if (myFile)
				action1="sh "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer shell script fileName not found";

		} else if (extention == W_SWF) {
			FilePath myFile  = FilePath(argFileName);
			if (myFile)
				action1="gnash "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer swf fileName not found";
		} else if (extention == W_PNG) {
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::IMAGE);
			if (myFile)
				action1="qiv "+ myFile.toString() + " &";
			else
				debug_message= "command_externalViewer png fileName not found";
		}

		if (action1 != W_NONE) {
			if (!CallSystem::useSystemCommand(action1))
				debug_message = "command 'externalviewer': system error";
			return executeCommandStatus();
		}
		return executeCommandStatus();
	}

	if (argAction == W_STOP) {
		std::string action1=W_NONE;;
		action1="killall mplayer";
		CallSystem::useSystemCommand(action1);
		action1="killall vlc";
		CallSystem::useSystemCommand(action1);
		return executeCommandStatus();
	}

	debug_message = _("command 'externalviewer' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandClear()
{
	std::string argState = args[W_STATE];

	if (argState == W_VARIABLE) {
		appEval->deleteVar();
		return executeCommandStatus();
	}

	// TODO move to stelcore
	// set sky to known, standard states (used by scripts for simplicity)
	executeCommand("set home_planet Earth");

	if (argState == W_NATURAL) {
		executeCommand(FLAG_ATMOSPHERE_ON);
		executeCommand(FLAG_LANDSCAPE_ON);
	} else {
		executeCommand(FLAG_ATMOSPHERE_OFF);
		executeCommand(FLAG_LANDSCAPE_OFF);
	}

	coreBackup->loadGridState();
	coreBackup->loadDisplayState();
	coreBackup->loadLineState();

	// turn off all labels
	//executeCommand("flag azimuthal_grid off");
	executeCommand(FLAG_MERIDIAN_LINE_OFF);
	executeCommand(FLAG_ZENITH_LINE_OFF);
	executeCommand(FLAG_POLAR_CIRCLE_OFF);
	executeCommand(FLAG_POLAR_POINT_OFF);
	executeCommand(FLAG_ECLIPTIC_CENTER_OFF);
	executeCommand(FLAG_GALACTIC_POLE_OFF);
	executeCommand(FLAG_GALACTIC_CENTER_OFF);
	executeCommand(FLAG_VERNAL_POINTS_OFF);
	executeCommand(FLAG_ANALEMMA_OFF);
	executeCommand(FLAG_ANALEMMA_LINE_OFF);
	executeCommand(FLAG_ARIES_LINE_OFF);
	executeCommand(FLAG_ZODIAC_OFF);
	executeCommand(FLAG_PERSONAL_OFF);
	executeCommand(FLAG_PERSONEQ_OFF);
	executeCommand(FLAG_NAUTICAL_ALT_OFF);
	executeCommand(FLAG_NAUTICAL_RA_OFF);
	executeCommand(FLAG_OBJECT_COORDINATES_OFF);
	executeCommand(FLAG_ANGULAR_DISTANCE_OFF);
	executeCommand(FLAG_LOXODROMY_OFF);
	executeCommand(FLAG_ORTHODROMY_OFF);
	executeCommand(FLAG_GREENWICH_LINE_OFF);
	executeCommand(FLAG_VERTICAL_LINE_OFF);
	executeCommand(FLAG_CARDINAL_POINTS_OFF);
	executeCommand(FLAG_CONSTELLATION_ART_OFF);
	executeCommand(FLAG_CONSTELLATION_DRAWING_OFF);
	executeCommand(FLAG_CONSTELLATION_NAMES_OFF);
	executeCommand(FLAG_CONSTELLATION_BOUNDARIES_OFF);
	executeCommand(FLAG_ECLIPTIC_LINE_OFF);
	//executeCommand(FLAG_EQUATORIAL_GRID_OFF);
	executeCommand(FLAG_EQUATOR_LINE_OFF);
	executeCommand(FLAG_GALACTIC_LINE_OFF);
	executeCommand(FLAG_TROPIC_LINES_OFF);
	executeCommand(FLAG_CIRCUMPOLAR_CIRCLE_OFF);
	executeCommand(FLAG_PRECESSION_CIRCLE_OFF);
	executeCommand(FLAG_FOG_OFF);
	executeCommand(FLAG_NEBULA_HINTS_OFF);
	executeCommand(FLAG_NEBULA_NAMES_OFF);
	executeCommand(FLAG_OBJECT_TRAILS_OFF);
	executeCommand(FLAG_PLANET_NAMES_OFF);
	executeCommand(FLAG_PLANET_ORBITS_OFF);
	executeCommand(FLAG_PLANETS_ORBITS_OFF);
	executeCommand(FLAG_SATELLITES_ORBITS_OFF);
	executeCommand(FLAG_SHOW_TUI_DATETIME_OFF);
	executeCommand(FLAG_STAR_NAMES_OFF);
	executeCommand(FLAG_SHOW_TUI_SHORT_OBJ_INFO_OFF);

	// make sure planets, stars, etc. are turned on!
	executeCommand(FLAG_STARS_ON);
	executeCommand(FLAG_PLANETS_ON);
	executeCommand(FLAG_NEBULAE_ON);

	// also deselect everything, set to default fov and real time rate
	executeCommand(DESELECT);
	executeCommand(TIMERATE_RATE_1);
	executeCommand(ZOOM_AUTO_INITIAL);

	return executeCommandStatus();
}


int AppCommandInterface::commandHeading()
{
	std::string argHeading=args[W_AZIMUTH];	
	if (!argHeading.empty() ) {
		if (argHeading==W_DEFAULT) {
			coreLink->setDefaultHeading();
			return executeCommandStatus();
		}
	
		double heading = evalDouble(argHeading);
		float fdelay = evalDouble(args[W_DURATION]);
		coreLink->setHeading(heading, (int)(fdelay*1000));
		return executeCommandStatus();
	}
	std::string argDeltaHeading=args[W_DELTA_AZIMUTH];	
	if (!argDeltaHeading.empty() ) {
		float fdelay = evalDouble(args[W_DURATION]);		
		double heading = evalDouble(argDeltaHeading) + coreLink->getHeading();
		if (heading > 180) heading -= 360;
		if (heading < -180) heading += 360;

		std::stringstream oss;
		oss << "heading from : " << coreLink->getHeading() << " to: " << heading;
		cLog::get()->write( oss.str(),LOG_TYPE::L_INFO, LOG_FILE::SCRIPT );
		coreLink->setHeading(heading, (int)(fdelay*1000));
		return executeCommandStatus();
	}
	// if (args["heading"]==W_DEFAULT) {
	// 	coreLink->setDefaultHeading();
	// }
	// else {
	// 	float fdelay = evalDouble(args[W_DURATION]);
	// 	double heading = evalDouble(args["heading"]);
	// 	if (fdelay <= 0) fdelay = 0;
	// 	if (args["heading"][0] == '+') {
	// 		heading += coreLink->getHeading();
	// 		if (heading > 180) heading -= 360;
	// 		std::stringstream oss;
	// 		oss << "FROM: " << coreLink->getHeading() << " TO: " << heading;
	// 		cLog::get()->write( oss.str(),LOG_TYPE::L_INFO, LOG_FILE::SCRIPT );
	// 	}
	// 	if (args["heading"][0] == '-') {
	// 		heading += coreLink->getHeading();
	// 		if (heading < -180) heading += 360;
	// 		std::stringstream oss;
	// 		oss << "FROM: " << coreLink->getHeading() << " TO: " << heading;
	// 		cLog::get()->write( oss.str(),LOG_TYPE::L_INFO, LOG_FILE::SCRIPT );
	// 	}
	// 	coreLink->setHeading(heading, (int)(fdelay*1000));
	// }
	debug_message = "command 'heading' : unknown argument";
	return executeCommandStatus();
}


int AppCommandInterface::commandMeteors()
{
	std::string argZhr=args[W_ZHR];
	if (! argZhr.empty()) {
		coreLink->setMeteorsRate(evalInt(args[W_ZHR]));
	} else
		debug_message = "command 'meteors' : no zhr argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandLandscape()
{
	std::string argAction = args[W_ACTION];
	if (!argAction.empty()) {
		if (argAction ==  W_LOAD) {
			// textures are relative to script
			args[W_PATH] = scriptInterface->getScriptPath();
			stcore->loadLandscape(args); //TODO retour d'erreurs
		} else if (argAction == W_ROTATE) {
			if (!args[W_ROTATION].empty()) {
				coreLink->rotateLandscape((M_PI/180.0)*evalDouble(args[W_ROTATION]));
			}
		} else {
			debug_message = "command 'landscape' : invalid action parameter";
		}
	} else
		debug_message = "command 'landscape' : unknown argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandText()
{
	std::string argAction = args[W_ACTION];

	if (argAction== W_CLEAR) {
		coreLink->textClear();
		return executeCommandStatus();
	}

	std::string argName = args[W_NAME];
	if (argName.empty()) {
		debug_message = _("Command 'text': argument 'name' needed");
		return executeCommandStatus();
	}
	argName = evalString(argName);

	if (argAction==W_DROP) {
		coreLink->textDel(argName);
		return executeCommandStatus();
	}

	std::string argDisplay = args[W_DISPLAY];
	std::string argString = args[W_STRING];

	argString = evalString(argString);

	if (!argAction.empty()) {
		if (argString.empty()) {
			debug_message = _("Command 'text': argument 'string' needed");
			return executeCommandStatus();
		}

		if (argAction==W_UPDATE) {
			coreLink->textNameUpdate(argName, argString);
			return executeCommandStatus();
		} else if (argAction== W_LOAD) {
			std::string argAzimuth = args[W_AZIMUTH];
			std::string argAltitude = args[W_ALTITUDE];
			if( !argAzimuth.empty() && !argAltitude.empty()) {
				float azimuth = evalDouble(argAzimuth);
				float altitude = evalDouble(argAltitude);
				int durationText = 1000*evalDouble(args[W_DURATION]);
				printf("Durée d'apparition du texte : %i\n", durationText);
				std::string argSize = args[W_SIZE];

				//gestion de la couleur
				Vec3f Vcolor;
				std::string argValue = args[W_COLOR_VALUE];
				std::string argR= args[W_R];
				std::string argG= args[W_G];
				std::string argB= args[W_B];
				AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG, argB);
				if (testColor)
					coreLink->textAdd(argName,argString, altitude, azimuth, argSize, Vcolor, durationText);
				else {
					debug_message.clear();
					coreLink->textAdd(argName,argString, altitude, azimuth, argSize, durationText);
				}
				// test si l'utilisateur spécifie argDisplay
				if (!argDisplay.empty()) {
					if ( isTrue(argDisplay) )
						coreLink->textDisplay(argName,true);
					else
						coreLink->textDisplay(argName,false);
					return executeCommandStatus();
				}
				return executeCommandStatus();
			} else {
				debug_message = _("Command 'text': parameter 'azimuth' or 'altitude' needed");
				return executeCommandStatus();
			}
		}
	}

	// test argDisplay en commande indépendante
	if (!argDisplay.empty()) {
		if ( isTrue(argDisplay) )
			coreLink->textDisplay(argName,true);
		else
			coreLink->textDisplay(argName,false);
		return executeCommandStatus();
	}

	debug_message = _("Command 'text': unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandSkyCulture()
{
	std::string argPath = args[W_PATH];
	if (!argPath.empty() && args[W_ACTION]== W_LOAD) {
		if (!stcore->loadSkyCulture(argPath))
			debug_message = "Error loading sky culture from path specified.";
	} else
		debug_message = "command_sky_culture : path or action missing";
	return executeCommandStatus();
}

int AppCommandInterface::commandScript(unsigned long int &wait)
{
	std::string argAction = args[W_ACTION];
	std::string filen = args[W_FILENAME];
	if (!argAction.empty()) {
		if (argAction==W_END) {
			scriptInterface->cancelScript();
			coreLink->textClear();
			media->audioMusicHalt();
			media->imageDropAllNoPersistent();
			swapCommand = false;
			swapIfCommand = false;
		} else if (argAction==W_PLAY && !filen.empty()) {
			int le=-1;

			std::string file_with_path = FilePath(filen, FilePath::TFP::SCRIPT);
			std::string new_script_path="";

			for (unsigned int i=0; i<file_with_path.length(); i++) {
				if (file_with_path[i]=='/') {
					le=i;
				}
			}
			for (int i=0; i<=le; i++) {
				new_script_path+=file_with_path[i];
			}

			if( !scriptInterface->playScript(file_with_path) ) {
				debug_message = "Unable to execute script : " + file_with_path;
				cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
			} else {
				scriptInterface->setScriptPath(new_script_path);
			}
		} else if (argAction==W_RECORD) {
			scriptInterface->recordScript(filen);
			recordable = 0;  // don't record this command!
		} else if (argAction==W_CANCEL) {
			scriptInterface->cancelRecordScript();
			recordable = 0;  // don't record this command!
		} else if (argAction==W_PAUSE && !scriptInterface->isScriptPaused()) {
			wait = 1;
			scriptInterface->pauseScript();
		} else if (argAction==W_PAUSE || argAction==W_RESUME) {
			scriptInterface->resumeScript();
		} else if (argAction==W_FASTER) {
			scriptInterface->fasterSpeed();
		} else if (argAction==W_SLOWER) {
			scriptInterface->slowerSpeed();
		} else if (argAction==W_DEFAULT) {
			scriptInterface->defaultSpeed();
		} else
			debug_message = "command_script : unknown parameter from 'action' argument";
		return executeCommandStatus();
	}

	std::string argSpeed = args[W_SPEED];
	if (!argSpeed.empty()) {
		if (argAction==W_FASTER) {
			scriptInterface->fasterSpeed();
		} else if (argAction==W_SLOWER) {
			scriptInterface->slowerSpeed();
		} else if (argAction==W_DEFAULT) {
			scriptInterface->defaultSpeed();
		} else
			debug_message = "command_script : unknown parameter from 'speed' argument";		
	}

	debug_message = "command_'script' : missing action argument";
	return executeCommandStatus();
}

int AppCommandInterface::commandAudio()
{
	//gestion du volume
	std::string argVolume = args[W_VOLUME];
	if (!argVolume.empty()) {
		if (argVolume == W_INCREMENT) {
			media->audioVolumeIncrement();
		} else if (argVolume == W_DECREMENT) {
			media->audioVolumeDecrement();
		} else
			media->audioSetVolume(evalInt(argVolume));
		return executeCommandStatus();
	}

	//gestion de la pause des audio dans les scripts
	std::string argMusicPause= args[W_NOPAUSE];
	if (!argMusicPause.empty()) {
		media->audioSetMusicToPause(isTrue(args[W_NOPAUSE]));
		return executeCommandStatus();
	}

	//gestion des actions
	std::string argAction = args[W_ACTION];
	if (!argAction.empty()) {
		if (argAction ==W_DROP) {
			media->audioMusicDrop();
			return executeCommandStatus();
		} else if (argAction==W_SYNC) {
			media->audioMusicSync();
			return executeCommandStatus();
		} else if (argAction==W_PAUSE) {
			media->audioMusicPause();
			return executeCommandStatus();
		} else if (argAction==W_RESUME) {
			media->audioMusicResume();
			return executeCommandStatus();
		} else if (argAction==W_PLAY){
			std::string argFileName = args[W_FILENAME];
			if (!argFileName.empty() ) {
				if (FilePath myFile  = FilePath(argFileName, FilePath::TFP::AUDIO)) {
					media->audioMusicLoad(myFile);
					media->audioMusicPlay(isTrue(args[W_LOOP]));
					return executeCommandStatus();
				} else {
					debug_message = _("command 'audio': filename not found");
					return executeCommandStatus();
				}
			} else {
				debug_message = _("command 'audio': filename not found");
				return executeCommandStatus();
			}
		}
	} else {
		debug_message = _("command 'audio': unknown action value");
		return executeCommandStatus();
	}

	debug_message= _("command 'audio' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandImage()
{
	std::string argAction = args[W_ACTION];
	if (argAction==W_PURGE) {
		media->imageDropAll();
		return executeCommandStatus();
	}

	std::string argName = args[W_NAME];
	if (argName.empty()) {
		debug_message = _("Image argument name required.");
		return executeCommandStatus();
	}

	std::string argFileName = args[W_FILENAME];
	if (argAction==W_DROP) {
		media->imageDrop(evalString(args[W_NAME]));
		return executeCommandStatus();
	}

	if (argAction=="twice") {
		media->imageClone(argName,2);
		return executeCommandStatus();
	}
	if (argAction=="thrice") {
		media->imageClone(argName,3);
		return executeCommandStatus();
	}

	if (argAction== W_LOAD && !argFileName.empty()) {
		FilePath myFile  = FilePath(evalString(argFileName), FilePath::TFP::IMAGE);
		if (!myFile.exist()) {
			debug_message = _("command 'image': filename not found");
			return executeCommandStatus();
		}
		std::string argCoordinate = args["coordinate_system"];
		if (!args[W_HP].empty()) {
			argCoordinate = "equatorial";
		}
		bool mipmap = 0; // Default off for historical reasons
		if (isTrue(args[W_MIPMAP]))
			mipmap = 1;

		//TODO récupérer une erreur compréhensible plutot qu'un int ?
		int status = media->imageLoad(myFile.toString(), evalString(argName), argCoordinate, mipmap);
		if (status!=1) {
			debug_message = _("Unable to load image: ") + argName;
			return executeCommandStatus();
		}
	}

	if (media->imageSet(evalString(argName)) != true) {
		debug_message = _("Unable to find image: ") + argName;
		return executeCommandStatus();
	}

	//initialisation de toutes les variables
	std::string argDuration = args[W_DURATION];
	std::string argAlpha = args[W_ALPHA];
	std::string argScale = args[W_SCALE];
	std::string argRotation = args[W_ROTATION];
	std::string argRatio = args[W_RATIO];
	std::string argXpos = args[W_XPOS];
	std::string argYpos = args[W_YPOS];
	std::string argAltitude = args[W_ALTITUDE];
	std::string argAzimuth = args[W_AZIMUTH];
	std::string argPersistent = args[W_PERSISTENT];
	std::string argAccelerate_x = args[W_ACCELERATE_ALT];
	std::string argAccelerate_y = args[W_ACCELERATE_AZ];
	std::string argDecelerate_x = args[W_DECELERATE_ALT];
	std::string argDecelerate_y = args[W_DECELERATE_AZ];
	std::string argHP = args[W_HP];

	if (!argAlpha.empty())
		media->imageSetAlpha(evalDouble(argAlpha), evalDouble(argDuration));

	if (!argScale.empty())
		media->imageSetScale(evalDouble(argScale), evalDouble(argDuration));

	if (!argRotation.empty())
		media->imageSetRotation(evalDouble(argRotation), evalDouble(argDuration));
	
	if (!argRatio.empty())
		media->imageSetRatio(evalDouble(argRatio), evalDouble(argDuration));

	if (!argHP.empty()) {
		const float rad2deg = 180.0f/M_PI;
		double az, alt;
		bool isStar = stcore->getStarEarthEquPosition(evalInt(argHP), az, alt);
		if (isStar) {
			media->imageSetLocation(alt*rad2deg, true,
					az*rad2deg, true,
					evalDouble(argDuration),
					(argAccelerate_x==W_ON), (argDecelerate_x==W_ON),
					(argAccelerate_y==W_ON), (argDecelerate_y==W_ON));
		} else {
			debug_message = _("command 'image': HP number ") + argHP + _(" is not a valid star");
			return executeCommandStatus();
		}
	}

	if (!argXpos.empty() || !argYpos.empty())
		media->imageSetLocation(evalDouble(argXpos), !argXpos.empty(),
		                        evalDouble(argYpos), !argYpos.empty(),
		                        evalDouble(argDuration),
		                        (argAccelerate_x==W_ON), (argDecelerate_x==W_ON),
		                        (argAccelerate_y==W_ON), (argDecelerate_y==W_ON));

	// for more human readable scripts, as long as someone doesn't do both...
	if (!argAltitude.empty() || !argAzimuth.empty() )
		media->imageSetLocation(evalDouble(argAltitude), !argAltitude.empty(),
		                        evalDouble(argAzimuth), !argAzimuth.empty(),
		                        evalDouble(argDuration),
		                        (argAccelerate_x==W_ON), (argDecelerate_x==W_ON),
		                        (argAccelerate_y==W_ON), (argDecelerate_y==W_ON));

	if (!argPersistent.empty()) {
		if (isTrue(argPersistent))
			media->imageSetPersistent(true);
		else
			media->imageSetPersistent(false);
	}


	std::string argKeyColor = args[W_KEYCOLOR];
	if (!argKeyColor.empty()) {
		if (isTrue(argKeyColor))
			media->imageSetKeyColor(true);
		else
			media->imageSetKeyColor(false);
	}

	Vec3f Vcolor;
	std::string argValue = args[W_COLOR_VALUE];
	std::string argR= args[W_R];
	std::string argG= args[W_G];
	std::string argB= args[W_B];
	AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG,argB);
	if (testColor) {
		std::string argIntensity = args[W_INTENSITY];
		if (!argIntensity.empty())
			media->imageSetKeyColor(Vcolor,Utility::strToDouble(argIntensity)) ;
		else
			media->imageSetKeyColor(Vcolor) ;
	} else
		debug_message.clear();

	return executeCommandStatus();
}

int AppCommandInterface::commandSelect()
{
	// default is to deselect current object
	stcore->unSelect();

	std::string select_type, identifier;

	if (args[W_CONSTELLATION] == W_ZODIAC) {
		stcore->selectZodiac();
		return executeCommandStatus();
	}

	if (args[W_HP]!="") {
		select_type = W_HP;
		identifier = args[W_HP];
	} else if (args[W_STAR]!="") {
		select_type = W_STAR;
		identifier = args[W_STAR];
	} else if (args[W_PLANET]!="") {
		select_type = W_PLANET;
		identifier = args[W_PLANET];
		if (args[W_PLANET] == ACP_SC_HOME_PLANET  )
			identifier = coreLink->getObserverHomePlanetEnglishName();
	} else if (args[W_NEBULA  ]!="") {
		select_type = W_NEBULA  ;
		identifier = args[W_NEBULA  ];
	} else if (args[W_CONSTELLATION]!="") {
		select_type = W_CONSTELLATION;
		identifier = args[W_CONSTELLATION];
	} else if (args[W_CONSTELLATION_STAR  ]!="") {
		select_type = W_CONSTELLATION_STAR  ;
		identifier = args[W_CONSTELLATION_STAR  ];
	} else {
		debug_message= "command 'select' : no object found";
		return executeCommandStatus();
	}

	stcore->selectObject(select_type, identifier);

	// determine if selected object pointer should be displayed
	if (isFalse(args[W_POINTER]))
		stcore->setFlagSelectedObjectPointer(false);
	else
		stcore->setFlagSelectedObjectPointer(true);

	return executeCommandStatus();
}

int AppCommandInterface::commandDeselect()
{
	std::string argConstellation = args[W_CONSTELLATION];
	if ( !argConstellation.empty())
		stcore->unsetSelectedConstellation(argConstellation);
	else
		stcore->deselect();
	return executeCommandStatus();
}


int AppCommandInterface::commandComment()
{
	swapCommand = true;
	return executeCommandStatus();
}


int AppCommandInterface::commandUncomment()
{
	swapCommand = false;
	return executeCommandStatus();
}


int AppCommandInterface::commandLook()
{
	std::string argAz  = args[W_AZIMUTH];
	std::string argAlt = args[W_ALTITUDE];

	if(!argAz.empty() && !argAlt.empty()){

		std::string argTime = args[W_DURATION];

		if(argTime.empty()){
			coreLink->lookAt(stod(argAz), stod(argAlt));
		}
		else{
			coreLink->lookAt(stod(argAz), stod(argAlt), stod(argTime));
		}

		return executeCommandStatus();
	}

	//change direction of view
	std::string argD_az  = args[W_DELTA_AZIMUTH];
	std::string argD_alt = args[W_DELTA_ALTITUDE];
	if (!argD_az.empty() || !argD_alt.empty()) {
		// immediately change viewing direction
		stcore->panView(evalDouble(argD_az), evalDouble(argD_alt), evalDouble(args[W_DURATION]));
	} else {
		debug_message = _("Command 'look_at': wrong argument");
	}
	return executeCommandStatus();
}


int AppCommandInterface::commandStarLines()
{
	if (args[W_ACTION]==W_DROP) {
		coreLink->starLinesDrop();
		return executeCommandStatus();
	}
	if (args[ W_LOAD]!="") {
		coreLink->starLinesLoadData(scriptInterface->getScriptPath() + args[ W_LOAD]);
		return executeCommandStatus();
	}
	if (args[W_ASTERISM  ]!="") {
		coreLink->starLinesLoadAsterism(args[W_ASTERISM  ]);
		return executeCommandStatus();
	}
	debug_message = _("Command 'star_lines': wrong argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandPosition()
{
	std::string argAction = args[W_ACTION];
	if (argAction == W_SAVE) {
		coreBackup->saveBackup();
		return executeCommandStatus();
	}
	if (argAction ==  W_LOAD) {
		coreBackup->loadBackup();
		return executeCommandStatus();
	}
	debug_message = _("Command 'position': unknown parameter");
	return executeCommandStatus();
}

int AppCommandInterface::commandZoom(unsigned long int &wait)
{
	double duration = Utility::strToPosDouble(args[W_DURATION]);
	std::string argAuto = args[W_AUTO  ];
	std::string argManual = args[W_MANUAL];

	if (!argAuto.empty()) {
		// auto zoom using specified or default duration
		if (args[W_DURATION]=="") duration = stcore->getAutoMoveDuration();

		if (argAuto==W_OUT) {
			if (isTrue(argManual)) stcore->autoZoomOut(duration, 0, 1);
			else stcore->autoZoomOut(duration, 0, 0);
		} else if (argAuto==W_INITIAL  ) stcore->autoZoomOut(duration, 1, 0);
		else if (isTrue(argManual)) {
			stcore->autoZoomIn(duration, 1);  // have to explicity allow possible manual zoom
		} else stcore->autoZoomIn(duration, 0);

	} else if (args[W_FOV  ]!="") {
		// zoom to specific field of view
		coreLink->zoomTo( evalDouble(args[W_FOV  ]), evalDouble(args[W_DURATION]));

	} else if (args[W_DELTA_FOV  ]!="") coreLink->setFov(coreLink->getFov() + evalDouble(args[W_DELTA_FOV  ]));
	// should we record absolute fov instead of delta? isn't usually smooth playback
	else if (args[W_CENTER]==W_ON) {
		float cdelay=5;
		if ( args[W_DURATION]!="") cdelay = evalDouble(args[W_DURATION]);
		stcore->gotoSelectedObject();  // center view to selected objet
		if (cdelay > 0) wait = (int)(cdelay*1000);
	} else {
		debug_message = _("Command 'zoom': unknown argument");
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandTimerate()
{
	std::string argRate = args[W_RATE];
	std::string argAction = args[W_ACTION];
	std::string argStep = args[W_STEP];
	std::string argDuration = args[W_DURATION];

	// NOTE: accuracy issue related to frame rate
	if (!argRate.empty()) {
		if (argDuration.empty()) {
			coreLink->timeSetSpeed(evalDouble(argRate)*JD_SECOND);
			coreLink->timeSaveSpeed();
			coreLink->timeSetFlagPause(false);
		} else {
			//std::cout << "Changing timerate to " << argRate << " duration: " << argDuration << std::endl;
			coreLink->timeChangeSpeed(evalDouble(argRate)*JD_SECOND, stod(argDuration));
		}
	} else if (argAction==W_PAUSE) {
		//std::cout << "Changing timerate to pause" << std::endl;
		coreLink->timeSetFlagPause(!coreLink->timeGetFlagPause());
		if (coreLink->timeGetFlagPause()) {
			// TODO pause should be all handled in core methods
			coreLink->timeSaveSpeed();
			coreLink->timeSetSpeed(0);
		} else {
			coreLink->timeLoadSpeed();
		}
	} else if (argAction==W_RESUME) {
		//std::cout << "Changing timerate to resume" << std::endl;
		coreLink->timeSetFlagPause(false);
		coreLink->timeLoadSpeed();
	} else if (argAction==W_INCREMENT) {
		// speed up time rate
		coreLink->timeSetFlagPause(false);
		double s = coreLink->timeGetSpeed();

		double sstep = 2.;

		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>=JD_SECOND) s*=sstep;
		else if (s<-JD_SECOND) s/=sstep;
		else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
		else if (s>=-JD_SECOND && s<0.) s=0.;
		coreLink->timeSetSpeed(s);
		coreLink->timeSaveSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);

	} else if (argAction == W_SINCREMENT) {
		// speed up time rate
		coreLink->timeSetFlagPause(false);
		double s = coreLink->timeGetSpeed();
		double sstep = 1.05;
		if ((abs(s)<3) && (coreLink->observatoryGetAltitude()>150E9)) s=3;
		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>=JD_SECOND) s*=sstep;
		else if (s<-JD_SECOND) s/=sstep;
		else if (s>=0. && s<JD_SECOND) s=JD_SECOND;
		else if (s>=-JD_SECOND && s<0.) s=0.;
		coreLink->timeSetSpeed(s);
		coreLink->timeSaveSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);
	} else if (argAction==W_DECREMENT) {
		coreLink->timeSetFlagPause(false);
		double s = coreLink->timeGetSpeed();

		double sstep = 2.;

		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>JD_SECOND) s/=sstep;
		else if (s<=-JD_SECOND) s*=sstep;
		else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
		else if (s>0. && s<=JD_SECOND) s=0.;
		coreLink->timeSetSpeed(s);
		coreLink->timeSaveSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);
	} else if (argAction == W_SDECREMENT) {
		coreLink->timeSetFlagPause(false);
		double s = coreLink->timeGetSpeed();
		double sstep = 1.05;
		if ((abs(s)<3) && (coreLink->observatoryGetAltitude()>150E9)) s=3;

		if( !argStep.empty() )
			sstep = evalDouble(argStep);

		if (s>JD_SECOND) s/=sstep;
		else if (s<=-JD_SECOND) s*=sstep;
		else if (s>-JD_SECOND && s<=0.) s=-JD_SECOND;
		else if (s>0. && s<=JD_SECOND) s=0.;
		coreLink->timeSetSpeed(s);
		coreLink->timeSaveSpeed();
		// for safest script replay, record as absolute amount
		commandline = "timerate rate " + Utility::doubleToStr(s/JD_SECOND);
	} else
		debug_message = _("Command 'time_rate': unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandMoveto()
{
	std::string argLat = args[W_LAT];
	std::string argLon = args[W_LON];
	std::string argAlt = args[W_ALT];

	std::string argDeltaLat = args[W_DELTA_LAT];
	std::string argDeltaLon = args[W_DELTA_LON];
	std::string argDeltaAlt = args[W_DELTA_ALT];
	std::string argMultAlt = args[W_MULTIPLY_ALT];

	if(argLat.empty()) argLat = args[W_LATITUDE  ];
	if(argLon.empty()) argLon = args[W_LONGITUDE ];
	if(argAlt.empty()) argAlt = args[W_ALTITUDE  ];


	if (argLat.empty() && argLon.empty() && argAlt.empty() && argDeltaLat.empty() && argDeltaLon.empty() && argDeltaAlt.empty() && argMultAlt.empty()) {
		debug_message = "command 'move_to' : missing lat && lon && alt";
		return executeCommandStatus();
	}
	double lat = coreLink->observatoryGetLatitude();
	double lon = coreLink->observatoryGetLongitude();
	double alt = coreLink->observatoryGetAltitude();

	int delay;
	if (!argLat.empty()) {
		if (argLat==W_DEFAULT)
			lat = coreLink->observatoryGetDefaultLatitude();
		else if (argLat == W_INVERSE)
			lat = -lat;
		else lat = evalDouble(argLat);
	}
	if (!argLon.empty()) {
		if (argLon==W_DEFAULT)
			lon = coreLink->observatoryGetDefaultLongitude();
		else if (argLon == W_INVERSE)
			lon = lon+180.0;
		else lon = evalDouble(argLon);
	}
	if (!argAlt.empty()) {
		if (argAlt==W_DEFAULT) alt = coreLink->observatoryGetDefaultAltitude();
		else {
			if (argAlt[0] == '+' || argAlt[0] == '-')
				alt += evalDouble(argAlt);
			else
				alt = evalDouble(argAlt);
		}
	}

	if (!argDeltaLat.empty()) {
			lat += evalDouble(argDeltaLat);
	}
	if (!argDeltaLon.empty()) {
			lon += evalDouble(argDeltaLon);
	}
	if (!argDeltaAlt.empty()) {
		alt += evalDouble(argDeltaAlt);
	}
	if (!argMultAlt.empty()) {
		alt *= evalDouble(argMultAlt);
	}

	delay = (int)(1000.*evalDouble(args[W_DURATION]));

	coreLink->observerMoveTo(lat,lon,alt,delay);

	return executeCommandStatus();
}


int AppCommandInterface::commandMedia()
{
	std::string argAction = args[W_ACTION];
	if (!argAction.empty() ) {

		if (argAction == W_PLAY) {
			std::string type = args[W_TYPE];
			if (type.empty()) {
				debug_message = "Command 'media' argument action need argument 'type'";
				return executeCommandStatus();
			}
			std::string videoName = args[W_VIDEONAME];
			std::string audioName = args[W_AUDIONAME];
			std::string argName =  args[W_NAME];
			std::string argPosition = args[W_POSITION];

			FilePath::TFP localRepertory;
			if (type == W_VR360 || type == W_VRCUBE)
				localRepertory = FilePath::TFP::VR360;
			else
				localRepertory = FilePath::TFP::MEDIA;

			FilePath fileVideo = FilePath(videoName, localRepertory);
			if (!fileVideo.exist()) {
				debug_message = _("command 'media': file videoname not found");
				return executeCommandStatus();
			}

			if (!audioName.empty()) {
				if ( audioName ==W_AUTO   ) {
					// On teste si un fichier de langue existe on prend videoName et on rajoute -fr par exemple à la place de son extention et on rajoute apres ogg
					audioName = videoName;
					if (audioName.size()>5) {
						audioName[audioName.size()-1]='.';
						audioName[audioName.size()-2]=stcore->getSkyLanguage()[1];
						audioName[audioName.size()-3]=stcore->getSkyLanguage()[0];
						audioName[audioName.size()-4]='_';
						audioName = audioName + W_OGG;
		
						FilePath fileAudio = FilePath(audioName, FilePath::TFP::MEDIA);
						if (fileAudio.exist()) {
								cLog::get()->write("command 'media':: succesfull locale audio "+audioName, LOG_TYPE::L_INFO, LOG_FILE::SCRIPT);
								media->playerPlay(type, fileVideo.toString(), fileAudio.toString(), argName, argPosition );
							}
						else {
							cLog::get()->write("command 'media':: locale audio not found "+audioName, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
							media->playerPlay(type, fileVideo.toString(), argName, argPosition );
						}
					}
				} else {
					// si l'audio existe sous forme -fr.ogg alors on le modifie en appliquant la langue de la sky_culture
					if (audioName.size()>8 && audioName[audioName.size()-7]=='-') { // internationalisation possible
						FilePath fileAudio = FilePath(audioName, stcore->getSkyLanguage() );
						if (!fileAudio.exist()) {
							cLog::get()->write("command 'media':: locale audio not found ", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
							media->playerPlay(type, fileVideo.toString(), argName, argPosition );
						} else
							media->playerPlay(type, fileVideo.toString(), fileAudio.toString(), argName, argPosition );
					} else { //fichier simple sans internationalisation
						FilePath fileAudio = FilePath(audioName, localRepertory);
						if (!fileAudio.exist()) {
							cLog::get()->write("command 'media':: audio not found ", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
							media->playerPlay(type, fileVideo.toString(), argName, argPosition);
						} else
							media->playerPlay(type, fileVideo.toString(), fileAudio.toString(), argName, argPosition);
					}
				}
			} else {
					media->playerPlay(type, fileVideo.toString(), argName, argPosition);
				}

			Vec3f Vcolor;
			std::string argValue = args[W_COLOR_VALUE];
			std::string argR= args[W_R];
			std::string argG= args[W_G];
			std::string argB= args[W_B];
			AppCommandColor testColor(Vcolor, debug_message, argValue, argR,argG,argB);
			if (testColor) {
				// VRAI
				//std::string argIntensity = args[W_INTENSITY];
				// PATCH
				std::string argIntensity;
				if (!args[W_INTENSITY].empty())
					argIntensity = args[W_INTENSITY];
				else
					argIntensity = args[W_INTENSITY];
				// fin du PATCH			
				
				if (!argIntensity.empty())
					media->setKeyColor(Vcolor,Utility::strToDouble(argIntensity)) ;
				else
					media->setKeyColor(Vcolor) ;
			} else
				debug_message.clear();
		
			std::string argKeyColor = args[W_KEYCOLOR];
			if (!argKeyColor.empty()) {
				if (isTrue(argKeyColor)) {
					media->setKeyColor(true);
					media->disableFader();
				}
				else
					media->setKeyColor(false);
			}
			return executeCommandStatus();

		} else if (argAction == W_STOP) {
			media->playerStop();
			return executeCommandStatus();
		} else if (argAction == W_PAUSE) {
			media->playerPause();
			return executeCommandStatus();
		} else if (argAction == W_JUMP) {
			media->playerJump(evalDouble(args[W_VALUE]));
			return executeCommandStatus();
		}
	}

	debug_message = _("command 'media': unknown parameter");

	return executeCommandStatus();
}


int AppCommandInterface::commandDomemasters()
{
	std::string argAction = args[W_ACTION];
	if (!argAction.empty()) {
		if (argAction == W_SNAPSHOT) {
			saveScreenInterface->takeScreenShot();
			return executeCommandStatus();
		}
		if (argAction == W_RECORD) {
			saveScreenInterface->takeVideoShot();
			return executeCommandStatus();
		} else
			debug_message = _("Command 'domemasters': unknown action value");
	}
	debug_message = _("command 'domemasters': unknown argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandDate()
{
	//cas du jday
	std::string argJday = args[W_JDAY];
	if (!argJday.empty() ) {
		//TODO stcore doit renvoyer un code rectour erreur
		coreLink->setJDay( evalDouble(argJday) );
		return executeCommandStatus();
	}

	//cas du local
	std::string argLocal = args[W_LOCAL];
	if (!argLocal.empty() ) {
		// ISO 8601-like format [[+/-]YYYY-MM-DD]Thh:mm:ss (no timzone offset, T is literal)
		double jd;
		std::string new_date;

		if (argLocal[0] == 'T') {
			// set time only (don't change day)
			std::string sky_date =spaceDate->getISO8601TimeLocal(coreLink->getJDay());
			new_date = sky_date.substr(0,10) + argLocal;
		} else new_date = argLocal;

		if (SpaceDate::StringToJday( new_date, jd )) {
			coreLink->setJDay(jd - (spaceDate->getGMTShift(jd) * JD_HOUR));
		} else {
			debug_message = _("Error parsing date local");
		}
		return executeCommandStatus();
	}

	//cas de l'utc
	std::string argUtc = args[W_UTC];
	if (!argUtc.empty()) {
		double jd;
		if (SpaceDate::StringToJday(argUtc, jd ) ) {
			coreLink->setJDay(jd);
		} else {
			debug_message = _("Error parsing date utc");
		}
		return executeCommandStatus();
	}

	//cas du relative
	std::string argRelative = args[W_RELATIVE];
	if (!argRelative.empty()) { // value is a float number of days
		double days = evalDouble(argRelative);
		const Body* home = coreLink->getObserverHomeBody();
		if (home==nullptr) {
			debug_message = _("Error date local, vous devez être sur un astre pour utiliser l'argument relative");
			return executeCommandStatus();
		}
		float sol_local_day = home->getSolLocalDay();
		if (abs(sol_local_day)>366.0) sol_local_day=1.0;
		coreLink->setJDay(coreLink->getJDay() + days*sol_local_day );
		return executeCommandStatus();
	}

	//cas du relative_year
	std::string argRelativeYear = args[W_RELATIVE_YEAR];
	if (!argRelativeYear.empty()) {
		int years = evalInt(argRelativeYear);
		stcore->setJDayRelative(years,0);
		return executeCommandStatus();
	}

	//cas du relative_month
	std::string argRelativeMonth = args[W_RELATIVE_MONTH];
	if (!argRelativeMonth.empty()) {
		int months = evalInt(argRelativeMonth);
		stcore->setJDayRelative(0, months);
		return executeCommandStatus();
	}

	//cas du sidereal
	std::string argSidereal = args[W_SIDEREAL];
	if (!argSidereal.empty()) { // value is a float number of sidereal days
		double days = evalDouble(argSidereal);
		const Body* home = coreLink->getObserverHomeBody();
		if (home==nullptr) {
			debug_message = _("Error date local, vous devez être sur un astre pour utiliser l'argument sideral");
			return executeCommandStatus();
		}
		float sol_sidereal_day = home->getSiderealDay();
		if (abs(sol_sidereal_day)>366.0) sol_sidereal_day=1.0;
		days *= sol_sidereal_day;
		coreLink->setJDay(coreLink->getJDay()+days);
		return executeCommandStatus();
	}

	//cas du load
	std::string argLoad = args[ W_LOAD];
	if (!argLoad.empty()) {
		if (argLoad == W_CURRENT) { //IIICCCCIIII
			// set date to current date
			coreLink->setJDay(SpaceDate::JulianFromSys());
		} else if (argLoad == W_PRESET) {
			// set date to preset (or current) date, based on user setup
			// TODO: should this record as the actual date used?
			if (stapp->getStartupTimeMode() == W_PRESET || stapp->getStartupTimeMode() == W_PRESET)
				coreLink->setJDay(stapp->getPresetSkyTime() -spaceDate->getGMTShift(stapp->getPresetSkyTime()) * JD_HOUR);
			else coreLink->setJDay(SpaceDate::JulianFromSys());
		} else if (argLoad == W_KEEPTIME) {
					double jd = coreLink->getJDay();
					ln_date current_date,temps;
					SpaceDate::JulianToDate(jd,&current_date);
					temps=current_date;
					coreLink->setJDay(SpaceDate::JulianFromSys());
					jd = coreLink->getJDay();
					SpaceDate::JulianToDate(jd,&current_date);
					current_date.hours=temps.hours;
					current_date.minutes=temps.minutes;
					current_date.seconds=temps.seconds;
					coreLink->setJDay(SpaceDate::JulianDayFromDateTime(current_date.years,current_date.months,current_date.days,current_date.hours,current_date.minutes,current_date.seconds));
		} else
			debug_message = _("Command 'date': unknown load value");
		return executeCommandStatus();
	}

	//cas du Sun
	std::string argSun = args[W_SUN];
	if (!argSun.empty()) {
		if (argSun == W_SET) {
			double tmp=coreLink->dateSunSet(coreLink->getJDay(), coreLink->observatoryGetLongitude(), coreLink->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreLink->setJDay(tmp);
		} else if (argSun == W_RISE) {
			double tmp=coreLink->dateSunRise(coreLink->getJDay(), coreLink->observatoryGetLongitude(), coreLink->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreLink->setJDay(tmp);
		} else if (argSun == W_MERIDIAN) {
			double tmp=coreLink->dateSunMeridian(coreLink->getJDay(), coreLink->observatoryGetLongitude(), coreLink->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreLink->setJDay(tmp);
		} else if (argSun == W_MIDNIGHT) {
			double tmp=coreLink->dateSunMeridian(coreLink->getJDay(), coreLink->observatoryGetLongitude()+180, -coreLink->observatoryGetLatitude());
			if (tmp != 0.0) //TODO et si ==?
				coreLink->setJDay(tmp);
		} else
			_("Command 'date': unknown sun value");
		return executeCommandStatus();
	}
	debug_message = _("command 'date' : unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandBody()
{
	//gestion des actions
	std::string argAction = args[W_ACTION];
	std::string argName = args[W_NAME];
    if (argName == ACP_SC_HOME_PLANET  ) argName = coreLink->getObserverHomePlanetEnglishName();
	std::string argMode = args[ACP_SC_MODE];

	// traitement des OJM
	if ((argMode=="in_universe" || argMode=="in_galaxy") && !argAction.empty()) {
		if (argAction == W_LOAD) {
			std::string argFileName = args[W_FILENAME];
			argFileName = argFileName +"/"+argFileName +".ojm";
			Vec3f Position( evalDouble(args[W_POSX]), evalDouble(args[W_POSY]), evalDouble(args[W_POSZ] ));
			FilePath myFile  = FilePath(argFileName, FilePath::TFP::MODEL3D);
			coreLink->BodyOJMLoad(argMode, argName, myFile.toString(), myFile.getPath() , Position, evalDouble(args[W_SCALE]));
			return executeCommandStatus();
		}
		if (argAction == W_REMOVE) {
			coreLink->BodyOJMRemove(argMode, argName);
			return executeCommandStatus();
		}
		if (argAction ==  ACP_CN_CLEAR) {
			coreLink->BodyOJMRemoveAll(argMode);
			return executeCommandStatus();
		}
	}

	std::string argSkinUse = args[W_SKINUSE];
	if (!argSkinUse.empty()) {
		//std::cout << "lancement de la commande skin_use" << std::endl;
		if (argSkinUse==W_TOGGLE) {
			coreLink->planetSwitchTexMap(argName, !coreLink->planetGetSwitchTexMap(argName));
		} else
			coreLink->planetSwitchTexMap(argName, isTrue(argSkinUse));

		return executeCommandStatus();
	}

	std::string argSkinTex = args[W_SKINTEX];
	if (!argSkinTex.empty()) {
		//std::cout << "lancement de la commande skin_tex" << std::endl;
		coreLink->planetCreateTexSkin(argName, argSkinTex);
		return executeCommandStatus();
	}

	if (!argAction.empty()) {
		if (argAction ==  W_LOAD ) {
			// textures relative to script
			args[W_PATH] = scriptInterface->getScriptPath();
			// Load a new solar system object
			stcore->addSolarSystemBody(args);
		} else if (argAction == W_DROP && argName != "") {
			// Delete an existing object, but only if was added by a script!
			stcore->removeSolarSystemBody( argName );
		} else if (argAction ==   ACP_CN_CLEAR) {
			// drop all bodies that are not in the original config file
			stcore->removeSupplementalSolarSystemBodies();
		} else if (argAction == W_INITIAL  ) {
			coreLink->initialSolarSystemBodies();
		} else {
			debug_message = "command 'body' : unknown action argument";
		}
		return executeCommandStatus();
	}

	//gestion des actions
	if (!argName.empty() ) {

		//sous cas hidden
		std::string argHidden = args[W_HIDDEN];
		if (!argHidden.empty()) {
			if (isTrue(argHidden)) {
				coreLink->setPlanetHidden(args[W_NAME], true);
			} else if (isFalse(argHidden)) {
				coreLink->setPlanetHidden(args[W_NAME], false);
			} else if (argHidden == W_TOGGLE) {
				coreLink->setPlanetHidden(args[W_NAME], !coreLink->getPlanetHidden(args[W_NAME]));
			} else
				debug_message = _("Command 'body': unknown hidden value");
			return executeCommandStatus();
		}

		//sous cas orbit
		std::string argOrbit = args[W_ORBIT];
		if (!argOrbit.empty()) {
			if (isTrue(argOrbit)) {
				coreLink->planetsSetFlagOrbits(args[W_NAME], true);
			} else if (isFalse(argOrbit)) {
				coreLink->planetsSetFlagOrbits(args[W_NAME], false);
			} else
				debug_message = _("Command 'body': unknown orbit value");
			return executeCommandStatus();
		}


		std::string argColor = args[W_COLOR_VALUE];
		if (argColor.empty())
			argColor = args["color"];

		if (!argColor.empty()) {
			//gestion de la couleur
			Vec3f Vcolor;
			std::string argR= args[W_R];
			std::string argG= args[W_G];
			std::string argB= args[W_B];
			AppCommandColor testColor(Vcolor, debug_message, argColor, argR,argG, argB);
			if (!testColor)
				return executeCommandStatus();

			coreLink->planetSetColor(argName, argColor, Vcolor);
			return executeCommandStatus();
		}
		debug_message = _("Command 'body': case name unknown argument");
		return executeCommandStatus();
	}

	if (!args[W_TESSELATION].empty()) {
		coreLink->planetTesselation(args[W_TESSELATION], evalInt(args[W_VALUE]));
		return executeCommandStatus();
	}

	debug_message = _("Command 'body': unknown argument");
	return executeCommandStatus();
}


int AppCommandInterface::commandFont()
{
	if (!args[W_FILENAME].empty()) {
		FilePath myFile  = FilePath(args[W_FILENAME], FilePath::TFP::FONTS);
			if (myFile) {
				int size = 10;
				if (!args[W_SIZE].empty())
					size = evalInt(args[W_SIZE]);
				stcore->loadFont(size, myFile.toString());
				return executeCommandStatus();
			} else {
				debug_message= "command 'font' : filename can't be found";
				cLog::get()->write( debug_message,LOG_TYPE::L_DEBUG, LOG_FILE::SCRIPT );
				return executeCommandStatus();
			}
	}
	debug_message = _("Command 'font': unknown argument");
	return executeCommandStatus();
}

int AppCommandInterface::commandCamera(unsigned long int &wait)
{
	//gestion des actions
	std::string argAction = args[W_ACTION];
	std::string argName = args[W_NAME];

	if (argAction.empty()) {
		debug_message = "command 'camera' : action argument";
		return executeCommandStatus();
	}

	if(argAction == W_ALIGN_WITH){

		std::string argBody = args[W_BODY  ];

		if (argBody.empty()) {
			debug_message = "command 'align_with' : missing body";
			return executeCommandStatus();
		}

		std::string argDuration = args[W_DURATION];
		double duration =0;

		if ( ! argDuration.empty()) {
			duration = stod(argDuration);
		}

		bool result = coreLink->cameraAlignWithBody(argBody, duration);

		if (!result)
			debug_message = "error align_with_body body";

		return executeCommandStatus();
	}


	if(argAction == W_TRANSITION_TO){
		std::string argTarget = args[W_TARGET];

		if (argTarget.empty()) {
			debug_message = "command 'transition_to' : missing target";
			return executeCommandStatus();
		}

		if(argTarget == W_POINT  ){
			bool result = coreLink->cameraTransitionToPoint("temp_point");

			if (!result)
				debug_message = "error transition_to point";
			return executeCommandStatus();
		}

		if(argTarget == W_BODY  ){
			argName = args[W_NAME];

			if (argName.empty()) {
				debug_message = "command 'transition_to' : missing target name";
				return executeCommandStatus();
			}

			bool result = coreLink->cameraTransitionToBody(argName);

			if (!result)
				debug_message = "error transition_to body";
			return executeCommandStatus();
		}

		debug_message = "command 'transition_to' : unknown target arg";
		return executeCommandStatus();
	}

	if(argAction == W_MOVE_TO){

		std::string argTarget = args[W_TARGET];

		if (argTarget.empty()) {
			debug_message = "command 'move_to' : missing target";
			return executeCommandStatus();
		}

		if(argTarget == W_POINT  ){
			std::string argX = args[W_X];
			std::string argY = args[W_Y];
			std::string argZ = args[W_Z];
			std::string argTime = args[W_DURATION];

			if(argX.empty() || argY.empty() || argZ.empty()){
				debug_message = "command 'move_to point' : missing a coordinate";
				return executeCommandStatus();
			}

			bool result;

			if(argTime.empty())
				result = coreLink->cameraMoveToPoint(stod(argX), stod(argY), stod(argZ));
			else {
				result = coreLink->cameraMoveToPoint(stod(argX), stod(argY), stod(argZ),stod(argTime));
				wait = evalInt(argTime)*1000;
			}

			if (!result)
				debug_message = "error move_to point";
			return executeCommandStatus();
		}

		if(argTarget == W_BODY  ){
			std::string argBodyName = args[W_BODYNAME];
			std::string argTime = args[W_DURATION];

			if(argBodyName.empty() || argTime.empty()){
				debug_message = "command 'move_to body' : missing a body_name or time";
				return executeCommandStatus();
			}

			std::string argAltitude = args[W_ALTITUDE  ];

			bool result;

			if(argAltitude.empty())
				result = coreLink->cameraMoveToBody(argBodyName, stod(argTime));
			else
				result = coreLink->cameraMoveToBody(argBodyName, stod(argTime), stod(argAltitude));

			if (!result)
				debug_message = "error move_to body";
			else
				wait = evalInt(argTime)*1000;

			return executeCommandStatus();
		}

		debug_message = "command 'camera' : unknown target arg";
		return executeCommandStatus();
	}

	if(argAction == W_SAVE){
		bool result;

		std::string argFileName = args[W_FILENAME];

		if (argFileName.empty())
			result = coreLink->cameraSave();
		else
			result = coreLink->cameraSave(argFileName);

		if (!result)
			debug_message = "error saving camera";
		return executeCommandStatus();
	}

	if(argAction ==  W_LOAD){
		std::string argFileName = args[W_FILENAME];

		if(argFileName.empty()){
			debug_message = "command 'camera load' : missing file name";
			return executeCommandStatus();
		}

		bool result = coreLink->loadCameraPosition(argFileName);

		if (!result)
			debug_message = "error loading CameraAnchor";
		return executeCommandStatus();
	}

	if(argAction == W_LIFT_OFF){

		std::string altStr = args[W_ALTITUDE];
		std::string durationStr = args[W_DURATION];

		if(altStr.empty()){
			debug_message = "command 'camera lift_off' : missing altitude";
			return executeCommandStatus();
		}

		if(durationStr.empty()){
			durationStr = "3";
		}

		std::ostringstream command;

		command << "moveto altitude " + altStr + " duration " + durationStr << std::endl;
		command << "wait duration " + durationStr << std::endl;
		command << "camera action transition_to target point" << std::endl;

		return scriptInterface->addScriptFirst(command.str());
	}

	if (argName.empty()) {
		debug_message = "command 'camera' : missing name";
		return executeCommandStatus();
	}

	if (argAction == W_CREATE ) {
		// load an anchor via script
		bool result = coreLink->cameraAddAnchor(args);
		if (!result)
			debug_message = "error creating CameraAnchor";
		return executeCommandStatus();
	}

	if (argAction == W_DROP) {
		// Delete an existing anchor
		bool result = coreLink->cameraRemoveAnchor(argName);
		if (!result)
			debug_message = "error drop CameraAnchor";
		return executeCommandStatus();
	}

	if (argAction == W_SWITCH) {
		// change the anchor
		bool result = coreLink->cameraSwitchToAnchor(argName);
		if (!result)
			debug_message = "error switch CameraAnchor";
		return executeCommandStatus();
	}

	if(argAction == W_FOLLOW_ROTATION){
		std::string valueStr = args[W_VALUE];

		bool value = valueStr == W_TRUE;

		bool result = coreLink->cameraSetFollowRotation(argName, value);
		if (!result)
			debug_message = "error camera follow_rotation";
		return executeCommandStatus();
	}

	debug_message = "command 'camera' : unknown action argument";
	return executeCommandStatus();
}

//
// variable management
//

std::string AppCommandInterface::evalString (const std::string &var)
{
	if (var.empty())
		return "";
	return appEval->evalString(var);
}

double AppCommandInterface::evalDouble (const std::string &var)
{
	if (var.empty())
		return 0.0;
	return appEval->evalDouble(var);
}


int AppCommandInterface::evalInt (const std::string &var)
{
	if (var.empty())
		return 0;
	return appEval->evalInt(var);
}


int AppCommandInterface::commandDefine()
{
	if (args.begin() != args.end()) {
		std::string mArg = args.begin()->first;
		std::string mValue = args.begin()->second;
		//std::cout << "Command define : " <<  mArg.c_str() << " => " << mValue.c_str() << std::endl;
		appEval->define(mArg,mValue);
	} else {
		debug_message = "Unexpected error in command_define";
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandAdd()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {
		std::string mArg = args.begin()->first;
		std::string mValue = args.begin()->second;
		appEval->commandAdd(mArg,mValue);
	} else { //est ce que ce cas peut vraiment se produire ?
		debug_message = "unexpected error in command_addition";
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandSub()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {
		std::string mArg = args.begin()->first;
		std::string mValue = args.begin()->second;
		appEval->commandSub(mArg,mValue);
	} else { //est ce que ce cas peut vraiment se produire ?
		debug_message = "unexpected error in command_substract";
	}
	return executeCommandStatus();
}


int AppCommandInterface::commandMultiply()
{
	// could loop if want to allow that syntax
	if (args.begin() != args.end()) {
		std::string mArg = args.begin()->first;
		std::string mValue = args.begin()->second;
		appEval->commandMul(mArg,mValue);
	} else {
		debug_message = "unexpected error in command__addition";
	}
	return executeCommandStatus();
}

int AppCommandInterface::commandStruct()
{
	const double error = 0.0001;
	// if case
	std::string argIf = args[W_IF];
	if (!argIf.empty() && swapCommand != true) {
		if (argIf==W_ELSE) {
			swapIfCommand = ! swapIfCommand;
			return executeCommandStatus();
		}
		if (argIf==W_END) {
			swapIfCommand = false;
			return executeCommandStatus();
		}
		if (args[W_EQUAL  ]!="")  // ! A==B => |A-B| > e
			if (fabs(evalDouble(argIf) - evalDouble(args[W_EQUAL  ]))>error) {
				swapIfCommand = true;
				return executeCommandStatus();
			}
		if (args[W_DIFF  ]!="")  // ! A!=B => |A-B| < e
			if (fabs(evalDouble(argIf) - evalDouble(args[W_DIFF  ]))<error) {
				swapIfCommand = true;
				return executeCommandStatus();
			}
		if (args[W_INF  ]!="")
			if (evalDouble(argIf) >= evalDouble(args[W_INF  ])) {
				swapIfCommand = true;
				return executeCommandStatus();
			}
		if (args[W_INF_ZQUAL  ]!="")
			if (evalDouble(argIf) > evalDouble(args[W_INF_ZQUAL  ])) {
				swapIfCommand = true;
				return executeCommandStatus();
			}
		if (args[W_SUP  ]!="")
			if (evalDouble(argIf) <= evalDouble(args[W_SUP  ])) {
				swapIfCommand = true;
				return executeCommandStatus();
			}
		if (args[W_SUP_EQUAL  ]!="")
			if (evalDouble(argIf) < evalDouble(args[W_SUP_EQUAL  ])) {
				swapIfCommand = true;
				return executeCommandStatus();
			}
		//retro-compatibilité
		if (evalDouble(argIf) == 0)
			swapIfCommand = true;
		return executeCommandStatus();
	}

	//comment case
	std::string argComment = args[W_COMMENT];
	if (!argComment.empty()) {
		if (isTrue(argComment)) {
			return commandComment();
		} else {
			return commandUncomment();
		}
	}

	//loop case
	std::string argLoop = args[W_LOOP];
	if (!argLoop.empty() && swapIfCommand != true) {
		if (argLoop ==W_END) {
			swapCommand = false; //cas ou nbrLoop était inférieur à 1
			scriptInterface->setScriptLoop(false);
			scriptInterface->initScriptIterator();
			return executeCommandStatus();
		}

		if (argLoop ==W_BREAK) {
			swapCommand = false; 
			scriptInterface->resetScriptLoop();
			return executeCommandStatus();
		}

		int nbrLoop = evalInt(argLoop);
		if (nbrLoop < 1) {
			swapCommand = true;
			return executeCommandStatus();
		} else if (nbrLoop >1) {
			scriptInterface->setScriptLoop(true);
			scriptInterface->setScriptNbrLoop(nbrLoop-1);
			return executeCommandStatus();
		}
	}

	if (args[W_PRINT] ==W_VAR) {
		appEval->printVar();
	}

	return executeCommandStatus();
}


int AppCommandInterface::commandRandom()
{
	bool status = false;
	std::string argMin = args[W_MIN];
	if (!argMin.empty()) {
		appEval->commandRandomMin(argMin);
		status = true;
	}
	std::string argMax = args[W_MAX];
	if (!argMax.empty()) {
		appEval->commandRandomMax(argMax);
		status = true;
	}
	if (status == false)
		debug_message= _("unknown random parameter");
	return executeCommandStatus();
}
