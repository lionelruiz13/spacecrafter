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
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},         // meshFrag (Gen-2 receiver block)
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // mapTexture
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // ShadowService layer array (was: eclipse LUT, retired - shadow-paths.md B4)
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
        // Vertex REUSED (its outputs are shadow-sufficient: Position is the
        // eye-space surface point the projection rows consume); fragment is
        // the Gen-2 receiver port - LUT loop replaced by projected-shadow
        // sampling with per-caster absorbtion (shadow-paths.md B4).
        color.shaderTable = {{0, {.vert = "body_normal.vert.spv", .frag = "bodyMesh.frag.spv"}}};
        // color.state: FixedState defaults == old shaderNormal (cull on,
        // BLEND_NONE, triangle list, depth test+write on).
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}

const PipelineFamily &MeshFamilies::meshTes()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        VkSamplerCreateInfo mapSampler = PipelineLayout::DEFAULT_SAMPLER;
        mapSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SetContractDesc contract;
        contract.name = "bodyTes";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT}, // globalVertProj (body_tes.vert + bodyTes.tese)
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},         // meshFrag (S5 receiver block)
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT}, // meshTescGeom (TesParam)
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // dayTexture
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // nightTexture (NIGHT row)
            {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // specularTexture (NIGHT row)
            {6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // normalTexture (BUMP row)
            {7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 1, mapSampler}, // heightmapTexture (displacement)
            {8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // ShadowService layer array
        };
        contract.expectedSets = 8; // live clients: Earth/Moon/Iapetus (+margin)
        PipelineFamilyDesc desc;
        desc.name = "MESH_TES";
        desc.vertex = Context::instance->ojmVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        desc.sets.push_back(renderer.globalUboContract()); // cam_block: vert/tese fisheye + frag ambient
        desc.specValues = {{7, Context::instance->isFloat64Supported}};
        // One tese PER ROW (exact interfaces - a superset tese fires
        // WARNING-Shader-OutputNotConsumed; the zero-validation-messages bar
        // forces the same three-tese structure the old path had).
        // NIGHT|BUMP combined is not a live class and not in the table - the
        // higher dropPriority (BUMP) falls off first.
        desc.axes = {
            {"night", VARIANT_NIGHT, VariantEffect::SHADER_SWAP, 1},
            {"bump", VARIANT_BUMP, VariantEffect::SHADER_SWAP, 2},
        };
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {
            {0,             {.vert = "body_tes.vert.spv", .tesc = "body_tes.tesc.spv", .tese = "bodyTesMesh.tese.spv", .frag = "bodyTesMesh.frag.spv"}},
            {VARIANT_NIGHT, {.vert = "body_tes.vert.spv", .tesc = "body_tes.tesc.spv", .tese = "bodyTesNight.tese.spv", .frag = "bodyTesNight.frag.spv"}},
            {VARIANT_BUMP,  {.vert = "body_tes.vert.spv", .tesc = "body_tes.tesc.spv", .tese = "bodyTesBump.tese.spv", .frag = "bodyTesBump.frag.spv"}},
        };
        // Old tessellated state (bodyShader.cpp myEarth/myMoon/shaderNormalTes):
        // PATCH(3), cull, REVERSED front face, BLEND_NONE, depth on.
        color.state.patchControlPoints = 3;
        color.state.reverseFrontFace = true;
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}

const PipelineFamily &MeshFamilies::meshLayered()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        VkSamplerCreateInfo mapSampler = PipelineLayout::DEFAULT_SAMPLER;
        mapSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SetContractDesc contract;
        contract.name = "bodyLayered";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},           // globalVertProj
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},         // meshFrag
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // dayTexture
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // nightTexture (NIGHT row)
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // normalTexture (BUMP row)
            {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // ShadowService layer array
        };
        contract.expectedSets = 8; // live clients: Io/Mars (+margin)
        PipelineFamilyDesc desc;
        desc.name = "MESH_LAYERED";
        desc.vertex = Context::instance->ojmVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        desc.sets.push_back(renderer.globalUboContract());
        desc.specValues = {{7, Context::instance->isFloat64Supported}};
        desc.axes = {
            {"night", VARIANT_NIGHT, VariantEffect::SHADER_SWAP, 1},
            {"bump", VARIANT_BUMP, VariantEffect::SHADER_SWAP, 2},
        };
        PassDesc color;
        color.pass = PassKind::COLOR;
        // body_night.vert reused for ALL rows (its varyings are the superset;
        // the BUMP frag is interface-adapted from body_bump.frag - the old
        // body_bump.vert pairing differed only by the Ambient varying, read
        // from cam_block instead). Old state: triangle, cull, normal winding.
        color.shaderTable = {
            {0,             {.vert = "body_night.vert.spv", .frag = "bodyLayeredDay.frag.spv"}},
            {VARIANT_NIGHT, {.vert = "body_night.vert.spv", .frag = "bodyLayeredNight.frag.spv"}},
            {VARIANT_BUMP,  {.vert = "body_night.vert.spv", .frag = "bodyLayeredBump.frag.spv"}},
        };
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}

const PipelineFamily &MeshFamilies::meshRayMarch()
{
    static PipelineFamily family = []() -> PipelineFamily {
        Renderer &renderer = Context::instance->renderer;
        VkSamplerCreateInfo mapSampler = PipelineLayout::DEFAULT_SAMPLER;
        mapSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        SetContractDesc contract;
        contract.name = "bodyRayMarch";
        contract.bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},           // rayMarchVert (old ShadowVert)
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},         // rayMarchFrag (old ShadowFrag, S5 rows)
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // heightMap
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // normalMap
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // dayTexture
            {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // nightTexture (NIGHT row)
            {6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, mapSampler}, // SpecularTexture (NIGHT row)
            {7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}, // ShadowService layer array
        };
        contract.expectedSets = 8; // rayMarchCapable bodies: Earth/Moon/Mars/Iapetus (+margin)
        PipelineFamilyDesc desc;
        desc.name = "MESH_RAYMARCH";
        desc.vertex = Context::instance->ojmVertexArray.get();
        desc.sets.push_back(renderer.allocateSetContract(std::move(contract)));
        // Set 1 = global UBO: the base row's frag reads cam_block.ambient
        // (my_moon_shadow parity); the NIGHT row's shaders don't reference it
        // - one layout for both rows is the 10.3 decision-4 normalization of
        // the old myEarthShadowed/shaderShadowedTes set-count divergence.
        desc.sets.push_back(renderer.globalUboContract());
        desc.axes = {
            {"night", VARIANT_NIGHT, VariantEffect::SHADER_SWAP, 1},
        };
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {
            {0,             {.vert = "body_tes_shadow.vert.spv", .frag = "bodyRayMarch.frag.spv"}},
            {VARIANT_NIGHT, {.vert = "body_tes_shadow.vert.spv", .frag = "bodyRayMarchNight.frag.spv"}},
        };
        // Old state (bodyShader.cpp myEarthShadowed/shaderShadowedTes):
        // triangle, cull, BLEND_NONE, normal-attr entry stripped. Depth is ON
        // in the base pass state (shaderShadowedTes); the old myEarthShadowed
        // depth-OFF quirk is reproduced at bind time via NIGHT|VARIANT_NO_DEPTH.
        color.state.removedVertexEntries = 1 << 2;
        desc.passes.push_back(std::move(color));
        return renderer.allocateFamily(std::move(desc));
    }();
    return family;
}
