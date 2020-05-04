/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014_2015 Association Sirius & LSS team
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

#include <map>
#include "mainModule/checkkeys.hpp"

static void checkMainSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> mainSettings;
	mainSettings["debug"]="false";
	// mainSettings["debug_opengl"]="false";
	mainSettings["flag_masterput"]="false";
	mainSettings["flag_navigation"]="false";
	mainSettings["flag_optoma"]="false";
	// mainSettings["script_debug"]="false";
	mainSettings["cpu_info"]="false";
	mainSettings["flag_always_visible"]="true";

	for (std::map<std::string,std::string>::iterator it=mainSettings.begin(); it!=mainSettings.end(); ++it) {
		if (!user_conf.findEntry("main:"+it->first))
			user_conf.setStr("main:"+it->first, it->second);
	}
	mainSettings.clear();
}


static void checkIoSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> ioSettings;
	ioSettings["enable_mkfifo"]="false";
	ioSettings["enable_tcp"]="true";
	// ioSettings["enable_mplayer"]="false";
	ioSettings["tcp_port_in"]="7805";
	ioSettings["tcp_buffer_in_size"]="1024";
	ioSettings["mkfifo_file_in"]="/tmp/spacecrafter.fifo";
	ioSettings["mkfifo_buffer_in_size"]="256";
	// ioSettings["mplayer_name"]="/usr/bin/mplayer";
	// ioSettings["mplayer_mkfifo_name"]="/tmp/mplayer_mkfifo_name.fifo";

	for (std::map<std::string,std::string>::iterator it=ioSettings.begin(); it!=ioSettings.end(); ++it) {
		if (!user_conf.findEntry("io:"+it->first))
			user_conf.setStr("io:"+it->first, it->second);
	}

	ioSettings.clear();
}


static void checkVideoSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> videoSettings;
	videoSettings["autoscreen"]="false";
	videoSettings["fullscreen"]="false";
	videoSettings["screen_w"]="1024";
	videoSettings["screen_h"]="768";
	videoSettings["bbp_mode"]="24";
	videoSettings["maximum_fps"]="60";
	videoSettings["rec_video_fps"]="30";
	videoSettings["min_tes_level"]="1";
	videoSettings["max_tes_level"]="1";
	videoSettings["planet_altimetry_level"]="1";
	videoSettings["earth_altimetry_level"]="1";
	videoSettings["moon_altimetry_level"]="1";

	for (std::map<std::string,std::string>::iterator it=videoSettings.begin(); it!=videoSettings.end(); ++it) {
		if (!user_conf.findEntry("video:"+it->first))
			user_conf.setStr("video:"+it->first, it->second);
	}
	videoSettings.clear();
}


static void checkRenderingSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> renderingSettings;
	renderingSettings["flag_antialias_lines"]="true";
	renderingSettings["antialiasing"]="8";
	renderingSettings["line_width"]="1.5";
	renderingSettings["landscape_slices"]="80";
	renderingSettings["landscape_stacks"]="20";
	//~ renderingSettings["milkyway_slices"]="80";
	//~ renderingSettings["milkyway_stacks"]="20";
	//~ renderingSettings["sphere_low"]="10";
	//~ renderingSettings["sphere_medium"]="60";
	//~ renderingSettings["sphere_high"]="80";
	renderingSettings["rings_low"]="64";
	renderingSettings["rings_medium"]="256";
	renderingSettings["rings_high"]="512";
	renderingSettings["oort_elements"]="10000";

	for (std::map<std::string,std::string>::iterator it=renderingSettings.begin(); it!=renderingSettings.end(); ++it) {
		if (!user_conf.findEntry("rendering:"+it->first))
			user_conf.setStr("rendering:"+it->first, it->second);
	}

	renderingSettings.clear();
}


static void checkLocalizationSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> localizationSettings;
	localizationSettings["sky_culture"]="western-color";
	localizationSettings["sky_locale"]="fr";
	localizationSettings["time_display_format"]="24h";
	localizationSettings["date_display_format"]="ddmmyyyy";
	localizationSettings["app_locale"]="fr";
	localizationSettings["time_zone"]="Europe/Paris";

	for (std::map<std::string,std::string>::iterator it=localizationSettings.begin(); it!=localizationSettings.end(); ++it) {
		if (!user_conf.findEntry("localization:"+it->first))
			user_conf.setStr("localization:"+it->first, it->second);
	}

	localizationSettings.clear();
}


