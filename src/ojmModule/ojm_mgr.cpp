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
// #include "renderGL/stateGL.hpp"
// #include "renderGL/Renderer.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Context.hpp"
#include "vulkanModule/VertexArray.hpp"

OjmMgr::OjmMgr(ThreadContext *context)
{
	cmdMgr = context->commandMgrDynamic;
	cmdMgrMaster = context->commandMgr;
	surface = context->surface;
	globalSet = context->global->globalSet;
	createShader(context);
}

OjmMgr::~OjmMgr()
{
	delete[] pipeline;
	std::vector<OjmContainer*>::iterator iter;
	for (iter=OjmVector.begin(); iter!=OjmVector.end(); iter++) {
			if (*iter) delete *iter;
			iter=OjmVector.erase(iter);
			iter--;
		}
	OjmVector.clear();

	// deleteShader();
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
		needRebuild = true;
	}
	OjmContainer * tmp=nullptr;
	tmp = new OjmContainer;
	tmp->myState = tmpState;
	tmp->name = name;
	tmp->model = Mat4f::translation(Position);
	tmp->Obj3D = new Ojm(fileName, pathFile, multiplier, surface);
	tmp->uniform = std::make_unique<Uniform>(surface, sizeof(Mat4f) * 2, true);
	tmp->pUniform = static_cast<typeof(tmp->pUniform)>(tmp->uniform->data);

	if (!tmp->Obj3D->getOk()) {
		delete tmp;
		cLog::get()->write("Error loading ojm "+ name, LOG_TYPE::L_ERROR);
		return false;
	} else {
		OjmVector.push_back(tmp);
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
		needRebuild = true;
	}
	std::vector<OjmContainer*>::iterator iter;
	for (iter=OjmVector.begin(); iter!=OjmVector.end(); iter++) {

		if ( ((*iter)->myState == state) && (*iter)->name == _name) {
			if (*iter) delete *iter;
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
		needRebuild = true;
	}
	std::vector<OjmContainer*>::iterator iter;
	for (iter=OjmVector.begin(); iter!=OjmVector.end(); ++iter) {

		if ( ((*iter)->myState == state)) {
			if (*iter) delete *iter;
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
	//glEnable(GL_CULL_FACE); // enable cull_face

	// if (state == STATE_POSITION::IN_UNIVERSE) {
	// 	StateGL::enable(GL_DEPTH_TEST);
	// 	glDepthFunc(GL_LESS);
	// }
	//
	// if (state == STATE_POSITION::IN_GALAXY) {
	// 	StateGL::enable(GL_BLEND);
	// 	StateGL::enable(GL_DEPTH_TEST);
	// 	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// 	//~ glFrontFace(GL_CW);
	// 	//~ glCullFace(GL_FRONT);
	// 	//~ glEnable(GL_CULL_FACE); // enable cull_face
	// }
	if (state != actualState) {
		actualState = state;
		rebuild();
		return; // we can't use command buffer of different state
	}

	view = nav->getHelioToEyeMat().convert();
	//~ view = view * Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);
	proj = prj->getMatProjection().convert();

	//shaderOJM->use();
	Mat4f modelViewMatrix;

	for(unsigned int i=0; i< OjmVector.size(); i++) {
		if (OjmVector[i]->myState != state)
			continue;

		modelViewMatrix = view * OjmVector[i]->model;
		OjmVector[i]->pUniform->ModelViewMatrix = modelViewMatrix;
		OjmVector[i]->pUniform->NormalMatrix = (modelViewMatrix.inverse()).transpose();
		// shaderOJM->setUniform("ModelViewMatrix" , modelViewMatrix);
		//shaderOJM->setUniform("ProjectionMatrix" , proj);
		// shaderOJM->setUniform("NormalMatrix" , normal);
		//shaderOJM->setUniform("MVP" , proj * view * OjmVector[i]->model);
		//shaderOJM->setUniform("inverseModelViewProjectionMatrix",(proj * view * OjmVector[i]->model).inverse());

		// shaderOJM->setUniform("Light.Position", Vec4f(0.0, 0.0, 0.0, 1.0));
		// shaderOJM->setUniform("Light.Intensity", Vec3f(1.0, 1.0, 1.0));

		//OjmVector[i]->Obj3D->draw(shaderOJM.get());
	}
	cmdMgr->setSubmission(commandIndex, false, cmdMgrMaster);

	if (needRebuild)
		rebuild();

	//shaderOJM->unuse();

	// if (state == STATE_POSITION::IN_UNIVERSE) {
	// 	StateGL::disable(GL_DEPTH_TEST);
	// 	glDepthFunc(GL_LEQUAL);
	// }
	//
	// if (state == STATE_POSITION::IN_GALAXY) {
	// 	StateGL::disable(GL_BLEND);
	// 	StateGL::disable(GL_DEPTH_TEST);
	// 	StateGL::BlendFunc(GL_ONE, GL_ZERO);
	// 	// glClear(GL_DEPTH_BUFFER_BIT);
	// 	Renderer::clearDepthBuffer();
	//
	// }
	// glDisable(GL_CULL_FACE);
}

void OjmMgr::rebuild()
{
	int tmp = commandIndex;
	commandIndex = commandIndexSwitch;
	commandIndexSwitch = tmp;
	tmp = 0; // selected pipeline is pipeline 0
	needRebuild = false;
	Vec4f pos(0.0, 0.0, 0.0, 1.0);
	Vec3f intensity(1.0, 1.0, 1.0);
	float buff[7];
	*reinterpret_cast<Vec4f *>(buff) = pos;
	*reinterpret_cast<Vec3f *>(buff + 4) = intensity;

	cmdMgr->init(commandIndex, pipeline, renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
	cmdMgr->bindSet(layout.get(), globalSet);
	cmdMgr->pushConstant(layout.get(), VK_SHADER_STAGE_FRAGMENT_BIT, 44, buff, 28);
	for(unsigned int i=0; i< OjmVector.size(); i++) {
		if (OjmVector[i]->myState != actualState)
			continue;
		set->setVirtualUniform(OjmVector[i]->uniform.get(), virtualUniformID);
		cmdMgr->bindSet(layout.get(), set.get(), 2);
		tmp = OjmVector[i]->Obj3D->record(cmdMgr, pipeline, layout.get(), pushSet, tmp);
	}
	cmdMgr->compile();
}

void OjmMgr::createShader(ThreadContext *context)
{
	// shaderOJM= std::make_unique<shaderProgram>();
	// shaderOJM->init("shaderOJM_noSUN.vert", "", "", "","shaderOJM_noSUN.frag");
	// shaderOJM->setUniformLocation("ModelViewMatrix");
	// shaderOJM->setUniformLocation("NormalMatrix");
	// shaderOJM->setUniformLocation("ProjectionMatrix");
	// shaderOJM->setUniformLocation("MVP");
	// shaderOJM->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderOJM->setUniformLocation("Light.Position");
	// shaderOJM->setUniformLocation("Light.Intensity");
	// shaderOJM->setUniformLocation("Material.Ka");
	// shaderOJM->setUniformLocation("Material.Kd");
	// shaderOJM->setUniformLocation("Material.Ks");
	// shaderOJM->setUniformLocation("Material.Ns");
	// shaderOJM->setUniformLocation("T");
	// shaderOJM->setUniformLocation("useTexture");
	//~ shaderOJM->printInformations(); */
	layout = std::make_unique<PipelineLayout>(surface);
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->buildLayout(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layout->buildLayout();
	layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 72);
	layout->build();

	vertex = std::make_unique<VertexArray>(surface);
	vertex->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	vertex->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	vertex->registerVertexBuffer(BufferType::NORMAL, BufferAccess::STATIC);

	pipeline = new Pipeline[2]{{surface, layout.get()}, {surface, layout.get()}};
	for (short i = 0; i < 2; ++i) {
		pipeline[i].setCullMode(true);
		pipeline[i].bindVertex(vertex.get());
		pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipeline[i].bindShader("shaderOJM_noSUN.vert.spv");
		pipeline[i].bindShader("shaderOJM_noSUN.frag.spv", (i == 0) ? "mainTextured" : "mainTextureless");
		pipeline[i].build();
	}
	set = std::make_unique<Set>(surface, context->setMgr, layout.get(), 2);
	uniformModel = std::make_unique<Uniform>(surface, sizeof(Mat4f) * 2, true);
	virtualUniformID = set->bindVirtualUniform(uniformModel.get(), 0);

	commandIndex = cmdMgr->getCommandIndex();
	commandIndexSwitch = cmdMgr->getCommandIndex();
}

// void OjmMgr::deleteShader()
// {
// 	if(shaderOJM) shaderOJM=nullptr;
// }
