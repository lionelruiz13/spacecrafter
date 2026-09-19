/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "tools/vecmath.hpp"
#include <iostream>
#include "tools/object_type.hpp"

class Navigator;
class TimeMgr;
class Observer;
class Projector;
class s_texture;
class ObjectBase;

class Object {
public:
	Object();
	~Object();
	Object(ObjectBase *r);
	Object(const Object &o);
	const Object &operator=(const Object &o);
	const Object &operator=(ObjectBase* const r );
	operator bool() const;
	bool operator==(const Object &o) const;

	// void update();
	void drawPointer(int delta_time,
	                  const Projector *prj,
	                  const Navigator *nav);

	//! Write I18n information about the object in string.
	std::string getInfoString(const Navigator *nav) const;

	//! The returned string can typically be used for object labeling in the sky
	std::string getShortInfoString(const Navigator *nav) const;

	//! This string is info for old nav edition
	std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const;

	//! return the Ra et Dec for a star
	void getAltAz(const Navigator *nav ,double *alt, double *az) const;

	//! return the Ra et Dec for a star
	void getRaDeValue(const Navigator *nav ,double *ra, double *de) const;

	//! Return object's type
	OBJECT_TYPE getType() const;

	//! Return object's name
	std::string getEnglishName() const;
	std::string getNameI18n() const;

	// float getStarDistance( void );

	//! Get position in earth equatorial frame
	Vec3d getEarthEquPos(const Navigator *nav) const;

	//! observer centered J2000 coordinates
	Vec3d getObsJ2000Pos(const Navigator *nav) const;

	//! Return object's magnitude
	float getMag(const Navigator *nav) const;

	//! Get object main color, used to display infos
	Vec3f getRGB() const;

	ObjectBaseP getBrightestStarInConstellation() const;

	// only needed for AutoZoomIn/Out, whatever this is:
	//! Return the best FOV in degree to use for a close view of the object
	double getCloseFov(const Navigator *nav) const;
	//! Return the best FOV in degree to use for a global view
	//! of the object satellite system (if there are satellites)
	double getSatellitesFov(const Navigator *nav) const;
	double getParentSatellitesFov(const Navigator *nav) const;

	float getOnScreenSize(const Projector *prj, const Navigator *nav,  bool orb_only = false);

	static void initTextures();
	static void deleteTextures();

	template<class T>
	T *as() const {
		return dynamic_cast<T *>(rep);
	}
	// static void deleteShaders();

	inline bool operator==(ObjectBase *other) const {
		return rep == other;
	}
private:
	ObjectBase *rep;
};

#endif
