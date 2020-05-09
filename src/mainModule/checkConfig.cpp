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
	tmpSettings["debug"]="false";
	// mainSettings["debug_opengl"]="false";
	tmpSettings["flag_optoma"]="false";
	// mainSettings["script_debug"]="false";
	tmpSettings["cpu_info"]="false";
	tmpSettings["flag_always_visible"]="true";

	sectionSettings.push_back("main");
	insertKeyFromTmpSettings("main");
	tmpSettings.clear();
}


void CheckConfig::checkIoSettings()
{
	tmpSettings["enable_mkfifo"]="false";
	tmpSettings["enable_tcp"]="true";
	// ioSettings["enable_mplayer"]="false";
	tmpSettings["tcp_port_in"]="7805";
	tmpSettings["tcp_buffer_in_size"]="1024";
	tmpSettings["mkfifo_file_in"]="/tmp/spacecrafter.fifo";
	tmpSettings["mkfifo_buffer_in_size"]="256";
	tmpSettings["flag_masterput"]="false";
	// ioSettings["mplayer_name"]="/usr/bin/mplayer";
	// ioSettings["mplayer_mkfifo_name"]="/tmp/mplayer_mkfifo_name.fifo";

	sectionSettings.push_back("io");
	insertKeyFromTmpSettings("io");
	tmpSettings.clear();
}


void CheckConfig::checkVideoSettings()
{
	tmpSettings["autoscreen"]="false";
	tmpSettings["fullscreen"]="false";
	tmpSettings["screen_w"]="1024";
	tmpSettings["screen_h"]="768";
	tmpSettings["bbp_mode"]="24";
	tmpSettings["maximum_fps"]="60";
	tmpSettings["rec_video_fps"]="30";

	sectionSettings.push_back("video");
	insertKeyFromTmpSettings("video");
	tmpSettings.clear();
}


void CheckConfig::checkRenderingSettings()
{
	tmpSettings["flag_antialias_lines"]="true";
	tmpSettings["antialiasing"]="8";
	tmpSettings["line_width"]="1.5";
	tmpSettings["landscape_slices"]="80";
	tmpSettings["landscape_stacks"]="20";
	tmpSettings["min_tes_level"]="1";
	tmpSettings["max_tes_level"]="1";
	tmpSettings["planet_altimetry_level"]="1";
	tmpSettings["earth_altimetry_level"]="1";
	tmpSettings["moon_altimetry_level"]="1";
	//~ renderingSettings["milkyway_slices"]="80";
	//~ renderingSettings["milkyway_stacks"]="20";
	//~ renderingSettings["sphere_low"]="10";
	//~ renderingSettings["sphere_medium"]="60";
	//~ renderingSettings["sphere_high"]="80";
	tmpSettings["rings_low"]="64";
	tmpSettings["rings_medium"]="256";
	tmpSettings["rings_high"]="512";
	tmpSettings["oort_elements"]="10000";

	sectionSettings.push_back("rendering");
	insertKeyFromTmpSettings("rendering");
	tmpSettings.clear();
}


void CheckConfig::checkLocalizationSettings()
{
	tmpSettings["sky_culture"]="western-color";
	tmpSettings["sky_locale"]="fr";
	tmpSettings["time_display_format"]="24h";
	tmpSettings["date_display_format"]="ddmmyyyy";
	tmpSettings["app_locale"]="fr";
	tmpSettings["time_zone"]="Europe/Paris";

	sectionSettings.push_back("localization");
	insertKeyFromTmpSettings("localization");
	tmpSettings.clear();
}


void CheckConfig::checkStarSettings()
{
	tmpSettings["star_scale"]="1.0";
	tmpSettings["star_mag_scale"]="1.0";
	tmpSettings["star_twinkle_amount"]="0.4";
	tmpSettings["max_mag_star_name"]="1.5";
	tmpSettings["flag_star_twinkle"]="true";
	tmpSettings["star_limiting_mag"]="6.5";
	tmpSettings["mag_converter_min_fov"]="0.1";
	tmpSettings["mag_converter_max_fov"]="60";
	tmpSettings["mag_converter_mag_shift"]="0.0";
	tmpSettings["mag_converter_max_mag"]="30";
	tmpSettings["illuminate_size"]="60";

	sectionSettings.push_back("stars");
	insertKeyFromTmpSettings("stars");
	tmpSettings.clear();
}

