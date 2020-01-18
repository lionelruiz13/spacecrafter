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

#ifndef _RING_H_
#define _RING_H_

#include "tools/object_base.hpp"
#include "tools/utility.hpp"
#include "tools/s_font.hpp"
#include "tools/tone_reproductor.hpp"
#include "tools/vecmath.hpp"
#include "coreModule/callbacks.hpp"
#include "tools/fader.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"


class Ring2D {
public:
	Ring2D(float _r_min, float _r_max, GLint slices, GLint stacks, bool h);
	~Ring2D();
	void draw();

private:
	void computeRing(GLint slices, GLint stacks, bool h);
	std::vector<GLfloat> dataTexture, dataVertex;
	DataGL cModel; //currentModel
	float r_min;
	float r_max;
};


// Class to manage rings for planets like saturn
class Ring {

public:
	Ring(double radius_min,double radius_max,const std::string &texname, /*const std::string &path,*/ const Vec3i &init);
	~Ring(void);

	void draw(const Projector* prj,const Mat4d& mat,double screen_sz,Vec3f& lightDirection,Vec3f& planetPosition, float planetRadius);

	double getOuterRadius(void) const {
		return radius_max*mc;
	}

	double getInnerRadius(void) const {
		return radius_min*mc;
	}

	unsigned int getTexID(void) const {
		return tex->getID();
	}

	void multiplyRadius(float f) {
		mc =f;
	}

private:
	const double radius_min;
	const double radius_max;
	const s_texture *tex;

	shaderProgram *shaderRing;	// Shader moderne
	void createShader();
	void deleteShader();

	Ring2D* lowUP;
	Ring2D* lowDOWN;
	Ring2D* mediumUP;
	Ring2D* mediumDOWN;
	Ring2D* highUP;
	Ring2D* highDOWN;

	Vec3i init;
	float mc = 1.0;
};


#endif // _RING_H_
