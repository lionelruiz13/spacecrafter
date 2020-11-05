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
// #include "vulkanModule/VertexArray.hpp"
//
//
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/VertexArray.hpp"
#include <cassert>

ViewPort::ViewPort()
{
	isAlive = false;
	transparency = false;
	fader = false;
	fader.setDuration(VP_FADER_DURATION);
}

ViewPort::~ViewPort()
{}

void ViewPort::createSC_context(ThreadContext *context)
{
	cmdMgr = context->commandMgrDynamic;
	cmdMgrTarget = context->commandMgr;
	commandIndex = cmdMgr->getCommandIndex();
	// FullScreen mode
	float viewportPoints[8] = {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	float viewportTex[8] =    { 0.0,  1.0, 1.0,  1.0,  0.0, 0.0, 1.0, 0.0};

	m_fullGL = std::make_unique<VertexArray>(context->surface);
	m_fullGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_fullGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	m_fullGL->build(4);
	m_fullGL->fillVertexBuffer(BufferType::POS2D, 8, viewportPoints);
	m_fullGL->fillVertexBuffer(BufferType::TEXTURE, 8, viewportTex);

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

	m_dualGL = std::make_unique<VertexArray>(context->surface);
	m_dualGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_dualGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	m_dualGL->build(8);
	m_dualGL->fillVertexBuffer(BufferType::POS2D, 16, halfPoints);
	m_dualGL->fillVertexBuffer(BufferType::TEXTURE, 16, halfTex);
	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setDepthStencilMode();
	pipeline->setRenderPassCompatibility(renderPassCompatibility::SINGLE_SAMPLE);
	pipeline->bindVertex(m_fullGL.get());
	pipeline->bindShader("videoplayer.vert.spv");
	pipeline->bindShader("videoplayer.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
	uFrag = std::make_unique<Uniform>(context->surface, sizeof(*pNoColor) + sizeof(*pFader) + sizeof(*pTransparency));
	pNoColor = static_cast<Vec4f *>(uFrag->data);
	pFader = static_cast<float *>(uFrag->data) + 4;
	assert(sizeof(float) == sizeof(VkBool32));
	pTransparency = static_cast<VkBool32 *>(uFrag->data) + 5;
	set->bindUniform(uFrag.get(), 3);
}

void ViewPort::build()
{
	cmdMgr->init(commandIndex, pipeline.get(), renderPassType::SINGLE_SAMPLE_DEFAULT, true, renderPassCompatibility::SINGLE_SAMPLE);
	cmdMgr->bindSet(layout.get(), set.get());
	if (fullScreen) {
		cmdMgr->bindVertex(m_fullGL.get());
		cmdMgr->draw(4);
	} else {
		cmdMgr->bindVertex(m_dualGL.get());
		cmdMgr->draw(4);
		cmdMgr->draw(4, 1, 4);
	}
	cmdMgr->compile();
}

void ViewPort::setTexture(VideoTexture _tex)
{
	// There must be no command using this set
	cmdMgrTarget->waitCompletion(0);
	cmdMgrTarget->waitCompletion(1);
	cmdMgrTarget->waitCompletion(2);
	set->bindTexture(_tex.y, 0);
	set->bindTexture(_tex.u, 1);
	set->bindTexture(_tex.v, 2);
}

void ViewPort::draw()
{
	if (! isAlive)
		return;
	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// for(int i=0; i<3; i++) {
	// 	glActiveTexture(GL_TEXTURE0+i);
	// 	glBindTexture(GL_TEXTURE_2D, videoTex[i]);
	// }

	// shaderViewPort->use();

	*pNoColor = noColor;
	*pFader = fader.getInterstate();
	*pTransparency = transparency ? VK_TRUE : VK_FALSE;
	// shaderViewPort->setUniform("noColor",noColor);
	// shaderViewPort->setUniform("fader", fader.getInterstate() );
	// shaderViewPort->setUniform("transparency",transparency);

	// if (fullScreen) {
		// m_fullGL->bind();
		// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,4);
		// m_fullGL->unBind();
		//Renderer::drawArrays(shaderViewPort.get(), m_fullGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,4);
	// }
	// else {
		// m_dualGL->bind();
		// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,4);
		// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,4,4);
		// m_dualGL->unBind();
		//Renderer::drawMultiArrays(shaderViewPort.get(), m_fullGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,2,4);
	// }
	// shaderViewPort->unuse();
	// StateGL::disable(GL_BLEND);
	cmdMgr->setSubmission(commandIndex, true, cmdMgrTarget);
}
