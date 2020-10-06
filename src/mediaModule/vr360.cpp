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
#include "eventModule/event_recorder.hpp"
#include "eventModule/EventScript.hpp"
#include "mediaModule/vr360.hpp"
#include "navModule/navigator.hpp"
#include "ojmModule/ojml.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
// #include "renderGL/stateGL.hpp"
// #include "renderGL/shader.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/VertexArray.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"

#define VR360_FADER_DURATION 3000

VR360::VR360()
{
	showFader = false;
	showFader.setDuration(VR360_FADER_DURATION);
}

void VR360::init(ThreadContext *context)
{
	sphere = new OjmL(AppSettings::Instance()->getModel3DDir()+"VR360Sphere.ojm", context);
	cube = new OjmL(AppSettings::Instance()->getModel3DDir()+"VR360Cube.ojm", context);

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
}

VR360::~VR360()
{
	if (sphere) delete sphere;
	if (cube) delete cube;
	// deleteShader();
}

void VR360::createSC_context(ThreadContext *_context)
{
	context = _context;
	commandIndex = context->commandMgrDynamic->getCommandIndex();
	// shaderVR360 = std::make_unique<shaderProgram>();
	// shaderVR360->init( "vr360.vert","vr360.frag");
	// shaderVR360->setUniformLocation({"intensity", "ModelViewMatrix"});
	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 3);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setDepthStencilMode();
	pipeline->setCullMode(true);
	sphere->bind(pipeline.get()); // bind Objl VertexArray to pipeline
	pipeline->bindShader("vr360.vert.spv");
	pipeline->bindShader("vr360.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
	uModelViewMatrix = std::make_unique<Uniform>(context->surface, sizeof(*pModelViewMatrix));
	pModelViewMatrix = static_cast<typeof(pModelViewMatrix)>(uModelViewMatrix->data);
	set->bindUniform(uModelViewMatrix.get(), 3);
}

// void VR360::deleteShader()
// {
// 	if(shaderVR360) shaderVR360=nullptr;
// }

void VR360::display(bool alive)
{
	//~ if (alive) printf("VR360 alive\n");
	//~ else printf("VR360 not alive\n");
	Event* event;
	if (alive) {
		event = new ScriptEvent( AppSettings::Instance()->getScriptDir()+"internal/initialVR360.sts");
		EventRecorder::getInstance()->queue(event);
		showFader = true;
		build();
	}
	else {
		event = new ScriptEvent( AppSettings::Instance()->getScriptDir()+"internal/clearVR360.sts");
		EventRecorder::getInstance()->queue(event);
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

void VR360::build()
{
	CommandMgr *cmdMgr = context->commandMgrDynamic;
	cmdMgr->init(commandIndex, pipeline.get());
	cmdMgr->bindSet(layout.get(), context->global->globalSet, 0);
	cmdMgr->bindSet(layout.get(), set.get(), 1);
	switch(typeVR360) {
		case TYPE::V_CUBE:
			cube->bind(cmdMgr);
			cmdMgr->drawIndexed(cube->getVertexArray()->getIndiceCount());
			break;
		case TYPE::V_SPHERE:
			sphere->bind(cmdMgr);
			cmdMgr->drawIndexed(sphere->getVertexArray()->getIndiceCount());
			break;
		default:
			break;
	}
	cmdMgr->compile();
}

void VR360::setTexture(VideoTexture _tex)
{
	// There must be no command using this set
	context->commandMgr->waitCompletion(0);
	context->commandMgr->waitCompletion(1);
	context->commandMgr->waitCompletion(2);
	set->bindTexture(_tex.y, 0);
	set->bindTexture(_tex.u, 1);
	set->bindTexture(_tex.v, 2);
}

void VR360::draw(const Projector* prj, const Navigator* nav)
{
	if (!canDraw || !isAlive)
		return;

	//shaderVR360->use();

	// for(int i=0; i<3; i++) {
	// 	glActiveTexture(GL_TEXTURE0+i);
	// 	glBindTexture(GL_TEXTURE_2D, videoTex[i]);
	// }

	//shaderVR360->setUniform("intensity", showFader.getInterstate());

	*pModelViewMatrix = (nav->getJ2000ToEyeMat() *
	                     Mat4d::xrotation(M_PI)*
	                     Mat4d::yrotation(M_PI)*
	                     Mat4d::zrotation(M_PI/180*270)).convert();

	//shaderVR360->setUniform("ModelViewMatrix",matrix);

	//StateGL::enable(GL_CULL_FACE);
	// switch(typeVR360) {
	// 	case TYPE::V_CUBE:
	// 		cube->draw();
	// 		break;
	// 	case TYPE::V_SPHERE:
	// 		sphere->draw();
	// 		break;
	// 	default:
	// 		break;
	// }
	//StateGL::disable(GL_CULL_FACE);
	//shaderVR360->unuse();
	context->commandMgrDynamic->setSubmission(commandIndex, false, context->commandMgr);
}
