#ifndef MESH_SHADOW_FILL_HPP_
#define MESH_SHADOW_FILL_HPP_

#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ShadowService.hpp"
#include "bodyShaderInterface.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

// ============================================================================
// Shared receiver-side fills of the generalized shadow block - single
// authority (I2) for every receiver family. Two idioms only (consolidated
// 2026-07-18 when the per-entry-rows restructure touched every fill at once;
// fillRayMarchShadows and fillOjmShadows had byte-identical bodies - the
// GLSL-include lesson applied to the CPU side before the ring made a 4th
// copy):
//
//  - fillPlainShadows: receivers whose fragment works in EYE SPACE (disc
//    families - meshFrag; the RING block at row 4). Entry rows/clip are
//    consumed UNFOLDED.
//  - fillFoldedShadows: receivers whose fragment works in MODEL space
//    (ray-march samplePos, OJM raw vertices). Entry rows AND clip fold
//    through the same model->eye map MV = mat * scaling(radius) (they are
//    affine forms over the same P - folding both through one map is what
//    keeps caller consistency by construction).
//
// Since 2026-07-18 the rows live PER ENTRY (ShadowProjection.hpp,
// [vixy: 2026-07-18]): every entry is self-contained, so multi-light lands
// in the selection alone - these fills and every receiver stay untouched.
//
// `self` = the RECEIVING module (pass `this`): entries are per (caster body,
// PROJECTING module), and a surface must skip the entries it produced itself
// (caster == its own body && source == itself) - the mesh layer of a planet
// is exactly its own disc centered on itself; sampling it would darken the
// whole surface. Cross-body entries and the OTHER modules' within-body
// entries (ring -> planet) pass through.
//
// Both fill ONLY the receive members of the block (nbShadowingBodies +
// shadowingBodies); sibling fields are the module's own. Gate on the service
// flag too: entries may be stale from the frame the flag switched off.
// ============================================================================

// Eye-space receivers: entries copied verbatim (rows already receiver-folded
// by the selection - ModularSystem::computeShadows).
template <typename Block>
inline void fillPlainShadows(SharedBuffer<Block> &frag, ModularBody *body, const BodyModule *self)
{
    const ReceivedShadows &received = body->getReceivedShadows();
    if (ShadowService::enabled && received) {
        auto &f = *frag;
        int nb = 0;
        for (const auto &e : received.entries) {
            if (e.caster == body && e.source == self)
                continue; // own silhouette (header block)
            f.shadowingBodies[nb].posRadius = Vec4f(e.pos.first, e.pos.second, e.size, 0);
            f.shadowingBodies[nb].absorbtionIdx = Vec4f(e.absorbtion[0], e.absorbtion[1], e.absorbtion[2], e.layerIdx);
            f.shadowingBodies[nb].clip = e.clip;
            f.shadowingBodies[nb].row0 = e.row0;
            f.shadowingBodies[nb].row1 = e.row1;
            f.shadowingBodies[nb].glow = Vec4f(e.glow[0], e.glow[1], e.glow[2], 0);
            ++nb;
        }
        f.nbShadowingBodies = nb;
    } else {
        frag->nbShadowingBodies = 0;
    }
}

// Model-space receivers: rows and clip folded per entry through
// MV = model->eye (including any radius/oblateness scale), radius = the span
// of the fragment's model-space unit (finalRadius for the ray-march unit
// sphere, body radius for OJM's normalized vertices):
//   rowL.xyz[j] = radius * dot(row.xyz, MV.column[j])   (j = 0..2)
//   rowL.w      = row.w + dot(row.xyz, MV.translation)
// The clip plane is an affine form over eye-space P exactly like the rows,
// so it folds through the SAME map (degenerate planes stay degenerate:
// xyz = 0 folds to 0, w unchanged).
template <typename Block>
inline void fillFoldedShadows(SharedBuffer<Block> &frag, ModularBody *body,
                              const Mat4f &MV, float radius, const BodyModule *self)
{
    const ReceivedShadows &received = body->getReceivedShadows();
    if (ShadowService::enabled && received) {
        auto &f = *frag;
        const auto foldRow = [&MV, radius](const Vec4f &row) -> Vec4f {
            return Vec4f(
                radius * (row.v[0]*MV.r[0] + row.v[1]*MV.r[1] + row.v[2]*MV.r[2]),
                radius * (row.v[0]*MV.r[4] + row.v[1]*MV.r[5] + row.v[2]*MV.r[6]),
                radius * (row.v[0]*MV.r[8] + row.v[1]*MV.r[9] + row.v[2]*MV.r[10]),
                row.v[3] + row.v[0]*MV.r[12] + row.v[1]*MV.r[13] + row.v[2]*MV.r[14]);
        };
        int nb = 0;
        for (const auto &e : received.entries) {
            if (e.caster == body && e.source == self)
                continue; // own silhouette (header block)
            f.shadowingBodies[nb].posRadius = Vec4f(e.pos.first, e.pos.second, e.size, 0);
            f.shadowingBodies[nb].absorbtionIdx = Vec4f(e.absorbtion[0], e.absorbtion[1], e.absorbtion[2], e.layerIdx);
            f.shadowingBodies[nb].clip = foldRow(e.clip);
            f.shadowingBodies[nb].row0 = foldRow(e.row0);
            f.shadowingBodies[nb].row1 = foldRow(e.row1);
            f.shadowingBodies[nb].glow = Vec4f(e.glow[0], e.glow[1], e.glow[2], 0);
            ++nb;
        }
        f.nbShadowingBodies = nb;
    } else {
        frag->nbShadowingBodies = 0;
    }
}

#endif /* end of include guard: MESH_SHADOW_FILL_HPP_ */
