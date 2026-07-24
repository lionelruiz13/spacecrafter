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

// THE single seed authority (I2, B5-oort-2 [vixy 2026-07-24]). Any fixed 32-bit
// value works; this one traces to the directive date. Making it a compile-time
// constant (not a runtime choice) is why there is no acting default to log under
// D12: both paths draw the identical cloud unconditionally, every launch.
static constexpr std::mt19937::result_type OORT_SEED = 20260724u;

std::mt19937 oortRng() noexcept
{
	return std::mt19937(OORT_SEED);
}

// Single authority for the oort cloud's spatial law (I2, B5 §6.9): the exact
// per-point formula the old populate loop below used, extracted so BOTH paths
// draw the SAME distribution without duplicating it. The distribution SHAPE is
// verbatim from the historical loop; only the source of the three random draws
// changed from the global rand() stream to the caller's dedicated frozen-seed
// generator (B5-oort-2 [vixy 2026-07-24]) - so a caller that seeds from oortRng()
// before the loop produces a cloud POINT-identical to the other path's, immune
// to any interleaved rand() consumption. The pre-2026-07-24 cloud used
// rand()%N; mt19937()%N keeps the same [0,N) uniform range, different values.
Vec3f oortSamplePoint(std::mt19937 &rng) noexcept
{
	float radius, theta, phi, r_theta, r_phi;
	Vec3f tmp;
	r_theta = (float) (rng()%3600);
	r_phi = (float) (rng()%1400);
	theta = r_theta /10.;
	phi   = -70. + r_phi /10.;
	if (abs(phi)>60) phi = phi*(1+(abs(phi)-60)/35);
	radius = 60. + (float) (rng()%5000);
	if (radius<2570) phi = phi*(radius-0)/2570;
	if (radius>4000) radius = radius*(1+(radius-4000)/4000);
	Utility::spheToRect(theta*M_PI/180,phi*M_PI/180, tmp);
	return tmp * radius;
}

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
	// Set specialization constant for projection type (constant_id = 8)
	pipeline->setSpecializedConstant(8, Context::projectionType);
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
	vertex = m_dataGL->createBuffer(0, nbr, Context::instance->globalBuffer.get());
	Vec3f *dataOort = (Vec3f *) Context::instance->transfer->planCopy(vertex->get());
	// Shared spatial law (I2, B5 §6.9 - oortSamplePoint above) drawn from a
	// dedicated frozen-seed generator (B5-oort-2): this cloud is POINT-identical
	// to the new-path OortModule's, which seeds its own generator from the SAME
	// constant. The old render path's gates/intensity/draw-order are untouched;
	// only the cloud's point positions changed vs the pre-2026-07-24 global-rand
	// cloud (authorized by the directive).
	std::mt19937 rng = oortRng();
	for(unsigned int i=0; i<nbr ; i++) {
		*(dataOort++) = oortSamplePoint(rng);
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
	// management of intensity
	if ((distance < 1e13) || (distance > 1.E16))
		return;

	intensity = (distance > 1.E14) ? (1.-0.01*(distance/1.E14)) : std::min(1.0, (distance/1.E13 - 1));
	//~ printf("distance : %f\n", distance);
	//~ printf("intensity : %f\n", intensity);

	*uMat = nav->getHelioToEyeMat().convert();
	uFrag->get().fader = intensity*fader.getInterstate();

	Context::instance->frame[Context::instance->frameIdx]->toExecute(cmds[Context::instance->frameIdx], PASS_BACKGROUND);
}
