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

//! A counted handle on an ObjectBase.
//!
//! OWNERSHIP CONTRACT (I1: this is what a caller needs, not how it is done).
//! An Object has RETAINED whatever `rep` points at, on every path including the
//! uninitialized singleton, and releases it when it stops pointing at it. Which
//! reps that actually keeps alive is the rep's own business: `ObjectBase::retain`
//! /`release` are no-ops by default (Body, Nebula, Constellation - owned by
//! their managers) and delete-at-zero in the four refcounted subclasses
//! (StarWrapperBase, ModularObject, TullyWrapper, Star3DWrapper), all four of
//! which are heap-minted per query and owned by nothing else. So an Object
//! assignment CAN be the last release, i.e. can destroy the wrapper it drops.
//!
//! WHO MAY THEREFORE HOLD A RAW POINTER PAST AN ASSIGNMENT - the enumeration
//! this class's invariant depends on (§5.34 / INTENT §11.140; the F29
//! writer-enumeration precedent: the list is here so a new holder is added
//! against it rather than in ignorance of it).
//!   * NOBODY stores a raw `ObjectBase*`. `Object::rep` below is the only such
//!     member in `src/`; every other long-lived holder is either an `Object`
//!     (Core::selected_object / Core::old_selected_object /
//!     SolarSystemSelected::selected / SSystemFactory::selected_object) or an
//!     `ObjectBaseP` (Constellation::asterism[], the searchAround/search results),
//!     and both of those count.
//!   * `as<T>()` and `operator==(ObjectBase*)` hand a raw pointer OUT. Every
//!     call site uses it within the expression or the block, while the Object it
//!     came from is alive, and only for `Body` (`solarsystem_selected.cpp`) and
//!     `ModularObject` (`ssystem_factory.cpp:771`). Anything longer-lived must
//!     hold an `Object` (or an `ObjectBaseP`), not the pointer.
//!   * Managers keep NAMES, not reps, for their selection sets
//!     (`HipStarMgr::selected_star*` = strings + HIP ints,
//!     `NebulaMgr::selected_nebulas` = strings, `ConstellationMgr::selected` =
//!     manager-owned `Constellation*`), so no selection set outlives a rep.
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

	//! Raw view of the rep as T, or nullptr. VALID ONLY WHILE THIS Object STILL
	//! HOLDS IT: assigning this Object can release the last reference and
	//! destroy the rep (see the ownership contract above).
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
