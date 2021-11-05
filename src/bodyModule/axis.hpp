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
#include "tools/context.hpp"
#include <vector>
#include <memory>

#include "EntityCore/Resource/SharedBuffer.hpp"

class Body;
class Projector;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;

class Axis {
public:

	Axis() = delete;
	Axis(const Axis&)=delete;
	Axis(Body * body);

	void setFlagAxis(bool b);

	void drawAxis(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat);

	void computeAxis(const Projector* prj, const Mat4d& mat);

	double getAngle() {
		return axisAngle;
	}

	void computeAxisAngle(const Projector* prj, const Mat4d& mat);

	static void createSC_context();
	static void destroySC_context();
private:
	Body * body;
	Vec3f *pPosAxis = nullptr;
	std::unique_ptr<VertexBuffer> m_AxisGL;

	static std::unique_ptr<SharedBuffer<Vec3f>> uColor;
	static std::unique_ptr<VertexArray> vertexModel;
	static std::unique_ptr<Set> set;
	static std::unique_ptr<Pipeline> pipeline;
	static std::unique_ptr<PipelineLayout> layout;

	double axisAngle;
	static bool actualdrawaxis;
};

#endif // _AXIS_HPP_
