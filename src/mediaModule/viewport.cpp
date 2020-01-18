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


#include "mediaModule/viewport.hpp"

ViewPort::ViewPort()
{
	texture = 0;
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
	shaderViewPort = nullptr;
	shaderViewPort= new shaderProgram();
	shaderViewPort->init("videoplayer.vert","videoplayer.frag");
	shaderViewPort->setUniformLocation("transparency");
	shaderViewPort->setUniformLocation("noColor");
	shaderViewPort->setUniformLocation("fader");

	float viewportPoints[8] = {-1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};
	float viewportTex[8] =    { 0.0,  1.0, 1.0,  1.0,  0.0, 0.0, 1.0, 0.0};

	glGenBuffers(1,&viewport.pos);
	glBindBuffer(GL_ARRAY_BUFFER,viewport.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8, viewportPoints,GL_STATIC_DRAW);

	glGenBuffers(1,&viewport.tex);
	glBindBuffer(GL_ARRAY_BUFFER,viewport.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8,viewportTex,GL_STATIC_DRAW);

	glGenVertexArrays(1,&viewport.vao);
	glBindVertexArray(viewport.vao);

	glBindBuffer (GL_ARRAY_BUFFER, viewport.pos);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer (GL_ARRAY_BUFFER, viewport.tex);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);
	glEnableVertexAttribArray(1);
}

void ViewPort::deleteShader()
{
	if(shaderViewPort) shaderViewPort=nullptr;

	glDeleteBuffers(1,&viewport.vao);
	glDeleteBuffers(1,&viewport.tex);
	glDeleteVertexArrays(1,&viewport.pos);
}

void ViewPort::draw()
{
	if (! isAlive)
		return;
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	shaderViewPort->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	shaderViewPort->setUniform("transparency",transparency);
	shaderViewPort->setUniform("noColor",noColor);
	shaderViewPort->setUniform("fader", fader.getInterstate() );

	glBindVertexArray(viewport.vao);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	StateGL::disable(GL_BLEND);
	shaderViewPort->unuse();
}
