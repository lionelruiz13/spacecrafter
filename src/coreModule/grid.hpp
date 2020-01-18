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

#ifndef _GRID_H_
#define _GRID_H_

#include "tools/vecmath.hpp"

class littleGrid {
public:
	littleGrid();
	virtual ~littleGrid();
	int GetNearest(Vec3f&);
	int Intersect(Vec3f pos, float fieldAngle); //! Return an array with the number of the zones in the field of view
	int * getResult(void) const {
		return result;
	}
	int getNbPoints(void) const {
		return NbPoints;
	}
private:
	float Angle;     //! Radius of each zone (in radians)
	int NbPoints;    //! Number of zones
	Vec3f * Points; //! The zones positions
	int * result;
};

#endif // _GRID_H_
