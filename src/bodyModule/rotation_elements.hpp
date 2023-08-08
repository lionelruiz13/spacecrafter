/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 Immersive Adventure
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

#ifndef _ROTATION_ELEMENTS_HPP_
#define _ROTATION_ELEMENTS_HPP_

// epoch J2000: 12 UT on 1 Jan 2000
#define J2000 2451545.0

// Class used to store orbital elements
struct RotationElements {
	float period = 1;        // rotation period
	float offset = 0;        // rotation at epoch
	double epoch = J2000;
	float obliquity = 0;     // tilt of rotation axis w.r.t. ecliptic
	float ascendingNode = 0; // long. of ascending node of equator on the ecliptic
	float precessionRate = 0; // rate of precession of rotation axis in rads/day
	double sidereal_period = 0; // sidereal period (Body year in earth days)
	float axialTilt = 0; // Only used for tropic lines on planets
};

#endif // _ROTATION_ELEMENTS_HPP_
