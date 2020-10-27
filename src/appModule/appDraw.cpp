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

#include <vulkan/vulkan.h>

#include "appModule/appDraw.hpp"
#include "tools/log.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include "tools/app_settings.hpp"
#include "vulkanModule/VertexArray.hpp"


#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Texture.hpp"
#include "vulkanModule/VirtualSurface.hpp"
#include "vulkanModule/Vulkan.hpp"
#include "vulkanModule/VertexBuffer.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Uniform.hpp"

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
    if (pRadius) {
        *pRadius = m_radius;
        *pDecalage = Vec2i(m_decalage_x, m_decalage_y);
    }
}

void AppDraw::createSC_context(ThreadContext *context)
{
    cmdMgr = context->commandMgr;

    layoutViewportShape = std::make_unique<PipelineLayout>(context->surface);
	layoutViewportShape->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 0);
	layoutViewportShape->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layoutViewportShape->buildLayout();
	layoutViewportShape->build();

    layoutColorInverse = std::make_unique<PipelineLayout>(context->surface);
    layoutColorInverse->build();

    uRadius = std::make_unique<Uniform>(context->surface, sizeof(int));
	pRadius = static_cast<int *>(uRadius->data);
    *pRadius = m_radius;
    uDecalage = std::make_unique<Uniform>(context->surface, sizeof(Vec2i));
	pDecalage = static_cast<Vec2i *>(uDecalage->data);
    *pDecalage = Vec2i(m_decalage_x, m_decalage_y);

    set = std::make_unique<Set>(context->surface, context->setMgr, layoutViewportShape.get());
	set->bindUniform(uRadius.get(), 0);
	set->bindUniform(uDecalage.get(), 1);

	// point en haut a gauche , en haut a droite, en bas à gauche, en bas à droite
	float points[8] = {-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f};

	m_viewportGL = std::make_unique<VertexArray>(context->surface, cmdMgr);
	m_viewportGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
    m_viewportGL->build(4);
	m_viewportGL->fillVertexBuffer(BufferType::POS2D, 8, points);

    pipelineViewportShape = std::make_unique<Pipeline>(context->surface, layoutViewportShape.get());
    pipelineViewportShape->setDepthStencilMode(VK_FALSE, VK_FALSE);
    pipelineViewportShape->setBlendMode(BLEND_SRC_ALPHA);
    pipelineViewportShape->setRenderPassCompatibility(renderPassCompatibility::SINGLE_SAMPLE);
    pipelineViewportShape->bindShader("viewportShape.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipelineViewportShape->bindShader("viewportShape.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineViewportShape->bindVertex(m_viewportGL.get());
    pipelineViewportShape->build();

    pipelineColorInverse = std::make_unique<Pipeline>(context->surface, layoutColorInverse.get());
	pipelineColorInverse->setDepthStencilMode(VK_FALSE, VK_FALSE);
    VkPipelineColorBlendAttachmentState blend = BLEND_NONE;
    blend.blendEnable = VK_TRUE;
    blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    pipelineColorInverse->setBlendMode(blend);
    pipelineColorInverse->setRenderPassCompatibility(renderPassCompatibility::SINGLE_SAMPLE);
	pipelineColorInverse->bindShader("colorInverse.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	pipelineColorInverse->bindShader("colorInverse.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipelineColorInverse->bindVertex(m_viewportGL.get());
	pipelineColorInverse->build();

    commandIndexViewportShape = cmdMgr->initNew(pipelineViewportShape.get(), renderPassType::SINGLE_SAMPLE_DEFAULT, true, renderPassCompatibility::SINGLE_SAMPLE);
    cmdMgr->bindSet(layoutViewportShape.get(), set.get());
    m_viewportGL->bind();
    cmdMgr->draw(4);
    cmdMgr->compile();

    commandIndexColorInverse = cmdMgr->initNew(pipelineColorInverse.get(), renderPassType::SINGLE_SAMPLE_DEFAULT, true, renderPassCompatibility::SINGLE_SAMPLE);
    m_viewportGL->bind();
    cmdMgr->draw(4);
    cmdMgr->compile();
}

//! Fill with black around the circle
void AppDraw::drawViewportShape()
{
    cmdMgr->setSubmission(commandIndexViewportShape);
}

void AppDraw::drawColorInverse()
{
    cmdMgr->setSubmission(commandIndexColorInverse);
}

void AppDraw::setLineWidth(float w)
{
   if (abs(m_lineWidth-w)<0.5f) {
       Pipeline::setDefaultLineWidth(m_lineWidth);
   }
   m_lineWidth = w;
}

void AppDraw::initSplash(ThreadContext *context)
{
    float dataPos[]= {-1.0,1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0};
    float dataTex[]= {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };

    std::unique_ptr<VertexArray> splash = std::make_unique<VertexArray>(context->surface, context->commandMgr);
    splash->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
    splash->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
    splash->build(4);
    splash->fillVertexBuffer(BufferType::POS2D, 8, dataPos);
    splash->fillVertexBuffer(BufferType::TEXTURE, 8, dataTex);
    splash->update();

    std::unique_ptr<PipelineLayout> layout = std::make_unique<PipelineLayout>(context->surface);
    layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
    layout->buildLayout();
    layout->build();

    //Texture texture(context->surface, context->global->textureMgr, AppSettings::Instance()->getUserDir()+"textures/splash/spacecrafter.png", false, false);
    Texture texture(context->surface, context->global->textureMgr, std::string("splash/spacecrafter.png"), false, false);
    Set set(context->surface, context->setMgr, layout.get());
    set.bindTexture(&texture, 0);
    set.update();

    std::unique_ptr<Pipeline> pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
    pipeline->setBlendMode(BLEND_NONE);
    pipeline->setDepthStencilMode();
    pipeline->setRenderPassCompatibility(renderPassCompatibility::SINGLE_SAMPLE);
    pipeline->bindShader("splash.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->bindShader("splash.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline->bindVertex(splash.get());
    pipeline->build();

    // Begin raw draw
    VirtualSurface *master = context->surface;

    // Acquire frame
    VkFence fence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(master->refDevice, &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence.");
    }
    uint32_t frameIndex;
    vkAcquireNextImageKHR(master->refDevice, *context->global->vulkan->assignSwapChain(), UINT64_MAX, VK_NULL_HANDLE, fence, &frameIndex);
    vkWaitForFences(master->refDevice, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(master->refDevice, fence, nullptr);
    // Allocate commandBuffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = master->getCommandPool();

    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(context->surface->refDevice, &allocInfo, &cmd) != VK_SUCCESS) {
        throw std::runtime_error("échec de l'allocation de command buffers!");
    }
    // Begin command
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to start command recording.");
    }

    // Begin renderPass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = master->refRenderPass[static_cast<uint8_t>(renderPassType::SINGLE_PASS)];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = master->swapChainExtent;
    renderPassInfo.framebuffer = master->refSingleSampleFramebuffers[frameIndex];

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.f, 0.f, 0.0f};
    clearValues[1].depthStencil = {0.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    // Bind and draw
    VkDeviceSize offset = 0;
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get());
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout->getPipelineLayout(), 0,  1, set.get(), 0, nullptr);
    vkCmdBindVertexBuffers(cmd, 0, 1, &splash->getVertexBuffer().get(), &offset);
    vkCmdDraw(cmd, 4, 1, 0, 0);
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
    // Draw frame
    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 0;
    if (vkQueueSubmit(master->getQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        std::runtime_error("Error : Failed to submit commands.");
    }
    // Present Frame and release resources
    vkQueueWaitIdle(master->getQueue());
    vkFreeCommandBuffers(master->refDevice, master->getCommandPool(), 1, &cmd);
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = context->global->vulkan->assignSwapChain();
    presentInfo.pImageIndices = &frameIndex;
    presentInfo.pResults = nullptr; // Optionnel
    if (vkQueuePresentKHR(master->getQueue(), &presentInfo) != VK_SUCCESS) {
        std::cerr << "Presentation FAILED\n";
    }
}
