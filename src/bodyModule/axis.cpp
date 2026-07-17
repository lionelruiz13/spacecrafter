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
		pPosAxis = static_cast<Vec3f *>(Context::instance->tinyMgr->getPtr(m_AxisGL->get()));
	}

	computeAxis(prj, mat);

	pipeline->bind(cmd);

	// Push ModelViewMatrix and clipping_fov to shader
	struct {
		Mat4f ModelViewMatrix;
		Vec3f clipping_fov;
	} pushData;
    pushData.ModelViewMatrix = mat.convert();
	pushData.clipping_fov = prj->getClippingFov();
	layout->pushConstant(cmd, 0, &pushData);

	VertexArray::bind(cmd, m_AxisGL->get());
	layout->bindSet(cmd, *set, 0);

	vkCmdDraw(cmd, 2, 1, 0, 0);
}

void Axis::computeAxis(const Projector* prj, const Mat4d& mat)
{
	// Send 3D positions - projection will be done in shader
	// Axis from +Z to -Z at 1.4 * radius distance
	pPosAxis[0] = Vec3f(0, 0, 1.4 * body->radius);
	pPosAxis[1] = Vec3f(0, 0, -1.4 * body->radius);
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
	assert(!vertexModel);

	vertexModel = std::make_unique<VertexArray>(vkmgr, 3*sizeof(float));
	vertexModel->createBindingEntry(3*sizeof(float));
	vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	layout->buildLayout();
	layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4f) + sizeof(Vec3f)); // Push constant: matrix + clipping_fov
	layout->build();

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipeline->setLineWidth(3.0);
	pipeline->bindShader("body_Axis.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	// Set specialization constant for projection type (constant_id = 8)
	pipeline->setSpecializedConstant(8, Context::projectionType);
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
