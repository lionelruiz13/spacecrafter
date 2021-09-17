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

#ifndef _SOLARSYSTEM_SCALE_
#define _SOLARSYSTEM_SCALE_

#include "bodyModule/solarsystem.hpp"

/**
 * \file solarsystem_scale.hpp
 * \brief Handle solar system scale functions
 * \author Jérémy Calvo
 * \version 1
 *
 * \class SolarSystemScale
 *
 * \brief Acts on the solar system objects' size.
 *
*/

class SolarSystemScale {
public:
    SolarSystemScale(ProtoSystem * _ssystem);
    ~SolarSystemScale();

	void changeSystem(ProtoSystem * _ssystem) {
		ssystem = _ssystem;
	}

    //! Set the scale factor s of the planet name
	//! @param name the planet's name
	void setPlanetSizeScale(const std::string &name, float s);

	//! Get the scale factor of the planet name
	float getPlanetSizeScale(const std::string &name);

    //! Set base planets display scaling factor
	void setScale(float scale) {
		Body::setScale(scale);
	}

	//! Get base planets display scaling factor
	float getScale(void) const {
		return Body::getScale();
    }

    //! Set base planets display limit in pixels
	void setSizeLimit(float scale) {
		Body::setSizeLimit(scale);
    }

    //! Get base planets display limit in pixels
	float getSizeLimit(void) const {
		return Body::getSizeLimit();
	}

private:
    ProtoSystem * ssystem;
};

#endif