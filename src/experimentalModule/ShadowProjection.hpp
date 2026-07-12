#ifndef SHADOW_PROJECTION_HPP_
#define SHADOW_PROJECTION_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <utility>

class ModularBody;

// ============================================================================
// Received-shadow state of one ModularBody - the "Received shadows" relation
// (ModularBody relations block): what a receiver needs to SAMPLE the shadows
// projected onto it this frame. Produced by the ModularSystem shadow
// orchestration (selection - WHICH bodies shadow which), consumed by the
// receiver's drawing modules (BasicMesh fills its fragment UBO from it).
// Design + old-path mapping: shadow-paths.md B1/B2.
//
// Geometry convention (validated against the old path at S5.5): everything
// lives in observer-local eye space (the only frame the new path has -
// projection-paths.md B). The sun frame of a receiver R under light L is
//   z = normalize(R - L),  x = normalize(axis x z),  y = z x x
// where axis is a WORLD-TIED vector (the receiver's spin axis, mat column 2)
// - NOT the eye-space up: with eye-up the frame would spin with the camera
// while cached layer content would not, rotating cached silhouettes against
// their mapping (the old path avoided this by working heliocentric; the
// world-tied axis restores the same camera-independence, since a camera
// rotation rotates axis, z and every position coherently).
// The receiver's fragment shader projects its eye-space surface point P as
//   shadowPos = (dot(row0.xyz, P) + row0.w, dot(row1.xyz, P) + row1.w)
// with row0.xyz = x, row0.w = -dot(x, R) (receiver-centered fold), idem row1.
// Old-path equivalence: ShadowMatrix = mat3(lookAt * model * zrot) applied to
// model-space positions, minus the per-caster pos subtraction - the same
// affine map, origin moved from the sun to the receiver center (the
// difference cancels in (shadowPos - pos), shadow-paths.md A2.7).
// ============================================================================

// One caster projecting onto this receiver.
struct ShadowProjection {
    ModularBody *caster;        // traceability/debug; the GPU path reads the fields below
    std::pair<float, float> pos; // caster center in the receiver's sun-frame xy, relative to the receiver center (eye-space units)
    float size;                 // casterRadius + smoothRadius (penumbra growth) - the shadow-map disc radius, same units
    uint8_t layerIdx;           // layer in the ShadowService R8 array
    Vec3f absorbtion;           // caster's per-channel shadow absorption
                                // (shadow_color; Earth {0,1,1} -> red umbra)
};

struct ReceivedShadows {
    Vec4f row0, row1;           // the folded projection rows (see above)
    std::vector<ShadowProjection> entries;
    inline void clear() {
        entries.clear();
    }
    inline operator bool() const {
        return !entries.empty();
    }
};

#endif /* end of include guard: SHADOW_PROJECTION_HPP_ */
