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
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"

ViewPort::ViewPort()
{
	isAlive = false;
	transparency = false;
	fader = false;
	fader.setDuration(VP_FADER_DURATION);
}

ViewPort::~ViewPort()
{
	deleteShader();

}

void ViewPort::createShader()
{
	shaderViewPort= std::make_unique<shaderProgram>();
	shaderViewPort->init("videoplayer.vert","videoplayer.frag");
	shaderViewPort->setUniformLocation("transparency");
	shaderViewPort->setUniformLocation("noColor");
	shaderViewPort->setUniformLocation("fader");
}

void ViewPort::createGL_context()
{
	// FullScreen mode
	float viewportPoints[8] = {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	float viewportTex[8] =    { 0.0,  1.0, 1.0,  1.0,  0.0, 0.0, 1.0, 0.0};

	viewport = std::make_unique<VertexArray>();
	viewport->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	viewport->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	
	viewport->fillVertexBuffer(BufferType::POS2D, 8, viewportPoints);
	viewport->fillVertexBuffer(BufferType::TEXTURE, 8, viewportTex);
	// glGenBuffers(1,&viewport.pos);
	// glBindBuffer(GL_ARRAY_BUFFER,viewport.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, viewportPoints,GL_STATIC_DRAW);

	// glGenBuffers(1,&viewport.tex);
	// glBindBuffer(GL_ARRAY_BUFFER,viewport.tex);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8,viewportTex,GL_STATIC_DRAW);

	// glGenVertexArrays(1,&viewport.vao);
	// glBindVertexArray(viewport.vao);

	// glBindBuffer (GL_ARRAY_BUFFER, viewport.pos);
	// glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);
	// glEnableVertexAttribArray(0);

	// glBindBuffer (GL_ARRAY_BUFFER, viewport.tex);
	// glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);
	// glEnableVertexAttribArray(1);

	// Dual Half Screen mode
	float halfPoints[16] = {-1.0, -1.0, 1.0, -1.0,
							-1.f, 0.f, 1.f, 0.f,
							-1.f, 0.f, 1.f, 0.f,
							-1.0, 1.0, 1.0, 1.0};
	float halfTex[16] =    { 0.0,  1.0, 1.0,  1.0, 
							0.f, 0.5f, 1.0f, 0.5f,

							1.f, 0.5f, 0.0f, 0.5f,
							1.0, 1.0f, 0.0, 1.f};

	dual = std::make_unique<VertexArray>();
	dual->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	dual->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	
	dual->fillVertexBuffer(BufferType::POS2D, 16, halfPoints);
	dual->fillVertexBuffer(BufferType::TEXTURE, 16, halfTex);

	// glGenBuffers(1,&dual.pos);
	// glBindBuffer(GL_ARRAY_BUFFER,dual.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*16, halfPoints,GL_STATIC_DRAW);

	// glGenBuffers(1,&dual.tex);
	// glBindBuffer(GL_ARRAY_BUFFER,dual.tex);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*16,halfTex,GL_STATIC_DRAW);

	// glGenVertexArrays(1,&dual.vao);
	// glBindVertexArray(dual.vao);

	// glBindBuffer (GL_ARRAY_BUFFER, dual.pos);
	// glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);
	// glEnableVertexAttribArray(0);

	// glBindBuffer (GL_ARRAY_BUFFER, dual.tex);
	// glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);
	// glEnableVertexAttribArray(1);	
}

void ViewPort::deleteShader()
{
	if(shaderViewPort) shaderViewPort=nullptr;

	// glDeleteBuffers(1,&viewport.vao);
	// glDeleteBuffers(1,&viewport.tex);
	// glDeleteVertexArrays(1,&viewport.pos);

	// glDeleteBuffers(1,&dual.vao);
	// glDeleteBuffers(1,&dual.tex);
	// glDeleteVertexArrays(1,&dual.pos);
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
		// glBindVertexArray(viewport.vao);
		viewport->bind();
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
		viewport->unBind();
	} else {
		//glBindVertexArray(dual.vao);
		dual->bind();
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
		glDrawArrays(GL_TRIANGLE_STRIP,4,4);
		dual->unBind();
	}
	shaderViewPort->unuse();
	StateGL::disable(GL_BLEND);
}
