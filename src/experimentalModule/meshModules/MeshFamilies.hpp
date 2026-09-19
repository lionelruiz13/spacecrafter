#ifndef MESH_FAMILIES_HPP_
#define MESH_FAMILIES_HPP_

#include "experimentalModule/PipelineFamily.hpp"

// Pipelines of the MESH modules, allocated at first use
namespace MeshFamilies {
    constexpr VariantKey VARIANT_NIGHT = 0x0001;
    constexpr VariantKey VARIANT_BUMP  = 0x0002; // tes/layered only

    const PipelineFamily &meshNormal();
    const PipelineFamily &meshTes();
    const PipelineFamily &meshLayered();
    const PipelineFamily &meshRayMarch();
}

#endif /* end of include guard: MESH_FAMILIES_HPP_ */
