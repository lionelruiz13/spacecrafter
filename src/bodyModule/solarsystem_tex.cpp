/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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

#include "solarsystem_tex.hpp"
#include "tools/log.hpp"

SolarSystemTex::SolarSystemTex(ProtoSystem * _ssystem)
{
    ssystem = _ssystem;

    bodyTesselation =  std::make_shared<BodyTesselation>();
	assert(bodyTesselation != nullptr);
	bodyTesselation->createTesselationParams();
	Body::setTesselation(bodyTesselation);
}

SolarSystemTex::~SolarSystemTex()
{
	Body::deleteDefaultTexMap();
}

void SolarSystemTex::switchPlanetTexMap(const std::string &name, bool a)
{
	std::shared_ptr<Body> body = ssystem->searchByEnglishName(name);
	if(body != nullptr){
		body->switchMapSkin(a);
	}
}

bool SolarSystemTex::getSwitchPlanetTexMap(const std::string &name)
{
	std::shared_ptr<Body> body = ssystem->searchByEnglishName(name);
	if(body != nullptr){
		return body->getSwitchMapSkin();
	}
	return false;
}

void SolarSystemTex::createTexSkin(const std::string &name, const std::string &texName)
{
	std::shared_ptr<Body> body = ssystem->searchByEnglishName(name);
	if(body != nullptr){
		body->createTexSkin(texName);
	}
}

void SolarSystemTex::iniTextures()
{
	if( ! Body::setTexEclipseMap("bodies/eclipse_map.png")) {
		cLog::get()->write("no tex_eclipse_map valid !", LOG_TYPE::L_ERROR);
	}

	if( ! Body::setTexDefaultMap("bodies/nomap.png")) {
		cLog::get()->write("no tex_default_map valid !", LOG_TYPE::L_ERROR);
	}

	if( ! Body::setTexHaloMap("planethalo.png")) {
		cLog::get()->write("no tex_halo valid !", LOG_TYPE::L_ERROR);
	}

	cLog::get()->write("(solar system) default textures loaded", LOG_TYPE::L_INFO);
}

void SolarSystemTex::planetTesselation(std::string name, int value) {
	if (name=="min_tes_level") {
		bodyTesselation->setMinTes(value);
		return;
	}
	if (name=="max_tes_level") {
		bodyTesselation->setMaxTes(value);
		return;
	}
	if (name=="planet_altimetry_level") {
		bodyTesselation->setPlanetTes(value);
		return;
	}
	if (name=="moon_altimetry_level") {
		bodyTesselation->setMoonTes(value);
		return;
	}
	if (name=="earth_altimetry_level") {
		bodyTesselation->setEarthTes(value);
		return;
	}
}