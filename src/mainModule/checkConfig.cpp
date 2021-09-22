/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014_2020 Association Sirius & LSS team
 * Spacecrafter astronomy simulation and visualization
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

#include <algorithm>
#include "mainModule/checkConfig.hpp"
#include "tools/log.hpp"

CheckConfig::CheckConfig()
{
	sectionKeySettings.push_back("main:version");
}

CheckConfig::~CheckConfig()
{
	sectionSettings.clear();
	sectionKeySettings.clear();
	tmpSettings.clear();
}

void CheckConfig::checkMainSettings()
{
	tmpSettings[SCK_DEBUG]="false";
	tmpSettings[SCK_LOG]="true";
	// mainSettings["debug_opengl"]="false";
	tmpSettings[SCK_MILKYWAY_IRIS] = "false";
	// tmpSettings[SCK_FLAG_OPTOMA]="false";
	// mainSettings["script_debug"]="false";
	tmpSettings[SCK_CPU_INFO]="false";
	tmpSettings[SCK_FLAG_ALWAYS_VISIBLE]="true";

	sectionSettings.push_back(SCS_MAIN);
	insertKeyFromTmpSettings(SCS_MAIN);
	tmpSettings.clear();
}


void CheckConfig::checkIoSettings()
{
	tmpSettings[SCK_ENABLE_MKFIFO]="false";
	tmpSettings[SCK_ENABLE_TCP]="true";
	// ioSettings["enable_mplayer"]="false";
	tmpSettings[SCK_TCP_PORT_IN]="7805";
	tmpSettings[SCK_TCP_BUFFER_IN_SIZE]="1024";
	tmpSettings[SCK_MKFIFO_FILE_IN]="/tmp/spacecrafter.fifo";
	tmpSettings[SCK_MKFIFO_BUFFER_IN_SIZE]="256";
	tmpSettings[SCK_FLAG_MASTERPUT]="false";
	// ioSettings["mplayer_name"]="/usr/bin/mplayer";
	// ioSettings["mplayer_mkfifo_name"]="/tmp/mplayer_mkfifo_name.fifo";

	sectionSettings.push_back(SCS_IO);
	insertKeyFromTmpSettings(SCS_IO);
	tmpSettings.clear();
}


void CheckConfig::checkVideoSettings()
{
	tmpSettings[SCK_AUTOSCREEN]="false";
	tmpSettings[SCK_FULLSCREEN]="false";
	tmpSettings[SCK_SCREEN_W]="1024";
	tmpSettings[SCK_SCREEN_H]="768";
	//tmpSettings[SCK_BBP_MODE]="24";
	tmpSettings[SCK_MAXIMUM_FPS]="60";
	tmpSettings[SCK_REC_VIDEO_FPS]="30";

	sectionSettings.push_back(SCS_VIDEO);
	insertKeyFromTmpSettings(SCS_VIDEO);
	tmpSettings.clear();
}


void CheckConfig::checkRenderingSettings()
{
	tmpSettings[SCK_FLAG_ANTIALIAS_LINES]="true";
	tmpSettings[SCK_LOW_RES]="false";
	tmpSettings[SCK_LOW_RES_MAX]="1024";
	tmpSettings[SCK_ANTIALIASING]="8";
	tmpSettings[SCK_LINE_WIDTH]="1.5";
	tmpSettings[SCK_LANDSCAPE_SLICES]="80";
	tmpSettings[SCK_LANDSCAPE_STACKS]="20";
	tmpSettings[SCK_MIN_TES_LEVEL]="1";
	tmpSettings[SCK_MAX_TES_LEVEL]="1";
	tmpSettings[SCK_PLANET_ALTIMETRY_LEVEL]="1";
	tmpSettings[SCK_EARTH_ALTIMETRY_LEVEL]="1";
	tmpSettings[SCK_MOON_ALTIMETRY_LEVEL]="1";
	//~ renderingSettings["milkyway_slices"]="80";
	//~ renderingSettings["milkyway_stacks"]="20";
	//~ renderingSettings["sphere_low"]="10";
	//~ renderingSettings["sphere_medium"]="60";
	//~ renderingSettings["sphere_high"]="80";
	tmpSettings[SCK_RINGS_LOW]="64";
	tmpSettings[SCK_RINGS_MEDIUM]="256";
	tmpSettings[SCK_RINGS_HIGH]="512";
	tmpSettings[SCK_OORT_ELEMENTS]="10000";

	sectionSettings.push_back(SCS_RENDERING);
	insertKeyFromTmpSettings(SCS_RENDERING);
	tmpSettings.clear();
}


