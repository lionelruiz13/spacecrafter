/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the LSS Team & Association Sirius
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

#ifndef _ILLUMINATE_H_
#define _ILLUMINATE_H_

#include "tools/object_base.hpp"
#include <vector>

class Navigator;
class Projector;
class s_texture;

class Illuminate {

public:

	Illuminate(){};
	~Illuminate(){};

	//! return the name of the illuminate
	const std::string& getName() const {
		return Name;
	}

	//! return cartesian equatorial position
	const Vec3f& getXYZ() const {
		return XYZ;
	}

	//! Create Illuminate from passed data and then read in texture
	bool createIlluminate(double ra, double de, double angular_size,const std::string& name, double r, double g, double b, float tex_rotation);
	//! rempli les buffers pour un tracé en groupe des illuminates
	void draw(const Projector* prj, std::vector<float> &position, std::vector<float> &texture, std::vector<float> &color );

private:

	std::string Name;					//!< name
	Vec3f XYZ;						//!< Cartesian equatorial position
	Vec3f texQuadVertex[4];					//!< 4 vertex used to draw the Illuminate texture
	//	float myRA, myDe; 						//!< RA et De in radians
	Vec3f texColor;							//!< texture color
};

#endif // _ILLUMINATE_H_
