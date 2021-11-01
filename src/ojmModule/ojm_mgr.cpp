/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of the LSS Team && Immersive Adventure
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

#include <vector>
#include "tools/object.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "tools/log.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

OjmMgr::OjmMgr()
{
}

OjmMgr::~OjmMgr()
{
	delete[] pipeline;
}

OjmMgr::STATE_POSITION OjmMgr::convert(const std::string & value)
{
	if (value =="in_galaxy")
		return STATE_POSITION::IN_GALAXY;
	if (value =="in_universe")
		return STATE_POSITION::IN_UNIVERSE;
	else
		return STATE_POSITION::OTHER;
}

bool OjmMgr::load(const std::string &mode, const std::string &name, const std::string &fileName, const std::string &pathFile, Vec3f Position, float multiplier)
{
	//~ std::cout << "name " << name << " filename " << fileName << " pathFile " << pathFile << std::endl;
	STATE_POSITION tmpState = convert(mode);
	if (tmpState == actualState) {
		needRebuild[0] = true;
		needRebuild[1] = true;
		needRebuild[2] = true;
	}
	OjmContainer * tmp=nullptr;
	tmp = new OjmContainer;
	tmp->myState = tmpState;
	tmp->name = name;
	tmp->model = Mat4f::translation(Position);
	tmp->Obj3D = std::make_unique<Ojm>(fileName, pathFile, multiplier);
	tmp->uniform = std::make_unique<SharedBuffer<OjmContainer::uniformData>>(*Context::instance->uniformMgr);

	if (!tmp->Obj3D->getOk()) {
		delete tmp;
		cLog::get()->write("Error loading ojm "+ name, LOG_TYPE::L_ERROR);
		return false;
	} else {
		OjmVector.emplace_back(tmp);
		cLog::get()->write("Succesfull loading ojm "+ name, LOG_TYPE::L_INFO);
		return true;
	}
}

bool OjmMgr::remove(const std::string &mode, const std::string& _name)
{
	STATE_POSITION tmpState = convert(mode);
	return this->remove(tmpState, _name);
}

bool OjmMgr::remove(STATE_POSITION state, const std::string& _name)
{
	if (state == actualState) {
		needRebuild[0] = true;
		needRebuild[1] = true;
		needRebuild[2] = true;
	}
	std::vector<std::unique_ptr<OjmContainer>>::iterator iter;
	for (iter=OjmVector.begin(); iter!=OjmVector.end(); iter++) {

		if ( ((*iter)->myState == state) && (*iter)->name == _name) {
			OjmVector.erase(iter);
			cLog::get()->write("OJM found : delete " + _name, LOG_TYPE::L_DEBUG);
			return true;
		}
	}
	cLog::get()->write("No OJM found ! no delete " + _name, LOG_TYPE::L_DEBUG);
	return false;
}

void OjmMgr::removeAll(const std::string &mode)
{
	this->removeAll(convert(mode));
}

void OjmMgr::removeAll(STATE_POSITION state)
{
	if (state == actualState) {
		needRebuild[0] = true;
		needRebuild[1] = true;
		needRebuild[2] = true;
	}
	std::vector<std::unique_ptr<OjmContainer>>::iterator iter;
	for (iter=OjmVector.begin(); iter!=OjmVector.end(); ++iter) {

		if ( ((*iter)->myState == state)) {
			OjmVector.erase(iter);
			iter--;
			cLog::get()->write("OJM found and deleted", LOG_TYPE::L_DEBUG);
		}
	}
}

void OjmMgr::update(int delta_time)
{}

void OjmMgr::draw(Projector *prj, const Navigator *nav, STATE_POSITION state)
{
	Context &context = *Context::instance;

	if (state != actualState) {
		needRebuild[0] = true;
		needRebuild[1] = true;
		needRebuild[2] = true;
		actualState = state;
	}
	if (needRebuild[context.frameIdx])
		rebuild();

	view = nav->getHelioToEyeMat().convert();
	//~ view = view * Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);
	proj = prj->getMatProjection().convert();

	//shaderOJM->use();
	Mat4f modelViewMatrix;

	for(unsigned int i=0; i< OjmVector.size(); i++) {
		if (OjmVector[i]->myState != state)
			continue;

		modelViewMatrix = view * OjmVector[i]->model;
		OjmVector[i]->uniform->get().ModelViewMatrix = modelViewMatrix;
		OjmVector[i]->uniform->get().NormalMatrix = (modelViewMatrix.inverse()).transpose();
	}
	context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
}

void OjmMgr::rebuild()
{
	Context &context = *Context::instance;
	int tmp = 0; // selected pipeline is pipeline 0
	needRebuild[context.frameIdx] = false;
	Vec4f pos(0.0, 0.0, 0.0, 1.0);
	Vec3f intensity(1.0, 1.0, 1.0);
	float buff[7];
	*reinterpret_cast<Vec4f *>(buff) = pos;
	*reinterpret_cast<Vec3f *>(buff + 4) = intensity;
	VkCommandBuffer &cmd = cmds[context.frameIdx];
	context.frame[context.frameIdx]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
    VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
    VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
    vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
	pipeline->bind(cmd);
	layout->pushConstant(cmd, 0, buff, 12*sizeof(float), 7*sizeof(float));
	for(unsigned int i=0; i< OjmVector.size(); i++) {
		if (OjmVector[i]->myState != actualState)
			continue;
		set->setVirtualUniform(OjmVector[i]->uniform->getOffset(), virtualUniformID);
		layout->bindSet(cmd, *set);
		tmp = OjmVector[i]->Obj3D->record(cmd, pipeline, layout.get(), pushSet.get(), tmp, (i == 0));
	}
	context.frame[context.frameIdx]->compile(cmd);
}

void OjmMgr::createShader()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->buildLayout(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0, 1, true);
	layout->buildLayout();
	layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 76);
	layout->build();

	pipeline = new Pipeline[2]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}};
	context.pipelineArray.push_back(pipeline);
	for (short i = 0; i < 2; ++i) {
		pipeline[i].setCullMode(true);
		pipeline[i].bindVertex(*context.ojmVertexArray);
		pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipeline[i].bindShader("shaderOJM_noSUN.vert.spv");
		pipeline[i].bindShader((i == 0) ? "shaderOJM_noSUN_tex.frag.spv" : "shaderOJM_noSUN_notex.frag.spv");
		pipeline[i].build();
	}
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get(), 2);
	virtualUniformID = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 0, sizeof(OjmContainer::uniformData));
	pushSet = std::make_unique<Set>();
	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		vkmgr.setObjectName(cmds[i], VK_OBJECT_TYPE_COMMAND_BUFFER, "OjmMgr");
	}
}
