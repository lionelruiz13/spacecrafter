/*
 * Spacecrafter astronomy simulation and visualization
 *
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

#include <iostream>
#include "coreModule/illuminate.hpp"
#include "tools/s_texture.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/log.hpp"
#include "tools/OpenGL.hpp"


// Read Illuminate data passed in and compute x,y and z;
Illuminate::Illuminate(unsigned int _name, double ra, double de, double angular_size, double r, double g, double b, float tex_rotation)
{
	name = _name;
	texColor.set(r,g,b);

	// Calc the RA and DE from the datas - keep base info for drawing (in radians)
	float myRA = ra*M_PI/180.;
	float myDe = de*M_PI/180.;

	// Calc the Cartesian coord with RA and DE
	Utility::spheToRect(myRA,myDe,XYZ);

	float tex_size = sin(angular_size/2/60*M_PI/180);

	// Precomputation of the rotation/translation matrix
	Mat4f mat_precomp = Mat4f::translation(XYZ) *
	                    Mat4f::zrotation(myRA) *
	                    Mat4f::yrotation(-myDe) *
	                    Mat4f::xrotation(tex_rotation*M_PI/180.);

	texQuadVertex[0] = mat_precomp * Vec3f(0.,-tex_size,-tex_size);
	texQuadVertex[1] = mat_precomp * Vec3f(0., tex_size,-tex_size);
	texQuadVertex[2] = mat_precomp * Vec3f(0.,-tex_size, tex_size);
	texQuadVertex[3] = mat_precomp * Vec3f(0., tex_size, tex_size);
}


void Illuminate::draw(const Projector* prj, std::vector<float> &position, std::vector<float> &texture, std::vector<float> &color )
{
	Vec3d v;
	Vec3f pos;

	//color
	insert_vec3(color, texColor,4);

	//texture
	insert_all(texture, 1.0,0.0, 1.0,1.0, 0.0,0.0, 0.0,1.0 );

	//position
	//~ glTexCoord2i(1,0);              // Bottom Right
	// prj->projectJ2000(texQuadVertex[0],v);
	insert_vec3(position, texQuadVertex[0]);

	//~ glTexCoord2i(1,1);              // Top Right
	// prj->projectJ2000(texQuadVertex[2],v);
	insert_vec3(position, texQuadVertex[2]);

	//~ glTexCoord2i(0,0);              // Bottom Left
	// prj->projectJ2000(texQuadVertex[1],v);
	insert_vec3(position, texQuadVertex[1]);
	
	//~ glTexCoord2i(0,1);              // Top Left
	// prj->projectJ2000(texQuadVertex[3],v);
	insert_vec3(position, texQuadVertex[3]);
}

