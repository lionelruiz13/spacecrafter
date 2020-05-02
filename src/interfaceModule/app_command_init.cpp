#include "interfaceModule/app_command_init.hpp"
#include "coreModule/coreLink.hpp"
#include "tools/log.hpp"

AppCommandInit::AppCommandInit(AppCommandInterface *_app, CoreLink *_coreLink, Core *core)
{
	appCommandInterface = _app;
	coreLink = _coreLink;
	stcore = core;
}

AppCommandInit::~AppCommandInit() {}

void AppCommandInit::initialiseCommandsName(std::map<const std::string, SC_COMMAND> &m_commands)
{
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
}

void AppCommandInit::initialiseFlagsName(std::map<const std::string, FLAG_NAMES> &m_flags)
{
	m_flags["antialias_lines"] = FLAG_NAMES::FN_ANTIALIAS_LINES;
	m_flags["constellation_drawing"] = FLAG_NAMES::FN_CONSTELLATION_DRAWING;
	m_flags["constellation_names"] = FLAG_NAMES::FN_CONSTELLATION_NAMES;
	m_flags["constellation_art"] = FLAG_NAMES::FN_CONSTELLATION_ART;
	m_flags["constellation_boundaries"] = FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES;
	m_flags["constellation_pick"] = FLAG_NAMES::FN_CONSTELLATION_PICK;

	m_flags["star_twinkle"] = FLAG_NAMES::FN_STAR_TWINKLE;
	m_flags["navigation"] = FLAG_NAMES::FN_NAVIGATION;
	m_flags["show_tui_datetime"] = FLAG_NAMES::FN_SHOW_TUI_DATETIME;
	m_flags["show_tui_short_obj_info"] = FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO;
	m_flags["manual_zoom"] = FLAG_NAMES::FN_MANUAL_ZOOM;

	m_flags["light_travel_time"] = FLAG_NAMES::FN_LIGHT_TRAVEL_TIME;
	m_flags["fog"] = FLAG_NAMES::FN_FOG;
	m_flags["atmosphere"] = FLAG_NAMES::FN_ATMOSPHERE;
	m_flags["azimuthal_grid"] = FLAG_NAMES::FN_AZIMUTHAL_GRID;
	m_flags["equatorial_grid"] = FLAG_NAMES::FN_EQUATORIAL_GRID;

	m_flags["ecliptic_grid"] = FLAG_NAMES::FN_ECLIPTIC_GRID;
	m_flags["galactic_grid"] = FLAG_NAMES::FN_GALACTIC_GRID;
	m_flags["equator_line"] = FLAG_NAMES::FN_EQUATOR_LINE;
	m_flags["galactic_line"] = FLAG_NAMES::FN_GALACTIC_LINE;
	m_flags["ecliptic_line"] = FLAG_NAMES::FN_ECLIPTIC_LINE;

	m_flags["precession_circle"] = FLAG_NAMES::FN_PRECESSION_CIRCLE;
	m_flags["circumpolar_circle"] = FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE;
	m_flags["tropic_lines"] = FLAG_NAMES::FN_TROPIC_LINES;
	m_flags["meridian_line"] = FLAG_NAMES::FN_MERIDIAN_LINE;
	m_flags["zenith_line"] = FLAG_NAMES::FN_ZENITH_LINE;

	m_flags["polar_circle"] = FLAG_NAMES::FN_POLAR_CIRCLE;
	m_flags["polar_point"] = FLAG_NAMES::FN_POLAR_POINT;
	m_flags["ecliptic_center"] = FLAG_NAMES::FN_ECLIPTIC_CENTER;
	m_flags["galactic_pole"] = FLAG_NAMES::FN_GALACTIC_POLE;
	m_flags["galactic_center"] = FLAG_NAMES::FN_GALACTIC_CENTER;
	m_flags["vernal_points"] = FLAG_NAMES::FN_VERNAL_POINTS;

	m_flags["analemma_line"] = FLAG_NAMES::FN_ANALEMMA_LINE;
	m_flags["analemma"] = FLAG_NAMES::FN_ANALEMMA;
	m_flags["aries_line"] = FLAG_NAMES::FN_ARIES_LINE;
	m_flags["zodiac"] = FLAG_NAMES::FN_ZODIAC;

	m_flags["personal"] = FLAG_NAMES::FN_PERSONAL;
	m_flags["personeq"] = FLAG_NAMES::FN_PERSONEQ;
	m_flags["nautical_alt"] = FLAG_NAMES::FN_NAUTICAL;
	m_flags["nautical_ra"] = FLAG_NAMES::FN_NAUTICEQ;
	m_flags["object_coordinates"] = FLAG_NAMES::FN_OBJCOORD;
	m_flags["mouse_coordinates"] = FLAG_NAMES::FN_MOUSECOORD;
	m_flags["angular_distance"] = FLAG_NAMES::FN_ANG_DIST;
	m_flags["loxodromy"] = FLAG_NAMES::FN_LOXODROMY;
	m_flags["orthodromy"] = FLAG_NAMES::FN_ORTHODROMY;

	m_flags["greenwich_line"] = FLAG_NAMES::FN_GREENWICH_LINE;
	m_flags["vertical_line"] = FLAG_NAMES::FN_VERTICAL_LINE;
	m_flags["cardinal_points"] = FLAG_NAMES::FN_CARDINAL_POINTS;
	m_flags["clouds"] = FLAG_NAMES::FN_CLOUDS;

	m_flags["moon_scaled"] = FLAG_NAMES::FN_MOON_SCALED;
	m_flags["sun_scaled"] = FLAG_NAMES::FN_SUN_SCALED;
	m_flags["landscape"] = FLAG_NAMES::FN_LANDSCAPE;
	m_flags["stars"] = FLAG_NAMES::FN_STARS;
	m_flags["star_names"] = FLAG_NAMES::FN_STAR_NAMES;
	m_flags["star_pick"] = FLAG_NAMES::FN_STAR_PICK;
	m_flags["atmospheric_refraction"] = FLAG_NAMES::FN_ATMOSPHERIC_REFRACTION;

	m_flags["planets"] = FLAG_NAMES::FN_PLANETS;
	m_flags["planet_names"] = FLAG_NAMES::FN_PLANET_NAMES;
	m_flags["planet_orbits"] = FLAG_NAMES::FN_PLANET_ORBITS;
	m_flags["orbits"] = FLAG_NAMES::FN_ORBITS;
	m_flags["planets_orbits"] = FLAG_NAMES::FN_PLANETS_ORBITS;

	m_flags["planets_axis"] = FLAG_NAMES::FN_PLANETS_AXIS;
	m_flags["satellites_orbits"] = FLAG_NAMES::FN_SATELLITES_ORBITS;
	m_flags["nebulae"] = FLAG_NAMES::FN_NEBULAE;
	m_flags["nebula_names"] = FLAG_NAMES::FN_NEBULA_NAMES;
	m_flags["nebula_hints"] = FLAG_NAMES::FN_NEBULA_HINTS;

	m_flags["milky_way"] = FLAG_NAMES::FN_MILKY_WAY;
	m_flags["bright_nebulae"] = FLAG_NAMES::FN_BRIGHT_NEBULAE;
	m_flags["object_trails"] = FLAG_NAMES::FN_OBJECT_TRAILS;
	m_flags["track_object"] = FLAG_NAMES::FN_TRACK_OBJECT;
	m_flags["script_gui_debug"] = FLAG_NAMES::FN_SCRIPT_GUI_DEBUG;

	m_flags["lock_sky_position"] = FLAG_NAMES::FN_LOCK_SKY_POSITION;
	m_flags["body_trace"] = FLAG_NAMES::FN_BODY_TRACE;
	m_flags["show_latlon"] = FLAG_NAMES::FN_SHOW_LATLON;
	m_flags["color_inverse"] = FLAG_NAMES::FN_COLOR_INVERSE;
	m_flags["oort"] = FLAG_NAMES::FN_OORT;

	m_flags["stars_trace"] = FLAG_NAMES::FN_STARS_TRACE;
	m_flags["star_lines"] = FLAG_NAMES::FN_STAR_LINES;
	m_flags["sky_draw"] = FLAG_NAMES::FN_SKY_DRAW;
	m_flags["dso_pictograms"] = FLAG_NAMES::FN_DSO_PICTOGRAMS;
	m_flags["zodiacal_light"] = FLAG_NAMES::FN_ZODIAC_LIGHT;

	m_flags["tully"] = FLAG_NAMES::FN_TULLY;
	m_flags["satellites"] = FLAG_NAMES::FN_SATELLITES;
}

