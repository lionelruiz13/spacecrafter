#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "tools/context.hpp"
#include "tools/s_texture.hpp"
#include "coreModule/projector.hpp"
#include "ojmModule/objl.hpp"
#include "atm_ext.hpp"

class AtmExt::_dataSet {
public:
    _dataSet() :
        layout(*VulkanMgr::instance),
        pipeline(*VulkanMgr::instance, *Context::instance->render, PASS_BACKGROUND),
        pipelineNoDepth(*VulkanMgr::instance, *Context::instance->render, PASS_BACKGROUND)
    {
        layout.setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0);
        layout.setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
        layout.buildLayout();
        layout.build();
        auto tmp = BLEND_SRC_ALPHA;
        tmp.colorBlendOp = VK_BLEND_OP_MAX;

        pipeline.bindLayout(layout);
        pipeline.setCullMode(true);
        pipeline.setFrontFace();
        pipeline.bindVertex(*Context::instance->ojmVertexArray);
        pipeline.setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
        pipeline.setTessellationState(3);
        pipeline.removeVertexEntry(1);
        pipeline.removeVertexEntry(2);
        pipeline.setBlendMode(tmp);
        pipeline.bindShader("atm.vert.spv");
        pipeline.bindShader("atm.tesc.spv");
        pipeline.bindShader("atm.tese.spv");
        pipeline.bindShader("atm.frag.spv");
        pipeline.build("AtmExt");

        pipelineNoDepth.setDepthStencilMode();
        pipelineNoDepth.bindLayout(layout);
        pipelineNoDepth.setCullMode(true);
        pipelineNoDepth.setFrontFace();
        pipelineNoDepth.bindVertex(*Context::instance->ojmVertexArray);
        pipelineNoDepth.setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
        pipelineNoDepth.setTessellationState(3);
        pipelineNoDepth.removeVertexEntry(1);
        pipelineNoDepth.removeVertexEntry(2);
        pipeline.setBlendMode(tmp);
        pipelineNoDepth.bindShader("atm.vert.spv");
        pipelineNoDepth.bindShader("atm.tesc.spv");
        pipelineNoDepth.bindShader("atm.tese.spv");
        pipelineNoDepth.bindShader("atm.frag.spv");
        pipelineNoDepth.build("AtmExt noDepth");
    }
    ~_dataSet() = default;
    PipelineLayout layout;
    Pipeline pipeline, pipelineNoDepth;
    bool enabled = true;
};

std::weak_ptr<AtmExt::_dataSet> AtmExt::_shared;

AtmExt::AtmExt(Body *parent, ObjL *obj, const std::string &model) :
    uniform(*Context::instance->uniformMgr), parent(*parent), obj(obj), texture(model, TEX_LOAD_TYPE_PNG_ALPHA)
{
    if (texture.getTexture().getTextureSize() < 32)
        return;
    shared = _shared.lock();
    if (!shared)
        _shared = shared = std::make_shared<_dataSet>();
    set = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, &shared->layout, -1, true, true);
    set->bindUniform(uniform, 0);
    set->bindTexture(texture.getTexture(), 1);
    enabled = true;
}

AtmExt::~AtmExt()
{
}

void AtmExt::draw(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav, const Mat4f &mat, const Vec3f &sunPos, const Vec3f &bodyPos, float planetOneMinusOblateness, const Vec2i &TesParam, float radius, float atmRadius, float screen_sz, bool depthTest)
{
    if (enabled) {
        uniform->ModelViewMatrix = mat;
        uniform->sunPos = sunPos;
        uniform->planetRadius = radius;
        uniform->bodyPos = bodyPos;
        uniform->planetOneMinusOblateness = planetOneMinusOblateness;
        uniform->clipping_fov = prj->getClippingFov();
        uniform->atmRadius = atmRadius;
        uniform->TesParam = TesParam;
        uniform->atmAlpha = 1; // Apply fader here
        if (depthTest)
            shared->pipeline.bind(cmd);
        else
            shared->pipelineNoDepth.bind(cmd);
        shared->layout.bindSet(cmd, *set);
        obj->draw(cmd, screen_sz);
    }
}
