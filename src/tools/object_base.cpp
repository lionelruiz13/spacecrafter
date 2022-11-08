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
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

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
float ObjectBase::m_fontResolution = 1.;

VkCommandBuffer ObjectBase::cmdPointer[6];
VkCommandBuffer ObjectBase::cmdStarPointer[3];
VertexArray *ObjectBase::pointerGL, *ObjectBase::starPointerGL;
VertexBuffer *ObjectBase::vertexPointer, *ObjectBase::vertexStarPointer;
Pipeline *ObjectBase::pipelinePointer, *ObjectBase::pipelineStarPointer;
PipelineLayout *ObjectBase::layoutPointer, *ObjectBase::layoutStarPointer;
Set *ObjectBase::setPlanetPointer, *ObjectBase::setNebulaPointer, *ObjectBase::setStarPointer;
SharedBuffer<Vec3f> *ObjectBase::uColor;
SharedBuffer<ObjectBase::objBaseGeom> *ObjectBase::uGeom;
OBJECT_TYPE ObjectBase::lastType = OBJECT_UNINITIALIZED;


void ObjectBase::createShaderPointeur()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	pointerGL = new VertexArray(vkmgr);
	pointerGL->createBindingEntry(3 * sizeof(float));
	pointerGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	pointerGL->addInput(VK_FORMAT_R32_SFLOAT);
	vertexPointer = pointerGL->newBuffer(0, 4, context.globalBuffer.get());

	layoutPointer = new PipelineLayout(vkmgr);
	layoutPointer->setGlobalPipelineLayout(context.layouts.front().get());
	layoutPointer->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layoutPointer->buildLayout();
	layoutPointer->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec3f));
	layoutPointer->build();
	pipelinePointer = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutPointer);
	pipelinePointer->bindVertex(*pointerGL);
	pipelinePointer->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipelinePointer->setDepthStencilMode();
	pipelinePointer->bindShader("object_base_pointer.vert.spv");
	pipelinePointer->bindShader("object_base_pointer.geom.spv");
	pipelinePointer->bindShader("object_base_pointer.frag.spv");
	pipelinePointer->build();
	setPlanetPointer = new Set(vkmgr, *context.setMgr, layoutPointer);
	setPlanetPointer->bindTexture(pointer_planet->getTexture(), 0);
	setNebulaPointer = new Set(vkmgr, *context.setMgr, layoutPointer);
	setNebulaPointer->bindTexture(pointer_nebula->getTexture(), 0);
	const Vec3f bodyColor(1.0f,0.3f,0.3f);
	const Vec3f nebulaColor(0.4f,0.5f,0.8f);
	context.cmdInfo.commandBufferCount = 6;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmdPointer);
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmdPointer[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipelinePointer->bind(cmd);
		layoutPointer->bindSets(cmd, {*context.uboSet, *setPlanetPointer});
		vertexPointer->bind(cmd);
		layoutPointer->pushConstant(cmd, 0, &bodyColor);
		vkCmdDraw(cmd, 4, 1, 0, 0);
		context.frame[i]->compile(cmd);
        context.frame[i]->setName(cmd, "Pointer " + std::to_string(i));
	}
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmdPointer[i + 3];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipelinePointer->bind(cmd);
		layoutPointer->bindSets(cmd, {*context.uboSet, *setNebulaPointer});
		vertexPointer->bind(cmd);
		layoutPointer->pushConstant(cmd, 0, &nebulaColor);
		vkCmdDraw(cmd, 4, 1, 0, 0);
		context.frame[i]->compile(cmd);
		context.frame[i]->setName(cmd, "Pointer " + std::to_string(i));
	}
}

