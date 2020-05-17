/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 Elitit-40
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

/* Define base command interface */


#ifndef _BASE_COMMAND_INTERFACE_HPP_
#define _BASE_COMMAND_INTERFACE_HPP_

enum class SC_COMMAND : char {SC_ADD = 30, SC_AUDIO, SC_BODY_TRACE, SC_BODY, SC_CAMERA, SC_CLEAR, SC_COLOR, SC_CONFIGURATION, SC_CONSTELLATION, SC_DATE, SC_DEFINE, SC_DESELECT,
							  SC_DOMEMASTERS,
                              SC_DSO, SC_EXTERNASC_VIEWER, SC_FONT, SC_FLAG, SC_GET, SC_HEADING, SC_ILLUMINATE, SC_IMAGE, SC_LANDSCAPE, SC_LOOK, SC_MEDIA, SC_METEORS,
                              SC_MOVETO, SC_MULTIPLY, SC_PERSONAL, SC_PERSONEQ, SC_PLANET_SCALE, SC_POSITION, SC_PRINT, SC_RANDOM,
                              SC_SCRIPT, SC_SEARCH, SC_SELECT, SC_SET, SC_SHUTDOWN, SC_SKY_CULTURE, SC_STAR_LINES, SC_STRUCT, SC_SUNTRACE, SC_TEXT,
                              SC_TIMERATE, SC_WAIT, SC_ZOOMR, SC_FLYTO
                             };

enum class FLAG_VALUES: char { FV_TOGGLE, FV_ON, FV_OFF};

enum class FLAG_NAMES: char {FN_ANTIALIAS_LINES = 30, FN_CONSTELLATION_DRAWING, FN_CONSTELLATION_NAMES, FN_CONSTELLATION_ART, FN_CONSTELLATION_BOUNDARIES, FN_CONSTELLATION_PICK,
                             FN_STAR_TWINKLE, FN_NAVIGATION, FN_SHOW_TUI_DATETIME, FN_SHOW_TUI_SHORT_OBJ_INFO, FN_MANUAL_ZOOM, FN_LIGHT_TRAVEL_TIME, FN_DSO_PICTOGRAMS,
                             FN_FOG, FN_ATMOSPHERE, FN_AZIMUTHAL_GRID, FN_EQUATORIAL_GRID, FN_ECLIPTIC_GRID, FN_GALACTIC_GRID, FN_EQUATOR_LINE, FN_GALACTIC_LINE,
                             FN_ECLIPTIC_LINE, FN_PRECESSION_CIRCLE, FN_CIRCUMPOLAR_CIRCLE, FN_TROPIC_LINES, FN_MERIDIAN_LINE, FN_ZENITH_LINE, FN_POLAR_CIRCLE, FN_POLAR_POINT,
                             FN_ECLIPTIC_CENTER, FN_GALACTIC_POLE, FN_GALACTIC_CENTER, FN_VERNAL_POINTS, FN_ANALEMMA_LINE, FN_ANALEMMA, FN_ARIES_LINE,
                             FN_ZODIAC, FN_PERSONAL, FN_PERSONEQ, FN_NAUTICAL, FN_NAUTICEQ, FN_OBJCOORD, FN_ANG_DIST, FN_LOXODROMY, FN_ORTHODROMY, FN_GREENWICH_LINE, FN_VERTICAL_LINE, FN_CARDINAL_POINTS, FN_CLOUDS, FN_MOON_SCALED, FN_SUN_SCALED,
                             FN_LANDSCAPE, FN_STARS, FN_STAR_NAMES,  FN_STAR_PICK, FN_PLANETS, FN_PLANET_NAMES, FN_PLANET_ORBITS, FN_ORBITS, FN_PLANETS_ORBITS, FN_PLANETS_AXIS,
                             FN_SATELLITES_ORBITS, FN_NEBULAE, FN_NEBULA_NAMES, FN_NEBULA_HINTS, FN_MILKY_WAY, FN_BRIGHT_NEBULAE, FN_OBJECT_TRAILS, FN_TRACK_OBJECT,
                             FN_SCRIPT_GUI_DEBUG, FN_LOCK_SKY_POSITION, FN_BODY_TRACE, FN_SHOW_LATLON, FN_COLOR_INVERSE, FN_OORT, FN_STARS_TRACE, FN_STAR_LINES,FN_STAR_LINES_SELECTED,
                             FN_SKY_DRAW, FN_ZODIAC_LIGHT , FN_TULLY, FN_SATELLITES, FN_MOUSECOORD, FN_ATMOSPHERIC_REFRACTION
                            };

