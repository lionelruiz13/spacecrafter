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

#include "bodyModule/axis.hpp"
#include "bodyModule/body.hpp"
#include "coreModule/projector.hpp"

shaderProgram* Axis::shaderAxis;
DataGL Axis::AxisData;

Axis::Axis(Body * _body)
{
	body = _body;
}

void Axis::drawAxis(const Projector* prj, const Mat4d& mat)
{
	if(!drawaxis)
		return;

	glLineWidth(3.0);
	Vec3f Color(1.0,0.0,0.0);

	shaderAxis->use();

	StateGL::enable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);

	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	shaderAxis->setUniform("MVP",proj*matrix);
	shaderAxis->setUniform("Color", Color);

	computeAxis(prj, mat);

	glBindVertexArray(AxisData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,AxisData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecAxisPos.size(),vecAxisPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_LINE_STRIP, 0,2);

	shaderAxis->unuse();

	vecAxisPos.clear();

	glLineWidth(1.0);
	glDisable(GL_LINE_SMOOTH);
}

void Axis::computeAxis(const Projector* prj, const Mat4d& mat)
{

	Vec3d posAxis = prj->sVertex3v(0, 0,  1.4 * body->radius, mat);
	vecAxisPos.push_back( posAxis[0] );
	vecAxisPos.push_back( posAxis[1] );
	vecAxisPos.push_back( posAxis[2] );

	posAxis = prj->sVertex3v(0, 0,  -1.4 * body->radius, mat);
	vecAxisPos.push_back( posAxis[0] );
	vecAxisPos.push_back( posAxis[1] );
	vecAxisPos.push_back( posAxis[2] );

}

void Axis::createShader()
{
	shaderAxis = new shaderProgram();
	shaderAxis->init( "body_Axis.vert", "body_Axis.frag");
	shaderAxis->setUniformLocation("MVP");
	shaderAxis->setUniformLocation("Color");
	glGenVertexArrays(1,&AxisData.vao);
	glBindVertexArray(AxisData.vao);
	glGenBuffers(1,&AxisData.pos);
	glEnableVertexAttribArray(0);
}

void Axis::deleteShader()
{
	if (shaderAxis != nullptr)
		delete shaderAxis;
	shaderAxis=nullptr;

	glDeleteBuffers(1,&AxisData.pos);
	glDeleteVertexArrays(1,&AxisData.vao);
}
