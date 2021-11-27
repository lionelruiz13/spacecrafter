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
#include "bodyModule/hints.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body.hpp"
#include "coreModule/projector.hpp"
#include "bodyModule/body_color.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

std::unique_ptr<VertexArray> Hints::m_HintsGL;
Pipeline *Hints::pipeline;
PipelineLayout *Hints::layout;
const int Hints::nbrFacets = 24;
const int Hints::hintCircleRadius = 8;

Hints::Hints(Body * _body)
{
	body = _body;
	drawData.flag = DRAW_HINT;
	drawData.self = this;
}

void Hints::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	m_HintsGL = std::make_unique<VertexArray>(vkmgr, context.multiVertexArray->alignment);
	m_HintsGL->createBindingEntry(sizeof(Vec2f));
	m_HintsGL->addInput(VK_FORMAT_R32G32_SFLOAT);

	context.layouts.emplace_back(new PipelineLayout(vkmgr));
	layout = context.layouts.back().get();
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec4f));
	layout->build();

	context.pipelines.emplace_back(new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout));
	pipeline = context.pipelines.back().get();
	pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
	pipeline->bindShader("bodyHints.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("bodyHints.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(*m_HintsGL);
	pipeline->build();
}

void Hints::drawHints(const Navigator* nav, const Projector* prj)
{
	if (!hint_fader.getInterstate())
		return;

	// Draw nameI18 + scaling if it's not == 1.
	float tmp = 10.f + body->getOnScreenSize(prj, nav)/2.f; // Shift for nameI18 printing

	Vec4f Color( body->myColor->getLabel(),hint_fader.getInterstate());
	prj->printGravity180(body->planet_name_font, body->screenPos[0], body->screenPos[1], body->getSkyLabel(nav), Color,/*1,*/ tmp, tmp);

	drawHintCircle(nav, prj);
}

void Hints::drawHintCircle(const Navigator* nav, const Projector* prj)
{
	drawData.color = body->myColor->getLabel();
	drawData.color[3] = hint_fader.getInterstate();
	Context::instance->helper->draw(&drawData);
}

int Hints::computeHints(float *&data)
{
	Vec3d pos = body->screenPos;
	float angle;

	for (int i = 0; i < nbrFacets; i++) {
		angle = 2.0f*M_PI*i/nbrFacets;
		// "*(data++) = value;" is similar to "data.push_back(value);"
		*(data++) = pos[0] + hintCircleRadius * sin(angle);
		*(data++) = pos[1] + hintCircleRadius * cos(angle);
	}
	return nbrFacets;
}

void Hints::updateShader(double delta_time)
{
	hint_fader.update(delta_time);
}

void Hints::bind(VkCommandBuffer cmd, const Vec4f &color)
{
	pipeline->bind(cmd);
	layout->bindSet(cmd, *Context::instance->uboSet);
	layout->pushConstant(cmd, 0, &color);
}