enum class COLORCOMMAND_NAMES: char {CC_CONSTELLATION_LINES = 30, CC_CONSTELLATION_NAMES, CC_CONSTELLATION_ART, CC_CONSTELLATION_BOUNDARIES, CC_CARDINAL_POINTS, 
                                     CC_PLANET_ORBITS, CC_PLANET_NAMES, CC_PLANET_TRAILS, CC_AZIMUTHAL_GRID, CC_EQUATOR_GRID, CC_ECLIPTIC_GRID, 
                                     CC_GALACTIC_GRID, CC_EQUATOR_LINE, CC_GALACTIC_LINE, CC_ECLIPTIC_LINE, CC_MERIDIAN_LINE, CC_ZENITH_LINE, 
                                     CC_POLAR_POINT, CC_POLAR_CIRCLE, CC_ECLIPTIC_CENTER, CC_GALACTIC_POLE, CC_GALACTIC_CENTER, CC_VERNAL_POINTS, 
                                     CC_ANALEMMA, CC_ANALEMMA_LINE, CC_GREENWICH_LINE, CC_ARIES_LINE, CC_ZODIAC, CC_PERSONAL, CC_PERSONEQ, 
                                     CC_NAUTICAL_ALT, CC_NAUTICAL_RA, CC_OBJECT_COORDINATES, CC_MOUSE_COORDINATES, CC_ANGULAR_DISTANCE, CC_LOXODROMY, 
                                     CC_ORTHODROMY, CC_VERTICAL_LINE, CC_NEBULA_NAMES, CC_NEBULA_CIRCLE, CC_PRECESSION_CIRCLE, CC_TEXT_USR_COLOR, CC_STAR_TABLE
                                    };

enum class SCD_NAMES: char {APP_ATMOSPHERE_FADE_DURATION = 30, APP_AUTO_MOVE_DURATION,APP_CONSTELLATION_ART_FADE_DURATION,APP_CONSTELLATION_ART_INTENSITY,
                            APP_LIGHT_POLLUTION_LIMITING_MAGNITUDE,APP_HEADING,APP_HOME_PLANET,APP_LANDSCAPE_NAME,APP_LINE_WIDTH,APP_MAX_MAG_NEBULA_NAME,
                            APP_MAX_MAG_STAR_NAME,APP_MOON_SCALE,APP_SUN_SCALE,APP_MILKY_WAY_TEXTURE,APP_SKY_CULTURE,APP_SKY_LOCALE,APP_UI_LOCALE,
                            APP_STAR_MAG_SCALE,APP_STAR_SIZE_LIMIT,APP_PLANET_SIZE_LIMIT,APP_STAR_SCALE,APP_STAR_TWINKLE_AMOUNT,APP_STAR_FADER_DURATION,
                            APP_STAR_LIMITING_MAG,APP_TIME_ZONE,APP_AMBIENT_LIGHT,APP_TEXT_FADING_DURATION,APP_MILKY_WAY_FADER_DURATION,APP_MILKY_WAY_INTENSITY,
                            APP_ZOOM_OFFSET,APP_STARTUP_TIME_MODE,APP_DATE_DISPLAY_FORMAT,APP_TIME_DISPLAY_FORMAT,APP_MODE,APP_SCREEN_FADER,
                            APP_STALL_RADIUS_UNIT,APP_TULLY_COLOR_MODE,APP_DATETIME_DISPLAY_POSITION,APP_DATETIME_DISPLAY_NUMBER,APP_FLAG_NONE
                            };

enum class SC_RESERVED_VAR: char {LONGITUDE=0, LATITUDE, ALTITUDE};


