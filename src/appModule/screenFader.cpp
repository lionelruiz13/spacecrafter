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
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

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

void ScreenFader::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	// point en haut a gauche
	// point en haut a droite
	// point en bas à gauche
	// point en bas à droite
	float points[8] = {-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f};

	m_screenGL = std::make_unique<VertexArray>(vkmgr);
	m_screenGL->createBindingEntry(2 * sizeof(float));
	m_screenGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	vertex = m_screenGL->createBuffer(0, 4, context.globalBuffer.get());
	memcpy(context.transfer->planCopy(vertex->get()), points, 8 * sizeof(float));

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float));
	layout->build();

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_FOREGROUND, layout.get());
	pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
	pipeline->bindShader("screenFader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipeline->bindShader("screenFader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->bindVertex(*m_screenGL);
	pipeline->build();

	for (int i = 0; i < 3; ++i)
		cmds[i] = context.frame[i]->create(1);
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
	if (intensity==0)
		return;
	Context &context = *Context::instance;
	VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_FOREGROUND);
	pipeline->bind(cmd);
	vertex->bind(cmd);
	layout->pushConstant(cmd, 0, &intensity);
	vkCmdDraw(cmd, 4, 1, 0, 0);
	context.frame[context.frameIdx]->compile(cmd);
	context.frame[context.frameIdx]->toExecute(cmd, PASS_FOREGROUND);
}
