#ifndef MESH_SHADOW_FILL_HPP_
#define MESH_SHADOW_FILL_HPP_

#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ShadowService.hpp"
#include "bodyShaderInterface.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

// ============================================================================
// Shared receiver-side fills of the S5 generalized shadow block - single
// authority (I2) for every disc fragment family (MESH, MESH_TES,
// MESH_LAYERED via meshFrag; MESH_RAYMARCH via rayMarchFrag). Lifted from
// BasicMesh.cpp's file-local fillShadows when row 2 added more receivers.
//
// `self` = the RECEIVING module (pass `this`): entries are per (caster body,
// PROJECTING module) since the composition-typed rework, and a surface must
// skip the entries it produced itself (caster == its own body && source ==
// itself) - the mesh layer of a planet is exactly its own disc centered on
// itself; sampling it would darken the whole surface. Cross-body entries and
// the OTHER modules' within-body entries (ring -> planet) pass through.
// ============================================================================

// Fill the Gen-2 receiver block from the body's received-shadow state
// (produced by ModularSystem::computeShadows - contract: ShadowProjection.hpp).
// Gate on the service flag too: entries may be stale from the frame the flag
// switched off.
inline void fillMeshShadows(SharedBuffer<meshFrag> &frag, ModularBody *body, const BodyModule *self)
{
    const ReceivedShadows &received = body->getReceivedShadows();
    if (ShadowService::enabled && received) {
        auto &f = *frag;
        f.shadowRow0 = received.row0;
        f.shadowRow1 = received.row1;
        int nb = 0;
        for (const auto &e : received.entries) {
            if (e.caster == body && e.source == self)
                continue; // own silhouette (header block)
            f.shadowingBodies[nb].posRadius = Vec4f(e.pos.first, e.pos.second, e.size, 0);
            f.shadowingBodies[nb].absorbtionIdx = Vec4f(e.absorbtion[0], e.absorbtion[1], e.absorbtion[2], e.layerIdx);
            f.shadowingBodies[nb].clip = e.clip;
            ++nb;
        }
        f.nbShadowingBodies = nb;
    } else {
        frag->nbShadowingBodies = 0;
    }
}

// Ray-march variant: same entries, but the projection rows are PRE-FOLDED
// through the model matrix so the fragment's unit-sphere samplePos projects
// directly into the S5 sun frame (eye-space AU - caster posRadius consumed
// unchanged, no initialRadius normalization; bodyShaderInterface.hpp
// rayMarchFrag comment carries the algebra):
//   rowL.xyz[j] = radius * dot(row.xyz, MV.column[j])   (j = 0..2)
//   rowL.w      = row.w + dot(row.xyz, MV.translation)
// MV = the ray-march ModelViewMatrix (incl. the oblateness scale), radius =
// finalRadius (the fragment's unit sphere spans finalRadius in body space).
// The clip plane is an affine form over eye-space P exactly like the rows,
// so it folds through the SAME map (degenerate planes stay degenerate:
// xyz = 0 folds to 0, w unchanged).
inline void fillRayMarchShadows(SharedBuffer<rayMarchFrag> &frag, ModularBody *body,
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
        f.shadowRow0 = foldRow(received.row0);
        f.shadowRow1 = foldRow(received.row1);
        int nb = 0;
        for (const auto &e : received.entries) {
            if (e.caster == body && e.source == self)
                continue; // own silhouette (header block)
            f.shadowingBodies[nb].posRadius = Vec4f(e.pos.first, e.pos.second, e.size, 0);
            f.shadowingBodies[nb].absorbtionIdx = Vec4f(e.absorbtion[0], e.absorbtion[1], e.absorbtion[2], e.layerIdx);
            f.shadowingBodies[nb].clip = foldRow(e.clip);
            ++nb;
        }
        f.nbShadowingBodies = nb;
    } else {
        frag->nbShadowingBodies = 0;
    }
}

// OJM variant (row 3): same entries, same fold as fillRayMarchShadows - the
// OJM frag works in MODEL space (Position = raw unit-normalized vertex), so
// rows and clip fold through the model->eye map MV = mat * scaling(radius).
// Fills ONLY the receive part of ojmShadowBlock (rows + entries); the
// self-shadow/lighting fields are the module's own (OjmModule::draw).
inline void fillOjmShadows(SharedBuffer<ojmShadowBlock> &frag, ModularBody *body,
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
        f.shadowRow0 = foldRow(received.row0);
        f.shadowRow1 = foldRow(received.row1);
        int nb = 0;
        for (const auto &e : received.entries) {
            if (e.caster == body && e.source == self)
                continue; // own silhouette (header block)
            f.shadowingBodies[nb].posRadius = Vec4f(e.pos.first, e.pos.second, e.size, 0);
            f.shadowingBodies[nb].absorbtionIdx = Vec4f(e.absorbtion[0], e.absorbtion[1], e.absorbtion[2], e.layerIdx);
            f.shadowingBodies[nb].clip = foldRow(e.clip);
            ++nb;
        }
        f.nbShadowingBodies = nb;
    } else {
        frag->nbShadowingBodies = 0;
    }
}

#endif /* end of include guard: MESH_SHADOW_FILL_HPP_ */