// nom des arguments des commandes
#define W_FILENAME   "filename"
#define W_SIZE       "size"
#define W_ACTION     "action"
#define W_SNAPSHOT   "snapshot"
#define W_RECORD     "record"
#define W_VOLUME     "volume"
#define W_INCREMENT  "increment"
#define W_DECREMENT  "decrement"
#define W_NOPAUSE    "nopause"
#define W_NAME       "name"
#define W_VALUE      "value"
#define W_DISPLAY    "display"
#define W_SIZE       "size"

// nom des arguments maps appCommandInit
//CommandsNames
#define ACP_CN_ADD                                  "add"
#define ACP_CN_AUDIO                                "audio"
#define ACP_CN_BODY_TRACE                           "body_trace"
#define ACP_CN_BODY                                 "body"
#define ACP_CN_CAMERA                               "camera"
#define ACP_CN_FLYTO                                "flyto"
#define ACP_CN_CLEAR                                "clear"
#define ACP_CN_COLOR                                "color"
#define ACP_CN_CONFIGURATION                        "configuration"
#define ACP_CN_CONSTELLATION                        "constellation"
#define ACP_CN_DATE                                 "date"
#define ACP_CN_DEFINE                               "define"
#define ACP_CN_DESELECT                             "deselect"
#define ACP_CN_DOMEMASTERS                          "domemasters"
#define ACP_CN_DSO                                  "dso"
#define ACP_CN_EXTERNAL_MPLAYER                     "external_mplayer"
#define ACP_CN_EXTERNAL_VIEWER                      "external_viewer"
#define ACP_CN_FONT                                 "font"
#define ACP_CN_HEADING                              "heading"
#define ACP_CN_FLAG                                 "flag"
#define ACP_CN_GET                                  "get"
#define ACP_CN_ILLUMINATE                           "illuminate"
#define ACP_CN_IMAGE                                "image"
#define ACP_CN_LANDSCAPE                            "landscape"
#define ACP_CN_LOOK_AT                              "look_at"
#define ACP_CN_MEDIA                                "media"
#define ACP_CN_METEORS                              "meteors"
#define ACP_CN_MOVETO                               "moveto"
#define ACP_CN_MOVETOCITY                           "movetocity"
#define ACP_CN_MULTIPLY                             "multiply"
#define ACP_CN_PERSONAL                             "personal"
#define ACP_CN_PERSONEQ                             "personeq"
#define ACP_CN_PLANET_SCALE                         "planet_scale"
#define ACP_CN_POSITION                             "position"
#define ACP_CN_PRINT                                "print"
#define ACP_CN_RANDOM                               "random"
#define ACP_CN_SCRIPT                               "script"
#define ACP_CN_SEARCH                               "search"
#define ACP_CN_SELECT                               "select"
#define ACP_CN_SET                                  "set"
#define ACP_CN_SHUTDOWN                             "shutdown"
#define ACP_CN_SKY_CULTURE                          "sky_culture"
#define ACP_CN_STAR_LINES                           "star_lines"
#define ACP_CN_STRUCT                               "struct"
#define ACP_CN_SUNTRACE                             "suntrace"
#define ACP_CN_TEXT                                 "text"
#define ACP_CN_TIMERATE                             "timerate"
#define ACP_CN_WAIT                                 "wait"
#define ACP_CN_ZOOM                                 "zoom"

