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
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

std::unique_ptr<shaderProgram> Axis::shaderAxis;
std::unique_ptr<VertexArray> Axis::m_AxisGL;

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

	m_AxisGL->fillVertexBuffer(BufferType::POS3D, vecAxisPos);

	Renderer::drawArrays(shaderAxis.get(), m_AxisGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0,2);

	vecAxisPos.clear();

	glLineWidth(1.0);
	glDisable(GL_LINE_SMOOTH);
}

void Axis::computeAxis(const Projector* prj, const Mat4d& mat)
{

	Vec3d posAxis = prj->sVertex3v(0, 0,  1.4 * body->radius, mat);
	insert_vec3(vecAxisPos,posAxis);

	posAxis = prj->sVertex3v(0, 0,  -1.4 * body->radius, mat);
	insert_vec3(vecAxisPos,posAxis);
}

// Calculate the angle of the axis on the screen
void Axis::computeAxisAngle(const Projector* prj, const Mat4d& mat) {

	//First point of the axis
	Vec3d win;
	Vec3d v(0,0,1000);
	prj->projectCustom(v, win, mat);

	//Second point of the axis
	Vec3d win2;
	prj->projectCustom(v*-1000, win2, mat);

	//Vector from the second point to the first point
	Vec2d axis;
	axis[0] = win[0]-win2[0];
	axis[1] = win[1]-win2[1];

	//calculate angle of the axis vector
	axisAngle = atan(axis[0]/axis[1]);

	//Fix angle in opposite direction
	if (axis[1] < 0) {
		if (axisAngle < 0) {
			axisAngle -= M_PI;
		} else {
			axisAngle += M_PI;
		}
	}
}

void Axis::createSC_context()
{
	shaderAxis = std::make_unique<shaderProgram>();
	shaderAxis->init( "body_Axis.vert", "body_Axis.frag");
	shaderAxis->setUniformLocation({"MVP", "Color"});

	m_AxisGL = std::make_unique<VertexArray>();
	m_AxisGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
}
