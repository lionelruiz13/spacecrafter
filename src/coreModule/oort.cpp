/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/oort.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"


#define NB_POINTS 200000


Oort::Oort()
{
	color = Vec3f(1.0,1.0,0.0);
	fader = false;
	createShader();
}



Oort::~Oort()
{
	clear();
	deleteShader();
}

void Oort::createShader()
{
	shaderOort = new shaderProgram();
	shaderOort->init("oort.vert","oort.frag");
	shaderOort->setUniformLocation("Mat");
	shaderOort->setUniformLocation("color");
	shaderOort->setUniformLocation("intensity");

	glGenVertexArrays(1,&sData.vao);
	glBindVertexArray(sData.vao);
	glGenBuffers(1,&sData.pos);
	glEnableVertexAttribArray(0);
}

void Oort::deleteShader()
{
	if(shaderOort) shaderOort=nullptr;

	glDeleteBuffers(1,&sData.pos);
	glDeleteVertexArrays(1,&sData.vao);
}

void Oort::clear()
{
	dataOort.clear();
}

void Oort::populate(unsigned int nbr) noexcept
{
	this->clear();
	float radius, theta, phi, r_theta, r_phi;
	Vec3f tmp;
	Vec3d tmp_local;

	for(unsigned int i=0; i<nbr ; i++) {
		r_theta = (float) (rand()%3600);
		r_phi = (float) (rand()%1400);
		theta = r_theta /10.;
		phi   = -70. + r_phi /10.;
		if (abs(phi)>60) phi = phi*(1+(abs(phi)-60)/35);
		radius = 60. + (float) (rand()%5000);
		if (radius<2570) phi = phi*(radius-0)/2570;
		if (radius>4000) radius = radius*(1+(radius-4000)/4000);

		Utility::spheToRect(theta*C_PI/180,phi*C_PI/180, tmp);

		dataOort.push_back((float) tmp[0]*radius);
		dataOort.push_back((float) tmp[1]*radius);
		dataOort.push_back((float) tmp[2]*radius);
		//~ printf("%5.3f %5.3f %5.3f \n", tmp_local[0], tmp_local[1], tmp_local[2]);
	}

	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,sData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataOort.size(),dataOort.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
}

void Oort::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate()) return;

	// gestion de l'intensité
	if ((abs(distance) < 1e13) || (abs(distance) > 5.E15))
	return;

	intensity = std::min(1.0, (abs(distance/1.e13)-1));
	if (abs(distance) > 1.E15) intensity = 1.25-0.25*(abs(distance/1.E15));
	//~ printf("distance : %f\n", distance);
	//~ printf("intensity : %f\n", intensity);

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	Mat4f matrix= nav->getHelioToEyeMat().convert();

	shaderOort->use();
	shaderOort->setUniform("Mat",matrix);
	shaderOort->setUniform("color", color);
	shaderOort->setUniform("intensity", intensity*fader.getInterstate());

	glBindVertexArray(sData.vao);

	glDrawArrays(GL_POINTS, 0, dataOort.size()/3 );

	shaderOort->unuse();
}