void AppCommandInit::initialiseColorCommand(std::map<const std::string, COLORCOMMAND_NAMES> &m_color)
{
	m_color["constellation_lines"] = COLORCOMMAND_NAMES::CC_CONSTELLATION_LINES;
	m_color["constellation_names"] = COLORCOMMAND_NAMES::CC_CONSTELLATION_NAMES;
	m_color["constellation_art"] = COLORCOMMAND_NAMES::CC_CONSTELLATION_ART;
	m_color["constellation_boundaries"] = COLORCOMMAND_NAMES::CC_CONSTELLATION_BOUNDARIES;
	m_color["cardinal_points"] = COLORCOMMAND_NAMES::CC_CARDINAL_POINTS;
	m_color["planet_orbits"] = COLORCOMMAND_NAMES::CC_PLANET_ORBITS;

	m_color["planet_names"] = COLORCOMMAND_NAMES::CC_PLANET_NAMES;
	m_color["planet_trails"] = COLORCOMMAND_NAMES::CC_PLANET_TRAILS;
	m_color["azimuthal_grid"] = COLORCOMMAND_NAMES::CC_AZIMUTHAL_GRID;
	m_color["equator_grid"] = COLORCOMMAND_NAMES::CC_EQUATOR_GRID;
	m_color["ecliptic_grid"] = COLORCOMMAND_NAMES::CC_ECLIPTIC_GRID;
	m_color["galactic_grid"] = COLORCOMMAND_NAMES::CC_GALACTIC_GRID;

	m_color["galactic_grid"] = COLORCOMMAND_NAMES::CC_EQUATOR_LINE;
	m_color["galactic_line"] = COLORCOMMAND_NAMES::CC_GALACTIC_LINE;
	m_color["ecliptic_line"] = COLORCOMMAND_NAMES::CC_ECLIPTIC_LINE;
	m_color["meridian_line"] = COLORCOMMAND_NAMES::CC_MERIDIAN_LINE;
	m_color["zenith_line"] = COLORCOMMAND_NAMES::CC_ZENITH_LINE;
	m_color["polar_point"] = COLORCOMMAND_NAMES::CC_POLAR_POINT;

	m_color["polar_circle"] = COLORCOMMAND_NAMES::CC_POLAR_CIRCLE;
	m_color["ecliptic_center"] = COLORCOMMAND_NAMES::CC_ECLIPTIC_CENTER;
	m_color["galactic_pole"] = COLORCOMMAND_NAMES::CC_GALACTIC_POLE;
	m_color["galactic_center"] = COLORCOMMAND_NAMES::CC_GALACTIC_CENTER;
	m_color["vernal_points"] = COLORCOMMAND_NAMES::CC_VERNAL_POINTS;
	m_color["analemma"] = COLORCOMMAND_NAMES::CC_ANALEMMA;

	m_color["analemma_line"] = COLORCOMMAND_NAMES::CC_ANALEMMA_LINE;
	m_color["greenwich_line"] = COLORCOMMAND_NAMES::CC_GREENWICH_LINE;
	m_color["aries_line"] = COLORCOMMAND_NAMES::CC_ARIES_LINE;
	m_color["zodiac"] = COLORCOMMAND_NAMES::CC_ZODIAC;
	m_color["personal"] = COLORCOMMAND_NAMES::CC_PERSONAL;
	m_color["personeq"] = COLORCOMMAND_NAMES::CC_PERSONEQ;

	m_color["nautical_alt"] = COLORCOMMAND_NAMES::CC_NAUTICAL_ALT;
	m_color["nautical_ra"] = COLORCOMMAND_NAMES::CC_NAUTICAL_RA;
	m_color["object_coordinates"] = COLORCOMMAND_NAMES::CC_OBJECT_COORDINATES;
	m_color["mouse_coordinates"] = COLORCOMMAND_NAMES::CC_MOUSE_COORDINATES;
	m_color["angular_distance"] = COLORCOMMAND_NAMES::CC_ANGULAR_DISTANCE;
	m_color["loxodromy"] = COLORCOMMAND_NAMES::CC_LOXODROMY;

	m_color["orthodromy"] = COLORCOMMAND_NAMES::CC_ORTHODROMY;
	m_color["vertical_line"] = COLORCOMMAND_NAMES::CC_VERTICAL_LINE;
	m_color["nebula_names"] = COLORCOMMAND_NAMES::CC_NEBULA_NAMES;
	m_color["nebula_circle"] = COLORCOMMAND_NAMES::CC_NEBULA_CIRCLE;
	m_color["precession_circle"] = COLORCOMMAND_NAMES::CC_PRECESSION_CIRCLE;
	m_color["text_usr_color"] = COLORCOMMAND_NAMES::CC_TEXT_USR_COLOR;

	m_color["star_table"] = COLORCOMMAND_NAMES::CC_STAR_TABLE;
}

