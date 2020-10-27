/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018-2020 of Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "appModule/screenFader.hpp"
#include "vulkanModule/VertexArray.hpp"




#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/CommandMgr.hpp"

ScreenFader::ScreenFader()
{
	intensity = 0.0;
	move_to_mult = 0;
	start_value = 0;
	end_value = 0;
	move_to_mult = 0;
}

ScreenFader::~ScreenFader()
{
}

void ScreenFader::createSC_context(ThreadContext *context)
{
	// point en haut a gauche
	// point en haut a droite
	// point en bas à gauche
	// point en bas à droite
	float points[8] = {-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f};

	m_screenGL = std::make_unique<VertexArray>(context->surface, context->commandMgr);
	m_screenGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	m_screenGL->build(4);
	m_screenGL->fillVertexBuffer(BufferType::POS2D, 8, points);

	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	layout->buildLayout();
	layout->build();

	uniform = std::make_unique<Uniform>(context->surface, sizeof(float));
	pIntensity = static_cast<float *>(uniform->data);

	set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
	set->bindUniform(uniform.get(), 0);

	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	pipeline->setRenderPassCompatibility(renderPassCompatibility::SINGLE_SAMPLE);
	pipeline->bindShader("screenFader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("screenFader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(m_screenGL.get());
	pipeline->build();

	cmdMgr = context->commandMgr;

	resolveCommandIndex = cmdMgr->getCommandIndex();
	cmdMgr->init(resolveCommandIndex);
	cmdMgr->beginRenderPass(renderPassType::SINGLE_SAMPLE_PRESENT, renderPassCompatibility::SINGLE_SAMPLE);
	cmdMgr->compile();

	commandIndex = cmdMgr->getCommandIndex();
	cmdMgr->init(commandIndex);
	cmdMgr->beginRenderPass(renderPassType::SINGLE_SAMPLE_PRESENT, renderPassCompatibility::SINGLE_SAMPLE);
	cmdMgr->bindPipeline(pipeline.get());
	cmdMgr->bindSet(layout.get(), set.get());
	m_screenGL->bind();
	cmdMgr->draw(4);
	cmdMgr->compile();
}

void ScreenFader::update(int delta_time)
{
	if (flag_change_intensity) {
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_change_intensity = 0;
		}
		intensity = start_value - move_to_mult*(start_value-end_value);
	}
}

void ScreenFader::draw()
{
	if (intensity==0) {
		cmdMgr->setSubmission(resolveCommandIndex);
		return;
	}
	*pIntensity = intensity;
	cmdMgr->setSubmission(commandIndex);
}
