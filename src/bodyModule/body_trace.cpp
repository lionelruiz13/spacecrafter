/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2020 of the LSS Team & Association Sirius
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

#include <string>

#include "bodyModule/body_trace.hpp"
#include "tools/utility.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "vulkanModule/VertexArray.hpp"


#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/VertexArray.hpp"
#include "vulkanModule/VertexBuffer.hpp"
#include "vulkanModule/Buffer.hpp"
#include "vulkanModule/Buffer.hpp"


BodyTrace::BodyTrace(ThreadContext *context)
{
	for(int i= 0; i<NB_MAX_LIST; i++) {
		bodyData[i].size=0;
		bodyData[i].old_punt[0]=0.0;
		bodyData[i].old_punt[1]=0.0;
		bodyData[i].hide=false;
	}
	createSC_context(context);
	*bodyData[0].color= Vec3f(1.0,0.0,0.0);
	*bodyData[1].color= Vec3f(0.2f, 0.2f, 0.7f);
	*bodyData[2].color= Vec3f(0.5f, 0.9f, 0.4f);
	*bodyData[3].color= Vec3f(0.3f, 0.6f, 0.1f);
	*bodyData[4].color= Vec3f(0.1f, 0.8f, 0.5f);
	*bodyData[5].color= Vec3f(0.f, 1.0f, 0.2f);
	*bodyData[6].color= Vec3f(0.2f, 1.0f, 0.4f);
	//*bodyData[7].color= Vec3f(0.4f, 1.0f, 0.6f); // OUT OF INDEX !
	is_tracing=true;
	currentUsedList=0;
}

BodyTrace::~BodyTrace()
{}

void BodyTrace::hide(int numberList)
{
	if ( numberList > (NB_MAX_LIST-1)) return;
	if (numberList==-1) {
		//-1 mean all the lists
		for(int i=0; i<NB_MAX_LIST; i++)
			bodyData[i].hide= !bodyData[i].hide;
	}
	else
		bodyData[numberList].hide= !bodyData[numberList].hide;
}

void BodyTrace::createSC_context(ThreadContext *context)
{
	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->buildLayout();
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->build();

	drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand) * NB_MAX_LIST, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	VkDrawIndirectCommand *tmp = static_cast<VkDrawIndirectCommand *>(drawData->data);

	VertexArray tmp2(context->surface, context->commandMgr);
	tmp2.registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);

	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipeline->setDepthStencilMode(VK_TRUE, VK_TRUE);
	pipeline->bindVertex(&tmp2);
	pipeline->bindShader("body_trace.vert.spv");
	pipeline->bindShader("body_trace.geom.spv");
	pipeline->bindShader("body_trace.frag.spv");
	pipeline->build();

	cmdMgr = context->commandMgr;

	commandIndex = cmdMgr->getCommandIndex();
	cmdMgr->init(commandIndex);
	cmdMgr->beginRenderPass(renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
	cmdMgr->bindPipeline(pipeline.get());
	cmdMgr->bindSet(layout.get(), context->global->globalSet, 1);

	for(int i= 0; i<NB_MAX_LIST; i++) {
		bodyData[i].uColor = std::make_unique<Uniform>(context->surface, sizeof(Vec3i));
		bodyData[i].color = static_cast<Vec3f *>(bodyData[i].uColor->data);
		bodyData[i].uMat = std::make_unique<Uniform>(context->surface, sizeof(Mat4f));
		bodyData[i].mat = static_cast<Mat4f *>(bodyData[i].uMat->data);
		bodyData[i].vertex = std::make_unique<VertexArray>(tmp2);
		bodyData[i].vertex->build(MAX_POINTS);
		bodyData[i].punts = static_cast<Vec3f *>(bodyData[i].vertex->getVertexBuffer().data);
		bodyData[i].drawData = tmp++;
		bodyData[i].drawData->vertexCount = 0;
		bodyData[i].drawData->instanceCount = 1;
		bodyData[i].drawData->firstVertex = 0;
		bodyData[i].drawData->firstInstance = 0;
		bodyData[i].set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
		bodyData[i].set->bindUniform(bodyData[i].uMat.get(), 0);
		bodyData[i].set->bindUniform(bodyData[i].uColor.get(), 1);
		cmdMgr->bindSet(layout.get(), bodyData[i].set.get());
		bodyData[i].vertex->bind();
		cmdMgr->indirectDraw(drawData.get(), i * sizeof(VkDrawIndirectCommand), 1);
	}

	cmdMgr->compile();
}

void BodyTrace::draw(const Projector *prj,const Navigator *nav)
{
	if (!fader.getInterstate()) return;

	for(int l=0; l<currentUsedList+1; l++) {
		if (bodyData[l].size>2 && !bodyData[l].hide) {
			//tracé en direct de la courbe de ci dessus
			*bodyData[l].mat = prj->getMatLocalToEye();
			bodyData[l].drawData->vertexCount = bodyData[l].size;
			bodyData[l].vertex->update();
		} else {
			bodyData[l].drawData->vertexCount = 0;
		}
	}
	drawData->update();
	cmdMgr->setSubmission(commandIndex, true);
}

void BodyTrace::addData(const Navigator *nav, double alt, double az)
{
	if (!fader.getInterstate()) return;
	if(!is_tracing) return;

	if (bodyData[currentUsedList].size==0) {
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		Utility::spheToRect(-(az+M_PI),alt,bodyData[currentUsedList].punts[0]);
		bodyData[currentUsedList].size=1;
		bodyData[currentUsedList].vertex->assumeVerticeChanged();
		bodyData[currentUsedList].drawData->vertexCount = bodyData[currentUsedList].size;
	}
	else {
		if ( (abs(alt-bodyData[currentUsedList].old_punt[0])< 0.001) && (abs(az-bodyData[currentUsedList].old_punt[1])< 0.001) ) return;
		if (bodyData[currentUsedList].size==(MAX_POINTS-1)) return;
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		bodyData[currentUsedList].size+=1;
		Utility::spheToRect(-(az+M_PI),alt,bodyData[currentUsedList].punts[bodyData[currentUsedList].size]);
		bodyData[currentUsedList].vertex->assumeVerticeChanged();
		bodyData[currentUsedList].drawData->vertexCount = bodyData[currentUsedList].size;
	}
}