void AppCommandInit::initialiseSetCommand(std::map<const std::string, SCD_NAMES> &m_appcommand)
{
	m_appcommand["atmosphere_fade_duration"] = SCD_NAMES::APP_ATMOSPHERE_FADE_DURATION;
	m_appcommand["auto_move_duration"] = SCD_NAMES::APP_AUTO_MOVE_DURATION;
	m_appcommand["constellation_art_fade_duration"] = SCD_NAMES::APP_CONSTELLATION_ART_FADE_DURATION;
	m_appcommand["constellation_art_intensity"] = SCD_NAMES::APP_CONSTELLATION_ART_INTENSITY;
	m_appcommand["light_pollution_limiting_magnitude"] = SCD_NAMES::APP_LIGHT_POLLUTION_LIMITING_MAGNITUDE;
	m_appcommand["font"] = SCD_NAMES::APP_FONT;

	m_appcommand["heading"] = SCD_NAMES::APP_HEADING;
	m_appcommand["home_planet"] = SCD_NAMES::APP_HOME_PLANET;
	m_appcommand["landscape_name"] = SCD_NAMES::APP_LANDSCAPE_NAME;
	m_appcommand["line_width"] = SCD_NAMES::APP_LINE_WIDTH;
	m_appcommand["max_mag_nebula_name"] = SCD_NAMES::APP_MAX_MAG_NEBULA_NAME;
	m_appcommand["max_mag_star_name"] = SCD_NAMES::APP_MAX_MAG_STAR_NAME;

	m_appcommand["moon_scale"] = SCD_NAMES::APP_MOON_SCALE;
	m_appcommand["sun_scale"] = SCD_NAMES::APP_SUN_SCALE;
	m_appcommand["milky_way_texture"] = SCD_NAMES::APP_MILKY_WAY_TEXTURE;
	m_appcommand["sky_culture"] = SCD_NAMES::APP_SKY_CULTURE;
	m_appcommand["sky_locale"] = SCD_NAMES::APP_SKY_LOCALE;
	m_appcommand["ui_locale"] = SCD_NAMES::APP_UI_LOCALE;

	m_appcommand["star_mag_scale"] = SCD_NAMES::APP_STAR_MAG_SCALE;
	m_appcommand["star_size_limit"] = SCD_NAMES::APP_STAR_SIZE_LIMIT;
	m_appcommand["planet_size_limit"] = SCD_NAMES::APP_PLANET_SIZE_LIMIT;
	m_appcommand["star_scale"] = SCD_NAMES::APP_STAR_SCALE;
	m_appcommand["star_twinkle_amount"] = SCD_NAMES::APP_STAR_TWINKLE_AMOUNT;
	m_appcommand["star_fader_duration"] = SCD_NAMES::APP_STAR_FADER_DURATION;

	m_appcommand["star_limiting_mag"] = SCD_NAMES::APP_STAR_LIMITING_MAG;
	m_appcommand["time_zone"] = SCD_NAMES::APP_TIME_ZONE;
	m_appcommand["ambient_light"] = SCD_NAMES::APP_AMBIENT_LIGHT;
	m_appcommand["text_fading_duration"] = SCD_NAMES::APP_TEXT_FADING_DURATION;
	m_appcommand["milky_way_fader_duration"] = SCD_NAMES::APP_MILKY_WAY_FADER_DURATION;
	m_appcommand["milky_way_intensity"] = SCD_NAMES::APP_MILKY_WAY_INTENSITY;

	m_appcommand["zoom_offset"] = SCD_NAMES::APP_ZOOM_OFFSET;
	m_appcommand["startup_time_mode"] = SCD_NAMES::APP_STARTUP_TIME_MODE;
	m_appcommand["date_display_format"] = SCD_NAMES::APP_DATE_DISPLAY_FORMAT;
	m_appcommand["time_display_format"] = SCD_NAMES::APP_TIME_DISPLAY_FORMAT;
	m_appcommand["mode"] = SCD_NAMES::APP_MODE;
	m_appcommand["screen_fader"] = SCD_NAMES::APP_SCREEN_FADER;

	m_appcommand["stall_radius_unit"] = SCD_NAMES::APP_STALL_RADIUS_UNIT;
	m_appcommand["tully_color_mode"] = SCD_NAMES::APP_TULLY_COLOR_MODE;
	m_appcommand["datetime_display_position"] = SCD_NAMES::APP_DATETIME_DISPLAY_POSITION;
	m_appcommand["datetime_display_number"] = SCD_NAMES::APP_DATETIME_DISPLAY_NUMBER;
}

