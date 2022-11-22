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

#ifndef _SOLARSYSTEM_H_
#define _SOLARSYSTEM_H_

#include <vector>
#include <functional>
#include <map>

#include "bodyModule/body_moon.hpp"
#include "bodyModule/body_bigbody.hpp"
#include "bodyModule/body_sun.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/no_copy.hpp"
#include "bodyModule/body_color.hpp"
#include "bodyModule/protosystem.hpp"

class SolarSystem: public ProtoSystem{
public:
	SolarSystem(ObjLMgr *_objLMgr, Observer *observatory, Navigator *navigation, TimeMgr *timeMgr);
	virtual ~SolarSystem();

	// virtual void setFont(float font_size, const std::string& font_name) override;
	virtual void registerFont(s_font* _font) override;

	//! get the position Alt Az for Sun
	void bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const;

	//!return Earth planet object
	std::shared_ptr<Body> getEarth(void) const {
		return earth;
	}

	//! return Sun planet object
	std::shared_ptr<Sun> getSun(void) const {
		return sun;
	}

	//! return Moon planet object
	std::shared_ptr<Moon> getMoon(void) const {
		return moon;
	}

	//! Set if Moon display is scaled
	void setFlagMoonScale(bool b) {
		if (!b) getMoon()->setSphereScale(1);
		else getMoon()->setSphereScale(moonScale);
		flagMoonScale = b;
	}

	//! Get if Moon display is scaled
	bool getFlagMoonScale(void) const {
		return flagMoonScale;
	}

	//! Set if Sun display is scaled
	void setFlagSunScale(bool b) {
		if (!b) {
			getSun()->setSphereScale(1);
			getSun()->setHaloSize(200);
		} else {
			getSun()->setSphereScale(SunScale);
			getSun()->setHaloSize(200+SunScale*40);
		}

		flagSunScale = b;
	}

	//! Get if Sun display is scaled
	bool getFlagSunScale(void) const {
		return flagSunScale;
	}

	//! Set Moon display scaling factor
	void setMoonScale(float f, bool resident = false) {
		moonScale = f;
		if (flagMoonScale)
			getMoon()->setSphereScale(moonScale, resident);
	}

	//! Get Moon display scaling factor
	float getMoonScale(void) const {
		return moonScale;
	}

	//! Set Sun display scaling factor
	void setSunScale(float f, bool resident = false) {
		SunScale = f;
		if (flagSunScale) getSun()->setSphereScale(SunScale, resident);
	}

	//! Get Sun display scaling factor
	float getSunScale(void) const {
		return SunScale;
	}

	// return the Sun altitude
	double getSunAltitude(const Navigator * nav) const;

	// return the Sun azimuth
	double getSunAzimuth(const Navigator * nav) const;

	// return the Selected body RA UNUSED?
	// double getSelectedRA(const Navigator * nav) const;

	// return the Selected body DE UNUSED?²
	// double getSelectedDE(const Navigator * nav) const;

private:


	// load one object from a hash
	virtual void addBody(stringHash_t param, bool deletable) override;


	std::shared_ptr<Sun> sun=nullptr; //return the Sun
	std::shared_ptr<Moon> moon=nullptr;	//return the Moon
	std::shared_ptr<BigBody> earth=nullptr;	//return the earth

	// solar system related settings
	float object_scale;  // should be kept synchronized with star scale...

	bool flagMoonScale;	// say if float moonScale is used or not
	bool flagSunScale;	// say if float SunScale is used or not
	float moonScale;	// Moon scale value
	float SunScale;	// Sun scale value

	bool nearLunarEclipse(const Navigator * nav, Projector * prj);
};

#endif // _SOLARSYSTEM_H_
