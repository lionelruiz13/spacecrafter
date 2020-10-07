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
#include "renderGL/shader.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/Renderer.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/ResourceTracker.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"

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

CommandMgr *ObjectBase::cmdMgr;
CommandMgr *ObjectBase::cmdMgrTarget;
int ObjectBase::commandIndexPointer, ObjectBase::commandIndexStarPointer;
VertexArray *ObjectBase::m_pointerGL, *ObjectBase::m_starPointerGL;
Pipeline *ObjectBase::m_pipelinePointer, *ObjectBase::m_pipelineStarPointer;
PipelineLayout *ObjectBase::m_layoutPointer, *ObjectBase::m_layoutStarPointer;
Set *ObjectBase::m_setPointer, *ObjectBase::m_setStarPointer, *ObjectBase::m_globalSet;
Uniform *ObjectBase::m_uColor, *ObjectBase::m_uGeom;
Vec3f *ObjectBase::m_pColor = nullptr;
ObjectBase::objBaseGeom *ObjectBase::m_pGeom;
OBJECT_TYPE ObjectBase::lastType = OBJECT_UNINITIALIZED;
// std::unique_ptr<shaderProgram> ObjectBase::m_shaderPointer;
// std::unique_ptr<shaderProgram> ObjectBase::m_shaderStarPointer;


void ObjectBase::createShaderPointeur(ThreadContext *context)
{
	cmdMgr = context->commandMgrDynamic;
	cmdMgrTarget = context->commandMgr;
	// m_shaderPointer = std::make_unique<shaderProgram>();
	// m_shaderPointer->init("object_base_pointer.vert","object_base_pointer.geom","object_base_pointer.frag");
	// m_shaderPointer->setUniformLocation("color");

	m_pointerGL = context->global->tracker->track(new VertexArray(context->surface));
	m_pointerGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_pointerGL->registerVertexBuffer(BufferType::MAG, BufferAccess::DYNAMIC);
	m_pointerGL->build(4);

	m_layoutPointer = context->global->tracker->track(new PipelineLayout(context->surface));
	m_layoutPointer->setGlobalPipelineLayout(context->global->globalLayout);
	m_layoutPointer->setTextureLocation(0);
	m_layoutPointer->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	m_layoutPointer->buildLayout();
	m_layoutPointer->build();
	m_pipelinePointer = context->global->tracker->track(new Pipeline(context->surface, m_layoutPointer));
	m_pipelinePointer->bindVertex(m_pointerGL);
	m_pipelinePointer->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	m_pipelinePointer->setDepthStencilMode();
	m_pipelinePointer->bindShader("object_base_pointer.vert.spv");
	m_pipelinePointer->bindShader("object_base_pointer.geom.spv");
	m_pipelinePointer->bindShader("object_base_pointer.frag.spv");
	m_pipelinePointer->build();
	m_setPointer = context->global->tracker->track(new Set(context->surface, context->setMgr, m_layoutPointer));
	if (m_pColor == nullptr) {
		m_uColor = context->global->tracker->track(new Uniform(context->surface, sizeof(*m_pColor)));
		m_pColor = static_cast<typeof(m_pColor)>(m_uColor->data);
	}
	m_setPointer->bindUniform(m_uColor, 1);
	commandIndexPointer = cmdMgr->getCommandIndex();
	m_globalSet = context->global->globalSet;
}

void ObjectBase::build()
{
	// Ensure this command is not in pending state
	cmdMgrTarget->waitCompletion(0);
	cmdMgrTarget->waitCompletion(1);
	cmdMgrTarget->waitCompletion(2);
	cmdMgr->init(commandIndexPointer, m_pipelinePointer);
	cmdMgr->bindVertex(m_pointerGL);
	cmdMgr->bindSet(m_layoutPointer, m_globalSet);
	cmdMgr->bindSet(m_layoutPointer, m_setPointer, 1);
	cmdMgr->draw(4);
	cmdMgr->compile();
}

