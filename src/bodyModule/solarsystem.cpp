/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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
#include <iostream>
#include <string>
#include <future>
#include <memory>
#include "bodyModule/solarsystem.hpp"
#include "tools/s_texture.hpp"
#include "bodyModule/orbit.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "tools/utility.hpp"
#include <fstream>
#include "tools/log.hpp"
//#include "tools/fmath.hpp"
#include "tools/sc_const.hpp"
#include "tools/app_settings.hpp"
#include "bodyModule/ring.hpp"
#include "bodyModule/halo.hpp"

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/orbit_creator_cor.hpp"
#include "appModule/space_date.hpp"

#define SOLAR_MASS 1.989e30
#define EARTH_MASS 5.976e24
#define LUNAR_MASS 7.354e22
#define MARS_MASS  0.64185e24

SolarSystem::SolarSystem(ObjLMgr *_objLMgr, Observer *observatory, Navigator *navigation, TimeMgr *timeMgr)
	:ProtoSystem(_objLMgr, observatory, navigation, timeMgr), sun(nullptr),moon(nullptr),earth(nullptr), moonScale(1.)
{
}

SolarSystem::~SolarSystem()
{
	systemBodies.clear();
	renderedBodies.clear();

	// BodyShader::deleteShader();
	Body::deleteDefaultatmosphereParams();
	Body::deleteShader();

	sun = nullptr;
	moon = nullptr;
	earth = nullptr;
}

void SolarSystem::registerFont(s_font* _font)
{
	font = _font;
	Body::setFont(font);
}

// Init and load one solar system object
// This is a the private method
void SolarSystem::addBody(stringHash_t param, bool deletable)
{
	const std::string &englishName = param["name"];
	BODY_TYPE typePlanet = setPlanetType(param["type"]);

	ProtoSystem::addBody(std::move(param), deletable);

	if (typePlanet == SUN && englishName == "Sun") {
		sun = std::dynamic_pointer_cast<Sun>(systemBodies["Sun"].body);
	} else if (typePlanet == MOON && englishName == "Moon") {
		moon = std::dynamic_pointer_cast<Moon>(systemBodies["Moon"].body);
		if (earth) {
			BinaryOrbit *earthOrbit = dynamic_cast<BinaryOrbit *>(earth->getOrbit());
			if (earthOrbit) {
				cLog::get()->write("Adding Moon to Earth binary orbit.", LOG_TYPE::L_INFO);
				earthOrbit->setSecondaryOrbit(systemBodies["Moon"].body->getOrbit());
			} else
				cLog::get()->write(englishName + " body could not be added to Earth orbit.", LOG_TYPE::L_WARNING);

		} else
			cLog::get()->write(englishName + " body could not be added to Earth orbit calculation, position may be inacurate", LOG_TYPE::L_WARNING);
	} else if (typePlanet == PLANET && englishName == "Earth") {
		earth = std::dynamic_pointer_cast<BigBody>(systemBodies["Earth"].body);
	}
}

// is a lunar eclipse close at hand?
bool SolarSystem::nearLunarEclipse(const Navigator * nav, Projector *prj)
{
	// TODO: could replace with simpler test
	Vec3d e = getEarth()->get_ecliptic_pos();
	Vec3d m = getMoon()->get_ecliptic_pos();  // relative to earth
	Vec3d mh = getMoon()->get_heliocentric_ecliptic_pos();  // relative to sun

	// shadow location at earth + moon distance along earth vector from sun
	Vec3d en = e;
	en.normalize();
	Vec3d shadow = en * (e.length() + m.length());

	// find shadow radii in AU
	double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;

	// modify shadow location for scaled moon
	Vec3d mdist = shadow - mh;
	if (mdist.length() > r_penumbra + 2000/AU) return 0;  // not visible so don't bother drawing

	return 1;
}

void SolarSystem::bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const
{
 	bodyTrace->getAltAz(nav,alt,az);
}

double SolarSystem::getSunAltitude(const Navigator * nav) const
{
	double alt, az;
	sun->getAltAz(nav, &alt, &az);
	return alt*180.0/M_PI;
}

// UNUSED ?
// double SolarSystem::getSelectedRA(const Navigator * nav) const
// {
// 	double alt, az;
// 	moon->getAltAz(nav, &alt, &az);
// 	return alt*180.0/M_PI;
// }
//
// // UNUSED ?
// double SolarSystem::getSelectedDE(const Navigator * nav) const
// {
// 	double alt, az;
// 	moon->getAltAz(nav, &alt, &az);
// 	return az*180.0/M_PI;
// }

double SolarSystem::getSunAzimuth(const Navigator * nav) const
{
	double alt, az;
	sun->getAltAz(nav, &alt, &az);
	return az*180.0/M_PI;
}