static void checkStarSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> starsSettings;
	starsSettings["star_scale"]="1.0";
	starsSettings["star_mag_scale"]="1.0";
	starsSettings["star_twinkle_amount"]="0.4";
	starsSettings["max_mag_star_name"]="1.5";
	starsSettings["flag_star_twinkle"]="true";
	starsSettings["star_limiting_mag"]="6.5";
	starsSettings["mag_converter_min_fov"]="0.1";
	starsSettings["mag_converter_max_fov"]="60";
	starsSettings["mag_converter_mag_shift"]="0.0";
	starsSettings["mag_converter_max_mag"]="30";
	starsSettings["illuminate_size"]="60";

	for (std::map<std::string,std::string>::iterator it=starsSettings.begin(); it!=starsSettings.end(); ++it) {
		if (!user_conf.findEntry("stars:"+it->first))
			user_conf.setStr("stars:"+it->first, it->second);
	}

	starsSettings.clear();
}


//~ static void checkJoystickSettings(InitParser &user_conf)
//~ {
	//~ std::map<std::string,std::string> joystickSettings;
	//~ joystickSettings["sensitivity"]="500";
	//~ joystickSettings["intensity"]="8000";

	//~ for (std::map<std::string,std::string>::iterator it=joystickSettings.begin(); it!=joystickSettings.end(); ++it) {
		//~ if (!user_conf.findEntry("joystick:"+it->first))
			//~ user_conf.setStr("joystick:"+it->first, it->second);
	//~ }

	//~ joystickSettings.clear();
//~ }


static void checkGuiSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> guiSettings;
	guiSettings["flag_show_fps"]="false";
	guiSettings["flag_show_fov"]="false";
	guiSettings["flag_show_latlon"]="false";
	guiSettings["flag_number_print"]="1";
	guiSettings["datetime_display_position"]="105";
	guiSettings["object_info_display_position"]="300";
	guiSettings["flag_show_planetname"]="true";
	guiSettings["mouse_cursor_timeout"]="1";
	guiSettings["menu_display_position"]="-150";
	guiSettings["flag_mouse_usable_in_script"]="true";

	for (std::map<std::string,std::string>::iterator it=guiSettings.begin(); it!=guiSettings.end(); ++it) {
		if (!user_conf.findEntry("gui:"+it->first))
			user_conf.setStr("gui:"+it->first, it->second);
	}

	guiSettings.clear();
}


static void checkFontSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> fontSettings;
	fontSettings["font_general_name"]="DejaVuSansMono.ttf";
	fontSettings["font_general_size"]="12";
	fontSettings["font_menu_name"]="DejaVuSans.ttf";
	fontSettings["font_menutui_size"]="18";
	fontSettings["font_planet_name"]="DejaVuSans.ttf";
	fontSettings["font_planet_size"]="20";
	fontSettings["font_constellation_name"]="DejaVuSans.ttf";
	fontSettings["font_constellation_size"]="22";
	fontSettings["font_cardinalpoints_size"]="30";
	fontSettings["font_text_name"]="DejaVuSans.ttf";
	fontSettings["font_text_size"]="16";
	fontSettings["font_menugui_size"]="12.5";

	for (std::map<std::string,std::string>::iterator it=fontSettings.begin(); it!=fontSettings.end(); ++it) {
		if (!user_conf.findEntry("font:"+it->first))
			user_conf.setStr("font:"+it->first, it->second);
	}

	fontSettings.clear();
}


static void checkTuiSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> tuiSettings;
	tuiSettings["flag_enable_tui_menu"]="true";
	tuiSettings["flag_show_gravity_ui"]="true";
	tuiSettings["flag_show_tui_datetime"]="false";
	tuiSettings["flag_show_tui_short_obj_info"]="false";
	tuiSettings["text_ui"] = "0.5,1.0,0.5";
	tuiSettings["text_tui_root"] = "0.5,0.7,1.0";

	for (std::map<std::string,std::string>::iterator it=tuiSettings.begin(); it!=tuiSettings.end(); ++it) {
		if (!user_conf.findEntry("tui:"+it->first))
			user_conf.setStr("tui:"+it->first, it->second);
	}

	tuiSettings.clear();
}


