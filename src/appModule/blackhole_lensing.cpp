/*
 * Spacecrafter astronomy simulation and visualization
 * Copyright (C) 2026
 */

#include "appModule/blackhole_lensing.hpp"
#include "bodyModule/blackhole_trajectory_lut.hpp"

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
Texture *BlackHoleLensing::pendingLutUpload = nullptr;

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
    layout->setImageLocation(2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    layout->buildLayout();
    layout->build();

    uniform = std::make_unique<SharedBuffer<LensUniform>>(*context.uniformMgr);
    sets.reserve(sceneTextures.size());
    inputAttachmentInfo.reserve(sceneTextures.size());
    for (const auto &texture : sceneTextures) {
        auto set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get(), -1, false);
        set->bindTexture(*texture, 0);
        set->bindUniform(uniform, 1);
        inputAttachmentInfo.push_back({VK_NULL_HANDLE, texture->getView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        set->getWrites().emplace_back(VkWriteDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, VK_NULL_HANDLE, 2, 0, 1,
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &inputAttachmentInfo.back(), nullptr, nullptr});
        sets.push_back(std::move(set));
    }

    copyPipeline = createScenePipeline(false);
    diskModelUniforms.resize(sceneTextures.size());
    lutSets.resize(sceneTextures.size());

    for (int i = 0; i < 3; ++i) {
        commands[i] = context.frame[i]->create(1);
        context.frame[i]->setName(commands[i], "Black hole lens composition");
        diskCommands[i] = context.frame[i]->create(1);
        context.frame[i]->setName(diskCommands[i], "Black hole disk lens composition");
    }
}


std::unique_ptr<Pipeline> BlackHoleLensing::createScenePipeline(bool lensing)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    auto result = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_LENS, layout.get());
    result->setDepthStencilMode(VK_FALSE, VK_FALSE);
    result->setBlendMode(BLEND_NONE);
    result->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    result->bindVertex(*vertexArray);
    result->bindShader("blackhole_lensing.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    result->bindShader("blackhole_lensing.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    result->setSpecializedConstant(0, static_cast<VkBool32>(lensing));
    result->build(lensing ? "Black hole gravitational lensing" : "Scene copy");
    return result;

}

void BlackHoleLensing::ensureDiskResources(bool trajectoriesNeeded)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    if (trajectoriesNeeded && !trajectoryLut) {
        auto trajectories = blackhole::makeTrajectoryLut();
        TextureInfo trajectoryInfo;
        trajectoryInfo.width = blackhole::lutWidth;
        trajectoryInfo.height = blackhole::lutHeight;
        trajectoryInfo.nbChannels = blackhole::lutChannels;
        trajectoryInfo.channelSize = sizeof(float);
        trajectoryInfo.content = trajectories.data();
        trajectoryInfo.mgr = context.stagingMgr.get();
        trajectoryInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        trajectoryLut = std::make_unique<Texture>(vkmgr, trajectoryInfo);
        trajectoryLut->rename("Schwarzschild trajectory LUT");
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            pendingLutUpload = trajectoryLut.get();
        }
    }
    if (lutPipeline)
        return;
    const VkPipelineColorBlendAttachmentState premultipliedBlend {
        VK_TRUE,
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    lutLayout = std::make_unique<PipelineLayout>(vkmgr);
    VkSamplerCreateInfo lutSampler = PipelineLayout::DEFAULT_SAMPLER;
    lutSampler.magFilter = VK_FILTER_LINEAR;
    lutSampler.minFilter = VK_FILTER_LINEAR;
    lutSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    lutSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    lutSampler.anisotropyEnable = VK_FALSE;
    lutSampler.maxLod = 0.f;
    lutLayout->setTextureLocation(0, &lutSampler);
    lutLayout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
    lutLayout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    lutLayout->buildLayout();
    lutLayout->build();
    lutPipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_BLACKHOLE_DISK_LENS, lutLayout.get());
    lutPipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
    lutPipeline->setBlendMode(premultipliedBlend);
    lutPipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    lutPipeline->bindVertex(*vertexArray);
    lutPipeline->bindShader("blackhole_lensing.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    lutPipeline->bindShader("blackhole_disk_lut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    lutPipeline->setSpecializedConstant(8, Context::projectionType);
    lutPipeline->build("Black hole LUT disk reconstruction");

}

BlackHoleLensing::~BlackHoleLensing()
{
    std::lock_guard<std::mutex> lock(stateMutex);
    if (pendingLutUpload == trajectoryLut.get())
        pendingLutUpload = nullptr;
}

void BlackHoleLensing::recordTransfer(VkCommandBuffer cmd)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    if (pendingLutUpload && pendingLutUpload->use(cmd, true))
        pendingLutUpload = nullptr;
}

void BlackHoleLensing::beginFrame()
{
    std::lock_guard<std::mutex> lock(stateMutex);
    pendingState.active = false;
    pendingState.disks.clear();
}

void BlackHoleLensing::submitDiskModel(const DiskModel &model, Texture &density, double distance)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    pendingState.disks.push_back({model, &density, distance});
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
    const bool lensing = state.active && state.distortionEnabled;
    if (lensing && !pipeline)
        pipeline = createScenePipeline(true);
    (lensing ? pipeline : copyPipeline)->bind(cmd);
    layout->bindSet(cmd, *sets[frameIdx]);
    vertexBuffer->bind(cmd);
    vkCmdDraw(cmd, 4, 1, 0, 0);
    frame.compile(cmd);
    frame.toExecute(cmd, PASS_LENS);

    if (state.disks.empty())
        return;

    const bool trajectoriesNeeded = std::any_of(state.disks.begin(), state.disks.end(), [](const auto &disk) {
        return disk.model.camera[3] > 0.f;
    });
    ensureDiskResources(trajectoriesNeeded);
    std::stable_sort(state.disks.begin(), state.disks.end(), [](const auto &a, const auto &b) {
        return a.distance > b.distance;
    });
    VkCommandBuffer diskCmd = frame.begin(diskCommands[frameIdx], PASS_BLACKHOLE_DISK_LENS);
    auto &models = diskModelUniforms[frameIdx];
    auto &diskSets = lutSets[frameIdx];
    lutPipeline->bind(diskCmd);
    vertexBuffer->bind(diskCmd);
    for (size_t i = 0; i < state.disks.size(); ++i) {
        if (i == models.size()) {
            models.push_back(std::make_unique<SharedBuffer<DiskModel>>(*context.uniformMgr));
            auto set = std::make_unique<Set>(*VulkanMgr::instance, *context.setMgr, lutLayout.get(), -1, false);
            set->bindUniform(models.back(), 2);
            diskSets.push_back(std::move(set));
        }
        models[i]->get() = state.disks[i].model;
        auto &set = *diskSets[i];
        // This frame's fence has completed before draw(); no in-flight set is changed.
        set.unGet();
        set.bindTexture(trajectoryLut ? *trajectoryLut : *state.disks[i].density, 0);
        set.bindTexture(*state.disks[i].density, 1);
        lutLayout->bindSet(diskCmd, set);
        vkCmdDraw(diskCmd, 4, 1, 0, 0);
    }
    frame.compile(diskCmd);
    frame.toExecute(diskCmd, PASS_BLACKHOLE_DISK_LENS);
}