void ObjectBase::createShaderStarPointeur()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	starPointerGL = new VertexArray(vkmgr);
	starPointerGL->createBindingEntry(3 * sizeof(float));
	starPointerGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexStarPointer = starPointerGL->newBuffer(0, 1, context.globalBuffer.get());

	layoutStarPointer = new PipelineLayout(vkmgr);
	layoutStarPointer->setGlobalPipelineLayout(context.layouts.front().get());
	layoutStarPointer->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layoutStarPointer->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layoutStarPointer->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 2);
	layoutStarPointer->buildLayout();
	layoutStarPointer->build();
	pipelineStarPointer = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutStarPointer);
	pipelineStarPointer->bindVertex(*starPointerGL);
	pipelineStarPointer->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipelineStarPointer->setDepthStencilMode();
	pipelineStarPointer->bindShader("star_pointer.vert.spv");
	pipelineStarPointer->bindShader("star_pointer.geom.spv");
	pipelineStarPointer->bindShader("star_pointer.frag.spv");
	pipelineStarPointer->build();
	setStarPointer = new Set(vkmgr, *context.setMgr, layoutStarPointer);
	uColor = new SharedBuffer<Vec3f>(*context.uniformMgr);
	uGeom = new SharedBuffer<objBaseGeom>(*context.uniformMgr);
	setStarPointer->bindTexture(pointer_star->getTexture(), 0);
	setStarPointer->bindUniform(*uColor, 1);
	setStarPointer->bindUniform(*uGeom, 2);

	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmdStarPointer);
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmdStarPointer[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipelineStarPointer->bind(cmd);
		layoutStarPointer->bindSets(cmd, {*context.uboSet, *setStarPointer});
		vertexStarPointer->bind(cmd);
		vkCmdDraw(cmd, 1, 1, 0, 0);
		context.frame[i]->compile(cmd);
	}
}

void ObjectBase::uninit()
{
	delete pointerGL;
	delete starPointerGL;
	delete vertexPointer;
	delete vertexStarPointer;
	delete pipelinePointer;
	delete pipelineStarPointer;
	delete layoutPointer;
	delete layoutStarPointer;
	delete setPlanetPointer;
	delete setNebulaPointer;
	delete setStarPointer;
	delete uColor;
	delete uGeom;
}

void ObjectBase::setFontResolution(int fontResolution)
{
	m_fontResolution = fontResolution;
}

// Draw a nice animated pointer around the object
void ObjectBase::drawPointer(int delta_time, const Projector* prj, const Navigator * nav)
{
	Vec3d screenposd;
	local_time += delta_time;
	// Compute 2D pos and return if outside screen
	if (!prj->projectEarthEqu(getEarthEquPos(nav), screenposd))
		return;

	Context &context = *Context::instance;
	switch (getType()) {
		case OBJECT_STAR: {
			*(Vec3f *) context.transfer->planCopy(vertexStarPointer->get(), 0, sizeof(Vec3f)) = Vec3f(screenposd[0], screenposd[1], screenposd[2]);

			//SET UNIFORM
			uGeom->get().matRotation = Mat4f::zrotation((float)local_time/750.);
			// factor for resolution size
			float factor = prj->getViewportHeight() / m_fontResolution;
			uGeom->get().radius = 13.f * factor;
			*uColor = getRGB();

			context.frame[context.frameIdx]->toExecute(cmdStarPointer[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
			break;
		}
		case OBJECT_BODY:
		case OBJECT_NEBULA: {
			float size = getOnScreenSize(prj, nav, false);
			// If object is large enough, no need for distracting pointer
			if (size > prj->getViewportRadius()*.1f)
				return;
			size += 20.f + 10.f*sin(0.002f * local_time);
			float factor = prj->getViewportHeight() / m_fontResolution;
			size *= factor;
			Vec2f screenpos(screenposd[0], screenposd[1]);
			float *data = (float *) context.transfer->planCopy(vertexPointer->get());
			*(data++) = (screenpos[0] -size/2);
			*(data++) = (screenpos[1] +size/2);
			*(data++) = (1.0);

			*(data++) = (screenpos[0] +size/2);
			*(data++) = (screenpos[1] +size/2);
			*(data++) = (2.0);

			*(data++) = (screenpos[0] +size/2);
			*(data++) = (screenpos[1] -size/2);
			*(data++) = (3.0);

			*(data++) = (screenpos[0] -size/2);
			*(data++) = (screenpos[1] -size/2);
			*(data++) = (4.0);

			context.frame[context.frameIdx]->toExecute(cmdPointer[
				(getType() == OBJECT_BODY) ? (context.frameIdx) : (context.frameIdx + 3)], PASS_MULTISAMPLE_DEPTH);
			break;
		}
		case OBJECT_STAR_CLUSTER: {
			*(Vec3f *) context.transfer->planCopy(vertexStarPointer->get(), 0, sizeof(Vec3f)) = Vec3f(screenposd[0], screenposd[1], screenposd[2]);

			//SET UNIFORM
			uGeom->get().matRotation = Mat4f::zrotation((float)local_time/750.);
			float factor = prj->getViewportHeight() / m_fontResolution;
			uGeom->get().radius = 13.f * factor;
			*uColor = getRGB();

			context.frame[context.frameIdx]->toExecute(cmdStarPointer[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
			break;
		}
		default:;
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
