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
#include "tools/stateGL.hpp"
#include "tools/Renderer.hpp"

OjmMgr::OjmMgr()
{
	createShader();
}

OjmMgr::~OjmMgr()
{
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
	OjmContainer * tmp=nullptr;
	tmp = new OjmContainer;
	tmp->myState = tmpState;
	tmp->name = name;
	tmp->model = Mat4f::translation(Position);
	tmp->Obj3D = new Ojm(fileName, pathFile, multiplier);
	if (!tmp->Obj3D->getOk()) {
		delete tmp;
		cLog::get()->write("Error loading ojm "+ name, LOG_TYPE::L_ERROR);
		return false;
	}
	else {
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
	glEnable(GL_CULL_FACE); // enable cull_face

	if (state == STATE_POSITION::IN_UNIVERSE) {
		StateGL::enable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}

	if (state == STATE_POSITION::IN_GALAXY) {
		StateGL::enable(GL_BLEND);
		StateGL::enable(GL_DEPTH_TEST);
		StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//~ glFrontFace(GL_CW);
		//~ glCullFace(GL_FRONT);
		//~ glEnable(GL_CULL_FACE); // enable cull_face
	}

	view = nav->getHelioToEyeMat().convert();
	//~ view = view * Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);
	proj = prj->getMatProjection().convert();

	shaderOJM->use();

	for(unsigned int i=0; i< OjmVector.size(); i++) {
		if (OjmVector[i]->myState != state)
			continue;

		normal = ((view * OjmVector[i]->model).inverse()).transpose();

		shaderOJM->setUniform("ModelViewMatrix" , view * OjmVector[i]->model);
		shaderOJM->setUniform("ProjectionMatrix" , proj);
		shaderOJM->setUniform("NormalMatrix" , normal);
		shaderOJM->setUniform("MVP" , proj * view * OjmVector[i]->model);
		shaderOJM->setUniform("inverseModelViewProjectionMatrix",(proj * view * OjmVector[i]->model).inverse());

		shaderOJM->setUniform("Light.Position", Vec4f(0.0, 0.0, 0.0, 1.0));
		shaderOJM->setUniform("Light.Intensity", Vec3f(1.0, 1.0, 1.0));

		OjmVector[i]->Obj3D->draw(shaderOJM.get());
	}

	shaderOJM->unuse();

	if (state == STATE_POSITION::IN_UNIVERSE) {
		StateGL::disable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}

	if (state == STATE_POSITION::IN_GALAXY) {
		StateGL::disable(GL_BLEND);
		StateGL::disable(GL_DEPTH_TEST);
		StateGL::BlendFunc(GL_ONE, GL_ZERO);
		// glClear(GL_DEPTH_BUFFER_BIT);
		Renderer::clearDepthBuffer();
		
	}
	glDisable(GL_CULL_FACE);
}


void OjmMgr::createShader()
{
	shaderOJM= std::make_unique<shaderProgram>();
	shaderOJM->init("shaderOJM_noSUN.vert", "", "", "","shaderOJM_noSUN.frag");
	shaderOJM->setUniformLocation("ModelViewMatrix");
	shaderOJM->setUniformLocation("NormalMatrix");
	shaderOJM->setUniformLocation("ProjectionMatrix");
	shaderOJM->setUniformLocation("MVP");
	shaderOJM->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderOJM->setUniformLocation("Light.Position");
	shaderOJM->setUniformLocation("Light.Intensity");
	shaderOJM->setUniformLocation("Material.Ka");
	shaderOJM->setUniformLocation("Material.Kd");
	shaderOJM->setUniformLocation("Material.Ks");
	shaderOJM->setUniformLocation("Material.Ns");
	shaderOJM->setUniformLocation("T");
	shaderOJM->setUniformLocation("useTexture");
	//~ shaderOJM->printInformations();
}

// void OjmMgr::deleteShader()
// {
// 	if(shaderOJM) shaderOJM=nullptr;
// }