void CheckConfig::checkLocalizationSettings()
{
	tmpSettings[SCK_SKY_CULTURE]="western-color";
	tmpSettings[SCK_SKY_LOCALE]="fr";
	tmpSettings[SCK_TIME_DISPLAY_FORMAT]="24h";
	tmpSettings[SCK_DATE_DISPLAY_FORMAT]="ddmmyyyy";
	tmpSettings[SCK_APP_LOCALE]="fr";
	tmpSettings[SCK_TIME_ZONE]="Europe/Paris";

	sectionSettings.push_back(SCS_LOCALIZATION);
	insertKeyFromTmpSettings(SCS_LOCALIZATION);
	tmpSettings.clear();
}


void CheckConfig::checkStarSettings()
{
	tmpSettings[SCK_STAR_SCALE]="1.0";
	tmpSettings[SCK_STAR_MAG_SCALE]="1.0";
	tmpSettings[SCK_STAR_TWINKLE_AMOUNT]="0.4";
	tmpSettings[SCK_MAX_MAG_STAR_NAME]="1.5";
	tmpSettings[SCK_FLAG_STAR_TWINKLE]="true";
	tmpSettings[SCK_STAR_LIMITING_MAG]="6.5";
	tmpSettings[SCK_MAG_CONVERTER_MIN_FOV]="0.1";
	tmpSettings[SCK_MAG_CONVERTER_MAX_FOV]="60";
	tmpSettings[SCK_MAG_CONVERTER_MAG_SHIFT]="0.0";
	tmpSettings[SCK_MAG_CONVERTER_MAX_MAG]="30";
	tmpSettings[SCK_ILLUMINATE_SIZE]="60";

	sectionSettings.push_back(SCS_STARS);
	insertKeyFromTmpSettings(SCS_STARS);
	tmpSettings.clear();
}

void CheckConfig::checkGuiSettings()
{
	tmpSettings[SCK_FLAG_SHOW_FPS]="false";
	tmpSettings[SCK_FLAG_SHOW_FOV]="false";
	tmpSettings[SCK_FLAG_SHOW_LATLON]="false";
	tmpSettings[SCK_FLAG_NUMBER_PRINT]="1";
	tmpSettings[SCK_DATETIME_DISPLAY_POSITION]="105";
	tmpSettings[SCK_OBJECT_INFO_DISPLAY_POSITION]="300";
	tmpSettings[SCK_FLAG_SHOW_PLANETNAME]="true";
	tmpSettings[SCK_MOUSE_CURSOR_TIMEOUT]="1";
	tmpSettings[SCK_MENU_DISPLAY_POSITION]="-150";
	tmpSettings[SCK_FLAG_MOUSE_USABLE_IN_SCRIPT]="true";

	sectionSettings.push_back(SCS_GUI);
	insertKeyFromTmpSettings(SCS_GUI);
	tmpSettings.clear();
}


