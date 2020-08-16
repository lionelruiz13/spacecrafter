/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2011 Digitalis Education Solutions, Inc.
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

#ifndef _STAR_GALAXY_HPP_
#define _STAR_GALAXY_HPP_

#include <vector>
#include <memory>

#include "tools/object_base.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/s_texture.hpp"
#include "tools/s_font.hpp"
#include "tools/tone_reproductor.hpp"
#include "tools/translator.hpp"
#include "coreModule/starManager.hpp"


class StarGalaxy : public ObjectBase {

public:
	StarGalaxy(starInfo* star);
	~StarGalaxy();

	//! display pretty information from the deepskyObject
	std::string getInfoString(const Navigator * nav) const;

	std::string getShortInfoString(const Navigator * nav = nullptr) const;

	std::string getShortInfoNavString(const Navigator * nav, const TimeMgr * timeMgr, const Observer* observatory) const;

	OBJECT_TYPE getType(void) const {
		return OBJECT_STARGALAXY;
	}

	Vec3d getEarthEquPos(const Navigator *nav) const;

	Vec3d getObsJ2000Pos(const Navigator *nav) const;

	//! Return the apparent magnitude for DSO object
	float getMag(const Navigator * nav = nullptr) const {
		return mag;
	}

	std::string getNameI18n(void) const {
		return nameI18;
	}

	//! return the Name of the DSO
	std::string getEnglishName(void) const {
		return englishName;
	}
	void translateName(Translator&);

	void setXY(const Projector *prj) {
		prj->projectJ2000(XYZ, XY);
	}

private:
	unsigned int hip;				//nom de l'étoile
	std::string englishName;		// English name
	std::string nameI18;			// translated englishName
	float mag;						// Apparent magnitude for object

	Vec3f XYZ;						// Cartesian equatorial position
	Vec3d XY;						// Store temporary 2D position
};

#endif // _NEBULA_H_
