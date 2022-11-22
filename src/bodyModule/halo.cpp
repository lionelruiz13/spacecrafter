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
#include "tools/context.hpp"
#include "tools/log.hpp"
#include "EntityCore/EntityCore.hpp"

Halo::HaloContext *Halo::global = nullptr;

Halo::Halo(Body * _body)
{
	body = _body;
}

void Halo::beginDraw()
{
	if (global->last_tex_halo != global->tex_halo.get()) {
		global->set->bindTexture(global->tex_halo->getTexture(), 0);
		global->last_tex_halo = global->tex_halo.get();
	}
}

void Halo::nextDraw(VkCommandBuffer &cmd)
{
	if (global->size) {
		global->pipeline->bind(cmd);
		global->layout->bindSets(cmd, {*Context::instance->uboSet->get(), *global->set->get()});
		global->vertex->bind(cmd);
		vkCmdDraw(cmd, global->size, 1, global->offset, 0);
		global->offset += global->size;
		global->size = 0;
	}
}

void Halo::endDraw()
{
	Context &context = *Context::instance;
	if (global->size) {
		auto &frame = *context.frame[context.frameIdx];
		auto &cmd = frame.begin(global->cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
		nextDraw(cmd);
		frame.compile(cmd);
		frame.toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
	}
	int size = (global->offset - global->initialOffset) * (HALO_STRIDE);
	const int offset = global->initialOffset * (HALO_STRIDE);
	global->initialOffset = (global->initialOffset) ? 0 : global->offset;
	global->offset = global->initialOffset;
	if (size == 0)
		return;
	if ((int)(size / (HALO_STRIDE)) > global->vertex->getVertexCount() / 2) {
		cLog::get()->write("Too many bodies are drawn (" + std::to_string(size / (HALO_STRIDE)) + " > " + std::to_string(global->vertex->getVertexCount() / 2) + "), THIS MAY CAUSE GRAPHICAL GLITCHES !", LOG_TYPE::L_WARNING);
		size = global->vertex->getVertexCount() / 2 * (HALO_STRIDE);
	}
	context.transfer->planCopyBetween(global->staging, global->vertex->get(), size, offset, offset);
}

void Halo::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	if (!global->tex_halo) return;
	computeHalo(nav, prj, eye);
	if (rmag<1.21 && cmag < 0.05)
		return;
	auto &data = global->pData[global->offset + global->size++];
	data.pos = Vec2f((float) body->screenPos[0], (float)body->screenPos[1]);
	data.Color = body->myColor->getHalo() * cmag;
	data.Color[3] = 1.;
	data.rmag = rmag;
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
		//if (body->computeMagnitude(nav->getObserverHelioPos())>5.0f) cmag=cmag*rmag*rmag/1.44f;
		if (body->computeMagnitude(nav->getObserverHelioPos())>6.5f) cmag=cmag*rmag*rmag/1.44f;
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
}

void Halo::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	assert(!global);
	global = new HaloContext();

	for (int i = 0; i < 3; ++i)
		global->cmds[i] = context.frame[i]->create(1);
	global->pattern = std::make_unique<VertexArray>(vkmgr, context.ojmAlignment);
	global->pattern->createBindingEntry(HALO_STRIDE);
	global->pattern->addInput(VK_FORMAT_R32G32_SFLOAT);
	global->pattern->addInput(VK_FORMAT_R32G32B32A32_SFLOAT);
	global->pattern->addInput(VK_FORMAT_R32_SFLOAT);
	//! Note : 16384 is enough while there is no more than 8192 halo drawn per frame
	global->vertex = global->pattern->createBuffer(0, 16384, context.ojmBufferMgr.get());
	global->staging = context.stagingMgr->acquireBuffer(global->vertex->get().size);
	global->pData = static_cast<HaloContext::pData_t *>(context.stagingMgr->getPtr(global->staging));

	global->layout = std::make_unique<PipelineLayout>(vkmgr);
	global->layout->setGlobalPipelineLayout(context.layouts.front().get());
	global->layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	global->layout->buildLayout();
	global->layout->build();

	global->pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, global->layout.get());
	global->pipeline->setBlendMode(BLEND_ADD);
	global->pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	global->pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	global->pipeline->bindShader("body_halo.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	global->pipeline->bindShader("body_halo.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);
	global->pipeline->bindShader("body_halo.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	global->pipeline->bindVertex(*global->pattern);
	global->pipeline->build();

	global->set = std::make_unique<Set>(vkmgr, *context.setMgr, global->layout.get(), -1);
}

void Halo::destroySC_context()
{
	if (global) {
		Context::instance->stagingMgr->releaseBuffer(global->staging);
		delete global;
	}
}

bool Halo::setTexHaloMap(const std::string &texMap)
{
	auto tex_halo = new s_texture(texMap, TEX_LOAD_TYPE_PNG_SOLID_REPEAT,1);
	if (tex_halo != nullptr) {
		global->tex_halo = std::unique_ptr<s_texture>(tex_halo);
		return true;
	} else
		return false;
}

void Halo::deleteDefaultTexMap()
{
	global->tex_halo = nullptr;
}
