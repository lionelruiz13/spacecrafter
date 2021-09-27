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

#include "bodyModule/solarsystem_color.hpp"
#include "bodyModule/ssystem_iterator.hpp"
#include "bodyModule/solarsystem.hpp"

SolarSystemColor::SolarSystemColor(ProtoSystem * _ssystem)
{
    ssystem = _ssystem;
}

SolarSystemColor::~SolarSystemColor() {};

void SolarSystemColor::setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c)
{
	if (englishName=="all") {
        auto it = ssystem->createIterator();
		for(it->begin(); !it->end(); (*it)++){
			it->current()->second->body->setColor(colorName,c);
		}
    }
	else{

		std::shared_ptr<Body> body = ssystem->searchByEnglishName(englishName);

		if(body != nullptr){
			body->setColor(colorName, c);
		}
	}
}

const Vec3f SolarSystemColor::getBodyColor(const std::string &englishName, const std::string& colorName) const
{
	std::shared_ptr<Body> body = ssystem->searchByEnglishName(englishName);

	if(body != nullptr){
		return body->getColor(colorName);
	}
	else{
		return v3fNull;
	}
}


void SolarSystemColor::setDefaultBodyColor(const std::string& colorName, const Vec3f& c){
	BodyColor::setDefault(colorName, c);
}

const Vec3f SolarSystemColor::getDefaultBodyColor(const std::string& colorName) const{
	return BodyColor::getDefault(colorName);
}