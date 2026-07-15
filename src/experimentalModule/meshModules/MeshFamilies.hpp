#ifndef MESH_FAMILIES_HPP_
#define MESH_FAMILIES_HPP_

#include "experimentalModule/PipelineFamily.hpp"

// ============================================================================
// Pipeline families of the MESH module family (INTENT.md 12 row 1-2).
// Registration-domain accessors: each allocates lazily on first use (module
// creation happens at body-loading time, after Core init - UBOCam and the
// render managers exist by then) and returns the shared handle; copies
// refcount through the registry.
//
// meshNormal() ports the old shaderNormal drawState verbatim (bodyShader.cpp
// "body_normal" block): set 0 = {vert UBO, frag UBO, map texture with
// REPEAT-U sampler, eclipse texture with default sampler}, set 1 = global
// UBO; spec constant 7 = isFloat64Supported; cull on, BLEND_NONE, triangle
// list; NO_DEPTH via the reserved variant bit (replaces the prebuilt
// pipelineNoDepth clone - first-use lazy build, base depth-on fallback for
// the frames until it is resident, reported through FamilyBound::got).
//
// Row 2 (2026-07-15) - the layered variants land as THREE further families
// (layout-invariance: extra textures change the descriptor contract; a
// tessellated and a non-tessellated representation differ in topology, which
// is per-pass FixedState - separate families, not axes):
//
// meshTes() - tessellated layered disc (PATCH 3, reversed winding, heightmap
//   displacement in bodyTes.tese). Rows: base = old body_normal_tes class,
//   NIGHT = my_earth class (night+specular, no ambient), BUMP = my_moon
//   class (tangent-space normal map). body_tes.vert/tesc REUSED verbatim.
//
// meshLayered() - non-tessellated layered disc (body_night.vert reused
//   verbatim). Rows: base (no live client - base-row obligation), NIGHT =
//   old body_night class (Io - diffuse only, the old specular is commented
//   out in source), BUMP = old body_bump class (Mars mid-range).
//
// meshRayMarch() - close-range per-pixel relief (body_tes_shadow.vert reused
//   verbatim; heightmap ray-march + terrain self-shadow in the frags). Rows:
//   base = old my_moon_shadow class, NIGHT = old my_earth_shadow class.
//   The old CoI depth quirk (myEarthShadowed depth OFF, shaderShadowedTes
//   depth ON) is reproduced by binding NIGHT|VARIANT_NO_DEPTH.
//
// All four frag rows receive shadows through the S5 generalized block
// (meshFrag/rayMarchFrag, bodyShaderInterface.hpp); the Gen-1 LUT slots of
// the old shaders are retired (shadow-paths.md B4).
// ============================================================================
namespace MeshFamilies {
    // SHADER_SWAP variant bits shared by meshTes()/meshLayered()/meshRayMarch()
    constexpr VariantKey VARIANT_NIGHT = 0x0001;
    constexpr VariantKey VARIANT_BUMP  = 0x0002; // tes/layered only

    const PipelineFamily &meshNormal();
    const PipelineFamily &meshTes();
    const PipelineFamily &meshLayered();
    const PipelineFamily &meshRayMarch();
}

#endif /* end of include guard: MESH_FAMILIES_HPP_ */
