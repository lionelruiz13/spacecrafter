/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/

#include "coreModule/projector.hpp"
#include "eventModule/event_manager.hpp"
#include "eventModule/ScriptEvent.hpp"
#include "mediaModule/vr360.hpp"
#include "navModule/navigator.hpp"
#include "ojmModule/ojml.hpp"
#include "tools/app_settings.hpp"
#include "tools/stateGL.hpp"
#include "tools/log.hpp"


VR360::VR360()
{
	showFader = false;
	showFader.setDuration(VR360_FADER_DURATION);

	sphere = new OjmL(AppSettings::Instance()->getModel3DDir()+"VR360Sphere.ojm");
	cube = new OjmL(AppSettings::Instance()->getModel3DDir()+"VR360Cube.ojm");

	if (sphere !=nullptr && sphere->getOk()) {
		cLog::get()->write("VR360 loading ojml sphere", LOG_TYPE::L_INFO);
		cLog::get()->write("VR360 actived", LOG_TYPE::L_INFO);
		//~ printf("VR360 sphere oki\n");
		canDraw = true;
	}
	else {
		cLog::get()->write("VR360 deactived", LOG_TYPE::L_ERROR);
		//~ printf("VR360 sphere error\n");
	}

	if (canDraw && cube !=nullptr && cube->getOk() ) {
		cLog::get()->write("VR360 loading ojml cube", LOG_TYPE::L_INFO);
		cLog::get()->write("VR360 actived", LOG_TYPE::L_INFO);
		//~ printf("VR360 cube oki\n");
		canDraw = true;
	}
	else {
		cLog::get()->write("VR360 deactived", LOG_TYPE::L_ERROR);
		//~ printf("VR360 cube error\n");
	}

	createShader();
}

VR360::~VR360()
{
	if (sphere) delete sphere;
	if (cube) delete cube;
	deleteShader();
}

void VR360::createShader()
{
	shaderVR360= new shaderProgram();
	shaderVR360->init( "VR360.vert","VR360.frag");
	shaderVR360->setUniformLocation("intensity");
	shaderVR360->setUniformLocation("ModelViewProjectionMatrix");
	shaderVR360->setUniformLocation("ModelViewMatrix");
	shaderVR360->setUniformLocation("inverseModelViewProjectionMatrix");
}

void VR360::deleteShader()
{
	if(shaderVR360) shaderVR360=nullptr;
}

void VR360::display(bool alive)
{
	//~ if (alive) printf("VR360 alive\n");
	//~ else printf("VR360 not alive\n");
	Event* event;
	if (alive) {
		event = new ScriptEvent( AppSettings::Instance()->getScriptDir()+"internal/initialVR360.sts");
		EventManager::getInstance()->queue(event);
		showFader = true;
	}
	else {
		event = new ScriptEvent( AppSettings::Instance()->getScriptDir()+"internal/clearVR360.sts");
		EventManager::getInstance()->queue(event);
		showFader = false;
		showFader.update(VR360_FADER_DURATION);
	}
	isAlive = alive;
}

void VR360::displayStop()
 {
	showFader = false;
	showFader.update(VR360_FADER_DURATION);
	isAlive = false;
}

void VR360::draw(const Projector* prj, const Navigator* nav)
{
	if (!canDraw || !isAlive)
		return;

	shaderVR360->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, videoTex);
	
	shaderVR360->setUniform("intensity", showFader.getInterstate());

	Mat4f proj=prj->getMatProjection().convert();
	Mat4f matrix = (nav->getJ2000ToEyeMat() *
	                Mat4d::xrotation(C_PI)*
	                Mat4d::yrotation(C_PI)*
	                Mat4d::zrotation(C_PI/180*270)).convert();

	shaderVR360->setUniform("inverseModelViewProjectionMatrix", (proj*matrix).inverse());
	shaderVR360->setUniform("ModelViewProjectionMatrix", proj*matrix);
	shaderVR360->setUniform("ModelViewMatrix",matrix);

	StateGL::enable(GL_CULL_FACE);
	switch(typeVR360) {
		case TYPE::V_CUBE: cube->draw(); break;
		case TYPE::V_SPHERE: sphere->draw(); break;
		default: break;
	}
	StateGL::disable(GL_CULL_FACE);
	shaderVR360->unuse();
}
