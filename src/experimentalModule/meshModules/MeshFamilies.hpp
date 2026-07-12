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
// The layered variants (night/clouds/specular/bump/tes/shadowed - INTENT.md
// 12 row 2) enter HERE as further families or SHADER_SWAP axes of this one,
// at their port (S1 wave 2); the old per-feature drawStates dissolve then.
// ============================================================================
namespace MeshFamilies {
    const PipelineFamily &meshNormal();
}

#endif /* end of include guard: MESH_FAMILIES_HPP_ */
