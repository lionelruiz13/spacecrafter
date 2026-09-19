#ifndef MESH_FAMILIES_HPP_
#define MESH_FAMILIES_HPP_

#include "experimentalModule/PipelineFamily.hpp"

// Pipeline families of the MESH modules; each accessor allocates lazily on first use and returns the shared handle
namespace MeshFamilies {
    // SHADER_SWAP variant bits shared by meshTes()/meshLayered()/meshRayMarch()
    constexpr VariantKey VARIANT_NIGHT = 0x0001;
    constexpr VariantKey VARIANT_BUMP  = 0x0002; // tes/layered only

    // Plain textured disc; NO_DEPTH through the reserved variant bit
    const PipelineFamily &meshNormal();
    // Tessellated layered disc (heightmap displacement); rows: base, NIGHT, BUMP
    const PipelineFamily &meshTes();
    // Flat layered disc; rows: base, NIGHT, BUMP
    const PipelineFamily &meshLayered();
    // Close-range per-pixel relief + terrain self-shadow, writes the true ray-hit depth; rows: base, NIGHT
    const PipelineFamily &meshRayMarch();
}

#endif /* end of include guard: MESH_FAMILIES_HPP_ */