void CheckConfig::checkFontSettings()
{
	tmpSettings[SCK_FONT_RESOLUTION_SIZE]= "1024";
	tmpSettings[SCK_FONT_GENERAL_NAME]="DejaVuSansMono.ttf";
	// tmpSettings[SCK_FONT_GENERAL_SIZE]="12";
	tmpSettings[SCK_FONT_MENU_NAME]="DejaVuSans.ttf";
	tmpSettings[SCK_FONT_MENUTUI_SIZE]="18";
	tmpSettings[SCK_FONT_PLANET_NAME]="DejaVuSans.ttf";
	tmpSettings[SCK_FONT_PLANET_SIZE]="20";
	tmpSettings[SCK_FONT_CONSTELLATION_NAME]="DejaVuSans.ttf";
	tmpSettings[SCK_FONT_CONSTELLATION_SIZE]="22";
	tmpSettings[SCK_FONT_CARDINALPOINTS_NAME] = "DejaVuSans.ttf";
	tmpSettings[SCK_FONT_CARDINALPOINTS_SIZE]="30";
	tmpSettings[SCK_FONT_TEXT_NAME]="DejaVuSans.ttf";
	tmpSettings[SCK_FONT_TEXT_SIZE]="16";
	tmpSettings[SCK_FONT_GRID_NAME] = "DejaVuSans.ttf";
	tmpSettings[SCK_FONT_GRID_SIZE]="12";
	tmpSettings[SCK_FONT_LINES_NAME] = "DejaVuSans.ttf";
	tmpSettings[SCK_FONT_LINE_SIZE]="12";
	tmpSettings[SCK_FONT_DISPLAY_NAME] = "DejaVuSans.ttf";
	tmpSettings[SCK_FONT_DISPLAY_SIZE]="12";
	tmpSettings[SCK_FONT_HIPSTARS_NAME] = "DejaVuSans.ttf";
	tmpSettings[SCK_FONT_HIPSTARS_SIZE]="12";
	tmpSettings[SCK_FONT_NEBULAS_NAME] = "DejaVuSans.ttf";
	tmpSettings[SCK_FONT_NEBULAS_SIZE]="12";

	tmpSettings[SCK_FONT_MENUGUI_SIZE]="12.5";
	sectionSettings.push_back(SCS_FONT);
	insertKeyFromTmpSettings(SCS_FONT);
	tmpSettings.clear();
}


void CheckConfig::checkTuiSettings()
{
	tmpSettings[SCK_FLAG_ENABLE_TUI_MENU]="true";
	tmpSettings[SCK_FLAG_SHOW_GRAVITY_UI]="true";
	tmpSettings[SCK_FLAG_SHOW_TUI_DATETIME]="false";
	tmpSettings[SCK_FLAG_SHOW_TUI_SHORT_OBJ_INFO]="false";
	tmpSettings[SCK_TEXT_UI] = "0.5,1.0,0.5";
	tmpSettings[SCK_TEXT_TUI_ROOT] = "0.5,0.7,1.0";

	sectionSettings.push_back(SCS_TUI);
	insertKeyFromTmpSettings(SCS_TUI);
	tmpSettings.clear();
}


void CheckConfig::checkLandscapeSettings()
{
	tmpSettings[SCK_FLAG_LANDSCAPE]="true";
	tmpSettings[SCK_FLAG_FOG]="false";
	tmpSettings[SCK_FLAG_ATMOSPHERE]="true";

	sectionSettings.push_back(SCS_LANDSCAPE);
	insertKeyFromTmpSettings(SCS_LANDSCAPE);
	tmpSettings.clear();
}


