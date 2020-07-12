/*
 * Copyright (C) 2020 of the LSS Team & Association Sirius
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

// text and font common definitions 

#ifndef _TEXT_FONT_COMMON_H
#define _TEXT_FONT_COMMON_H

#include <string>
#include "tools/vecmath.hpp"

#define SIZE_MIN_TO_DISPLAY 12

enum class TEXT_ALIGN : char {LEFT, RIGHT, CENTER};


enum class FONT_SIZE : char {T_XX_SMALL, T_X_SMALL, T_SMALL, T_MEDIUM, T_LARGE, T_X_LARGE, T_XX_LARGE};

struct TEXT_MGR_PARAM {
    std::string string;
    float altitude;
    float azimuth;
    std::string fontSize;
    std::string textAlign;
    Vec3f color;
    bool useColor;
};

#endif
