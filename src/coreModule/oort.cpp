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
#include "navModule/navigator.hpp"

#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

#define NB_POINTS 200000

Oort::Oort()
{
	fader = false;
	createSC_context();
	uFrag->get().color = Vec3f(1.0,1.0,0.0);
}

Oort::~Oort()
{}

void Oort::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	m_dataGL = std::make_unique<VertexArray>(vkmgr);
	m_dataGL->createBindingEntry(3*sizeof(float));
	m_dataGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_BACKGROUND, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipeline->setDepthStencilMode();
	pipeline->bindVertex(*m_dataGL);
	pipeline->bindShader("oort.vert.spv");
	pipeline->bindShader("oort.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	uMat = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
	set->bindUniform(uMat, 0);
	uFrag = std::make_unique<SharedBuffer<frag>>(*context.uniformMgr);
	set->bindUniform(uFrag, 1);
}

void Oort::populate(unsigned int nbr) noexcept
{
	float radius, theta, phi, r_theta, r_phi;
	Vec3f tmp;
	vertex = m_dataGL->createBuffer(0, nbr, Context::instance->globalBuffer.get());
	Vec3f *dataOort = (Vec3f *) Context::instance->transfer->planCopy(vertex->get());
	for(unsigned int i=0; i<nbr ; i++) {
		r_theta = (float) (rand()%3600);
		r_phi = (float) (rand()%1400);
		theta = r_theta /10.;
		phi   = -70. + r_phi /10.;
		if (abs(phi)>60) phi = phi*(1+(abs(phi)-60)/35);
		radius = 60. + (float) (rand()%5000);
		if (radius<2570) phi = phi*(radius-0)/2570;
		if (radius>4000) radius = radius*(1+(radius-4000)/4000);

		Utility::spheToRect(theta*M_PI/180,phi*M_PI/180, tmp);
		*(dataOort++) = tmp * radius;
	}
	nbAsteroids = nbr;
}

void Oort::build()
{
	Context &context = *Context::instance;

	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		VkCommandBuffer cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_BACKGROUND);
		pipeline->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		vertex->bind(cmd);
		vkCmdDraw(cmd, nbAsteroids, 1, 0, 0);
		context.frame[i]->compile(cmd);
		context.frame[i]->setName(cmd, "Oort " + std::to_string(i));
	}
}

void Oort::draw(double distance, const Navigator *nav) noexcept
{
	if (!fader.getInterstate()) return;

	distance = abs(distance);
	// gestion de l'intensité
	if ((distance < 1e13) || (distance > 5.E15))
		return;

	intensity = (distance > 1.E15) ? (1.25-0.25*(distance/1.E15)) : std::min(1.0, (distance/1.e13 - 1));
	//~ printf("distance : %f\n", distance);
	//~ printf("intensity : %f\n", intensity);

	*uMat = nav->getHelioToEyeMat().convert();
	uFrag->get().fader = intensity*fader.getInterstate();

	Context::instance->frame[Context::instance->frameIdx]->toExecute(cmds[Context::instance->frameIdx], PASS_BACKGROUND);
}