void ObjectBase::createShaderStarPointeur(ThreadContext *context)
{
	cmdMgr = context->commandMgr;
	// m_shaderStarPointer = std::make_unique<shaderProgram>();
	// m_shaderStarPointer->init("star_pointer.vert","star_pointer.geom","star_pointer.frag");
	// m_shaderStarPointer->setUniformLocation({"radius","color", "matRotation"});

	m_starPointerGL = context->global->tracker->track(new VertexArray(context->surface));
	m_starPointerGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
	m_starPointerGL->build(1);

	m_layoutStarPointer = context->global->tracker->track(new PipelineLayout(context->surface));
	m_layoutStarPointer->setGlobalPipelineLayout(context->global->globalLayout);
	m_layoutStarPointer->setTextureLocation(0);
	m_layoutStarPointer->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	m_layoutStarPointer->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 2);
	m_layoutStarPointer->buildLayout();
	m_layoutStarPointer->build();
	m_pipelineStarPointer = context->global->tracker->track(new Pipeline(context->surface, m_layoutStarPointer));
	m_pipelineStarPointer->bindVertex(m_starPointerGL);
	m_pipelineStarPointer->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	m_pipelineStarPointer->setDepthStencilMode();
	m_pipelineStarPointer->bindShader("star_pointer.vert.spv");
	m_pipelineStarPointer->bindShader("star_pointer.geom.spv");
	m_pipelineStarPointer->bindShader("star_pointer.frag.spv");
	m_pipelineStarPointer->build();
	m_setStarPointer = context->global->tracker->track(new Set(context->surface, context->setMgr, m_layoutStarPointer));
	if (m_pColor == nullptr) {
		m_uColor = context->global->tracker->track(new Uniform(context->surface, sizeof(*m_pColor)));
		m_pColor = static_cast<typeof(m_pColor)>(m_uColor->data);
	}
	m_setStarPointer->bindTexture(pointer_star->getTexture(), 0);
	m_setStarPointer->bindUniform(m_uColor, 1);
	m_uGeom = context->global->tracker->track(new Uniform(context->surface, sizeof(*m_pGeom)));
	m_pGeom = static_cast<typeof(m_pGeom)>(m_uGeom->data);
	m_setStarPointer->bindUniform(m_uGeom, 2);
	commandIndexStarPointer = cmdMgr->initNew(m_pipelineStarPointer);
	cmdMgr->bindVertex(m_starPointerGL);
	cmdMgr->bindSet(m_layoutStarPointer, context->global->globalSet);
	cmdMgr->bindSet(m_layoutStarPointer, m_setStarPointer, 1);
	cmdMgr->draw(1);
	cmdMgr->compile();
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
		m_starPointerGL->update();

		//m_shaderStarPointer->use();

		// glBindTexture (GL_TEXTURE_2D, pointer_star->getID());
		// StateGL::enable(GL_BLEND);

		//SET UNIFORM
		m_pGeom->matRotation = matRotation;
		m_pGeom->radius = radius;
		*m_pColor = color;
		// m_shaderStarPointer->setUniform("radius", radius);
		// m_shaderStarPointer->setUniform("matRotation", matRotation);
		// m_shaderStarPointer->setUniform("color", color);
		// m_starPointerGL->bind();
		// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_POINT_LIST,0,1);
		// m_starPointerGL->unBind();
		// m_shaderStarPointer->unuse();
		//Renderer::drawArrays(m_shaderStarPointer.get(), m_starPointerGL.get(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST,0,1);
		cmdMgr->setSubmission(commandIndexStarPointer);
	}

	if (getType()==OBJECT_NEBULA || getType()==OBJECT_BODY) {
		//glActiveTexture(GL_TEXTURE0);
		if (getType() != lastType) {
			lastType = getType();
			if (lastType==OBJECT_BODY)
				m_setPointer->bindTexture(pointer_planet->getTexture(), 0);
			if (lastType==OBJECT_NEBULA)
				m_setPointer->bindTexture(pointer_nebula->getTexture(), 0);
			build();
		}
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
		m_pointerGL->update();

		// StateGL::enable(GL_BLEND);

		// m_shaderPointer->use();
		// m_shaderPointer->setUniform("color", color);
		*m_pColor = color;
		// m_pointerGL->bind();
		// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_POINT_LIST,0,4);
		// m_pointerGL->unBind();
		// m_shaderPointer->unuse();
		//Renderer::drawArrays(m_shaderPointer.get(), m_pointerGL.get(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST,0,4);
		cmdMgr->setSubmission(commandIndexPointer, false, cmdMgrTarget);

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
