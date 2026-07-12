#include "MeshFamilies.hpp"
#include "experimentalModule/Renderer.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"

const PipelineFamily &MeshFamilies::meshNormal()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        // The old path's map sampler: default + REPEAT on U
        // (bodyShader.cpp createShader, `tmp`).
        VkSamplerCreateInfo mapSampler = PipelineLayout::DEFAULT_SAMPLER;
        mapSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SetContractDesc contract;
        contract.name = "bodyNormal";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},           // globalVertProj
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},         // globalFrag
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // mapTexture
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // eclipse map, default sampler
        };
        contract.expectedSets = 64; // D5 parameter: live MESH modules; pools grow by aggregate if exceeded
        PipelineFamilyDesc desc;
        desc.name = "MESH";
        desc.vertex = Context::instance->ojmVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        desc.sets.push_back(renderer.globalUboContract());
        desc.specValues = {{7, Context::instance->isFloat64Supported}};
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "body_normal.vert.spv", .frag = "body_normal.frag.spv"}}};
        // color.state: FixedState defaults == old shaderNormal (cull on,
        // BLEND_NONE, triangle list, depth test+write on).
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}
