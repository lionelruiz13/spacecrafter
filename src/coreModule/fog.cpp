/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2020 Association Sirius 
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

#include "coreModule/fog.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "tools/s_texture.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"

#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"
#include "tools/Renderer.hpp"



std::unique_ptr<shaderProgram> Fog::shaderFog;
s_texture* Fog::fog_tex;

Fog::Fog(float _radius) : radius(_radius)
{
	m_fogGL = std::make_unique<VertexArray>();
	m_fogGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_fogGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
}


Fog::~Fog()
{}


void Fog::createSC_context()
{
	shaderFog = std::make_unique<shaderProgram>();
	shaderFog-> init( "fog.vert","fog.frag");

	shaderFog->setUniformLocation("fader");
	shaderFog->setUniformLocation("sky_brightness");
	shaderFog->setUniformLocation("ModelViewMatrix");

	fog_tex = new s_texture("fog.png",TEX_LOAD_TYPE_PNG_SOLID_REPEAT,false);
}

void Fog::initShader()
{
	std::vector<float> dataTex;
	std::vector<float> dataPos;

	createFogMesh(radius, radius*sinf(alt_angle*M_PI/180.) , 128,1, &dataTex, &dataPos);

	m_fogGL->fillVertexBuffer(BufferType::POS3D, dataPos);
	m_fogGL->fillVertexBuffer(BufferType::TEXTURE, dataTex);

	dataTex.clear();
	dataPos.clear();
}


// Draw the horizon fog
void Fog::draw(const Projector* prj, const Navigator* nav) const
{
	if (!fader.getInterstate()) return;

	StateGL::BlendFunc(GL_ONE, GL_ONE);

	StateGL::enable(GL_BLEND);
	StateGL::enable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	shaderFog->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fog_tex->getID());
	shaderFog->setUniform("fader", fader.getInterstate());
	shaderFog->setUniform("sky_brightness", sky_brightness);
	Mat4f matrix = (nav->getLocalToEyeMat() * Mat4d::translation(Vec3d(0.,0.,radius*sinf(angle_shift*M_PI/180.)))).convert();
	shaderFog->setUniform("ModelViewMatrix",matrix);

	Renderer::drawArrays(shaderFog.get(), m_fogGL.get(), GL_TRIANGLE_STRIP,0,nbVertex);

	glCullFace(GL_BACK);
	StateGL::disable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
}


void Fog::createFogMesh(GLdouble radius, GLdouble height, GLint slices, GLint stacks, std::vector<float>* dataTex, std::vector<float>* dataPos)
{
	nbVertex=0;
	GLdouble da, r, dz;
	GLfloat z ;
	GLint i;

	da = 2.0 * M_PI / slices;
	dz = height / stacks;

	GLfloat ds = 1.0 / slices;
	GLfloat dt = 1.0 / stacks;
	GLfloat t = 0.0;
	z = 0.0;
	r = radius;
	GLfloat s = 0.0;
	for (i = 0; i <= slices; i++) {
		GLfloat x, y;
		if (i == slices) {
			x = sinf(0.0);
			y = cosf(0.0);
		} else {
			x = sinf(i * da);
			y = cosf(i * da);
		}
		dataTex->push_back(s);
		dataTex->push_back(t);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z);
		nbVertex++;

		dataTex->push_back(s);
		dataTex->push_back(t+dt);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z+dz);
		nbVertex++;
		s += ds;
	}
}