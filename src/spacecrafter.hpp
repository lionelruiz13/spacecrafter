/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2015 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _SPACECRAFTER_HPP_
#define _SPACECRAFTER_HPP_

#include "config.h"

// Windows
#if defined( WIN32 ) || defined ( __MWERKS__ ) || defined( _MSC_VER ) || defined( MINGW32 )
#undef WIN32
#define WIN32 1
#undef USER_EDITION
#define USER_EDITION "Windows"
#include <windows.h>
#endif /*WIN32*/

// Linux
#ifndef WIN32
#define LINUX 1
#define PATH_FILE_LOCK "/tmp/spacecrafter.lock"
#define CONFIG_DATA_DIR "/usr/local/share/spacecrafter/"
#define LOCALEDIR "/usr/local/share/locale/"
#endif /*Linux*/


//#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <string>


//#include <cassert>

const std::string REP_SCRIPT = "scripts";
const std::string REP_SCREENSHOT = "screenshot";
const std::string REP_FONT = "fonts";
const std::string REP_AUDIO = "audio";
const std::string REP_LOG = "log";
const std::string REP_FTP = "ftp";
const std::string REP_VFRAME = "vframes";
const std::string REP_PICTURE = "pictures";
const std::string REP_TEXTURE = "textures";
const std::string REP_VIDEO = "videos";
const std::string REP_MEDIA = "media";
const std::string REP_VR360 = "vr360";
const std::string REP_WEB = "www";
const std::string REP_MODEL3D = "model3D";
const std::string REP_LANDSCAPE = "landscapes";
const std::string REP_SKY_CULTURE = "sky_cultures";

const std::string REP_DATA = "data";
const std::string REP_SHADER = "shaders";
const std::string REP_LANGUAGE = "language"; 

#endif /*_SPACECRAFTER_HPP_*/
