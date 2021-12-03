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
* (c) 2017 - 2020 all rights reserved
*
*/


#include "mediaModule/viewport.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include <cassert>

ViewPort::ViewPort()
{
	isAlive = false;
	fader = false;
	fader.setDuration(VP_FADER_DURATION);
}

ViewPort::~ViewPort()
{}

void ViewPort::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	context.cmdInfo.commandBufferCount = 3;
    vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
	vertexModel = std::make_unique<VertexArray>(vkmgr);
	vertexModel->createBindingEntry(4 * sizeof(float));
	vertexModel->addInput(VK_FORMAT_R32G32_SFLOAT); // POS2D
	vertexModel->addInput(VK_FORMAT_R32G32_SFLOAT); // TEXTURE
	// FullScreen mode
	float viewportPoints[8] = {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	float viewportTex[8] =    { 0.0,  1.0, 1.0,  1.0,  0.0, 0.0, 1.0, 0.0};

	// Dual Half Screen mode
	float halfPoints[16] = {-1.0, -1.0, 1.0, -1.0,
	                        -1.f, 0.f, 1.f, 0.f,
	                        -1.f, 0.f, 1.f, 0.f,
	                        -1.0, 1.0, 1.0, 1.0
	                       };
	float halfTex[16] =    { 0.0,  1.0, 1.0,  1.0,
	                         0.f, 0.5f, 1.0f, 0.5f,

	                         1.f, 0.5f, 0.0f, 0.5f,
	                         1.0, 1.0f, 0.0, 1.f
	                       };

    vertex = vertexModel->createBuffer(0, 12, context.globalBuffer.get());
	float *data = (float *) context.transfer->planCopy(vertex->get());
	vertex->fillEntry(2, 4, viewportPoints, data);
	vertex->fillEntry(2, 4, viewportTex, data + 2);
	data += 4 * 4;
	vertex->fillEntry(2, 8, halfPoints, data);
	vertex->fillEntry(2, 8, halfTex, data + 2);
	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_FOREGROUND, layout.get());
	pipeline->setDepthStencilMode();
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	pipeline->bindVertex(*vertexModel);
	pipeline->bindShader("videoplayer.vert.spv");
	pipeline->bindShader("videoplayer.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	uFrag = std::make_unique<SharedBuffer<s_frag>>(*context.uniformMgr);
	set->bindUniform(uFrag, 3);
	uFrag->get().transparency = false;
	uFrag->get().noColor = Vec4f::null();
}

void ViewPort::build(int frameIdx)
{
	Context &context = *Context::instance;
	VkCommandBuffer cmd = cmds[frameIdx];
	context.frame[frameIdx]->begin(cmd, PASS_FOREGROUND);
	sync->syncIn->dstDependency(cmd);
	pipeline->bind(cmd);
	layout->bindSet(cmd, *set);
	vertex->bind(cmd);
	if (fullScreen) {
		vkCmdDraw(cmd, 4, 1, 0, 0);
	} else {
		vkCmdDraw(cmd, 4, 1, 4, 0);
		vkCmdDraw(cmd, 4, 1, 8, 0);
	}
	context.frame[frameIdx]->compile(cmd);
}

void ViewPort::setTexture(VideoTexture _tex)
{
	// There must be no command using this set
	set->unGet();
	set->bindTexture(*_tex.y, 0);
	set->bindTexture(*_tex.u, 1);
	set->bindTexture(*_tex.v, 2);
	sync = _tex.sync;
	for (int i = 0; i < 3; ++i)
		needUpdate[i] = true;
}

void ViewPort::draw()
{
	if (! isAlive)
		return;
	sync->inUse = true;

	uFrag->get().fader = fader.getInterstate();

	const int frameIdx = Context::instance->frameIdx;
	if (needUpdate[frameIdx]) {
		build(frameIdx);
		needUpdate[frameIdx] = false;
	}
	Context::instance->frame[frameIdx]->toExecute(cmds[frameIdx], PASS_FOREGROUND);
}

void ViewPort::displayStop()
{
	isAlive = false;
	fader=false;
	fader.reset(false);
	uFrag->get().transparency = VK_FALSE;
	uFrag->get().noColor = Vec4f::null();
}

void ViewPort::setTransparency(bool v)
{
	uFrag->get().transparency = (v) ? VK_TRUE : VK_FALSE;
}

void ViewPort::setKeyColor(const Vec3f&color, float intensity)
{
	uFrag->get().noColor = Vec4f(color[0], color[1], color[2],intensity);
}
