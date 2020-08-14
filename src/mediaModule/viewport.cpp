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
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

ViewPort::ViewPort()
{
	isAlive = false;
	transparency = false;
	fader = false;
	fader.setDuration(VP_FADER_DURATION);
}

ViewPort::~ViewPort()
{}

void ViewPort::createShader()
{
	shaderViewPort= std::make_unique<shaderProgram>();
	shaderViewPort->init("videoplayer.vert","videoplayer.frag");
	shaderViewPort->setUniformLocation({"transparency", "noColor", "fader"});
}

void ViewPort::createSC_context()
{
	// FullScreen mode
	float viewportPoints[8] = {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	float viewportTex[8] =    { 0.0,  1.0, 1.0,  1.0,  0.0, 0.0, 1.0, 0.0};

	m_fullGL = std::make_unique<VertexArray>();
	m_fullGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_fullGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);

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

	m_dualGL = std::make_unique<VertexArray>();
	m_dualGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_dualGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);

	m_dualGL->fillVertexBuffer(BufferType::POS2D, 16, halfPoints);
	m_dualGL->fillVertexBuffer(BufferType::TEXTURE, 16, halfTex);
}

void ViewPort::draw()
{
	if (! isAlive)
		return;
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for(int i=0; i<3; i++) {
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, videoTex[i]);
	}

	shaderViewPort->use();

	shaderViewPort->setUniform("transparency",transparency);
	shaderViewPort->setUniform("noColor",noColor);
	shaderViewPort->setUniform("fader", fader.getInterstate() );

	if (fullScreen) {
		// m_fullGL->bind();
		// glDrawArrays(GL_TRIANGLE_STRIP,0,4);
		// m_fullGL->unBind();
		Renderer::drawArrays(shaderViewPort.get(), m_fullGL.get(), GL_TRIANGLE_STRIP,0,4);
	}
	else {
		// m_dualGL->bind();
		// glDrawArrays(GL_TRIANGLE_STRIP,0,4);
		// glDrawArrays(GL_TRIANGLE_STRIP,4,4);
		// m_dualGL->unBind();
		Renderer::drawMultiArrays(shaderViewPort.get(), m_fullGL.get(), GL_TRIANGLE_STRIP,2,4);
	}
	// shaderViewPort->unuse();
	StateGL::disable(GL_BLEND);
}