static void checkLandscapeSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> landscapeSettings;
	landscapeSettings["flag_landscape"]="true";
	landscapeSettings["flag_fog"]="false";
	landscapeSettings["flag_atmosphere"]="true";

	for (std::map<std::string,std::string>::iterator it=landscapeSettings.begin(); it!=landscapeSettings.end(); ++it) {
		if (!user_conf.findEntry("landscape:"+it->first))
			user_conf.setStr("landscape:"+it->first, it->second);
	}

	landscapeSettings.clear();
}



static void checkColorSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> colorSettings;

	colorSettings["azimuthal_color"] = "0,0.4,0.6";
	colorSettings["equatorial_color"] = "0.5,1,0.5";
	colorSettings["ecliptic_color"] = "1,0.2,0.2";
	colorSettings["galactic_color"] = "0.8,0.8,0.8";
	colorSettings["ecliptic_color"] = "1,0.2,0.2";
	colorSettings["ecliptic_center_color"] = "0.8,0.8,0.8";
	colorSettings["galactic_center_color"] = "0.8,0.8,0.8";
	colorSettings["galactic_pole_color"] = "0.8,0.8,0.8";
	colorSettings["nebula_label_color"] = "0.5,0.5,0.5";
	colorSettings["nebula_circle_color"] = "0.15,0.15,0.15";
	colorSettings["precession_circle_color"] = "0.6,0.4,0";
	colorSettings["circumpolar_circle_color"] = "0.8,0.8,0.8";
	colorSettings["oort_color"] = "0.0,0.5,1.0";
	colorSettings["galactic_color"] = "0.8,0.8,0.8";
	colorSettings["vernal_points_color"] = "0.8,0.8,0.8";
	colorSettings["planet_halo_color"] = "1.0,1.0,1.0";
	colorSettings["planet_names_color"] = "0.3,0.7,1";
	colorSettings["planet_orbits_color"] = "0.2,0.2,0.2";
	colorSettings["object_trails_color"] = "1,0.5,0";
	colorSettings["equator_color"] = "0.5,1,0.5";
	colorSettings["const_lines_color"] = "0.05,0.05,0.3";
	colorSettings["const_lines3D_color"] = "0.5,0.2,0.2";
	colorSettings["const_boundary_color"] = "0.4,0.3,0";
	colorSettings["const_names_color"] = "0.6,0.7,0";
	colorSettings["const_art_color"] = "1,1,1";
	colorSettings["analemma_line_color"] = "1,0.5,0";
	colorSettings["analemma_color"] = "1,1,0.5";
	colorSettings["aries_color"] = "0.8,0.8,0.8";
	colorSettings["cardinal_color"] = "1,1,0.6";
	colorSettings["ecliptic_center_color"] = "0.8,0.8,0.8";
	colorSettings["galactic_pole_color"] = "0.8,0.8,0.8";
	colorSettings["galactic_center_color"] = "0.8,0.8,0.8";
	colorSettings["greenwich_color"] = "1,0,0";
	colorSettings["meridian_color"] = "0,0.8,1";
	colorSettings["personal_color"] = "0.8,0.8,0";
	colorSettings["personeq_color"] = "0.3,0.3,0.3";
	colorSettings["nautical_alt_color"] = "0.8,0.8,0";
	colorSettings["nautical_ra_color"] = "0.3,0.3,0.3";
	colorSettings["object_coordinates_color"] = "0.8,0.8,0";
	colorSettings["mouse_coordinates_color"] = "0.8,0.8,0";
	colorSettings["angular_distance_color"] = "0.8,0.8,0";
	colorSettings["loxodromy_color"] = "0.9,0.4,0.4";	// Set color
	colorSettings["orthodromy_color"] = "0.4,0.4,0.9";	// Set color
	colorSettings["polar_color"] = "0.5,0.3,0";
	colorSettings["text_usr_color"] = "0.8,0.8,0.8";
	colorSettings["vernal_points_color"] = "0.8,0.8,0.8";
	colorSettings["vertical_color"] = "0.0,0.8,1.0";
	colorSettings["zenith_color"] = "0.0,1.0,0.0";
	colorSettings["zodiac_color"] = "1.0,0,1.0";

	for (std::map<std::string,std::string>::iterator it=colorSettings.begin(); it!=colorSettings.end(); ++it) {
		if (!user_conf.findEntry("color:"+it->first))
			user_conf.setStr("color:"+it->first, it->second);
	}
	colorSettings.clear();
}


