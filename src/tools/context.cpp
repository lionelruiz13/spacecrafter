#include "context.hpp"
#include "draw_helper.hpp"
#include "EntityCore/EntityCore.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "EntityCore/Resource/SetMgr.hpp"
#include "EntityCore/Tools/CaptureMetrics.hpp"
#include <atomic>

ShadowData::ShadowData() :
    uniform(*Context::instance->uniformMgr), shadowMat(*Context::instance->uniformMgr)
{
    auto &vkmgr = *VulkanMgr::instance;
    if (!Context::instance->shadowLayout) {
        Context::instance->shadowLayout = std::make_unique<PipelineLayout>(vkmgr);
        {
            auto &layout = *Context::instance->shadowLayout;
            layout.setUniformLocation(VK_SHADER_STAGE_COMPUTE_BIT, 0);
            layout.setImageLocation(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
            layout.setImageLocation(2);
            layout.buildLayout();
            layout.build();
        }
        Context::instance->traceLayout = std::make_unique<PipelineLayout>(vkmgr);
        {
            auto &layout = *Context::instance->traceLayout;
            layout.setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // mat3
            layout.buildLayout();
            layout.build();
        }
    }
    set = std::make_unique<Set>(vkmgr, *Context::instance->setMgr, Context::instance->shadowLayout.get());
    set->bindUniform(uniform, 0);
    set->bindImage(*Context::instance->shadowTrace, 1);
    traceSet = std::make_unique<Set>(vkmgr, *Context::instance->setMgr, Context::instance->traceLayout.get());
    traceSet->bindUniform(shadowMat, 0);
}

ShadowData::~ShadowData()
{
}

void ShadowData::init(VkImageView target)
{
    set->bindStorageImage(target, 2);
}

void ShadowData::compute(VkCommandBuffer cmd)
{
    Context::instance->shadowPipelines[static_cast<int32_t>(radius)].bind(cmd);
    Context::instance->shadowLayout->bindSet(cmd, *set, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
    vkCmdDispatch(cmd, Context::instance->shadowRes/SHADOW_LOCAL_SIZE, 1, 1);
}

Context::Context()
{
    instance = this;
}

void Context::nextTick()
{
    if (!shadow_ready) {
        shadow_ready = (shadowPipelineSegmentBuilt.load(std::memory_order_acquire) == SHADOW_PIPELINE_BUILD_THREAD);
    }
}

void Context::initShadowStructures()
{
    shadow = std::make_unique<Texture>(*VulkanMgr::instance, TextureInfo{.width=(int) shadowRes, .height=(int) shadowRes, .nbChannels=1, .arrayLayers=maxShadowCast, .usage=VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, .format=VK_FORMAT_R8_UNORM, .name="Projected shadows"});
    shadow->use();
    shadowView.reserve(maxShadowCast);
    shadowData = new ShadowData[maxShadowCast];
    for (uint32_t i = 0; i < maxShadowCast; ++i) {
        shadowView.push_back(shadow->createView(0, 1, i, 1));
        shadowData[i].init(shadowView.back());
    }
    const uint32_t maxRadius = std::min(shadowRes / 2, MAX_RADIUS_HARD_LIMIT+2U) - 1U;
    shadowPipelines = std::allocator<ComputePipeline>{}.allocate(maxRadius);
    uint32_t offset = 0;
    for (size_t i = 0; i < SHADOW_PIPELINE_BUILD_THREAD;) {
        const uint32_t begin = offset;
        offset = maxRadius * ++i / SHADOW_PIPELINE_BUILD_THREAD;
        std::thread(&Context::buildShadowPipeline, this, begin, offset).detach();
    }
}

void Context::buildShadowPipeline(uint32_t begin, uint32_t end)
{
    auto &vkmgr = *VulkanMgr::instance;
    const auto layout = shadowLayout.get();
    const auto pipelines = shadowPipelines;
    const auto res = shadowRes;
    do {
        auto pipeline = new (pipelines+begin) ComputePipeline(vkmgr, layout);
        pipeline->bindShader("shadow.comp.spv");
        pipeline->setSpecializedConstant(0, begin);
        pipeline->setSpecializedConstant(1, res);
        pipeline->build();
    } while (++begin < end);
    shadowPipelineSegmentBuilt.fetch_add(1, std::memory_order_release);
}

Context::~Context()
{
    instance = nullptr;
    helper.reset();
    for (auto p : pipelineArray)
        delete[] p;
    for (auto v : shadowView) {
        vkDestroyImageView(VulkanMgr::instance->refDevice, v, nullptr);
    }
    delete[] shadowData;
    const uint32_t maxRadius = std::min(shadowRes / 2, MAX_RADIUS_HARD_LIMIT+2U) - 1U;
    for (uint32_t i = 0; i < maxRadius; ++i) {
        shadowPipelines[i].~ComputePipeline();
    }
    std::allocator<ComputePipeline>{}.deallocate(shadowPipelines, maxRadius);
}

bool Context::shadow_ready = false;
bool Context::experimental_shadows = false;
bool Context::default_experimental_shadows = false;
