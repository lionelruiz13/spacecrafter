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

#include "tools/vecmath.hpp"
#include <vector>
#include <memory>

#include "vulkanModule/Context.hpp"

class Body;
class Projector;
class VertexArray;
class PipelineLayout;
class Pipeline;
class Uniform;
class Buffer;

class Axis {
public:

	Axis() = delete;
	Axis(const Axis&)=delete;
	Axis(Body * body);

	void setFlagAxis(bool b);

	void drawAxis(const Projector* prj, const Mat4d& mat);

	void computeAxis(const Projector* prj, const Mat4d& mat);

	double getAngle() {
		return axisAngle;
	}

	void computeAxisAngle(const Projector* prj, const Mat4d& mat);

	static void createSC_context(ThreadContext *context);
private:
	int commandIndex = -1;
	Body * body;
	std::unique_ptr<VertexArray> m_AxisGL;
	std::unique_ptr<Set> set;
	std::unique_ptr<Uniform> uMat;
	Mat4f *MVP;
	static VirtualSurface *surface;
	static SetMgr *setMgr;
	static CommandMgr *cmdMgr;

	static Uniform *uColor;
	static VertexArray *vertexModel;
	static Pipeline *pipeline;
	static PipelineLayout *layout;

	double axisAngle;
	std::vector<float> vecAxisPos;
	static Buffer *bdrawaxis;
	static int *drawaxis;  // display or not Body axis
	static bool actualdrawaxis;
};

#endif // _AXIS_HPP_
