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
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

std::unique_ptr<SharedBuffer<Vec3f>> Axis::uColor;
std::unique_ptr<VertexArray> Axis::vertexModel;
std::unique_ptr<Set> Axis::set;
std::unique_ptr<Pipeline> Axis::pipeline;
std::unique_ptr<PipelineLayout> Axis::layout;
bool Axis::actualdrawaxis = false;

Axis::Axis(Body * _body)
{
	body = _body;
}

void Axis::setFlagAxis(bool b)
{
	actualdrawaxis = b;
}

void Axis::drawAxis(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat)
{
	if (!actualdrawaxis)
		return;
	if (!m_AxisGL) {
		m_AxisGL = vertexModel->createBuffer(0, 2, Context::instance->tinyMgr.get());
		pPosAxis = static_cast<typeof(pPosAxis)>(Context::instance->tinyMgr->getPtr(m_AxisGL->get()));
	}

	pipeline->bind(cmd);
	VertexArray::bind(cmd, m_AxisGL->get());
	layout->bindSet(cmd, *set, 0);

	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	Mat4f MVP = proj*matrix;
	layout->pushConstant(cmd, 0, reinterpret_cast<void *>(&MVP));

	computeAxis(prj, mat);

	vkCmdDraw(cmd, 2, 1, 0, 0);
}

void Axis::computeAxis(const Projector* prj, const Mat4d& mat)
{
	pPosAxis[0] = prj->sVertex3v(0, 0,  1.4 * body->radius, mat);
	pPosAxis[1] = prj->sVertex3v(0, 0,  -1.4 * body->radius, mat);
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
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	vertexModel = std::make_unique<VertexArray>(vkmgr, 3*sizeof(float));
	vertexModel->createBindingEntry(3*sizeof(float));
	vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	layout->buildLayout();
	layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4f));
	layout->build();

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipeline->setLineWidth(3.0);
	pipeline->bindShader("body_Axis.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("body_Axis.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(*vertexModel);
	pipeline->build();

	uColor = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
	*uColor = Vec3f{1.0, 0.0, 0.0};

	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	set->bindUniform(uColor, 0);
	set->update();
}

void Axis::destroySC_context()
{
	uColor.reset();
	pipeline.reset();
	layout.reset();
	set.reset();
	vertexModel.reset();
}
