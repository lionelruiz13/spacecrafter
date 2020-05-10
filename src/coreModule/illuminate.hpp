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
	friend class IlluminateMgr;
public:

	Illuminate();
	~Illuminate();

	//! return the name of the illuminate
	std::string getName(void) const {
		return Name;
	}

	//! Read the Illuminate data from a string
	// bool createIlluminate(const std::string&);

	//! Create Illuminate from passed data and then read in texture
	bool createIlluminate(const std::string& filename, double ra, double de, double angular_size,const std::string& name, double r, double g, double b, float tex_rotation);

private:
	void drawTex(const Projector* prj, const Navigator * nav);

	std::string Name;					//!< name
	Vec3f XYZ;						//!< Cartesian equatorial position
	//Vec3d XY;						//!< Store temporary 2D position

	static s_texture * illuminateTex;		//!< Common texture
	s_texture * illuminateSpecialTex;		//!< extra texture
	Vec3f texQuadVertex[4];					//!< 4 vertex used to draw the Illuminate texture

	float myRA, myDe; 						//!< RA et De in radians
	Vec3f texColor;							//!< texture color
	bool specialTex;						//!<  indique si la texture finale est utilisée ou pas

protected :

	static void createShader();
	static void deleteShader();

	//Opengl
	static shaderProgram* shaderIllum;
	static DataGL Illum;
	std::vector<float> vecIllumPos;
};

#endif // _ILLUMINATE_H_