//FlagNames
#define ACP_FN_ANTIALIAS_LINES                      "antialias_lines"
#define ACP_FN_CONSTELLATION_DRAWING                "constellation_drawing"
#define ACP_FN_CONSTELLATION_NAMES                  "constellation_names"
#define ACP_FN_CONSTELLATION_ART                    "constellation_art"
#define ACP_FN_CONSTELLATION_BOUNDARIES             "constellation_boundaries"
#define ACP_FN_CONSTELLATION_PICK                   "constellation_pick"
#define ACP_FN_STAR_TWINKLE                         "star_twinkle"
#define ACP_FN_NAVIGATION                           "navigation"
#define ACP_FN_SHOW_TUI_DATETIME                    "show_tui_datetime"
#define ACP_FN_SHOW_TUI_SHORT_OBJ_INFO              "show_tui_short_obj_info"
#define ACP_FN_MANUAL_ZOOM                          "manual_zoom"
#define ACP_FN_LIGHT_TRAVEL_TIME                    "light_travel_time"
#define ACP_FN_FOG                                  "fog"
#define ACP_FN_ATMOSPHERE                           "atmosphere"
#define ACP_FN_AZIMUTHAL_GRID                       "azimuthal_grid"
#define ACP_FN_EQUATORIAL_GRID                      "equatorial_grid"
#define ACP_FN_ECLIPTIC_GRID                        "ecliptic_grid"
#define ACP_FN_GALACTIC_GRID                        "galactic_grid"
#define ACP_FN_EQUATOR_LINE                         "equator_line"
#define ACP_FN_GALACTIC_LINE                        "galactic_line"
#define ACP_FN_ECLIPTIC_LINE                        "ecliptic_line"
#define ACP_FN_PRECESSION_CIRCLE                    "precession_circle"
#define ACP_FN_CIRCUMPOLAR_CIRCLE                   "circumpolar_circle"
#define ACP_FN_TROPIC_LINES                         "tropic_lines"
#define ACP_FN_MERIDIAN_LINE                        "meridian_line"
#define ACP_FN_ZENITH_LINE                          "zenith_line"
#define ACP_FN_POLAR_CIRCLE                         "polar_circle"
#define ACP_FN_POLAR_POINT                          "polar_point"
#define ACP_FN_ECLIPTIC_CENTER                      "ecliptic_center"
#define ACP_FN_GALACTIC_POLE                        "galactic_pole"
#define ACP_FN_GALACTIC_CENTER                      "galactic_center"
#define ACP_FN_VERNAL_POINTS                        "vernal_points"
#define ACP_FN_ANALEMMA_LINE                        "analemma_line"
#define ACP_FN_ANALEMMA                             "analemma"
#define ACP_FN_ARIES_LINE                           "aries_line"
#define ACP_FN_ZODIAC                               "zodiac"
#define ACP_FN_PERSONAL                             "personal"
#define ACP_FN_PERSONEQ                             "personeq"
#define ACP_FN_NAUTICAL_ALT                         "nautical_alt"
#define ACP_FN_NAUTICAL_RA                          "nautical_ra"
#define ACP_FN_OBJECT_COORDINATES                   "object_coordinates"
#define ACP_FN_MOUSE_COORDINATES                    "mouse_coordinates"
#define ACP_FN_ANGULAR_DISTANCE                     "angular_distance"
#define ACP_FN_LOXODROMY                            "loxodromy"
#define ACP_FN_ORTHODROMY                           "orthodromy"
#define ACP_FN_GREENWICH_LINE                       "greenwich_line"
#define ACP_FN_VERTICAL_LINE                        "vertical_line"
#define ACP_FN_CARDINAL_POINTS                      "cardinal_points"
#define ACP_FN_CLOUDS                               "clouds"
#define ACP_FN_MOON_SCALED                          "moon_scaled"
#define ACP_FN_SUN_SCALED                           "sun_scaled"
#define ACP_FN_LANDSCAPE                            "landscape"
#define ACP_FN_STARS                                "stars"
#define ACP_FN_STAR_NAMES                           "star_names"
#define ACP_FN_STAR_PICK                            "star_pick"
#define ACP_FN_ATMOSPHERIC_REFRACTION               "atmospheric_refraction"
#define ACP_FN_PLANETS                              "planets"
#define ACP_FN_PLANET_NAMES                         "planet_names"
#define ACP_FN_PLANET_ORBITS                        "planet_orbits"
#define ACP_FN_ORBITS                               "orbits"
#define ACP_FN_PLANETS_ORBITS                       "planets_orbits"
#define ACP_FN_PLANETS_AXIS                         "planets_axis"
#define ACP_FN_SATELLITES_ORBITS                    "satellites_orbits"
#define ACP_FN_NEBULAE                              "nebulae"
#define ACP_FN_NEBULA_NAMES                         "nebula_names"
#define ACP_FN_NEBULA_HINTS                         "nebula_hints"
#define ACP_FN_MILKY_WAY                            "milky_way"
#define ACP_FN_BRIGHT_NEBULAE                       "bright_nebulae"
#define ACP_FN_OBJECT_TRAILS                        "object_trails"
#define ACP_FN_TRACK_OBJECT                         "track_object"
#define ACP_FN_SCRIPT_GUI_DEBUG                     "script_gui_debug"
#define ACP_FN_LOCK_SKY_POSITION                    "lock_sky_position"
#define ACP_FN_BODY_TRACE                           "body_trace"
#define ACP_FN_SHOW_LATLON                          "show_latlon"
#define ACP_FN_COLOR_INVERSE                        "color_inverse"
#define ACP_FN_OORT                                 "oort"
#define ACP_FN_STARS_TRACE                          "stars_trace"
#define ACP_FN_STAR_LINES                           "star_lines"
#define ACP_FN_SKY_DRAW                             "sky_draw"
#define ACP_FN_DSO_PICTOGRAMS                       "dso_pictograms"
#define ACP_FN_ZODIACAL_LIGHT                       "zodiacal_light"
#define ACP_FN_TULLY                                "tully"
#define ACP_FN_SATELLITES                           "satellites"