void CheckConfig::checkGuiSettings()
{
	tmpSettings["flag_show_fps"]="false";
	tmpSettings["flag_show_fov"]="false";
	tmpSettings["flag_show_latlon"]="false";
	tmpSettings["flag_number_print"]="1";
	tmpSettings["datetime_display_position"]="105";
	tmpSettings["object_info_display_position"]="300";
	tmpSettings["flag_show_planetname"]="true";
	tmpSettings["mouse_cursor_timeout"]="1";
	tmpSettings["menu_display_position"]="-150";
	tmpSettings["flag_mouse_usable_in_script"]="true";

	sectionSettings.push_back("gui");
	insertKeyFromTmpSettings("gui");
	tmpSettings.clear();
}


void CheckConfig::checkFontSettings()
{
	tmpSettings["font_resolution_size"]= "1024";
	tmpSettings["font_general_name"]="DejaVuSansMono.ttf";
	tmpSettings["font_general_size"]="12";
	tmpSettings["font_menu_name"]="DejaVuSans.ttf";
	tmpSettings["font_menutui_size"]="18";
	tmpSettings["font_planet_name"]="DejaVuSans.ttf";
	tmpSettings["font_planet_size"]="20";
	tmpSettings["font_constellation_name"]="DejaVuSans.ttf";
	tmpSettings["font_constellation_size"]="22";
	tmpSettings["font_cardinalpoints_size"]="30";
	tmpSettings["font_text_name"]="DejaVuSans.ttf";
	tmpSettings["font_text_size"]="16";
	tmpSettings["font_menugui_size"]="12.5";
	sectionSettings.push_back("font");
	insertKeyFromTmpSettings("font");
	tmpSettings.clear();
}


void CheckConfig::checkTuiSettings()
{
	tmpSettings["flag_enable_tui_menu"]="true";
	tmpSettings["flag_show_gravity_ui"]="true";
	tmpSettings["flag_show_tui_datetime"]="false";
	tmpSettings["flag_show_tui_short_obj_info"]="false";
	tmpSettings["text_ui"] = "0.5,1.0,0.5";
	tmpSettings["text_tui_root"] = "0.5,0.7,1.0";

	sectionSettings.push_back("tui");
	insertKeyFromTmpSettings("tui");
	tmpSettings.clear();
}


void CheckConfig::checkLandscapeSettings()
{
	tmpSettings["flag_landscape"]="true";
	tmpSettings["flag_fog"]="false";
	tmpSettings["flag_atmosphere"]="true";

	sectionSettings.push_back("landscape");
	insertKeyFromTmpSettings("landscape");
	tmpSettings.clear();
}


void CheckConfig::checkColorSettings()
{
	tmpSettings["azimuthal_color"] = "0,0.4,0.6";
	tmpSettings["equatorial_color"] = "0.5,1,0.5";
	tmpSettings["ecliptic_color"] = "1,0.2,0.2";
	tmpSettings["galactic_color"] = "0.8,0.8,0.8";
	tmpSettings["ecliptic_color"] = "1,0.2,0.2";
	tmpSettings["ecliptic_center_color"] = "0.8,0.8,0.8";
	tmpSettings["galactic_center_color"] = "0.8,0.8,0.8";
	tmpSettings["galactic_pole_color"] = "0.8,0.8,0.8";
	tmpSettings["nebula_label_color"] = "0.5,0.5,0.5";
	tmpSettings["nebula_circle_color"] = "0.15,0.15,0.15";
	tmpSettings["precession_circle_color"] = "0.6,0.4,0";
	tmpSettings["circumpolar_circle_color"] = "0.8,0.8,0.8";
	tmpSettings["oort_color"] = "0.0,0.5,1.0";
	tmpSettings["galactic_color"] = "0.8,0.8,0.8";
	tmpSettings["vernal_points_color"] = "0.8,0.8,0.8";
	tmpSettings["planet_halo_color"] = "1.0,1.0,1.0";
	tmpSettings["planet_names_color"] = "0.3,0.7,1";
	tmpSettings["planet_orbits_color"] = "0.2,0.2,0.2";
	tmpSettings["object_trails_color"] = "1,0.5,0";
	tmpSettings["equator_color"] = "0.5,1,0.5";
	tmpSettings["const_lines_color"] = "0.05,0.05,0.3";
	tmpSettings["const_lines3D_color"] = "0.5,0.2,0.2";
	tmpSettings["const_boundary_color"] = "0.4,0.3,0";
	tmpSettings["const_names_color"] = "0.6,0.7,0";
	tmpSettings["const_art_color"] = "1,1,1";
	tmpSettings["analemma_line_color"] = "1,0.5,0";
	tmpSettings["analemma_color"] = "1,1,0.5";
	tmpSettings["aries_color"] = "0.8,0.8,0.8";
	tmpSettings["cardinal_color"] = "1,1,0.6";
	tmpSettings["ecliptic_center_color"] = "0.8,0.8,0.8";
	tmpSettings["galactic_pole_color"] = "0.8,0.8,0.8";
	tmpSettings["galactic_center_color"] = "0.8,0.8,0.8";
	tmpSettings["greenwich_color"] = "1,0,0";
	tmpSettings["meridian_color"] = "0,0.8,1";
	tmpSettings["personal_color"] = "0.8,0.8,0";
	tmpSettings["personeq_color"] = "0.3,0.3,0.3";
	tmpSettings["nautical_alt_color"] = "0.8,0.8,0";
	tmpSettings["nautical_ra_color"] = "0.3,0.3,0.3";
	tmpSettings["object_coordinates_color"] = "0.8,0.8,0";
	tmpSettings["mouse_coordinates_color"] = "0.8,0.8,0";
	tmpSettings["angular_distance_color"] = "0.8,0.8,0";
	tmpSettings["loxodromy_color"] = "0.9,0.4,0.4";
	tmpSettings["orthodromy_color"] = "0.4,0.4,0.9";
	tmpSettings["polar_color"] = "0.5,0.3,0";
	tmpSettings["text_usr_color"] = "0.8,0.8,0.8";
	tmpSettings["vernal_points_color"] = "0.8,0.8,0.8";
	tmpSettings["vertical_color"] = "0.0,0.8,1.0";
	tmpSettings["zenith_color"] = "0.0,1.0,0.0";
	tmpSettings["zodiac_color"] = "1.0,0,1.0";

	sectionSettings.push_back("color");
	insertKeyFromTmpSettings("color");
	tmpSettings.clear();
}


