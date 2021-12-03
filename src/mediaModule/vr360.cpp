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
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

#define VR360_FADER_DURATION 3000

VR360::VR360()
{
	showFader = false;
	showFader.setDuration(VR360_FADER_DURATION);
}

void VR360::init()
{
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
}

VR360::~VR360()
{
	if (sphere) delete sphere;
	if (cube) delete cube;
	// deleteShader();
}

void VR360::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	context.cmdInfo.commandBufferCount = 3;
    vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 3);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_BACKGROUND, layout.get());
	pipeline->setDepthStencilMode();
	pipeline->setCullMode(true);
	sphere->bind(*pipeline); // bind Objl VertexBuffer to pipeline (common to every Obj/Ojm)
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline->bindShader("vr360.vert.spv");
	pipeline->bindShader("vr360.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	uModelViewMatrix = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
	set->bindUniform(uModelViewMatrix, 3);
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
	Context &context = *Context::instance;
	for (int i = 0; i < 3; ++i) {
		VkCommandBuffer cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_BACKGROUND);
		pipeline->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		switch(typeVR360) {
			case TYPE::V_CUBE:
				cube->bind(cmd);
				cube->draw(cmd);
				break;
			case TYPE::V_SPHERE:
				sphere->bind(cmd);
				sphere->draw(cmd);
				break;
			default:
				break;
		}
		context.frame[i]->compile(cmd);
	}
}

void VR360::setTexture(VideoTexture _tex)
{
	// There must be no command using this set
	set->unGet();
	set->bindTexture(*_tex.y, 0);
	set->bindTexture(*_tex.u, 1);
	set->bindTexture(*_tex.v, 2);
	sync = _tex.sync;
}

void VR360::draw(const Projector* prj, const Navigator* nav)
{
	if (!canDraw || !isAlive)
		return;
	sync->inUse = true;

	*uModelViewMatrix = (nav->getJ2000ToEyeMat() *
	                     Mat4d::xrotation(M_PI)*
	                     Mat4d::yrotation(M_PI)*
	                     Mat4d::zrotation(M_PI/180*270)).convert();

	Context::instance->frame[Context::instance->frameIdx]->toExecute(cmds[Context::instance->frameIdx], PASS_BACKGROUND);
}