//ColorCommand
#define ACP_CC_CONSTELLATION_LINES                  "constellation_lines"
#define ACP_CC_CONSTELLATION_NAMES                  "constellation_names"
#define ACP_CC_CONSTELLATION_ART                    "constellation_art"
#define ACP_CC_CONSTELLATION_BOUNDARIES             "constellation_boundaries"
#define ACP_CC_CARDINAL_POINTS                      "cardinal_points"
#define ACP_CC_PLANET_ORBITS                        "planet_orbits"
#define ACP_CC_PLANET_NAMES                         "planet_names"
#define ACP_CC_PLANET_TRAILS                        "planet_trails"
#define ACP_CC_AZIMUTHAL_GRID                       "azimuthal_grid"
#define ACP_CC_EQUATOR_GRID                         "equator_grid"
#define ACP_CC_ECLIPTIC_GRID                        "ecliptic_grid"
#define ACP_CC_GALACTIC_GRID                        "galactic_grid"
#define ACP_CC_GALACTIC_GRID                        "galactic_grid"
#define ACP_CC_GALACTIC_LINE                        "galactic_line"
#define ACP_CC_ECLIPTIC_LINE                        "ecliptic_line"
#define ACP_CC_MERIDIAN_LINE                        "meridian_line"
#define ACP_CC_ZENITH_LINE                          "zenith_line"
#define ACP_CC_POLAR_POINT                          "polar_point"
#define ACP_CC_POLAR_CIRCLE                         "polar_circle"
#define ACP_CC_ECLIPTIC_CENTER                      "ecliptic_center"
#define ACP_CC_GALACTIC_POLE                        "galactic_pole"
#define ACP_CC_GALACTIC_CENTER                      "galactic_center"
#define ACP_CC_VERNAL_POINTS                        "vernal_points"
#define ACP_CC_ANALEMMA                             "analemma"
#define ACP_CC_ANALEMMA_LINE                        "analemma_line"
#define ACP_CC_GREENWICH_LINE                       "greenwich_line"
#define ACP_CC_ARIES_LINE                           "aries_line"
#define ACP_CC_ZODIAC                               "zodiac"
#define ACP_CC_PERSONAL                             "personal"
#define ACP_CC_PERSONEQ                             "personeq"
#define ACP_CC_NAUTICAL_ALT                         "nautical_alt"
#define ACP_CC_NAUTICAL_RA                          "nautical_ra"
#define ACP_CC_OBJECT_COORDINATES                   "object_coordinates"
#define ACP_CC_MOUSE_COORDINATES                    "mouse_coordinates"
#define ACP_CC_ANGULAR_DISTANCE                     "angular_distance"
#define ACP_CC_LOXODROMY                            "loxodromy"
#define ACP_CC_ORTHODROMY                           "orthodromy"
#define ACP_CC_VERTICAL_LINE                        "vertical_line"
#define ACP_CC_NEBULA_NAMES                         "nebula_names"
#define ACP_CC_NEBULA_CIRCLE                        "nebula_circle"
#define ACP_CC_PRECESSION_CIRCLE                    "precession_circle"
#define ACP_CC_TEXT_USR_COLOR                       "text_usr_color"
#define ACP_CC_STAR_TABLE                           "star_table"