static void checkViewingSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> viewingSettings;
	viewingSettings["nebula_picto_size"] = "6";
	viewingSettings["atmosphere_fade_duration"] = "2";
	viewingSettings["flag_constellation_drawing"] = "false";
	viewingSettings["flag_constellation_name"] = "false";
	viewingSettings["flag_constellation_boundaries"] = "false";
	viewingSettings["flag_constellation_art"] = "false";
	viewingSettings["flag_constellation_pick"] = "true";
	viewingSettings["constellation_art_intensity"] = "0.5";
	viewingSettings["constellation_art_fade_duration"] = "2";
	viewingSettings["flag_azimutal_grid"] = "false";
	viewingSettings["flag_equatorial_grid"] = "false";
	viewingSettings["flag_ecliptic_grid"] = "false";
	viewingSettings["flag_galactic_grid"] = "false";
	viewingSettings["flag_equator_line"] = "false";
	viewingSettings["flag_galactic_line"] = "false";
	viewingSettings["flag_ecliptic_line"] = "false";
	viewingSettings["flag_precession_circle"] = "false";
	viewingSettings["flag_circumpolar_circle"] = "false";
	viewingSettings["flag_tropic_lines"] = "false";
	viewingSettings["flag_meridian_line"] = "false";
	viewingSettings["flag_zenith_line"] = "false";
	viewingSettings["flag_polar_circle"] = "false";
	viewingSettings["flag_polar_point"] = "false";
	viewingSettings["flag_ecliptic_center"] = "false";
	viewingSettings["flag_galactic_pole"] = "false";
	viewingSettings["flag_galactic_center"] = "false";
	viewingSettings["flag_vernal_points"] = "false";
	viewingSettings["flag_analemma_line"] = "false";
	viewingSettings["flag_analemma"] = "false";
	viewingSettings["flag_aries_line"] = "false";
	viewingSettings["flag_zodiac"] = "false";
	viewingSettings["flag_cardinal_points"] = "false";
	viewingSettings["flag_vertical_line"] = "false";
	viewingSettings["flag_greenwich_line"] = "false";
	viewingSettings["flag_personal"] = "false";
	viewingSettings["flag_personeq"] = "false";
	viewingSettings["flag_nautical_ra"] = "false";
	viewingSettings["flag_nautical_alt"] = "false";
	viewingSettings["flag_object_coordinates"] = "false";
	viewingSettings["flag_mouse_coordinates"] = "false";
	viewingSettings["flag_angular_distance"] = "false";
	viewingSettings["flag_loxodromy"] = "false";
	viewingSettings["flag_orthodromy"] = "false";
	viewingSettings["flag_oort"] = "true";
	viewingSettings["flag_moon_scaled"] = "true";
	viewingSettings["flag_sun_scaled"] = "false";
	viewingSettings["flag_atmospheric_refraction"] = "false";
	viewingSettings["moon_scale"] = "5";
	viewingSettings["sun_scale"] = "5";
	viewingSettings["light_pollution_limiting_magnitude"] = "6";


	for (std::map<std::string,std::string>::iterator it=viewingSettings.begin(); it!=viewingSettings.end(); ++it) {
		if (!user_conf.findEntry("viewving:"+it->first))
			user_conf.setStr("viewing:"+it->first, it->second);
	}

	viewingSettings.clear();
}


