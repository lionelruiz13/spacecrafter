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
#include "vulkanModule/VertexArray.hpp"


#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/ResourceTracker.hpp"

VertexArray *Hints::m_HintsGL;
Pipeline *Hints::pipeline;
PipelineLayout *Hints::layout;
const int Hints::nbrFacets = 24;
const int Hints::hintCircleRadius = 8;
ThreadContext *Hints::context;

Hints::Hints(Body * _body)
{
	body = _body;
	vertex = std::make_unique<VertexArray>(*m_HintsGL);
	vertex->build(nbrFacets);

	set = std::make_unique<Set>(context->surface, context->setMgr, layout);
	uColor = std::make_unique<Uniform>(context->surface, sizeof(*pColor));
	pColor = static_cast<typeof(pColor)>(uColor->data);
	uFader = std::make_unique<Uniform>(context->surface, sizeof(*pFader));
	pFader = static_cast<typeof(pFader)>(uFader->data);
	set->bindUniform(uColor.get(), 0);
	set->bindUniform(uFader.get(), 1);

	CommandMgr *cmdMgr = context->commandMgr;
	commandIndex = cmdMgr->getCommandIndex();
	cmdMgr->init(commandIndex);
	cmdMgr->beginRenderPass(renderPassType::DEFAULT);
	cmdMgr->bindPipeline(pipeline);
	cmdMgr->bindVertex(vertex.get());
	cmdMgr->bindSet(layout, context->global->globalSet);
	cmdMgr->bindSet(layout, set.get(), 1);
	cmdMgr->draw(nbrFacets);

	cmdMgr->compile();
}

void Hints::createSC_context(ThreadContext *_context)
{
	context = _context;
	// shaderHints = std::make_unique<shaderProgram>();
	// shaderHints->init( "bodyHints.vert", "bodyHints.frag");
	// shaderHints->setUniformLocation({"Color", "fader"});

	m_HintsGL = context->global->tracker->track(new VertexArray(context->surface, context->commandMgr));
	m_HintsGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STREAM);
	//m_HintsGL->build(nbrFacets);

	layout = context->global->tracker->track(new PipelineLayout(context->surface));
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->buildLayout();
	layout->build();

	pipeline = context->global->tracker->track(new Pipeline(context->surface, layout));
	pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	pipeline->bindShader("bodyHints.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("bodyHints.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(m_HintsGL);
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
	computeHints();

	// shaderHints->use();
	// shaderHints->setUniform("Color", body->myColor->getLabel());
	// shaderHints->setUniform("fader", hint_fader.getInterstate() );
	*pColor = body->myColor->getLabel();
	*pFader = hint_fader.getInterstate();

	vertex->fillVertexBuffer(BufferType::POS2D, vecHintsPos);
	vertex->update();

	// m_HintsGL->bind();
	// glDrawArrays(GL_LINE_LOOP,0,nbrFacets);
	// m_HintsGL->unBind();
	// shaderHints->unuse();
	context->commandMgr->setSubmission(commandIndex);
	//Renderer::drawArrays(shaderHints.get(), m_HintsGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,0,nbrFacets);

	vecHintsPos.clear();
}

void Hints::computeHints()
{

	Vec3d pos = body->screenPos;
	float angle;

	for (int i = 0; i < nbrFacets; i++) {
		angle = 2.0f*M_PI*i/nbrFacets;
		vecHintsPos.push_back( pos[0] + hintCircleRadius * sin(angle) );
		vecHintsPos.push_back( pos[1] + hintCircleRadius * cos(angle) );
	}
}

void Hints::updateShader(double delta_time)
{
	hint_fader.update(delta_time);
}
