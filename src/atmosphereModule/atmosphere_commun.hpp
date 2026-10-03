/*
* Copyright (C) 2003 Fabien Chereau
* Copyright (C) 2009 Digitalis Education Solutions, Inc.
* Copyright (C) 2013 of the LSS team
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

#ifndef _ATM_COMMUN_
#define _ATM_COMMUN_

#include <string>

enum class ATMOSPHERE_MODEL : char {NONE_MODEL, EARTH_MODEL, VENUS_MODEL, MARS_MODEL};

// Rendering controls shared by all atmosphere models.  A body selects a
// profile; the colour and rendering code remains model-agnostic.
struct AtmosphereProfile {
	float turbidity = 5.f;
	float luminanceScale = 1.f;
	float solarGlareDamping = 0.f;
	float cieTintX = 0.f;
	float cieTintY = 0.f;
	float cieTintStrength = 0.f;
	bool applyScotopicCorrection = true;
};

inline AtmosphereProfile getAtmosphereProfile(ATMOSPHERE_MODEL model)
{
	switch (model) {
	case ATMOSPHERE_MODEL::MARS_MODEL: {
		// Nominal dusty daylight: a dark yellowish-brown sky, with no Earth-like
		// blue cast away from the Sun. Values are CIE xy chromaticity controls.
		AtmosphereProfile profile;
		profile.turbidity = 2.8f;
		profile.luminanceScale = 0.20f;
		profile.solarGlareDamping = 0.70f;
		profile.cieTintX = 0.370f;
		profile.cieTintY = 0.370f;
		profile.cieTintStrength = 0.90f;
		profile.applyScotopicCorrection = false;
		return profile;
	}
	case ATMOSPHERE_MODEL::EARTH_MODEL:
	case ATMOSPHERE_MODEL::VENUS_MODEL:
	case ATMOSPHERE_MODEL::NONE_MODEL:
	default:
		return {};
	}
}

#endif
