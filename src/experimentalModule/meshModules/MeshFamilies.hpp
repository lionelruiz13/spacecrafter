#ifndef MESH_FAMILIES_HPP_
#define MESH_FAMILIES_HPP_

#include "experimentalModule/PipelineFamily.hpp"

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
