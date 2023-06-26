#include "context.hpp"
#include "draw_helper.hpp"
#include "EntityCore/EntityCore.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "EntityCore/Resource/SetMgr.hpp"
#include "EntityCore/Tools/CaptureMetrics.hpp"

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
    pipeline = std::make_unique<ComputePipeline>(vkmgr, Context::instance->shadowLayout.get());
    pipeline->bindShader("shadow.comp.spv");
    pipeline->setSpecializedConstant(0, Context::instance->shadowRes);
    pipeline->setSpecializedConstant(1, Context::instance->shadowRes);
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
    pipeline->bind(cmd);
    Context::instance->shadowLayout->bindSet(cmd, *set, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
    vkCmdDispatch(cmd, Context::instance->shadowRes/SHADOW_LOCAL_SIZE, 1, 1);
}

Context::Context()
{
    instance = this;
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
}

bool Context::experimental_shadows = false;
bool Context::default_experimental_shadows = false;
