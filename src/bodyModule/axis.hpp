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
//! \file axis.hpp
//! \brief Draws a body's axis of rotation
//! \author Julien LAFILLE
//! \date april 2018

#ifndef _AXIS_HPP_
#define _AXIS_HPP_

#include "tools/fader.hpp"
#include "tools/stateGL.hpp"
#include "tools/vecmath.hpp"
#include <vector>
#include <memory>

class Body;
class Projector;
class VertexArray;
class shaderProgram;

class Axis {
public:

	Axis() = delete;
	Axis(const Axis&)=delete;
	Axis(Body * body);

	void setFlagAxis(bool b) {
		drawaxis = b;
	}

	void drawAxis(const Projector* prj, const Mat4d& mat);

	void computeAxis(const Projector* prj, const Mat4d& mat);

	double getAngle() {
		return axisAngle;
	}

	void computeAxisAngle(const Projector* prj, const Mat4d& mat);

	static void createSC_context();
private :

	Body * body;
	static std::unique_ptr<shaderProgram> shaderAxis;
	static std::unique_ptr<VertexArray> m_AxisGL;

	double axisAngle;
	std::vector<float> vecAxisPos;
	bool drawaxis = false;  // display or not Body axis

};

#endif // _AXIS_HPP_