static void checkNavigationSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> navigationSettings;
	navigationSettings["preset_sky_time"]="2453065.333344907";
	navigationSettings["auto_move_duration"]="5";
	navigationSettings["day_key_mode"]="calendar";
	// navigationSettings["flag_enable_drag_mouse"]="true";
	navigationSettings["flag_enable_move_keys"]="true";
	navigationSettings["flag_enable_zoom_keys"]="true";
	navigationSettings["flag_manual_zoom"]="false";
	navigationSettings["heading"]="0";
	navigationSettings["init_fov"]="180";
	navigationSettings["init_view_pos"]="1e-04,1e-04,1";
	navigationSettings["mouse_zoom"]="30";
	navigationSettings["move_speed"]="0.0001";
	navigationSettings["startup_time_mode"]="Actual";
	navigationSettings["view_offset"]="0";
	navigationSettings["viewing_mode"]="equator";
	navigationSettings["zoom_speed"]="0.0001";
	navigationSettings["stall_radius_unit"]= "5.0";

	for (std::map<std::string,std::string>::iterator it=navigationSettings.begin(); it!=navigationSettings.end(); ++it) {
		if (!user_conf.findEntry("navigation:"+it->first))
			user_conf.setStr("navigation:"+it->first, it->second);
	}

	navigationSettings.clear();

}


static void checkAstroSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> astroSettings;
	astroSettings["flag_stars"]="true";
	astroSettings["flag_star_name"]="false";
	astroSettings["flag_star_lines"]="false";
	astroSettings["flag_planets"]="true";
	astroSettings["flag_planets_hints"]="false";
	astroSettings["flag_planets_orbits"]="false";
	astroSettings["flag_nebula"]="true";
	astroSettings["flag_milky_way"]="true";
	astroSettings["flag_zodiacal_light"]="false";
	astroSettings["flag_bright_nebulae"]="true";
	astroSettings["milky_way_fader_duration"]="2";
	astroSettings["milky_way_intensity"]="0.7";
	astroSettings["zodiacal_intensity"]="0.7";
	astroSettings["milky_way_texture"]="milkyway.png";
	astroSettings["milky_way_iris_texture"]="";
	astroSettings["zodiacal_light_texture"]="zodiacale.png";
	astroSettings["flag_nebula_hints"]="false";
	astroSettings["flag_nebula_names"]="false";
	astroSettings["max_mag_nebula_name"]="99";
	astroSettings["flag_object_trails"]="false";
	astroSettings["flag_light_travel_time"]="true";
	astroSettings["planet_size_marginal_limit"]="0";
	astroSettings["star_size_limit"]="9";
	astroSettings["meteor_rate"]="10";

	for (std::map<std::string,std::string>::iterator it=astroSettings.begin(); it!=astroSettings.end(); ++it) {
		if (!user_conf.findEntry("astro:"+it->first))
			user_conf.setStr("astro:"+it->first, it->second);
	}

	astroSettings.clear();
}

static void checkLocationSettings(InitParser &user_conf)
{
	std::map<std::string,std::string> init_locationSettings;
	init_locationSettings["landscape_name"]="forest";
	init_locationSettings["name"]="guereins";
	init_locationSettings["home_planet"]="Earth";
	init_locationSettings["altitude"]="230";
	init_locationSettings["latitude"]="+46d6'29.0\"";
	init_locationSettings["longitude"]="+4d46'47.0\"";

	for (std::map<std::string,std::string>::iterator it=init_locationSettings.begin(); it!=init_locationSettings.end(); ++it) {
		if (!user_conf.findEntry("init_location:"+it->first))
			user_conf.setStr("init_location:"+it->first, it->second);
	}

	init_locationSettings.clear();
}


void checkConfigIni(const std::string &fullpathfile, const std::string &_VERSION)
{
	InitParser user_conf;
	user_conf.load(fullpathfile);

	if (user_conf.getStr("main:version") == _VERSION) {
		return; //(nothing to do, config.ini isn't outdated)
	}

	checkMainSettings(user_conf);

	checkIoSettings(user_conf);

	checkVideoSettings(user_conf);

	checkRenderingSettings(user_conf);

	checkLocalizationSettings(user_conf);

	checkStarSettings(user_conf);

	//~ checkJoystickSettings(user_conf);

	checkGuiSettings(user_conf);

	checkFontSettings(user_conf);

	checkTuiSettings(user_conf);

	checkLandscapeSettings(user_conf);

	checkColorSettings(user_conf);

	checkViewingSettings(user_conf);

	checkNavigationSettings(user_conf);

	checkAstroSettings(user_conf);

	checkLocationSettings(user_conf);

	user_conf.setStr("main:version", _VERSION);

	user_conf.save(fullpathfile);
}

