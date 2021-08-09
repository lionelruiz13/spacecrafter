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

#include "solarsystem_scale.hpp"
#include "solarsystem.hpp"

SolarSystemScale::SolarSystemScale(ProtoSystem * _ssystem) {
    ssystem = _ssystem;
}

SolarSystemScale::~SolarSystemScale(){};


void SolarSystemScale::setPlanetSizeScale(const std::string &name, float s)
{
	Body * body = ssystem->searchByEnglishName(name);

	if(body != nullptr){
		body->setSphereScale(s);
	}
}

float SolarSystemScale::getPlanetSizeScale(const std::string &name)
{

	Body * body = ssystem->searchByEnglishName(name);

	return body == nullptr ? 1.0 : body->getSphereScale();
}