/*
 * Spacecrafter astronomy simulation and visualization
 *
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

#include <vector>
#include <memory>

#include "coreModule/StarGalaxy.hpp"

StarGalaxy::StarGalaxy(starInfo* star)
{
    hip = star->HIP;
    XYZ = star->posXYZ;
    englishName= std::to_string(hip);
    nameI18 = englishName;
    std::cout << "StarGalaxy " <<  hip << " " << XYZ << " " << englishName << std::endl;
}

StarGalaxy::~StarGalaxy()
{}

Vec3d StarGalaxy::getEarthEquPos(const Navigator *nav) const
{
    return nav->getHelioToEyeMat() * Mat4d::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180) * XYZ;
    //return XYZ;
    //return nav->j2000ToEarthEqu(XYZ);
}

Vec3d StarGalaxy::getObsJ2000Pos(const Navigator *nav) const {
	return XYZ;
}

std::string StarGalaxy::getInfoString(const Navigator * nav) const
{
    return englishName;
}

std::string StarGalaxy::getShortInfoString(const Navigator * nav) const
{
    return englishName;
}

std::string StarGalaxy::getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const
{
    return englishName;
}

void StarGalaxy::translateName(Translator& trans)
{
	nameI18 = trans.translateUTF8(englishName);
}

