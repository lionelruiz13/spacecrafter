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

// #include "renderGL/OpenGL.hpp"
// #include "renderGL/shader.hpp"
// #include "renderGL/Renderer.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/VertexArray.hpp"
#include "vulkanModule/Buffer.hpp"

#define MAX_METEOR 4096

MeteorMgr::MeteorMgr(int zhr, int maxv, ThreadContext *context)
{
	ZHR = zhr;
	max_velocity = maxv;

	// calculate factor for meteor creation rate per second since visible area ZHR is for estimated visible radius of 458km
	// (calculated for average meteor magnitude of +2.5 and limiting magnitude of 5)
	zhr_to_wsr = 1.6667f/3600.f;
	// this is a correction factor to adjust for the model as programmed to match observed rates
	createSC_context(context);
}

MeteorMgr::~MeteorMgr()
{}


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

void MeteorMgr::createSC_context(ThreadContext *context)
{
	cmdMgr = context->commandMgr;
	// m_shaderMeteor = std::make_unique<shaderProgram>();
	// m_shaderMeteor->init("meteor.vert","meteor.frag");

 	m_meteorGL = std::make_unique<VertexArray>(context->surface);
	m_meteorGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_meteorGL->registerVertexBuffer(BufferType::COLOR4, BufferAccess::DYNAMIC);
	m_meteorGL->build(MAX_METEOR * 3);
	m_meteorGL->registerIndexBuffer(BufferAccess::STATIC, MAX_METEOR * 4, 2, VK_INDEX_TYPE_UINT16);
	{ // initialize index buffer
		std::vector<uint16_t> tmpIndex;
		tmpIndex.reserve(MAX_METEOR * 4);
		for (int i = 0; i < MAX_METEOR * 3; i += 3) {
			tmpIndex.push_back(i + 0);
			tmpIndex.push_back(i + 1);
			tmpIndex.push_back(i + 1);
			tmpIndex.push_back(i + 2);
		}
		m_meteorGL->fillIndexBuffer(MAX_METEOR * 2, reinterpret_cast<uint32_t *>(tmpIndex.data()));
	}
	pipeline = std::make_unique<Pipeline>(context->surface, context->global->globalLayout);
	pipeline->setDepthStencilMode();
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	pipeline->bindVertex(m_meteorGL.get());
	pipeline->bindShader("meteor.vert.spv");
	pipeline->bindShader("meteor.frag.spv");
	pipeline->build();
	drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	pNbVertex = static_cast<typeof(pNbVertex)>(drawData->data);
	pNbVertex[1] = 1; // instanceCount
	pNbVertex[2] = pNbVertex[3] = pNbVertex[4] = 0; // offsets
	commandIndex = cmdMgr->initNew(pipeline.get());
	cmdMgr->bindVertex(m_meteorGL.get());
	cmdMgr->bindSet(context->global->globalLayout, context->global->globalSet);
	cmdMgr->indirectDrawIndexed(drawData.get());
	cmdMgr->compile();
}

void MeteorMgr::draw(Projector *proj, Navigator* nav)
{
	for (auto& iter : m_activeMeteor) {
    	iter->draw(proj, nav, vecPos, vecColor);
	}

	if (vecPos.size()==0)
		return;

	m_meteorGL->fillVertexBuffer(BufferType::POS2D, vecPos);
	m_meteorGL->fillVertexBuffer(BufferType::COLOR4, vecColor);
	m_meteorGL->update();

	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// StateGL::enable(GL_BLEND);

	// m_shaderMeteor->use();
	// m_meteorGL->bind();
	// for(unsigned int i=0; i < (vecPos.size()/3) ; i++)
	// 	glDrawArrays(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 3*i, 3);
	// m_meteorGL->unBind();
	// m_shaderMeteor->unuse();
	*pNbVertex = vecPos.size() / 2 * 4/3; // index per meteor / vertex per meteor
	drawData->update();
	cmdMgr->setSubmission(commandIndex);
	//Renderer::drawMultiArrays(m_shaderMeteor.get(), m_meteorGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, vecPos.size()/3 , 3);

	vecPos.clear();
	vecColor.clear();
}
