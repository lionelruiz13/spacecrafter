#ifndef SHADOW_PROJECTION_HPP_
#define SHADOW_PROJECTION_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <utility>

class ModularBody;
class BodyModule;

// ============================================================================
// Received-shadow state of one ModularBody - the "Received shadows" relation
// (ModularBody relations block): what a receiver needs to SAMPLE the shadows
// projected onto it this frame. Produced by the ModularSystem shadow
// orchestration (selection - WHICH modules shadow which surfaces), consumed
// by the receiving modules (BMT_RECEIVE_SHADOW; meshShadowFill.hpp fills the
// fragment UBOs from it). Design + old-path mapping: shadow-paths.md B1/B2.
//
// Granularity: one entry per (caster body, PROJECTING MODULE) - not per body.
// Application multiplies (1 - cov_i * absorbtion_i) over entries; products
// commute, so per-module layers compose exactly like one combined caster
// transmission map, while keeping per-module absorbtion (ring 0.7-neutral vs
// Earth {0,1,1}) and within-body pairs (ring<->planet on one ModularBody)
// expressible. A receiving module must SKIP its own entries
// (caster == its body && source == itself): a surface never samples a layer
// containing its own silhouette - the mesh layer of a planet is exactly its
// own disc centered on itself.
//
// The clip half-space (planar casters - rings): the blurred layer is Z-LESS
// (coverage over sun-frame xy), so a ray that crosses the ring plane BEHIND
// the surface it lights would still read coverage - without a gate, false
// ring bands appear on the caster's sun-side hemisphere where the rings pass
// behind the disc. Exact rule, any receiver point P: the plane crossing along
// P's light ray happens BEFORE the hit iff P lies beyond the ring plane as
// seen from the sun. Gate: apply the entry iff dot(P, clip.xyz) + clip.w <= 0
// with clip.xyz = the caster's plane normal oriented toward the sun,
// clip.w = -dot(clip.xyz, casterCenter). For the own-planet sphere this is
// exact everywhere (no band can straddle: an annulus crossing inside the
// disc footprint satisfies x2+y2+z2 >= inner2 > R2, so |z| >= the surface
// depth by construction); for cross-body receivers (rings onto a moon) the
// whole far ring correctly shades, minus what the planet's own G1 entry
// already covers. Solid casters carry the degenerate plane (0,0,0,-1):
// always applies. [derived 2026-07-16, replacing the old analytic test
// dot(intersect, ModelLight) >= 0 of body_ringed.frag:50]
// ============================================================================

// One (caster body, projecting module) entry shading this receiver.
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

struct ShadowProjection {
    ModularBody *caster;        // caster BODY - receiving modules of the same body test it for self-exclusion
    const BodyModule *source;   // the PROJECTING module - the other half of the self-exclusion key
    std::pair<float, float> pos; // caster center in the entry's sun-frame xy, relative to the receiver center (eye-space units)
    float size;                 // module castRadius + smoothRadius (penumbra growth) - the shadow-map disc radius, same units
    uint8_t layerIdx;           // layer in the ShadowService layer array
    // Physical-sharp split (2026-07-18 [vixy]) - the module's declared
    // absorbtion (ShadowCaster) is MAPPED by the selection into two roles,
    // because it conflates two physical mechanisms:
    //  - absorbtion here = TRANSMISSION absorption aT: how much of the
    //    covered direct light the caster's MATERIAL blocks. Solid casters:
    //    {1,1,1} (opaque - direct light fully blocked, giving the neutral
    //    (1-c) penumbra/antumbra ramp). Graded casters (rings): the declared
    //    material absorption (legacy 1 - c*a transmission physics).
    //  - glow = REFRACTION glow gR = 1 - declared absorbtion for solid
    //    atmosphere-bearing casters (Earth {0.4,0.12,0} - light bent around
    //    the limb into the TRUE UMBRA only; matches the umbra floor legacy
    //    had at c=1, so the umbra look is unchanged). Zero for rings and
    //    airless bodies. Calibration criterion: real-eclipse photography at
    //    the same timestamp [vixy: 2026-07-18].
    // Receiver formula (receivedShadows.glsl): T = 1 - c*aT + u*gR, with
    // u = layer G (true umbra) <= c = layer R (mean coverage) keeping T in
    // [0,1] by construction.
    Vec3f absorbtion;
    Vec3f glow;
    Vec4f clip;                 // eye-space half-space gate (header block);
                                // (0,0,0,-1) for solid casters
    // The folded projection rows of THIS entry (see the geometry convention
    // above). PER-ENTRY, not per-receiver [vixy: 2026-07-18]: an entry's sun
    // frame depends on the LIGHT that casts it - carrying the rows here makes
    // every entry self-contained, so multiple light sources (binary systems)
    // land later purely in the selection (one entry per (caster module,
    // light)), with zero rework of any receiver family. Today's single-light
    // selection fills the same receiver-frame value into every entry.
    Vec4f row0, row1;
};

struct ReceivedShadows {
    std::vector<ShadowProjection> entries; // each self-contained (rows included)
    inline void clear() {
        entries.clear();
    }
    inline operator bool() const {
        return !entries.empty();
    }
};

#endif /* end of include guard: SHADOW_PROJECTION_HPP_ */
