#ifndef MESH_SHADOW_FILL_HPP_
#define MESH_SHADOW_FILL_HPP_

#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ShadowService.hpp"
#include "bodyShaderInterface.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

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