void CheckConfig::checkColorSettings()
{
	tmpSettings[SCK_AZIMUTHAL_COLOR] = "0,0.4,0.6";
	tmpSettings[SCK_EQUATORIAL_COLOR] = "0.5,1,0.5";
	tmpSettings[SCK_ECLIPTIC_COLOR] = "1,0.2,0.2";
	tmpSettings[SCK_GALACTIC_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_ECLIPTIC_COLOR] = "1,0.2,0.2";
	tmpSettings[SCK_ECLIPTIC_CENTER_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_GALACTIC_CENTER_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_GALACTIC_POLE_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_NEBULA_LABEL_COLOR] = "0.5,0.5,0.5";
	tmpSettings[SCK_NEBULA_CIRCLE_COLOR] = "0.15,0.15,0.15";
	tmpSettings[SCK_PRECESSION_CIRCLE_COLOR] = "0.6,0.4,0";
	tmpSettings[SCK_CIRCUMPOLAR_CIRCLE_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_OORT_COLOR] = "0.0,0.5,1.0";
	tmpSettings[SCK_GALACTIC_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_VERNAL_POINTS_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_PLANET_HALO_COLOR] = "1.0,1.0,1.0";
	tmpSettings[SCK_PLANET_NAMES_COLOR] = "0.3,0.7,1";
	tmpSettings[SCK_PLANET_ORBITS_COLOR] = "0.2,0.2,0.2";
	tmpSettings[SCK_OBJECT_TRAILS_COLOR] = "1,0.5,0";
	tmpSettings[SCK_EQUATOR_COLOR] = "0.5,1,0.5";
	tmpSettings[SCK_CONST_LINES_COLOR] = "0.05,0.05,0.3";
	tmpSettings[SCK_CONST_LINES3D_COLOR] = "0.5,0.2,0.2";
	tmpSettings[SCK_CONST_BOUNDARY_COLOR] = "0.4,0.3,0";
	tmpSettings[SCK_CONST_NAMES_COLOR] = "0.6,0.7,0";
	tmpSettings[SCK_CONST_ART_COLOR] = "1,1,1";
	tmpSettings[SCK_ANALEMMA_LINE_COLOR] = "1,0.5,0";
	tmpSettings[SCK_ANALEMMA_COLOR] = "1,1,0.5";
	tmpSettings[SCK_ARIES_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_CARDINAL_COLOR] = "1,1,0.6";
	tmpSettings[SCK_ECLIPTIC_CENTER_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_GALACTIC_POLE_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_GALACTIC_CENTER_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_GREENWICH_COLOR] = "1,0,0";
	tmpSettings[SCK_MERIDIAN_COLOR] = "0,0.8,1";
	tmpSettings[SCK_PERSONAL_COLOR] = "0.8,0.8,0";
	tmpSettings[SCK_PERSONEQ_COLOR] = "0.3,0.3,0.3";
	tmpSettings[SCK_NAUTICAL_ALT_COLOR] = "0.8,0.8,0";
	tmpSettings[SCK_NAUTICAL_RA_COLOR] = "0.3,0.3,0.3";
	tmpSettings[SCK_OBJECT_COORDINATES_COLOR] = "0.8,0.8,0";
	tmpSettings[SCK_MOUSE_COORDINATES_COLOR] = "0.8,0.8,0";
	tmpSettings[SCK_ANGULAR_DISTANCE_COLOR] = "0.8,0.8,0";
	tmpSettings[SCK_LOXODROMY_COLOR] = "0.9,0.4,0.4";
	tmpSettings[SCK_ORTHODROMY_COLOR] = "0.4,0.4,0.9";
	tmpSettings[SCK_POLAR_COLOR] = "0.5,0.3,0";
	tmpSettings[SCK_TEXT_USR_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_VERNAL_POINTS_COLOR] = "0.8,0.8,0.8";
	tmpSettings[SCK_VERTICAL_COLOR] = "0.0,0.8,1.0";
	tmpSettings[SCK_ZENITH_COLOR] = "0.0,1.0,0.0";
	tmpSettings[SCK_ZODIAC_COLOR] = "1.0,0,1.0";

	sectionSettings.push_back(SCS_COLOR);
	insertKeyFromTmpSettings(SCS_COLOR);
	tmpSettings.clear();
}


