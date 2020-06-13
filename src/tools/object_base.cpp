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

#include "tools/object_base.hpp"
#include "tools/object.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/utility.hpp"
#include "tools/s_texture.hpp"
#include "tools/shader.hpp"
#include "tools/OpenGL.hpp"


void intrusivePtrAddRef(ObjectBase* p)
{
	p->retain();
}

void intrusivePtrRelease(ObjectBase* p)
{
	p->release();
}

ObjectBaseP ObjectBase::getBrightestStarInConstellation() const
{
	return ObjectBaseP();
}

s_texture * ObjectBase::pointer_star = nullptr;
s_texture * ObjectBase::pointer_planet = nullptr;
s_texture * ObjectBase::pointer_nebula = nullptr;

int ObjectBase::local_time = 0;

std::unique_ptr<VertexArray> ObjectBase::m_pointerGL;
std::unique_ptr<VertexArray> ObjectBase::m_starPointerGL;

std::unique_ptr<shaderProgram> ObjectBase::m_shaderPointer;
std::unique_ptr<shaderProgram> ObjectBase::m_shaderStarPointer;


void ObjectBase::createShaderPointeur()
{
	m_shaderPointer = std::make_unique<shaderProgram>();
	m_shaderPointer->init("object_base_pointer.vert","object_base_pointer.geom","object_base_pointer.frag");
	m_shaderPointer->setUniformLocation("color");

	m_pointerGL = std::make_unique<VertexArray>();
	m_pointerGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_pointerGL->registerVertexBuffer(BufferType::MAG, BufferAccess::DYNAMIC);
}

void ObjectBase::createShaderStarPointeur()
{
	m_shaderStarPointer = std::make_unique<shaderProgram>();
	m_shaderStarPointer->init("star_pointer.vert","star_pointer.geom","star_pointer.frag");
	m_shaderStarPointer->setUniformLocation({"radius","color", "matRotation"});

	m_starPointerGL = std::make_unique<VertexArray>();
	m_starPointerGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
}


// Draw a nice animated pointer around the object
void ObjectBase::drawPointer(int delta_time, const Projector* prj, const Navigator * nav)
{
	local_time+=delta_time;
	Vec3d pos=getEarthEquPos(nav);
	Vec3d screenposd;
	// Compute 2D pos and return if outside screen
	if (!prj->projectEarthEqu(pos, screenposd)) return;

	Vec2f screenpos = Vec2f((float) screenposd[0], (float) screenposd[1]);

	// If object is large enough, no need for distracting pointer
	if (getType()==OBJECT_NEBULA || getType()==OBJECT_BODY) {
		double size = getOnScreenSize(prj, nav, false);
		if ( size > prj->getViewportRadius()*.1f ) return;
	}

	if (getType()==OBJECT_NEBULA)
		color=Vec3f(0.4f,0.5f,0.8f);
	if (getType()==OBJECT_BODY)
		color=Vec3f(1.0f,0.3f,0.3f);

	float size = getOnScreenSize(prj, nav, false);
	size+=20.f;
	size+=10.f*sin(0.002f * local_time);

	if (getType()==OBJECT_STAR ) {

		Vec3d screenPos;

		Mat4f matRotation = Mat4f::zrotation((float)local_time/750.);

		// Compute 2D pos and return if outside screen
		if (!prj->projectEarthEqu(pos, screenPos)) return;

		Vec3f color = getRGB();
		float radius=13.f;

		m_starPointerGL->fillVertexBuffer(BufferType::POS3D, 3, (Vec3f) screenPos);

		m_shaderStarPointer->use();

		glBindTexture (GL_TEXTURE_2D, pointer_star->getID());
		StateGL::enable(GL_BLEND);

		//SET UNIFORM
		m_shaderStarPointer->setUniform("radius", radius);
		m_shaderStarPointer->setUniform("matRotation", matRotation);
		m_shaderStarPointer->setUniform("color", color);
		m_starPointerGL->bind();
		glDrawArrays(GL_POINTS,0,1);
		m_starPointerGL->unBind();
		m_shaderStarPointer->unuse();
	}

	if (getType()==OBJECT_NEBULA || getType()==OBJECT_BODY) {
		glActiveTexture(GL_TEXTURE0);
		if (getType()==OBJECT_BODY)
			glBindTexture(GL_TEXTURE_2D, pointer_planet->getID());
		if (getType()==OBJECT_NEBULA)
			glBindTexture(GL_TEXTURE_2D, pointer_nebula->getID());

		m_pos.push_back(screenpos[0] -size/2);
		m_pos.push_back(screenpos[1] +size/2);
		m_indice.push_back(1.0);

		m_pos.push_back(screenpos[0] +size/2);
		m_pos.push_back(screenpos[1] +size/2);
		m_indice.push_back(2.0);

		m_pos.push_back(screenpos[0] +size/2);
		m_pos.push_back(screenpos[1] -size/2);
		m_indice.push_back(3.0);

		m_pos.push_back(screenpos[0] -size/2);
		m_pos.push_back(screenpos[1] -size/2);
		m_indice.push_back(4.0);

		m_pointerGL->fillVertexBuffer(BufferType::POS2D, m_pos);
		m_pointerGL->fillVertexBuffer(BufferType::MAG, m_indice);

		StateGL::enable(GL_BLEND);

		m_shaderPointer->use();
		m_shaderPointer->setUniform("color", color);
		m_pointerGL->bind();		
		glDrawArrays(GL_POINTS,0,4);
		m_pointerGL->unBind();
		m_shaderPointer->unuse();

		m_pos.clear();
		m_indice.clear();
	}
}

void ObjectBase::initTextures()
{
	pointer_star = new s_texture("pointer_star.png");
	pointer_planet = new s_texture("pointer_planet.png");
	pointer_nebula = new s_texture("pointer_nebula.png");
}

void ObjectBase::deleteTextures()
{
	delete pointer_star;
	pointer_star = nullptr;
	delete pointer_planet;
	pointer_planet = nullptr;
	delete pointer_nebula;
	pointer_nebula = nullptr;
}
