/*
 * Spacecrafter astronomy simulation and visualization
 * Copyright (C) 2026
 */

#include "appModule/blackhole_lensing.hpp"

#include "tools/context.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

#include <algorithm>

std::mutex BlackHoleLensing::stateMutex;
BlackHoleLensing::LensState BlackHoleLensing::pendingState;

BlackHoleLensing::BlackHoleLensing(const std::vector<std::unique_ptr<Texture>> &sceneTextures,
                                   float viewportWidth,
                                   float viewportHeight) :
    viewportWidth(viewportWidth),
    viewportHeight(viewportHeight)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    vertexArray = std::make_unique<VertexArray>(vkmgr);
    vertexArray->createBindingEntry(2 * sizeof(float));
    vertexArray->addInput(VK_FORMAT_R32G32_SFLOAT);
    vertexBuffer = vertexArray->createBuffer(0, 4, context.tinyMgr.get());
    float *points = static_cast<float *>(context.tinyMgr->getPtr(vertexBuffer->get()));
    const float vertices[8] = {-1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f};
    std::copy(vertices, vertices + 8, points);

    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
    layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    layout->buildLayout();
    layout->build();

    uniform = std::make_unique<SharedBuffer<LensUniform>>(*context.uniformMgr);
    sets.reserve(sceneTextures.size());
    for (const auto &texture : sceneTextures) {
        auto set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
        set->bindTexture(*texture, 0);
        set->bindUniform(uniform, 1);
        sets.push_back(std::move(set));
    }

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_LENS, layout.get());
    pipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
    pipeline->setBlendMode(BLEND_NONE);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    pipeline->bindVertex(*vertexArray);
    pipeline->bindShader("blackhole_lensing.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->bindShader("blackhole_lensing.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline->build("Black hole gravitational lensing");

    for (int i = 0; i < 3; ++i) {
        commands[i] = context.frame[i]->create(1);
        context.frame[i]->setName(commands[i], "Black hole lens composition");
    }
}

BlackHoleLensing::~BlackHoleLensing() = default;

void BlackHoleLensing::beginFrame()
{
    std::lock_guard<std::mutex> lock(stateMutex);
    pendingState.active = false;
}

void BlackHoleLensing::submit(const Vec2f &center, float eventRadius, float strength, bool distortionEnabled)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    if (!pendingState.active || eventRadius > pendingState.eventRadius) {
        pendingState.center = center;
        pendingState.eventRadius = eventRadius;
        pendingState.strength = strength;
        pendingState.distortionEnabled = distortionEnabled;
        pendingState.active = true;
    }
}

void BlackHoleLensing::draw()
{
    Context &context = *Context::instance;
    const int frameIdx = context.frameIdx;
    if (frameIdx < 0 || frameIdx >= static_cast<int>(sets.size()))
        return;

    LensState state;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        state = pendingState;
    }

    uniform->get().centerRadiusStrength = Vec4f(
        state.center[0], viewportHeight - state.center[1], state.eventRadius, state.strength);
    uniform->get().viewportActive = Vec4f(viewportWidth, viewportHeight,
                                          state.active ? 1.f : 0.f,
                                          state.distortionEnabled ? 1.f : 0.f);

    FrameMgr &frame = *context.frame[frameIdx];
    VkCommandBuffer cmd = frame.begin(commands[frameIdx], PASS_LENS);
    pipeline->bind(cmd);
    layout->bindSet(cmd, *sets[frameIdx]);
    vertexBuffer->bind(cmd);
    vkCmdDraw(cmd, 4, 1, 0, 0);
    frame.compile(cmd);
    frame.toExecute(cmd, PASS_LENS);
}