void CheckConfig::checkViewingSettings()
{
	tmpSettings["nebula_picto_size"] = "6";
	tmpSettings["atmosphere_fade_duration"] = "2";
	tmpSettings["flag_constellation_drawing"] = "false";
	tmpSettings["flag_constellation_name"] = "false";
	tmpSettings["flag_constellation_boundaries"] = "false";
	tmpSettings["flag_constellation_art"] = "false";
	tmpSettings["flag_constellation_pick"] = "true";
	tmpSettings["constellation_art_intensity"] = "0.5";
	tmpSettings["constellation_art_fade_duration"] = "2";
	tmpSettings["flag_azimutal_grid"] = "false";
	tmpSettings["flag_equatorial_grid"] = "false";
	tmpSettings["flag_ecliptic_grid"] = "false";
	tmpSettings["flag_galactic_grid"] = "false";
	tmpSettings["flag_equator_line"] = "false";
	tmpSettings["flag_galactic_line"] = "false";
	tmpSettings["flag_ecliptic_line"] = "false";
	tmpSettings["flag_precession_circle"] = "false";
	tmpSettings["flag_circumpolar_circle"] = "false";
	tmpSettings["flag_tropic_lines"] = "false";
	tmpSettings["flag_meridian_line"] = "false";
	tmpSettings["flag_zenith_line"] = "false";
	tmpSettings["flag_polar_circle"] = "false";
	tmpSettings["flag_polar_point"] = "false";
	tmpSettings["flag_ecliptic_center"] = "false";
	tmpSettings["flag_galactic_pole"] = "false";
	tmpSettings["flag_galactic_center"] = "false";
	tmpSettings["flag_vernal_points"] = "false";
	tmpSettings["flag_analemma_line"] = "false";
	tmpSettings["flag_analemma"] = "false";
	tmpSettings["flag_aries_line"] = "false";
	tmpSettings["flag_zodiac"] = "false";
	tmpSettings["flag_cardinal_points"] = "false";
	tmpSettings["flag_vertical_line"] = "false";
	tmpSettings["flag_greenwich_line"] = "false";
	tmpSettings["flag_personal"] = "false";
	tmpSettings["flag_personeq"] = "false";
	tmpSettings["flag_nautical_ra"] = "false";
	tmpSettings["flag_nautical_alt"] = "false";
	tmpSettings["flag_object_coordinates"] = "false";
	tmpSettings["flag_mouse_coordinates"] = "false";
	tmpSettings["flag_angular_distance"] = "false";
	tmpSettings["flag_loxodromy"] = "false";
	tmpSettings["flag_orthodromy"] = "false";
	tmpSettings["flag_oort"] = "true";
	tmpSettings["flag_moon_scaled"] = "true";
	tmpSettings["flag_sun_scaled"] = "false";
	tmpSettings["flag_atmospheric_refraction"] = "false";
	tmpSettings["moon_scale"] = "5";
	tmpSettings["sun_scale"] = "5";
	tmpSettings["light_pollution_limiting_magnitude"] = "6";

	sectionSettings.push_back("viewing");
	insertKeyFromTmpSettings("viewing");
	tmpSettings.clear();
}


