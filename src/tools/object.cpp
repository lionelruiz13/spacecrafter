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

#include "tools/object.hpp"
#include "tools/object_base.hpp"


class ObjectUninitialized : public ObjectBase {
public:
	ObjectUninitialized() {}
private:
	std::string getInfoString(const Navigator *nav) const
	{
		return "";
	}
	void getRaDeValue(const Navigator *nav ,double *ra, double *de) const
	{
	}

	std::string getShortInfoString(const Navigator *nav) const
	{
		return "";
	}

	std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const
	{
		return "";
	}

	/*ObjectRecord::*/OBJECT_TYPE getType() const
	{
		return /*ObjectRecord::*/OBJECT_UNINITIALIZED;
	}
	std::string getEnglishName() const
	{
		return "";
	}
	std::string getNameI18n() const
	{
		return "";
	}
	Vec3d getEarthEquPos(const Navigator*) const
	{
		return Vec3d(1,0,0);
	}
	Vec3d getObsJ2000Pos(const Navigator*) const
	{
		return Vec3d(1,0,0);
	}
	float getMag(const Navigator * nav) const
	{
		return -10;
	}
	//~ bool isDeleteable() const
	//~ {
		//~ return 0;
	//~ }
};

static ObjectUninitialized uninitialized_object;

Object::~Object()
{
	rep->release();
}

Object::Object()
	:rep(&uninitialized_object)
{
	rep->retain();
}

Object::Object(ObjectBase *r)
	:rep(r?r:&uninitialized_object)
{
	rep->retain();
}

Object::Object(const Object &o)
	:rep(o.rep)
{
	rep->retain();
}

const Object &Object::operator=(const Object &o)
{
	if (this != &o) {
		rep = o.rep;
		rep->retain();
	}
	return *this;
}

const Object &Object::operator=(ObjectBase* const r)
{
	if(r) {
		rep = r;
		rep->retain();
	} else
		rep = &uninitialized_object;

	return *this;
}

Object::operator bool() const
{
	return (rep != &uninitialized_object);
}

bool Object::operator==(const Object &o) const
{
	return (rep == o.rep);
}

// void Object::update()
// {
// 	rep->update();
// }

void Object::drawPointer(int delta_time,
                          const Projector *prj,
                          const Navigator *nav)
{
	rep->drawPointer(delta_time,prj,nav);
}

std::string Object::getInfoString(const Navigator *nav) const
{
	return rep->getInfoString(nav);
}

void Object::getAltAz(const Navigator *nav ,double *alt, double *az) const
{
	return rep->getAltAz(nav,alt,az);
}

void Object::getRaDeValue(const Navigator *nav ,double *ra, double *de) const
{
	return rep->getRaDeValue(nav,ra,de);
}

std::string Object::getShortInfoString(const Navigator *nav) const
{
	return rep->getShortInfoString(nav);
}

std::string Object::getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const
{
	return rep->getShortInfoNavString(nav, timeMgr, observatory);
}

/*ObjectRecord::*/OBJECT_TYPE Object::getType() const
{
	return rep->getType();
}

std::string Object::getEnglishName() const
{
	return rep->getEnglishName();
}

//~ bool Object::isDeleteable() const
//~ {
	//~ return rep->isDeleteable();
//~ }


std::string Object::getNameI18n() const
{
	return rep->getNameI18n();
}

Vec3d Object::getEarthEquPos(const Navigator *nav) const
{
	return rep->getEarthEquPos(nav);
}

Vec3d Object::getObsJ2000Pos(const Navigator *nav) const
{
	return rep->getObsJ2000Pos(nav);
}

float Object::getMag(const Navigator *nav) const
{
	return rep->getMag(nav);
}

Vec3f Object::getRGB() const
{
	return rep->getRGB();
}

ObjectBaseP Object::getBrightestStarInConstellation() const
{
	return rep->getBrightestStarInConstellation();
}

double Object::getCloseFov(const Navigator *nav) const
{
	return rep->getCloseFov(nav);
}

double Object::getSatellitesFov(const Navigator *nav) const
{
	return rep->getSatellitesFov(nav);
}

double Object::getParentSatellitesFov(const Navigator *nav) const
{
	return rep->getParentSatellitesFov(nav);
}

void Object::initTextures()
{
	ObjectBase::initTextures();
}

void Object::deleteTextures()
{
	ObjectBase::deleteTextures();
}

// void Object::deleteShaders()
// {
// 	ObjectBase::deleteShaderStarPointeur();
// 	ObjectBase::deleteShaderPointeur();
// }

float Object::getOnScreenSize(const Projector *prj, const Navigator *nav, bool orb_only)
{
	return rep->getOnScreenSize(prj, nav, orb_only);
}

// float Object::getStarDistance( void )
// {
// 	return rep->getStarDistance();
// }
