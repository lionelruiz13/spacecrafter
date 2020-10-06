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


#include "bodyModule/halo.hpp"
#include "bodyModule/body.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "bodyModule/body_color.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/s_texture.hpp"
#include <iostream>
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Texture.hpp"
#include "vulkanModule/ResourceTracker.hpp"

VirtualSurface *Halo::surface;
SetMgr *Halo::setMgr;
Set *Halo::globalSet;
CommandMgr *Halo::cmdMgr;
CommandMgr *Halo::cmdMgrTarget;
TextureMgr *Halo::texMgr;
Pipeline *Halo::pipeline;
PipelineLayout *Halo::layout;
VertexArray *Halo::m_haloGL;
s_texture *Halo::tex_halo = nullptr;

Halo::Halo(Body * _body)
{
	body = _body;

	vertex = std::make_unique<VertexArray>(*m_haloGL);
	vertex->build(4);

	set = std::make_unique<Set>(surface, setMgr, layout);
	uniform = std::make_unique<Uniform>(surface, sizeof(*uData));
	uData = static_cast<typeof(uData)>(uniform->data);
	set->bindUniform(uniform.get(), 1);

	commandIndex = cmdMgr->getCommandIndex();
}

void Halo::build()
{
	cmdMgrTarget->waitCompletion(0);
	cmdMgrTarget->waitCompletion(1);
	cmdMgrTarget->waitCompletion(2);
	cmdMgr->init(commandIndex);
	cmdMgr->beginRenderPass(renderPassType::DEFAULT);
	cmdMgr->bindPipeline(pipeline);
	cmdMgr->bindSet(layout, globalSet);
	cmdMgr->bindSet(layout, set.get(), 1);
	cmdMgr->bindVertex(vertex.get());
	cmdMgr->draw(4);
	cmdMgr->compile();
}

void Halo::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	if (!tex_halo) return;
	computeHalo(nav, prj, eye);
	if (rmag<1.21 && cmag < 0.05)
		return;

	// StateGL::BlendFunc(GL_ONE, GL_ONE);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, tex_halo->getID());

	uData->Color = body->myColor->getHalo();
	uData->cmag = cmag;

	vertex->fillVertexBuffer(BufferType::POS2D, vecHaloPos);
	vertex->fillVertexBuffer(BufferType::TEXTURE, vecHaloTex);

	if (tex_halo != last_tex_halo) {
		set->bindTexture(tex_halo->getTexture(), 0);
		last_tex_halo = tex_halo;
		build();
	}

	// m_haloGL->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,4);
	// m_haloGL->unBind();
	// shaderHalo->unuse();
	cmdMgr->setSubmission(commandIndex, false, cmdMgrTarget);

	vecHaloPos.clear();
	vecHaloTex.clear();
}

void Halo::computeHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	float fov_q = prj->getFov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	rmag = sqrtf(eye->adaptLuminance((expf(-0.92103f*(body->computeMagnitude(nav->getObserverHelioPos()) + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * Body::object_scale;

	if (body->is_satellite)	{
		if (prj->getFov()>60) rmag=rmag/25; // usefull when going there
		else rmag=rmag/5; // usefull when zooming onto planet
	}
	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<1.2f) {
		if (body->computeMagnitude(nav->getObserverHelioPos())>0.) cmag=rmag*rmag/1.44f;
		else cmag=rmag/1.2f;
		rmag=1.2f;
	}
	else {

		float limit = Body::object_size_limit/1.8;
		if (rmag>limit) {
			rmag = limit + sqrt(rmag-limit)/(limit + 1);

			if (rmag > Body::object_size_limit) {
				rmag = Body::object_size_limit;
			}
		}
	}

	float screen_r = body->getOnScreenSize(prj, nav);
	cmag *= 0.5*rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {
		cmag*=rmag/screen_r;
		rmag = screen_r;
	}

	if (body->is_satellite) {
		Vec3d _planet = body->get_parent()->get_heliocentric_ecliptic_pos();
		Vec3d _satellite = body->get_heliocentric_ecliptic_pos();
		double c = _planet.dot(_satellite);
		double OP = _planet.length();
		double OS = _satellite.length();
		if (c>0 && OP < OS)
			if (fabs(acos(c/(OP*OS)))<atan(body->get_parent()->getRadius()/OP)) {
				cmag = 0.0;
			}
	}

	if (rmag<1.21 && cmag < 0.05)
		return;

	Vec2f screenPosF ((float) body->screenPos[0], (float)body->screenPos[1]);

	insert_all(vecHaloPos, screenPosF[0]-rmag, screenPosF[1]-rmag, screenPosF[0]-rmag, screenPosF[1]+rmag);
	insert_all(vecHaloPos, screenPosF[0]+rmag, screenPosF[1]-rmag, screenPosF[0]+rmag, screenPosF[1]+rmag);
	insert_all(vecHaloTex, 0, 0, 0, 1, 1, 0, 1, 1);
}

void Halo::createSC_context(ThreadContext *context)
{
	surface = context->surface;
	cmdMgr = context->commandMgrDynamic;
	cmdMgrTarget = context->commandMgr;
	setMgr = context->setMgr;
	texMgr = context->global->textureMgr;
	globalSet = context->global->globalSet;

	m_haloGL = context->global->tracker->track(new VertexArray(surface, cmdMgr));
	m_haloGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_haloGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::DYNAMIC);

	layout = context->global->tracker->track(new PipelineLayout(surface));
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->buildLayout();
	layout->build();

	pipeline = context->global->tracker->track(new Pipeline(context->surface, layout));
	pipeline->setBlendMode(BLEND_ADD);
	pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	pipeline->bindShader("body_halo.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("body_halo.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(m_haloGL);
	pipeline->build();
}

bool Halo::setTexHaloMap(const std::string &texMap)
{
	tex_halo = new s_texture(texMap, TEX_LOAD_TYPE_PNG_SOLID_REPEAT,1);
	if (tex_halo != nullptr)
		return true;
	else
		return false;
}

void Halo::deleteDefaultTexMap()
{
	if(tex_halo != nullptr) {
		delete tex_halo;
		tex_halo = nullptr;
	}
}
