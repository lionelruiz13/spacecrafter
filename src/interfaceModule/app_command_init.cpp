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

std::map<const std::string, FLAG_NAMES> AppCommandInit::initialiseFlagsName() {
    std::map<const std::string, FLAG_NAMES> m_flags;

    m_flags["antialias_lines"]= FLAG_NAMES::FN_ANTIALIAS_LINES;
	m_flags["constellation_drawing"]= FLAG_NAMES::FN_CONSTELLATION_DRAWING;
	m_flags["constellation_names"]= FLAG_NAMES::FN_CONSTELLATION_NAMES;
	m_flags["constellation_art"]= FLAG_NAMES::FN_CONSTELLATION_ART;
	m_flags["constellation_boundaries"]= FLAG_NAMES::FN_CONSTELLATION_BOUNDARIES;
	m_flags["constellation_pick"]= FLAG_NAMES::FN_CONSTELLATION_PICK;

	m_flags["star_twinkle"]= FLAG_NAMES::FN_STAR_TWINKLE;
	m_flags["navigation"]= FLAG_NAMES::FN_NAVIGATION;
	m_flags["show_tui_datetime"]= FLAG_NAMES::FN_SHOW_TUI_DATETIME;
	m_flags["show_tui_short_obj_info"]= FLAG_NAMES::FN_SHOW_TUI_SHORT_OBJ_INFO;
	m_flags["manual_zoom"]= FLAG_NAMES::FN_MANUAL_ZOOM;

	m_flags["light_travel_time"]= FLAG_NAMES::FN_LIGHT_TRAVEL_TIME;
	m_flags["fog"]= FLAG_NAMES::FN_FOG;
	m_flags["atmosphere"]= FLAG_NAMES::FN_ATMOSPHERE;
	m_flags["azimuthal_grid"]= FLAG_NAMES::FN_AZIMUTHAL_GRID;
	m_flags["equatorial_grid"]= FLAG_NAMES::FN_EQUATORIAL_GRID;

	m_flags["ecliptic_grid"]= FLAG_NAMES::FN_ECLIPTIC_GRID;
	m_flags["galactic_grid"]= FLAG_NAMES::FN_GALACTIC_GRID;
	m_flags["equator_line"]= FLAG_NAMES::FN_EQUATOR_LINE;
	m_flags["galactic_line"]= FLAG_NAMES::FN_GALACTIC_LINE;
	m_flags["ecliptic_line"]= FLAG_NAMES::FN_ECLIPTIC_LINE;

	m_flags["precession_circle"]= FLAG_NAMES::FN_PRECESSION_CIRCLE;
	m_flags["circumpolar_circle"]= FLAG_NAMES::FN_CIRCUMPOLAR_CIRCLE;
	m_flags["tropic_lines"]= FLAG_NAMES::FN_TROPIC_LINES;
	m_flags["meridian_line"]= FLAG_NAMES::FN_MERIDIAN_LINE;
	m_flags["zenith_line"]= FLAG_NAMES::FN_ZENITH_LINE;

	m_flags["polar_circle"]= FLAG_NAMES::FN_POLAR_CIRCLE;
	m_flags["polar_point"]= FLAG_NAMES::FN_POLAR_POINT;
	m_flags["ecliptic_center"]= FLAG_NAMES::FN_ECLIPTIC_CENTER;
	m_flags["galactic_pole"]= FLAG_NAMES::FN_GALACTIC_POLE;
	m_flags["galactic_center"]= FLAG_NAMES::FN_GALACTIC_CENTER;
	m_flags["vernal_points"]= FLAG_NAMES::FN_VERNAL_POINTS;

	m_flags["analemma_line"]= FLAG_NAMES::FN_ANALEMMA_LINE;
	m_flags["analemma"]= FLAG_NAMES::FN_ANALEMMA;
	m_flags["aries_line"]= FLAG_NAMES::FN_ARIES_LINE;
	m_flags["zodiac"]= FLAG_NAMES::FN_ZODIAC;

	m_flags["personal"]= FLAG_NAMES::FN_PERSONAL;
	m_flags["personeq"]= FLAG_NAMES::FN_PERSONEQ;
	m_flags["nautical_alt"]= FLAG_NAMES::FN_NAUTICAL;
	m_flags["nautical_ra"]= FLAG_NAMES::FN_NAUTICEQ;
	m_flags["object_coordinates"]=FLAG_NAMES::FN_OBJCOORD;
	m_flags["mouse_coordinates"]=FLAG_NAMES::FN_MOUSECOORD;
	m_flags["angular_distance"]=FLAG_NAMES::FN_ANG_DIST;
	m_flags["loxodromy"]=FLAG_NAMES::FN_LOXODROMY;
	m_flags["orthodromy"]=FLAG_NAMES::FN_ORTHODROMY;

	m_flags["greenwich_line"]= FLAG_NAMES::FN_GREENWICH_LINE;
	m_flags["vertical_line"]= FLAG_NAMES::FN_VERTICAL_LINE;
	m_flags["cardinal_points"]= FLAG_NAMES::FN_CARDINAL_POINTS;
	m_flags["clouds"]= FLAG_NAMES::FN_CLOUDS;

	m_flags["moon_scaled"]= FLAG_NAMES::FN_MOON_SCALED;
	m_flags["sun_scaled"]= FLAG_NAMES::FN_SUN_SCALED;
	m_flags["landscape"]= FLAG_NAMES::FN_LANDSCAPE;
	m_flags["stars"]= FLAG_NAMES::FN_STARS;
	m_flags["star_names"]= FLAG_NAMES::FN_STAR_NAMES;
	m_flags["star_pick"]= FLAG_NAMES::FN_STAR_PICK;
	m_flags["atmospheric_refraction"]=FLAG_NAMES::FN_ATMOSPHERIC_REFRACTION;

	m_flags["planets"]= FLAG_NAMES::FN_PLANETS;
	m_flags["planet_names"]= FLAG_NAMES::FN_PLANET_NAMES;
	m_flags["planet_orbits"]= FLAG_NAMES::FN_PLANET_ORBITS;
	m_flags["orbits"]= FLAG_NAMES::FN_ORBITS;
	m_flags["planets_orbits"]= FLAG_NAMES::FN_PLANETS_ORBITS;

	m_flags["planets_axis"]= FLAG_NAMES::FN_PLANETS_AXIS;
	m_flags["satellites_orbits"]= FLAG_NAMES::FN_SATELLITES_ORBITS;
	m_flags["nebulae"]= FLAG_NAMES::FN_NEBULAE;
	m_flags["nebula_names"]= FLAG_NAMES::FN_NEBULA_NAMES;
	m_flags["nebula_hints"]= FLAG_NAMES::FN_NEBULA_HINTS;

	m_flags["milky_way"]= FLAG_NAMES::FN_MILKY_WAY;
	m_flags["bright_nebulae"]= FLAG_NAMES::FN_BRIGHT_NEBULAE;
	m_flags["object_trails"]= FLAG_NAMES::FN_OBJECT_TRAILS;
	m_flags["track_object"]= FLAG_NAMES::FN_TRACK_OBJECT;
	m_flags["script_gui_debug"]= FLAG_NAMES::FN_SCRIPT_GUI_DEBUG;

	m_flags["lock_sky_position"]= FLAG_NAMES::FN_LOCK_SKY_POSITION;
	m_flags["body_trace"]= FLAG_NAMES::FN_BODY_TRACE;
	m_flags["show_latlon"]= FLAG_NAMES::FN_SHOW_LATLON;
	m_flags["color_inverse"]= FLAG_NAMES::FN_COLOR_INVERSE;
	m_flags["oort"]= FLAG_NAMES::FN_OORT;

	m_flags["stars_trace"]= FLAG_NAMES::FN_STARS_TRACE;
	m_flags["star_lines"]= FLAG_NAMES::FN_STAR_LINES;
	m_flags["sky_draw"]= FLAG_NAMES::FN_SKY_DRAW;
	m_flags["dso_pictograms"]= FLAG_NAMES::FN_DSO_PICTOGRAMS;
	m_flags["zodiacal_light"]= FLAG_NAMES::FN_ZODIAC_LIGHT;

	m_flags["tully"]= FLAG_NAMES::FN_TULLY;
	m_flags["satellites"] = FLAG_NAMES::FN_SATELLITES;

    return m_flags;
}
std::map<const std::string, COLORCOMMAND_NAMES> AppCommandInit::initialiseColorCommand() {}
std::map<const std::string, SCD_NAMES> AppCommandInit::initialiseSetCommand() {}