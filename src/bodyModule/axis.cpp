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

#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/ResourceTracker.hpp"
#include "vulkanModule/Buffer.hpp"

Pipeline *Axis::pipeline;
PipelineLayout *Axis::layout;
Uniform *Axis::uColor;
VertexArray *Axis::vertexModel;
VirtualSurface *Axis::surface;
SetMgr *Axis::setMgr;
CommandMgr *Axis::cmdMgr;
Buffer *Axis::bdrawaxis;
int *Axis::drawaxis;
bool Axis::actualdrawaxis = false;

Axis::Axis(Body * _body)
{
	body = _body;

	m_AxisGL = std::make_unique<VertexArray>(*vertexModel);
	m_AxisGL->build(2);

	set = std::make_unique<Set>(surface, setMgr, layout);
	uMat = std::make_unique<Uniform>(surface, sizeof(Mat4f));
	MVP = static_cast<Mat4f *>(uMat->data);
	set->bindUniform(uMat.get(), 0);
	set->bindUniform(uColor, 1);
}

void Axis::setFlagAxis(bool b)
{
	if (actualdrawaxis != b) {
		*drawaxis = static_cast<int>(b);
		bdrawaxis->update();
		actualdrawaxis = b;
	}
}

void Axis::drawAxis(const Projector* prj, const Mat4d& mat)
{
	if (commandIndex == -1) {
		commandIndex = cmdMgr->getCommandIndex();
		cmdMgr->init(commandIndex);
		cmdMgr->beginRenderPass(renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
		cmdMgr->vkIf(bdrawaxis);
		cmdMgr->bindPipeline(pipeline);
		cmdMgr->bindVertex(m_AxisGL.get());
		cmdMgr->bindSet(layout, set.get());
		cmdMgr->draw(2);
		cmdMgr->vkEndIf();
		return;
	}
	if(!actualdrawaxis) {
		cmdMgr->setSubmission(commandIndex, false);
		return;
	}

	//glLineWidth(3.0);

	//glEnable(GL_LINE_SMOOTH);

	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	*MVP = proj*matrix;

	computeAxis(prj, mat);

	m_AxisGL->fillVertexBuffer(BufferType::POS3D, vecAxisPos);

	cmdMgr->setSubmission(commandIndex, false);

	vecAxisPos.clear();

	//glLineWidth(1.0);
	//glDisable(GL_LINE_SMOOTH);
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

void Axis::createSC_context(ThreadContext *context)
{
	surface = context->surface;
	cmdMgr = context->commandMgr;
	setMgr = context->setMgr;

	vertexModel = context->global->tracker->track(new VertexArray(surface, cmdMgr));
	vertexModel->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);

	layout = context->global->tracker->track(new PipelineLayout(context->surface));
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->buildLayout();
	layout->build();

	pipeline = context->global->tracker->track(new Pipeline(context->surface, layout));
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipeline->setLineWidth(3.0);
	pipeline->bindShader("body_Axis.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("body_Axis.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(vertexModel);
	pipeline->build();

	uColor = context->global->tracker->track(new Uniform(surface, sizeof(Vec3f)));
	Vec3f Color(1.0,0.0,0.0);
	*static_cast<Vec3f *>(uColor->data) = Color;

	bdrawaxis = context->global->tracker->track(new Buffer(surface, 4, VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT));
	drawaxis = static_cast<int *>(bdrawaxis->data);
	*drawaxis = static_cast<int>(actualdrawaxis);
	bdrawaxis->update();
}
