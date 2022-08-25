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

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sstream>

#include "appModule/appDraw.hpp"
#include "tools/log.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include "tools/app_settings.hpp"

#include "tools/context.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"

AppDraw::AppDraw()
{}


AppDraw::~AppDraw()
{}

void AppDraw::init(unsigned int _width, unsigned int _height)
{
    width=_width;
    height=_height;
	m_radius = std::min(width, height)/2;
	m_decalage_x = (width  - std::min(width, height))/2;
	m_decalage_y = (height - std::min(width, height))/2;
}

void AppDraw::createSC_context()
{
    Context &context = *Context::instance;
    VulkanMgr &vkmgr = *VulkanMgr::instance;

    assert(cmds.empty());
    layoutEmpty = std::make_unique<PipelineLayout>(vkmgr);
	layoutEmpty->build();

    int radius = m_radius;
    Vec2i decalage {m_decalage_x + m_radius, m_decalage_y + m_radius};

	float points[] = {
        // points top left, top right, bottom left, bottom right
        -1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f,
        // triangle top left, top right, bottom right, bottom left
        -1, 0, -1, -1, 0, -1,
        0, -1, 1, -1, 1, 0,
        1, 0, 1, 1, 0, 1,
        0, 1, -1, 1, -1, 0
    };
	m_viewportGL = std::make_unique<VertexArray>(vkmgr, sizeof(Vec2f));
	m_viewportGL->createBindingEntry(sizeof(Vec2f));
    m_viewportGL->addInput(VK_FORMAT_R32G32_SFLOAT);

    vertexBuffer = m_viewportGL->createBuffer(0, 16, context.globalBuffer.get());
    memcpy(context.transfer->planCopy(vertexBuffer->get()), points, vertexBuffer->get().size);

    pipelineViewportShape = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_FOREGROUND, layoutEmpty.get());
    pipelineViewportShape->setDepthStencilMode(VK_FALSE, VK_FALSE);
    pipelineViewportShape->setBlendMode(BLEND_SRC_ALPHA);
    pipelineViewportShape->bindShader("viewportShape.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipelineViewportShape->bindShader("viewportShape.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineViewportShape->setSpecializedConstant(0, &radius, sizeof(radius));
    pipelineViewportShape->setSpecializedConstant(1, &decalage[0], sizeof(int));
    pipelineViewportShape->setSpecializedConstant(2, &decalage[1], sizeof(int));
    pipelineViewportShape->bindVertex(*m_viewportGL);
    pipelineViewportShape->build();

    pipelineColorInverse = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_FOREGROUND, layoutEmpty.get());
	pipelineColorInverse->setDepthStencilMode(VK_FALSE, VK_FALSE);
    VkPipelineColorBlendAttachmentState blend = BLEND_NONE;
    blend.blendEnable = VK_TRUE;
    blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    pipelineColorInverse->setBlendMode(blend);
    pipelineColorInverse->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	pipelineColorInverse->bindShader("colorInverse.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipelineColorInverse->bindShader("colorInverse.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineColorInverse->bindVertex(*m_viewportGL);
	pipelineColorInverse->build();

    cmds.resize(6);
    context.cmdInfo.commandBufferCount = 6;
    vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds.data());
    for (int i = 0; i < 3; ++i) {
        FrameMgr &frame = *context.frame[i];

        VkCommandBuffer cmd1 = cmds[i];
        frame.begin(cmd1, PASS_FOREGROUND);
        pipelineViewportShape->bind(cmd1);
        const VkDeviceSize offset = vertexBuffer->get().offset + 8 * sizeof(float);
        vkCmdBindVertexBuffers(cmd1, 0, 1, &vertexBuffer->get().buffer, &offset);
        vkCmdDraw(cmd1, 12, 1, 0, 0);
        frame.compile(cmd1);

        cmd1 = cmds[i + 3];
        frame.begin(cmd1, PASS_FOREGROUND);
        pipelineColorInverse->bind(cmd1);
        VertexArray::bind(cmd1, vertexBuffer->get());
        vkCmdDraw(cmd1, 4, 1, 0, 0);
        frame.compile(cmd1);
    }
}

//! Fill with black around the circle
void AppDraw::drawViewportShape()
{
    Context &context = *Context::instance;
    context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx], PASS_FOREGROUND);
}

void AppDraw::drawColorInverse()
{
    Context &context = *Context::instance;
    context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx + 3], PASS_FOREGROUND);
}

void AppDraw::setLineWidth(float w)
{
    m_lineWidth = (w > 0.5f) ? w : 0.5f;
    Pipeline::setDefaultLineWidth(m_lineWidth);
}

void AppDraw::initSplash()
{
    Context &context = *Context::instance;
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    if (vkmgr.getSwapchainView().empty()) {
        cLog::get()->write("No swapchain available, skip splash screen.", LOG_TYPE::L_DEBUG);
    }
    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
    layout->buildLayout();
    layout->build();

    // This is not the main SpaceCrafter loop, don't invoke s_texture mechanics
    texture = std::make_unique<Texture>(vkmgr, *context.stagingMgr, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, std::string("splash/spacecrafter.png"));
    texture->init();
    Set set(vkmgr, *context.setMgr, layout.get()); // If not temporary, not destroyed
    set.bindTexture(*texture, 0);
    set.update();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_FOREGROUND, layout.get());
    pipeline->setBlendMode(BLEND_NONE);
    pipeline->setDepthStencilMode();
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    pipeline->bindShader("splash.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->bindShader("splash.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline->build();

    context.lastFrameIdx = 2;
    vkAcquireNextImageKHR(vkmgr.refDevice, vkmgr.getSwapchain(), UINT32_MAX, context.semaphores[context.lastFrameIdx + 3], VK_NULL_HANDLE, &context.frameIdx);
    vkResetFences(vkmgr.refDevice, 1, &context.fences[context.frameIdx]);

    FrameMgr &frame = *context.frame[context.frameIdx];
    int cmdIdx = frame.create(1);
    frame.setName(cmdIdx, "splash screen");
    VkCommandBuffer &cmd = frame.begin(cmdIdx, PASS_FOREGROUND);
    pipeline->bind(cmd);
    layout->bindSet(cmd, set);
    vkCmdDraw(cmd, 4, 1, 0, 0);
    frame.compile();
    frame.toExecute(cmd, PASS_FOREGROUND);
    auto mainCmd = context.frame[context.frameIdx]->preBegin();
    texture->use(mainCmd, true);
    context.transfer->copy(mainCmd);
    context.transferSync->placeBarrier(mainCmd);
    context.frame[context.frameIdx]->postBegin();
    context.frame[context.frameIdx]->submitInline();
    context.transfer = context.transfers[context.frameIdx].get();
}