//SetCommand
#define ACP_SC_ATMOSPHERE_FADE_DURATION             "atmosphere_fade_duration"
#define ACP_SC_AUTO_MOVE_DURATION                   "auto_move_duration"
#define ACP_SC_CONSTELLATION_ART_FADE_DURATION      "constellation_art_fade_duration"
#define ACP_SC_CONSTELLATION_ART_INTENSITY          "constellation_art_intensity"
#define ACP_SC_LIGHT_POLLUTION_LIMITING_MAGNITUDE   "light_pollution_limiting_magnitude"
#define ACP_SC_HEADING                              "heading"
#define ACP_SC_HOME_PLANET                          "home_planet"
#define ACP_SC_LANDSCAPE_NAME                       "landscape_name"
#define ACP_SC_LINE_WIDTH                           "line_width"
#define ACP_SC_MAX_MAG_NEBULA_NAME                  "max_mag_nebula_name"
#define ACP_SC_MAX_MAG_STAR_NAME                    "max_mag_star_name"
#define ACP_SC_MOON_SCALE                           "moon_scale"
#define ACP_SC_SUN_SCALE                            "sun_scale"
#define ACP_SC_MILKY_WAY_TEXTURE                    "milky_way_texture"
#define ACP_SC_SKY_CULTURE                          "sky_culture"
#define ACP_SC_SKY_LOCALE                           "sky_locale"
#define ACP_SC_UI_LOCALE                            "ui_locale"
#define ACP_SC_STAR_MAG_SCALE                       "star_mag_scale"
#define ACP_SC_STAR_SIZE_LIMIT                      "star_size_limit"
#define ACP_SC_PLANET_SIZE_LIMIT                    "planet_size_limit"
#define ACP_SC_STAR_SCALE                           "star_scale"
#define ACP_SC_STAR_TWINKLE_AMOUNT                  "star_twinkle_amount"
#define ACP_SC_STAR_FADER_DURATION                  "star_fader_duration"
#define ACP_SC_STAR_LIMITING_MAG                    "star_limiting_mag"
#define ACP_SC_TIME_ZONE                            "time_zone"
#define ACP_SC_AMBIENT_LIGHT                        "ambient_light"
#define ACP_SC_TEXT_FADING_DURATION                 "text_fading_duration"
#define ACP_SC_MILKY_WAY_FADER_DURATION             "milky_way_fader_duration"
#define ACP_SC_MILKY_WAY_INTENSITY                  "milky_way_intensity"
#define ACP_SC_ZOOM_OFFSET                          "zoom_offset"
#define ACP_SC_STARTUP_TIME_MODE                    "startup_time_mode"
#define ACP_SC_DATE_DISPLAY_FORMAT                  "date_display_format"
#define ACP_SC_TIME_DISPLAY_FORMAT                  "time_display_format"
#define ACP_SC_MODE                                 "mode"
#define ACP_SC_SCREEN_FADER                         "screen_fader"
#define ACP_SC_STALL_RADIUS_UNIT                    "stall_radius_unit"
#define ACP_SC_TULLY_COLOR_MODE                     "tully_color_mode"
#define ACP_SC_DATETIME_DISPLAY_POSITION            "datetime_display_position"
#define ACP_SC_DATETIME_DISPLAY_NUMBER              "datetime_display_number"

// list of reserved variable
#define ACI_RW_LONGITUDE            "longitude"
#define ACI_RW_LATITUDE             "latitude"
#define ACI_RW_ALTITUDE             "altitude"


#endif //_BASE_COMMAND_INTERFACE_HPP_
