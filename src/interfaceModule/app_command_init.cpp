#include "interfaceModule/app_command_init.hpp"


AppCommandInit::AppCommandInit() {}

AppCommandInit::~AppCommandInit() {}

std::map<const std::string, SC_COMMAND> AppCommandInit::initialiseCommandsName() {
    std::map<const std::string, SC_COMMAND> m_commands;

    m_commands["add"] = SC_COMMAND::SC_ADD;
	m_commands["audio"] = SC_COMMAND::SC_AUDIO;
	m_commands["body_trace"] = SC_COMMAND::SC_BODY_TRACE;
	m_commands["audio"] = SC_COMMAND::SC_AUDIO;
	m_commands["body"] = SC_COMMAND::SC_BODY;

	m_commands["camera"] = SC_COMMAND::SC_CAMERA;
	m_commands["flyto"] = SC_COMMAND::SC_CAMERA; //alias de camera

	m_commands["clear"] = SC_COMMAND::SC_CLEAR;
	m_commands["color"] = SC_COMMAND::SC_COLOR;
	m_commands["configuration"] = SC_COMMAND::SC_CONFIGURATION;
	m_commands["constellation"] = SC_COMMAND::SC_CONSTELLATION;
	m_commands["date"] = SC_COMMAND::SC_DATE;
	m_commands["define"] = SC_COMMAND::SC_DEFINE;

	m_commands["deselect"] = SC_COMMAND::SC_DESELECT;
	m_commands["domemasters"] = SC_COMMAND::SC_DOMEMASTERS;
	m_commands["dso"] = SC_COMMAND::SC_DSO;
	m_commands["external_mplayer"] = SC_COMMAND::SC_EXERNASC_MPLAYER;
	m_commands["external_viewer"] = SC_COMMAND::SC_EXTERNASC_VIEWER;

	m_commands["flag"] = SC_COMMAND::SC_FLAG;
	m_commands["get"] = SC_COMMAND::SC_GET;
	m_commands["illuminate"] = SC_COMMAND::SC_ILLUMINATE;
	m_commands["image"] = SC_COMMAND::SC_IMAGE;
	m_commands["landscape"] = SC_COMMAND::SC_LANDSCAPE;

	m_commands["look_at"] = SC_COMMAND::SC_LOOK;
	m_commands["media"] = SC_COMMAND::SC_MEDIA;
	m_commands["meteors"] = SC_COMMAND::SC_METEORS;
	m_commands["moveto"] = SC_COMMAND::SC_MOVETO;
	m_commands["movetocity"] = SC_COMMAND::SC_MOVETOCITY;

	// m_commands["multiplier"] = SC_COMMAND::SC_MULTIPLIER;
	m_commands["multiply"] = SC_COMMAND::SC_MULTIPLY;
	m_commands["personal"] = SC_COMMAND::SC_PERSONAL;
	m_commands["personeq"] = SC_COMMAND::SC_PERSONEQ;
	m_commands["planet_scale"] = SC_COMMAND::SC_PLANET_SCALE;

	m_commands["position"] = SC_COMMAND::SC_POSITION;
	m_commands["print"] = SC_COMMAND::SC_PRINT;
	m_commands["random"] = SC_COMMAND::SC_RANDOM;
	m_commands["script"] = SC_COMMAND::SC_SCRIPT;
	m_commands["search"] = SC_COMMAND::SC_SEARCH;

	m_commands["select"] = SC_COMMAND::SC_SELECT;
	m_commands["set"] = SC_COMMAND::SC_SET;
	m_commands["shutdown"] = SC_COMMAND::SC_SHUTDOWN;
	m_commands["sky_culture"] = SC_COMMAND::SC_SKY_CULTURE;

	m_commands["star_lines"] = SC_COMMAND::SC_STAR_LINES;
	m_commands["struct"] = SC_COMMAND::SC_STRUCT;
	m_commands["suntrace"] = SC_COMMAND::SC_SUNTRACE;
	m_commands["text"] = SC_COMMAND::SC_TEXT;
	m_commands["timerate"] = SC_COMMAND::SC_TIMERATE;

	m_commands["wait"] = SC_COMMAND::SC_WAIT;
	m_commands["zoom"] = SC_COMMAND::SC_ZOOMR;

    return m_commands;
}

std::map<const std::string, FLAG_NAMES> AppCommandInit::initialiseFlagsName() {}
std::map<const std::string, COLORCOMMAND_NAMES> AppCommandInit::initialiseColorCommand() {}
std::map<const std::string, SCD_NAMES> AppCommandInit::initialiseSetCommand() {}