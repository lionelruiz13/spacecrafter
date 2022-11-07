/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2004 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2018-2020 AssociationSirius
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

#include "coreModule/meteor_mgr.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

#define MAX_METEOR 4096

MeteorMgr::MeteorMgr(int zhr, int maxv)
{
	ZHR = zhr;
	max_velocity = maxv;

	// calculate factor for meteor creation rate per second since visible area ZHR is for estimated visible radius of 458km
	// (calculated for average meteor magnitude of +2.5 and limiting magnitude of 5)
	zhr_to_wsr = 1.6667f/3600.f;
	// this is a correction factor to adjust for the model as programmed to match observed rates
	createSC_context();
}

MeteorMgr::~MeteorMgr()
{
	Context::instance->indexBufferMgr->releaseBuffer(index);
}

void MeteorMgr::update(Projector *proj, Navigator* nav, TimeMgr* timeMgr, ToneReproductor* eye, int delta_time)
{
	// step through and update all active meteors and delete all inactive meteors too
	for (auto iter = m_activeMeteor.begin(); iter != m_activeMeteor.end(); ++iter) {
		if ( !( (*iter)->update(delta_time) ) ) {
			//printf("Meteor \tdied\n");
			iter=m_activeMeteor.erase(iter);
		}
	}

	// only makes sense given lifetimes of meteors to draw when time_speed is realtime
	// otherwise high overhead of large numbers of meteors
	double tspeed = timeMgr->getTimeSpeed() *86400;  // sky seconds per actual second
	if (tspeed <= 0 || fabs(tspeed) > 1 ) {
		// don't start any more meteors
		return;
	}

	// if application has been suspended, don't create huge number of meteors to make up for lost time!
	if ( delta_time > 500 ) {
		delta_time = 500;
	}

	// determine average meteors per frame needing to be created
	int mpf = (int)((double)ZHR*zhr_to_wsr*(double)delta_time/1000.0f + 0.5);
	if ( mpf < 1 ) mpf = 1;

	int mlaunch = 0;
	for (int i=0; i<mpf; i++) {

		// start new meteor based on ZHR time probability
		double prob = (double)rand()/((double)RAND_MAX+1);
		if ( ZHR > 0 && prob < ((double)ZHR*zhr_to_wsr*(double)delta_time/1000.0f/(double)mpf) ) {
			auto m = std::make_unique<Meteor>(proj, nav, eye, max_velocity);
			m_activeMeteor.push_back(std::move(m));
			mlaunch++;
		}
	}
	//  printf("mpf: %d\tm launched: %d\t(mps: %f)\t%d\n", mpf, mlaunch, ZHR*zhr_to_wsr, delta_time);
}

void MeteorMgr::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

 	m_meteorGL = std::make_unique<VertexArray>(vkmgr);
	m_meteorGL->createBindingEntry(6*sizeof(float));
	m_meteorGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	m_meteorGL->addInput(VK_FORMAT_R32G32B32A32_SFLOAT);
	vertex = m_meteorGL->createBuffer(0, MAX_METEOR * 3, context.globalBuffer.get());
	index = context.indexBufferMgr->acquireBuffer(MAX_METEOR * 4 * sizeof(uint16_t));
	uint16_t *tmpIndex = (uint16_t *) context.transfer->planCopy(index);
	for (int i = 0; i < MAX_METEOR * 3; i += 3) {
		*(tmpIndex++) = i + 0;
		*(tmpIndex++) = i + 1;
		*(tmpIndex++) = i + 1;
		*(tmpIndex++) = i + 2;
	}
	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_FRONT, context.layouts.front().get());
	pipeline->setDepthStencilMode();
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	pipeline->bindVertex(*m_meteorGL);
	pipeline->bindShader("meteor.vert.spv");
	pipeline->bindShader("meteor.frag.spv");
	pipeline->build();

	drawData = std::make_unique<SharedBuffer<VkDrawIndexedIndirectCommand[3]>>(*context.tinyMgr);
	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		VkCommandBuffer &cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_FRONT);
		pipeline->bind(cmd);
		context.layouts.front()->bindSets(cmd, {*context.uboSet});
		vertex->bind(cmd);
		vkCmdBindIndexBuffer(cmd, index.buffer, index.offset, VK_INDEX_TYPE_UINT16);
		// vkCmdDrawIndexed(cmd, nbNebulae, 1, 0, 0);
		vkCmdDrawIndexedIndirect(cmd, drawData->getBuffer().buffer, drawData->getOffset() + i * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
		context.frame[i]->compile(cmd);
		drawData->get()[i] = VkDrawIndexedIndirectCommand{0, 1, 0, 0, 0};
		pNbVertex[i] = &drawData->get()[i].indexCount;
	}
}

void MeteorMgr::draw(Projector *proj, Navigator* nav)
{
	Context &context = *Context::instance;

	int nbMeteor = 0;
	float *data = (float *) context.transfer->beginPlanCopy(MAX_METEOR * 3 * (6 * sizeof(float)));
	for (auto& iter : m_activeMeteor) {
    	if (iter->draw(proj, nav, data))
			++nbMeteor;
	}
	if (nbMeteor > MAX_METEOR)
		nbMeteor = MAX_METEOR;

	context.transfer->endPlanCopy(vertex->get(), nbMeteor * 3 * (6 * sizeof(float)));
	if (nbMeteor) {
		*pNbVertex[context.frameIdx] = nbMeteor * 4;
		context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_FRONT);
	}
}

void MeteorMgr::createRadiant(int day, const Vec3f newRadiant)
{
	for (auto& iter : m_activeMeteor) {
    	iter->createRadiant(day, newRadiant);
	}
}

void MeteorMgr::clearRadiants()
{
	for (auto& iter : m_activeMeteor) {
    	iter->clear();
	}
}