void CheckConfig::checkViewingSettings()
{
	tmpSettings[SCK_NEBULA_PICTO_SIZE] = "6";
	tmpSettings[SCK_ATMOSPHERE_FADE_DURATION] = "2";
	tmpSettings[SCK_FLAG_CONSTELLATION_DRAWING] = "false";
	tmpSettings[SCK_FLAG_CONSTELLATION_NAME] = "false";
	tmpSettings[SCK_FLAG_CONSTELLATION_BOUNDARIES] = "false";
	tmpSettings[SCK_FLAG_CONSTELLATION_ART] = "false";
	tmpSettings[SCK_FLAG_CONSTELLATION_PICK] = "true";
	tmpSettings[SCK_CONSTELLATION_ART_INTENSITY] = "0.5";
	tmpSettings[SCK_CONSTELLATION_ART_FADE_DURATION] = "2";
	tmpSettings[SCK_FLAG_AZIMUTAL_GRID] = "false";
	tmpSettings[SCK_FLAG_EQUATORIAL_GRID] = "false";
	tmpSettings[SCK_FLAG_ECLIPTIC_GRID] = "false";
	tmpSettings[SCK_FLAG_GALACTIC_GRID] = "false";
	tmpSettings[SCK_FLAG_EQUATOR_LINE] = "false";
	tmpSettings[SCK_FLAG_GALACTIC_LINE] = "false";
	tmpSettings[SCK_FLAG_ECLIPTIC_LINE] = "false";
	tmpSettings[SCK_FLAG_PRECESSION_CIRCLE] = "false";
	tmpSettings[SCK_FLAG_CIRCUMPOLAR_CIRCLE] = "false";
	tmpSettings[SCK_FLAG_TROPIC_LINES] = "false";
	tmpSettings[SCK_FLAG_MERIDIAN_LINE] = "false";
	tmpSettings[SCK_FLAG_ZENITH_LINE] = "false";
	tmpSettings[SCK_FLAG_POLAR_CIRCLE] = "false";
	tmpSettings[SCK_FLAG_POLAR_POINT] = "false";
	tmpSettings[SCK_FLAG_ECLIPTIC_CENTER] = "false";
	tmpSettings[SCK_FLAG_GALACTIC_POLE] = "false";
	tmpSettings[SCK_FLAG_GALACTIC_CENTER] = "false";
	tmpSettings[SCK_FLAG_VERNAL_POINTS] = "false";
	tmpSettings[SCK_FLAG_ANALEMMA_LINE] = "false";
	tmpSettings[SCK_FLAG_ANALEMMA] = "false";
	tmpSettings[SCK_FLAG_ARIES_LINE] = "false";
	tmpSettings[SCK_FLAG_ZODIAC] = "false";
	tmpSettings[SCK_FLAG_CARDINAL_POINTS] = "false";
	tmpSettings[SCK_FLAG_VERTICAL_LINE] = "false";
	tmpSettings[SCK_FLAG_GREENWICH_LINE] = "false";
	tmpSettings[SCK_FLAG_PERSONAL] = "false";
	tmpSettings[SCK_FLAG_PERSONEQ] = "false";
	tmpSettings[SCK_FLAG_NAUTICAL_RA] = "false";
	tmpSettings[SCK_FLAG_NAUTICAL_ALT] = "false";
	tmpSettings[SCK_FLAG_OBJECT_COORDINATES] = "false";
	tmpSettings[SCK_FLAG_MOUSE_COORDINATES] = "false";
	tmpSettings[SCK_FLAG_ANGULAR_DISTANCE] = "false";
	tmpSettings[SCK_FLAG_LOXODROMY] = "false";
	tmpSettings[SCK_FLAG_ORTHODROMY] = "false";
	tmpSettings[SCK_FLAG_OORT] = "true";
	tmpSettings[SCK_FLAG_MOON_SCALED] = "true";
	tmpSettings[SCK_FLAG_SUN_SCALED] = "false";
	tmpSettings[SCK_FLAG_ATMOSPHERIC_REFRACTION] = "false";
	tmpSettings[SCK_MOON_SCALE] = "5";
	tmpSettings[SCK_SUN_SCALE] = "5";
	tmpSettings[SCK_LIGHT_POLLUTION_LIMITING_MAGNITUDE] = "6";

	sectionSettings.push_back(SCS_VIEWING);
	insertKeyFromTmpSettings(SCS_VIEWING);
	tmpSettings.clear();
}


void CheckConfig::checkNavigationSettings()
{
	tmpSettings[SCK_FLAG_NAVIGATION]="false";
	tmpSettings[SCK_PRESET_SKY_TIME]="2453065.333344907";
	tmpSettings[SCK_AUTO_MOVE_DURATION]="5";
	tmpSettings[SCK_DAY_KEY_MODE]="calendar";
	// navigationSettings["flag_enable_drag_mouse"]="true";
	tmpSettings[SCK_FLAG_ENABLE_MOVE_KEYS]="true";
	tmpSettings[SCK_FLAG_ENABLE_ZOOM_KEYS]="true";
	tmpSettings[SCK_FLAG_MANUAL_ZOOM]="false";
	tmpSettings[SCK_HEADING]="0";
	tmpSettings[SCK_INIT_FOV]="180";
	tmpSettings[SCK_INIT_VIEW_POS]="1e-04,1e-04,1";
	tmpSettings[SCK_MOUSE_ZOOM]="30";
	tmpSettings[SCK_MOVE_SPEED]="0.0001";
	tmpSettings[SCK_STARTUP_TIME_MODE]="Actual";
	tmpSettings[SCK_VIEW_OFFSET]="0";
	tmpSettings[SCK_VIEWING_MODE]="equator";
	tmpSettings[SCK_ZOOM_SPEED]="0.0001";
	tmpSettings[SCK_STALL_RADIUS_UNIT]= "5.0";

	sectionSettings.push_back(SCS_NAVIGATION );
	insertKeyFromTmpSettings(SCS_NAVIGATION );
	tmpSettings.clear();
}


