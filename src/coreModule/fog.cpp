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

#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

s_texture* Fog::fog_tex;
VertexArray *Fog::vertexModel;
PipelineLayout *Fog::layout;
Pipeline *Fog::pipeline;
int Fog::vUniformID1;
int Fog::vUniformID2;
Set *Fog::set;

Fog::Fog(float _radius) : radius(_radius)
{
	uMV = std::make_unique<SharedBuffer<Mat4f>>(*Context::instance->uniformMgr);
	uFrag = std::make_unique<SharedBuffer<frag>>(*Context::instance->uniformMgr);
}

Fog::~Fog()
{}

void Fog::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	vertexModel = new VertexArray(vkmgr);
	vertexModel->createBindingEntry(5*sizeof(float));
	vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexModel->addInput(VK_FORMAT_R32G32_SFLOAT);

	layout = new PipelineLayout(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 1, 1, true);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1, true);
	layout->buildLayout();
	layout->build();
	pipeline = new Pipeline(vkmgr, *context.render, PASS_FOREGROUND, layout);
	pipeline->setDepthStencilMode();
	pipeline->setFrontFace();
	pipeline->setCullMode(true);
	pipeline->setBlendMode(BLEND_ADD);
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	pipeline->bindVertex(*vertexModel);
	pipeline->bindShader("fog.vert.spv");
	pipeline->bindShader("fog.frag.spv");
	pipeline->build();
	set = new Set(vkmgr, *context.setMgr, layout);
	fog_tex = new s_texture("fog.png",TEX_LOAD_TYPE_PNG_SOLID_REPEAT,false);
	set->bindTexture(fog_tex->getTexture(), 0);
	vUniformID1 = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 1, 16);
	vUniformID2 = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 2, 2);
}

void Fog::destroySC_context()
{
	delete vertexModel;
	delete pipeline;
	delete layout;
	delete set;
	delete fog_tex;
}

void Fog::initShader()
{
	Context &context = *Context::instance;
	const int slices = 128;

	nbVertex = (slices + 1) * 2;
	vertex = vertexModel->createBuffer(0, nbVertex, context.globalBuffer.get());
	createFogMesh(radius, radius*sinf(alt_angle*M_PI/180.), slices, 1, (float *) context.transfer->planCopy(vertex->get()));

	set->setVirtualUniform(uMV->getOffset(), vUniformID1);
	set->setVirtualUniform(uFrag->getOffset(), vUniformID2);
	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_FOREGROUND);
		pipeline->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set}, *set);
		vertex->bind(cmd);
		vkCmdDraw(cmd, nbVertex, 1, 0, 0);
		context.frame[i]->compile(cmd);
	}
}

// Draw the horizon fog
void Fog::draw(const Projector* prj, const Navigator* nav)
{
	if (!fader.getInterstate()) return;

	uFrag->get().fader = fader.getInterstate();
	uFrag->get().sky_brightness = sky_brightness;
	*uMV = (nav->getLocalToEyeMat() * Mat4d::translation(Vec3d(0.,0.,radius*sinf(angle_shift*M_PI/180.)))).convert();

	const int frameIdx = Context::instance->frameIdx;
	Context::instance->frame[frameIdx]->toExecute(cmds[frameIdx], PASS_FOREGROUND);
}

void Fog::createFogMesh(double radius, double height, int slices, int stacks, float *data)
{
	double da, r, dz;
	float z ;
	int i;

	da = 2.0 * M_PI / slices;
	dz = height / stacks;

	float ds = 1.0 / slices;
	float dt = 1.0 / stacks;
	float t = 0.0;
	z = 0.0;
	r = radius;
	float s = 0.0;
	for (i = 0; i <= slices; i++) {
		float x, y;
		if (i == slices) {
			x = sinf(0.0);
			y = cosf(0.0);
		}
		else {
			x = sinf(i * da);
			y = cosf(i * da);
		}
		*(data++) = x*r;
		*(data++) = y*r;
		*(data++) = z;
		*(data++) = s;
		*(data++) = t;

		*(data++) = x*r;
		*(data++) = y*r;
		*(data++) = z+dz;
		*(data++) = s;
		*(data++) = t+dt;
		s += ds;
	}
}
