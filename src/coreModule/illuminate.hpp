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
	//! Create Illuminate from passed data and then read in texture
	Illuminate(unsigned int name, double ra, double de, double angular_size, double r, double g, double b, float tex_rotation);
	~Illuminate(){};

	//! return the name of the illuminate
	unsigned int getName() const {
		return name;
	}

	//! return cartesian equatorial position
	const Vec3f& getXYZ() const {
		return XYZ;
	}

	//! rempli les buffers pour un tracé en groupe des illuminates
	void draw(const Projector* prj, float *&data);

private:

	unsigned int name;				//!< name
	Vec3f XYZ;						//!< Cartesian equatorial position
	Vec3f texQuadVertex[4];			//!< 4 vertex used to draw the Illuminate texture
	//	float myRA, myDe; 			//!< RA et De in radians
	Vec3f texColor;					//!< texture color
	float raw[(3+2+3)*4];			//!< datas packed in {POS3D, TEXTURE, COLOR} for vertex
};

#endif // _ILLUMINATE_H_