void CheckConfig::checkAstroSettings()
{
	sectionSettings.push_back(SCS_ASTRO);
	tmpSettings[SCK_FLAG_STARS]="true";
	tmpSettings[SCK_FLAG_STAR_NAME]="false";
	tmpSettings[SCK_FLAG_STAR_LINES]="false";
	tmpSettings[SCK_FLAG_PLANETS]="true";
	tmpSettings[SCK_FLAG_PLANETS_HINTS]="false";
	tmpSettings[SCK_FLAG_PLANETS_ORBITS]="false";
	tmpSettings[SCK_FLAG_NEBULA]="true";
	tmpSettings[SCK_FLAG_MILKY_WAY]="true";
	tmpSettings[SCK_FLAG_ZODIACAL_LIGHT]="false";
	tmpSettings[SCK_FLAG_BRIGHT_NEBULAE]="true";
	tmpSettings[SCK_MILKY_WAY_FADER_DURATION]="2";
	tmpSettings[SCK_MILKY_WAY_INTENSITY]="0.7";
	tmpSettings[SCK_ZODIACAL_INTENSITY]="0.3";
	tmpSettings[SCK_MILKY_WAY_TEXTURE]="milkyway.png";
	tmpSettings[SCK_MILKY_WAY_IRIS_TEXTURE]="milkyway_iris.png";
	tmpSettings[SCK_ZODIACAL_LIGHT_TEXTURE]="zodiacale.png";
	tmpSettings[SCK_FLAG_NEBULA_HINTS]="false";
	tmpSettings[SCK_FLAG_NEBULA_NAMES]="false";
	tmpSettings[SCK_MAX_MAG_NEBULA_NAME]="99";
	tmpSettings[SCK_FLAG_OBJECT_TRAILS]="false";
	tmpSettings[SCK_FLAG_LIGHT_TRAVEL_TIME]="true";
	tmpSettings[SCK_PLANET_SIZE_MARGINAL_LIMIT]="0";
	tmpSettings[SCK_STAR_SIZE_LIMIT]="9";
	tmpSettings[SCK_METEOR_RATE]="10";

	sectionSettings.push_back(SCS_ASTRO);
	insertKeyFromTmpSettings(SCS_ASTRO);
	tmpSettings.clear();
}

void CheckConfig::checkLocationSettings()
{
	tmpSettings[SCK_LANDSCAPE_NAME]="forest";
	tmpSettings[SCK_NAME]="guereins";
	tmpSettings[SCK_HOME_PLANET]="Earth";
	tmpSettings[SCK_ALTITUDE]="230";
	tmpSettings[SCK_LATITUDE]="+46d6'29.0\"";
	tmpSettings[SCK_LONGITUDE]="+4d46'47.0\"";

	sectionSettings.push_back(SCS_INIT_LOCATION);
	insertKeyFromTmpSettings(SCS_INIT_LOCATION);
	tmpSettings.clear();
}


