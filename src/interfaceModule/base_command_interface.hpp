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

enum class SC_COMMAND : char {SC_ADD, SC_AUDIO, SC_BODY_TRACE, SC_BODY, SC_CAMERA, SC_CLEAR, SC_COLOR, SC_CONFIGURATION, SC_CONSTELLATION, SC_DATE, SC_DEFINE, SC_DESELECT,
							  SC_DOMEMASTERS,
                              SC_DSO, SC_EXERNASC_MPLAYER, SC_EXTERNASC_VIEWER, SC_FLAG, SC_GET, SC_ILLUMINATE, SC_IMAGE, SC_LANDSCAPE, SC_LOOK, SC_MEDIA, SC_METEORS,
                              SC_MOVETO, SC_MOVETOCITY, SC_MULTIPLIER, SC_MULTIPLY, SC_PERSONAL, SC_PERSONEQ, SC_PLANET_SCALE, SC_POSITION, SC_PRINT, SC_RANDOM,
                              SC_SCRIPT, SC_SEARCH, SC_SELECT, SC_SET, SC_SHUTDOWN, SC_SKY_CULTURE, SC_STAR_LINES, SC_STRUCT, SC_SUNTRACE, SC_TEXT,
                              SC_TIMERATE, SC_WAIT, SC_ZOOMR, SC_FLYTO
                             };

enum class FLAG_VALUES: char { FV_TOGGLE, FV_ON, FV_OFF};

enum class FLAG_NAMES: char {FN_ANTIALIAS_LINES, FN_CONSTELLATION_DRAWING, FN_CONSTELLATION_NAMES, FN_CONSTELLATION_ART, FN_CONSTELLATION_BOUNDARIES, FN_CONSTELLATION_PICK,
                             FN_STAR_TWINKLE, FN_NAVIGATION, FN_SHOW_TUI_DATETIME, FN_SHOW_TUI_SHORT_OBJ_INFO, FN_MANUAL_ZOOM, FN_LIGHT_TRAVEL_TIME, FN_DSO_PICTOGRAMS,
                             FN_FOG, FN_ATMOSPHERE, FN_AZIMUTHAL_GRID, FN_EQUATORIAL_GRID, FN_ECLIPTIC_GRID, FN_GALACTIC_GRID, FN_EQUATOR_LINE, FN_GALACTIC_LINE,
                             FN_ECLIPTIC_LINE, FN_PRECESSION_CIRCLE, FN_CIRCUMPOLAR_CIRCLE, FN_TROPIC_LINES, FN_MERIDIAN_LINE, FN_ZENITH_LINE, FN_POLAR_CIRCLE, FN_POLAR_POINT,
                             FN_ECLIPTIC_CENTER, FN_GALACTIC_POLE, FN_GALACTIC_CENTER, FN_VERNAL_POINTS, FN_ANALEMMA_LINE, FN_ANALEMMA, FN_ARIES_LINE,
                             FN_ZODIAC, FN_PERSONAL, FN_PERSONEQ, FN_NAUTICAL, FN_NAUTICEQ, FN_OBJCOORD, FN_ANG_DIST, FN_LOXODROMY, FN_ORTHODROMY, FN_GREENWICH_LINE, FN_VERTICAL_LINE, FN_CARDINAL_POINTS, FN_CLOUDS, FN_MOON_SCALED, FN_SUN_SCALED,
                             FN_LANDSCAPE, FN_STARS, FN_STAR_NAMES,  FN_STAR_PICK, FN_PLANETS, FN_PLANET_NAMES, FN_PLANET_ORBITS, FN_ORBITS, FN_PLANETS_ORBITS, FN_PLANETS_AXIS,
                             FN_SATELLITES_ORBITS, FN_NEBULAE, FN_NEBULA_NAMES, FN_NEBULA_HINTS, FN_MILKY_WAY, FN_BRIGHT_NEBULAE, FN_OBJECT_TRAILS, FN_TRACK_OBJECT,
                             FN_SCRIPT_GUI_DEBUG, FN_LOCK_SKY_POSITION, FN_BODY_TRACE, FN_SHOW_LATLON, FN_COLOR_INVERSE, FN_OORT, FN_STARS_TRACE, FN_STAR_LINES,
                             FN_SKY_DRAW, FN_ZODIAC_LIGHT , FN_TULLY, FN_SATELLITES, FN_MOUSECOORD, FN_ATMOSPHERIC_REFRACTION
                            };

#endif //_BASE_COMMAND_INTERFACE_HPP_