void CheckConfig::checkNavigationSettings()
{
	tmpSettings["flag_navigation"]="false";
	tmpSettings["preset_sky_time"]="2453065.333344907";
	tmpSettings["auto_move_duration"]="5";
	tmpSettings["day_key_mode"]="calendar";
	// navigationSettings["flag_enable_drag_mouse"]="true";
	tmpSettings["flag_enable_move_keys"]="true";
	tmpSettings["flag_enable_zoom_keys"]="true";
	tmpSettings["flag_manual_zoom"]="false";
	tmpSettings["heading"]="0";
	tmpSettings["init_fov"]="180";
	tmpSettings["init_view_pos"]="1e-04,1e-04,1";
	tmpSettings["mouse_zoom"]="30";
	tmpSettings["move_speed"]="0.0001";
	tmpSettings["startup_time_mode"]="Actual";
	tmpSettings["view_offset"]="0";
	tmpSettings["viewing_mode"]="equator";
	tmpSettings["zoom_speed"]="0.0001";
	tmpSettings["stall_radius_unit"]= "5.0";

	sectionSettings.push_back("navigation");
	insertKeyFromTmpSettings("navigation");
	tmpSettings.clear();
}


void CheckConfig::checkAstroSettings()
{
	sectionSettings.push_back("astro");
	tmpSettings["flag_stars"]="true";
	tmpSettings["flag_star_name"]="false";
	tmpSettings["flag_star_lines"]="false";
	tmpSettings["flag_planets"]="true";
	tmpSettings["flag_planets_hints"]="false";
	tmpSettings["flag_planets_orbits"]="false";
	tmpSettings["flag_nebula"]="true";
	tmpSettings["flag_milky_way"]="true";
	tmpSettings["flag_zodiacal_light"]="false";
	tmpSettings["flag_bright_nebulae"]="true";
	tmpSettings["milky_way_fader_duration"]="2";
	tmpSettings["milky_way_intensity"]="0.7";
	tmpSettings["zodiacal_intensity"]="0.7";
	tmpSettings["milky_way_texture"]="milkyway.png";
	tmpSettings["milky_way_iris_texture"]="milkyway_iris.png";
	tmpSettings["zodiacal_light_texture"]="zodiacale.png";
	tmpSettings["flag_nebula_hints"]="false";
	tmpSettings["flag_nebula_names"]="false";
	tmpSettings["max_mag_nebula_name"]="99";
	tmpSettings["flag_object_trails"]="false";
	tmpSettings["flag_light_travel_time"]="true";
	tmpSettings["planet_size_marginal_limit"]="0";
	tmpSettings["star_size_limit"]="9";
	tmpSettings["meteor_rate"]="10";

	sectionSettings.push_back("astro");
	insertKeyFromTmpSettings("astro");
	tmpSettings.clear();
}

void CheckConfig::checkLocationSettings()
{
	tmpSettings["landscape_name"]="forest";
	tmpSettings["name"]="guereins";
	tmpSettings["home_planet"]="Earth";
	tmpSettings["altitude"]="230";
	tmpSettings["latitude"]="+46d6'29.0\"";
	tmpSettings["longitude"]="+4d46'47.0\"";

	sectionSettings.push_back("init_location");
	insertKeyFromTmpSettings("init_location");
	tmpSettings.clear();
}


void CheckConfig::checkConfigIni(const std::string &fullpathfile, const std::string &_VERSION)
{
	user_conf.load(fullpathfile);
	if (user_conf.getStr("main:version") == _VERSION) {
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
	user_conf.setBoolean("io:flag_masterput", user_conf.getBoolean("main:flag_masterput"));
	user_conf.setBoolean("navigation:flag_navigation", user_conf.getBoolean("main:flag_navigation"));		
}


void CheckConfig::insertKeyFromTmpSettings(std::string nameSection)
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
					cLog::get()->write("key" + *itKey + " doesn't exist, you can safely discard it", LOG_TYPE::L_WARNING);
				}
			}
		}
	}
}