void CheckConfig::checkConfigIni(const std::string &fullpathfile, const std::string &_VERSION)
{
	user_conf.load(fullpathfile);
	if (user_conf.getStr(SCS_MAIN, SCK_VERSION) == _VERSION) {
		// 	std::cout << "nothing to do" << std::endl;
		//  for (auto i =0; i<user_conf.getNsec(); i++) {
		//	 	std::cout << user_conf.getSecname(i) << std::endl;

		// 	std::list<std::string> tmp = user_conf.getKeyFromSection(i);
		// 	for (auto it=tmp.begin(); it != tmp.end(); ++it)
		// 		std::cout << "|->" << *it << std::endl;
		// }
		cLog::get()->write("config.ini is up to date");
		// checkUselessSection();
		return; //(nothing to do, config.ini isn't outdated)
	}

	tmpSettings.clear();

	checkMainSettings();
	checkIoSettings();
	checkVideoSettings();
	checkRenderingSettings();
	checkLocalizationSettings();
	checkStarSettings();
	checkGuiSettings();
	checkFontSettings();
	checkTuiSettings();
	checkLandscapeSettings();
	checkColorSettings();
	checkViewingSettings();
	checkNavigationSettings();
	checkAstroSettings();
	checkLocationSettings();

	/*
	for (auto it=sectionSettings.begin(); it != sectionSettings.end(); ++it) {
		std::cout << (*it) << std::endl;
	}
	for (auto it=sectionKeySettings.begin(); it != sectionKeySettings.end(); ++it) {
		std::cout << (*it) << std::endl;
	}*/

	checkMigration2020();
	checkUselessSection();
	checkUselessKey();

//	std::cout << "i did " << std::endl;
	user_conf.setStr("main:version", _VERSION);
	user_conf.save(fullpathfile);

	// for (auto itKey=sectionKeySettings.begin(); itKey != sectionKeySettings.end(); ++itKey) {
	// 	std::cout << "clef : " << *itKey << std::endl;
	// }
}

void CheckConfig::checkMigration2020()
{
	user_conf.setBoolean("io:flag_masterput", user_conf.getBoolean(SCS_MAIN, SCK_FLAG_MASTERPUT));
	user_conf.setBoolean("navigation:flag_navigation", user_conf.getBoolean(SCS_MAIN, SCK_FLAG_NAVIGATION));
	if (double tmp = user_conf.getDouble(SCS_ASTRO, SCK_MILKY_WAY_FADER_DURATION); tmp>1000) {
		user_conf.setDouble(SCS_ASTRO, SCK_MILKY_WAY_FADER_DURATION, tmp/1000.);
	}
}


void CheckConfig::insertKeyFromTmpSettings(const std::string& nameSection)
{
	for (auto it=tmpSettings.begin(); it!=tmpSettings.end(); ++it) {
		//important to keep a track  name:key
		sectionKeySettings.push_back(nameSection+":"+it->first);
		if (!user_conf.findEntry(nameSection+":"+it->first))
			user_conf.setStr(nameSection+":"+it->first, it->second);
	}
}


void CheckConfig::checkUselessSection()
{
	for (auto i =0; i<user_conf.getNsec(); i++) {
		std::string test = user_conf.getSecname(i);
		auto it = std::find(sectionSettings.begin(), sectionSettings.end(), test);
		if(it == sectionSettings.end())
			// std::cout << "section [" << test << "] doesn't exist, you can safely discard it" << std::endl;
			cLog::get()->write("section [" + test + "] doesn't exist, you can safely discard it", LOG_TYPE::L_WARNING);
	}
}


void CheckConfig::checkUselessKey()
{
	for (auto i =0; i<user_conf.getNsec(); i++) {
		std::string test = user_conf.getSecname(i);
		// on recherche si la section existe
		auto itSec = std::find(sectionSettings.begin(), sectionSettings.end(), test);
		if(itSec != sectionSettings.end()) {
			// std::cout << "section " << *itSec << std::endl;
			std::list<std::string> listCandidateKey = user_conf.getKeyFromSection(i);
			//std::cout << "tmp " << tmp << std::endl;
			for (auto itKey=listCandidateKey.begin(); itKey != listCandidateKey.end(); ++itKey) {
				// std::cout << "clef candidate " << *itKey << std::endl;
				auto it3 = std::find(sectionKeySettings.begin(), sectionKeySettings.end(), *itKey);
				// Si la clef n'esite pas, on le notifie
				if(it3 == sectionKeySettings.end()) {
					//std::cout << "key " << *itKey << std::endl;/*" doesn't exist, you can safely discard it" << std::endl;*/
					cLog::get()->write("key " + *itKey + " doesn't exist, you can safely discard it", LOG_TYPE::L_WARNING);
				}
			}
		}
	}
}
