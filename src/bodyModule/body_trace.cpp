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

#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

BodyTrace::BodyTrace()
{
	for(int i= 0; i<NB_MAX_LIST; i++) {
		bodyData[i].size=0;
		bodyData[i].old_punt[0]=0.0;
		bodyData[i].old_punt[1]=0.0;
		bodyData[i].hide=false;
	}
	bodyData[0].color= Vec3f(1.0,0.0,0.0);
	bodyData[1].color= Vec3f(0.2f, 0.2f, 0.7f);
	bodyData[2].color= Vec3f(0.5f, 0.9f, 0.4f);
	bodyData[3].color= Vec3f(0.3f, 0.6f, 0.1f);
	bodyData[4].color= Vec3f(0.1f, 0.8f, 0.5f);
	bodyData[5].color= Vec3f(0.f, 1.0f, 0.2f);
	bodyData[6].color= Vec3f(0.2f, 1.0f, 0.4f);
	//*bodyData[7].color= Vec3f(0.4f, 1.0f, 0.6f); // OUT OF INDEX !
	createSC_context();
	is_tracing=true;
	currentUsedList=0;
}

BodyTrace::~BodyTrace()
{}

void BodyTrace::hide(int numberList)
{
	if (numberList >= NB_MAX_LIST) return;
	if (numberList==-1) {
		//-1 mean all the lists
		for(int i=0; i<NB_MAX_LIST; i++)
			bodyData[i].hide= !bodyData[i].hide;
	}
	else
		bodyData[numberList].hide= !bodyData[numberList].hide;
}

void BodyTrace::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setPushConstant(VK_SHADER_STAGE_GEOMETRY_BIT, 0, sizeof(Mat4f));
	layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Mat4f), sizeof(Vec3f));
	layout->build();

	pattern = std::make_unique<VertexArray>(vkmgr, sizeof(Vec3f));
	pattern->createBindingEntry(sizeof(Vec3f));
	pattern->addInput(VK_FORMAT_R32G32B32_SFLOAT);

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipeline->setDepthStencilMode(VK_TRUE, VK_TRUE);
	pipeline->bindVertex(*pattern);
	pipeline->bindShader("body_trace.vert.spv");
	pipeline->bindShader("body_trace.geom.spv");
	pipeline->bindShader("body_trace.frag.spv");
	pipeline->build();

	vertexBufferMgr = std::make_unique<BufferMgr>(vkmgr, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, sizeof(Vec3f) * MAX_POINTS * NB_MAX_LIST);
	vertexBufferMgr->setName("BodyTrace VertexBuffers");

	for(int i= 0; i<NB_MAX_LIST; i++) {
		bodyData[i].vertex = pattern->createBuffer(0, MAX_POINTS, vertexBufferMgr.get());
	}

	for (int i = 0; i < 3; ++i) {
		cmds[i] = context.frame[i]->create(1);
		context.frame[i]->setName(cmds[i], "Body Trace " + std::to_string(i));
	}
}

void BodyTrace::draw(const Projector *prj,const Navigator *nav)
{
	if (!fader.getInterstate()) return;

	Context &context = *Context::instance;
	FrameMgr &frame = *context.frame[context.frameIdx];
	VkCommandBuffer &cmd = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
	pipeline->bind(cmd);
	layout->bindSet(cmd, *context.uboSet);
	auto tmp = prj->getMatLocalToEye();
	layout->pushConstant(cmd, 0, &tmp);
	VertexArray::bindGlobal(cmd, bodyData[0].vertex->get());

	for(int l=0; l<currentUsedList+1; l++) {
		if (bodyData[l].size>2 && !bodyData[l].hide) {
			layout->pushConstant(cmd, 1, &bodyData[l].color);
			vkCmdDraw(cmd, bodyData[l].size, 1, bodyData[l].vertex->getOffset(), 0);
		}
	}
	frame.compile(cmds[context.frameIdx]);
	frame.toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
}

void BodyTrace::addData(const Navigator *nav, double alt, double az)
{
	if (!fader.getInterstate()) return;
	if(!is_tracing) return;

	if (bodyData[currentUsedList].size==0) {
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		Utility::spheToRect(-(az+M_PI),alt, *(Vec3f *) Context::instance->transfer->planCopy(bodyData[currentUsedList].vertex->get(), 0, sizeof(Vec3f)));
		bodyData[currentUsedList].size=1;
	}
	else {
		if ( (abs(alt-bodyData[currentUsedList].old_punt[0])< 0.001) && (abs(az-bodyData[currentUsedList].old_punt[1])< 0.001) ) return;
		if (bodyData[currentUsedList].size==(MAX_POINTS-1)) return;
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		Utility::spheToRect(-(az+M_PI),alt, *(Vec3f *) Context::instance->transfer->planCopy(bodyData[currentUsedList].vertex->get(), sizeof(Vec3f) * bodyData[currentUsedList].size, sizeof(Vec3f)));
		bodyData[currentUsedList].size+=1;
	}
}
