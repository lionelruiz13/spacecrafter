/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
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
#include <iomanip>

#include "bodyModule/ring.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "tools/s_font.hpp"
#include "planetsephems/sideral_time.h"
#include "tools/log.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"


Ring::Ring(double radius_min,double radius_max,const std::string &texname, const Vec3i &_init )
	:radius_min(radius_min),radius_max(radius_max)
{
	init = _init;
	tex = new s_texture(texname,TEX_LOAD_TYPE_PNG_ALPHA,1);
	createGL_context();
	lowUP = new Ring2D((float) radius_min, (float) radius_max, init[0], 4, true);
	lowDOWN = new Ring2D((float) radius_min, (float) radius_max, init[0], 4, false);

	mediumUP = new Ring2D((float) radius_min, (float) radius_max, init[1], 8, true);
	mediumDOWN = new Ring2D((float) radius_min, (float) radius_max, init[1], 8, false);

	highUP = new Ring2D((float) radius_min, (float) radius_max, init[2], 16, true);
	highDOWN = new Ring2D((float) radius_min, (float) radius_max, init[2], 16, false);
}


void Ring::createGL_context()
{
	shaderRing = std::make_unique<shaderProgram>();
	shaderRing->init( "ring_planet.vert","ring_planet.frag");

	shaderRing->setUniformLocation("Texture");
	shaderRing->setUniformLocation("LightDirection");
	shaderRing->setUniformLocation("PlanetRadius");
	shaderRing->setUniformLocation("PlanetPosition");
	shaderRing->setUniformLocation("SunnySideUp");
	shaderRing->setUniformLocation("RingScale");

	shaderRing->setUniformLocation("ModelViewProjectionMatrix");
	shaderRing->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderRing->setUniformLocation("ModelViewMatrix");
	shaderRing->setUniformLocation("clipping_fov");
	shaderRing->setUniformLocation("NormalMatrix");
	shaderRing->setUniformLocation("ModelViewMatrixInverse");
}



Ring::~Ring(void)
{
	if (tex) delete tex;
	tex = nullptr;
	delete lowUP;
	delete lowDOWN;
	delete mediumUP;
	delete mediumDOWN;
	delete highUP;
	delete highDOWN;
}

void Ring::draw(const Projector* prj,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius)
{
	Vec3f lightDirection=_lightDirection;
	Vec3f planetPosition=_planetPosition;

	// Normal transparency mode
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	StateGL::enable(GL_CULL_FACE);
	StateGL::enable(GL_BLEND);


	shaderRing->use();

	glBindTexture (GL_TEXTURE_2D, tex->getID());

	// solve the ring wraparound by culling: decide if we are above or below the ring plane
	const double h = mat.r[ 8]*mat.r[12]
	                 + mat.r[ 9]*mat.r[13]
	                 + mat.r[10]*mat.r[14];

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex->getID());

	shaderRing->setUniform("Texture",0);
	shaderRing->setUniform("LightDirection",lightDirection);
	shaderRing->setUniform("PlanetPosition", planetPosition);
	shaderRing->setUniform("PlanetRadius",planetRadius);
	shaderRing->setUniform("RingScale",mc);


	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	Mat4f inv_matrix = matrix.inverse();
	shaderRing->setUniform("ModelViewProjectionMatrix",proj*matrix);
	shaderRing->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	shaderRing->setUniform("ModelViewMatrix",matrix);
	shaderRing->setUniform("clipping_fov",prj->getClippingFov());
	shaderRing->setUniform("NormalMatrix", inv_matrix.transpose());
	shaderRing->setUniform("ModelViewMatrixInverse", inv_matrix);

	if (h>0.0) {
		shaderRing->setUniform("SunnySideUp",1.0);
	}
	else {
		shaderRing->setUniform("SunnySideUp",0.0);
	}

	if (screen_sz < 30.f) {
		if (h>0.0) lowUP->draw();
		else lowDOWN->draw();
	}
	else {
		if (screen_sz >300.f) {
			if (h>0.0) highUP->draw();
			else highDOWN->draw();
		}
		else {
			if (h>0.0) mediumUP->draw();
			else mediumDOWN->draw();
		}
	}


	shaderRing->unuse();
	glActiveTexture(GL_TEXTURE0);

	StateGL::disable(GL_CULL_FACE);
}


// class Ring2D
Ring2D::Ring2D(float _r_min, float _r_max, GLint _slices, GLint _stacks, bool h)
{
	r_min = _r_min;
	r_max = _r_max;

	computeRing(_slices, _stacks, h);

	m_dataGL = std::make_unique<VertexArray>();
	m_dataGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_dataGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	m_dataGL->fillVertexBuffer(BufferType::POS2D, dataVertex);
	m_dataGL->fillVertexBuffer(BufferType::TEXTURE, dataTexture);
}

Ring2D::~Ring2D()
{
	dataTexture.clear();
	dataVertex.clear();
}

void Ring2D::draw()
{
	m_dataGL->bind();
	glDrawArrays(GL_TRIANGLE_STRIP,0,dataVertex.size()/2);
	m_dataGL->unBind();
}


void Ring2D::computeRing(GLint slices, GLint stacks, bool h)
{
	double theta;
	double x,y;
	int j;

	const double dr = (r_max-r_min) / stacks;
	const double dtheta = 2.0 * M_PI / slices*(1-2*h);

	//~ if (slices < 0) slices = -slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (j = 0; j <= slices; j++) {
		theta = (j == slices) ? 0.0 : j * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// draw intermediate stacks as quad strips
	for (double r = r_min; r < r_max; r+=dr) {
		const double tex_r0 = (r-r_min)/(r_max-r_min);
		const double tex_r1 = (r+dr-r_min)/(r_max-r_min);

		for (j=0,cos_sin_theta_p=cos_sin_theta; j<=slices; j++,cos_sin_theta_p+=2) {
			theta = (j == slices) ? 0.0 : j * dtheta;

			x = r*cos_sin_theta_p[0];
			y = r*cos_sin_theta_p[1];

			//~ glTexCoord2d(tex_r0, 0.5);
			dataTexture.push_back(tex_r0);
			dataTexture.push_back(0.5);

			//~ glColor3f(x,y,0);
			dataVertex.push_back(x);
			dataVertex.push_back(y);

			x = (r+dr)*cos_sin_theta_p[0];
			y = (r+dr)*cos_sin_theta_p[1];

			//~ glTexCoord2d(tex_r1, 0.5);
			dataTexture.push_back(tex_r1);
			dataTexture.push_back(0.5);

			//~ glColor3f(x,y,0);
			dataVertex.push_back(x);
			dataVertex.push_back(y);
		}
	}
}