void AppCommandInit::initialiseSetFlag(std::map<FLAG_NAMES, AppCommandInterface::stFct> &m_setFlag)
{
	m_setFlag[FLAG_NAMES::FN_ANTIALIAS_LINES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->constellationGetFlagLines(); },
		 [&](bool x) { coreLink->constellationSetFlagLines(x); }};
	m_setFlag[FLAG_NAMES::FN_CONSTELLATION_NAMES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->constellationGetFlagNames(); },
		 [&](bool x) { coreLink->constellationSetFlagNames(x); }};
	m_setFlag[FLAG_NAMES::FN_CONSTELLATION_ART] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->constellationGetFlagArt(); },
		 [&](bool x) { coreLink->constellationSetFlagArt(x); }};
	m_setFlag[FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->constellationGetFlagBoundaries(); },
		 [&](bool x) { coreLink->constellationSetFlagBoundaries(x); }};
	m_setFlag[FLAG_NAMES::FN_CONSTELLATION_PICK] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->constellationGetFlagIsolateSelected(); },
		 [&](bool x) { coreLink->constellationSetFlagIsolateSelected(x); }};
	m_setFlag[FLAG_NAMES::FN_STAR_TWINKLE] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->starGetFlagTwinkle(); },
		 [&](bool x) { coreLink->starSetFlagTwinkle(x); }};
	m_setFlag[FLAG_NAMES::FN_MANUAL_ZOOM] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return stcore->getFlagManualAutoZoom(); },
		 [&](bool x) { stcore->setFlagManualAutoZoom(x); }};
	m_setFlag[FLAG_NAMES::FN_NAVIGATION] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return stcore->getFlagNav(); },
		 [&](bool x) { stcore->setFlagNav(x); }};
	m_setFlag[FLAG_NAMES::FN_LIGHT_TRAVEL_TIME] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->getFlagLightTravelTime(); },
		 [&](bool x) { coreLink->setFlagLightTravelTime(x); }};
	m_setFlag[FLAG_NAMES::FN_FOG] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->fogGetFlag(); },
		 [&](bool x) { coreLink->fogSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_FOG] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->fogGetFlag(); },
		 [&](bool x) {if(!x){ coreLink->fogSetFlag(false); } coreLink->starSetFlagTwinkle(x); coreLink->atmosphereSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_CARDINAL_POINTS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->cardinalsPointsGetFlag(); },
		 [&](bool x) { coreLink->cardinalsPointsSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_CLOUDS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->getFlagClouds(); },
		 [&](bool x) { coreLink->setFlagClouds(x); }};
	m_setFlag[FLAG_NAMES::FN_MOON_SCALED] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->getFlagMoonScaled(); },
		 [&](bool x) { coreLink->setFlagMoonScaled(x); }};
	m_setFlag[FLAG_NAMES::FN_SUN_SCALED] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->getFlagSunScaled(); },
		 [&](bool x) { coreLink->setFlagSunScaled(x); }};
	m_setFlag[FLAG_NAMES::FN_LANDSCAPE] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->landscapeGetFlag(); },
		 [&](bool x) { coreLink->landscapeSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_STARS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->starGetFlag(); },
		 [&](bool x) { coreLink->starSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_STAR_NAMES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->starGetFlagName(); },
		 [&](bool x) { coreLink->starSetFlagName(x); }};
	m_setFlag[FLAG_NAMES::FN_STAR_PICK] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->starGetFlagIsolateSelected(); },
		 [&](bool x) { coreLink->starSetFlagIsolateSelected(x); }};
	m_setFlag[FLAG_NAMES::FN_PLANETS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlag(); },
		 [&](bool x) { coreLink->planetsSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_PLANET_NAMES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlagHints(); },
		 [&](bool x) { coreLink->planetsSetFlagHints(x); if (coreLink->planetsGetFlagHints()) coreLink->planetsSetFlag(true); }};
	m_setFlag[FLAG_NAMES::FN_PLANET_ORBITS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlagOrbits(); },
		 [&](bool x) { coreLink->planetsSetFlagOrbits(x); coreLink->satellitesSetFlagOrbits(x); }};
	m_setFlag[FLAG_NAMES::FN_PLANETS_AXIS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlagAxis(); },
		 [&](bool x) { coreLink->planetsSetFlagAxis(x); }};
	m_setFlag[FLAG_NAMES::FN_ORBITS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlagOrbits(); },
		 [&](bool x) { coreLink->planetsSetFlagOrbits(x); coreLink->satellitesSetFlagOrbits(x); }};
	m_setFlag[FLAG_NAMES::FN_PLANETS_ORBITS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlagOrbits(); },
		 [&](bool x) { coreLink->planetsSetFlagOrbits(x); }};
	m_setFlag[FLAG_NAMES::FN_SATELLITES_ORBITS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->satellitesGetFlagOrbits(); },
		 [&](bool x) { coreLink->satellitesSetFlagOrbits(x); }};
	m_setFlag[FLAG_NAMES::FN_NEBULAE] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->nebulaGetFlag(); },
		 [&](bool x) { coreLink->nebulaSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_NEBULA_HINTS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->nebulaGetFlagHints(); },
		 [&](bool x) { coreLink->nebulaSetFlagHints(x); }};
	m_setFlag[FLAG_NAMES::FN_DSO_PICTOGRAMS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return stcore->getDsoPictograms(); },
		 [&](bool x) { stcore->setDsoPictograms(x); }};
	m_setFlag[FLAG_NAMES::FN_NEBULA_NAMES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->nebulaGetFlagNames(); },
		 [&](bool x) { if (x) coreLink->nebulaSetFlagNames(true); coreLink->nebulaSetFlagNames(x); }};
	m_setFlag[FLAG_NAMES::FN_MILKY_WAY] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->milkyWayGetFlag(); },
		 [&](bool x) { coreLink->milkyWaySetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_ZODIAC_LIGHT] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->milkyWayGetFlagZodiacal(); },
		 [&](bool x) { coreLink->milkyWaySetFlagZodiacal(x); }};
	m_setFlag[FLAG_NAMES::FN_BRIGHT_NEBULAE] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->nebulaGetFlagBright(); },
		 [&](bool x) { coreLink->nebulaSetFlagBright(x); }};
	m_setFlag[FLAG_NAMES::FN_OBJECT_TRAILS] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->planetsGetFlagTrails(); },
		 [&](bool x) { coreLink->planetsSetFlagTrails(x); }};
	m_setFlag[FLAG_NAMES::FN_TRACK_OBJECT] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return stcore->getFlagTracking(); },
		 [&](bool x) { stcore->setFlagTracking(x); }};
	m_setFlag[FLAG_NAMES::FN_SCRIPT_GUI_DEBUG] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return !cLog::get()->getDebug(); },
		 [&](bool x) { cLog::get()->setDebug(x); }};
	m_setFlag[FLAG_NAMES::FN_LOCK_SKY_POSITION] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return stcore->getFlagLockSkyPosition(); },
		 [&](bool x) { stcore->setFlagLockSkyPosition(x); }};
	m_setFlag[FLAG_NAMES::FN_OORT] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->oortGetFlagShow(); },
		 [&](bool x) { coreLink->oortSetFlagShow(x); }};
	m_setFlag[FLAG_NAMES::FN_TULLY] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->tullyGetFlagShow(); },
		 [&](bool x) { coreLink->tullySetFlagShow(x); }};
	m_setFlag[FLAG_NAMES::FN_BODY_TRACE] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->bodyTraceGetFlag(); },
		 [&](bool x) { coreLink->bodyTraceSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_STARS_TRACE] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->starGetTraceFlag(); },
		 [&](bool x) { coreLink->starSetTraceFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_STAR_LINES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->starLinesGetFlag(); },
		 [&](bool x) { coreLink->starLinesSetFlag(x); }};
	m_setFlag[FLAG_NAMES::FN_SATELLITES] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->hideSatellitesFlag(); },
		 [&](bool x) { coreLink->setHideSatellites(x); }};
	m_setFlag[FLAG_NAMES::FN_ATMOSPHERIC_REFRACTION] =
		{[&](FLAG_VALUES f, std::function<bool()> f1, std::function<void(bool)> f2, bool &b) { appCommandInterface->setFlagFct(f, f1, f2, b); },
		 [&]() { return coreLink->atmosphericRefractionGetFlag(); },
		 [&](bool x) { coreLink->atmosphericRefractionSetFlag(x); }};
}
