#ifndef SHADOW_PROJECTION_HPP_
#define SHADOW_PROJECTION_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <utility>

class ModularBody;
class BodyModule;

// Shadow of one (caster, source) on a receiver, in eye space; a receiving module skips its own
// Transmission = 1 - coverage * absorbtion + umbra * glow, entries multiply
struct ShadowProjection {
    ModularBody *caster;
    const BodyModule *source;   // Projecting module
    std::pair<float, float> pos; // Caster center in the sun frame, relative to the receiver center
    float size;                 // Shadow-map disc radius, penumbra included
    uint8_t layerIdx;
    Vec3f absorbtion;           // Share of the covered light blocked, {1,1,1} if solid
    Vec3f glow;                 // Light refracted into the umbra, zero without atmosphere
    Vec4f clip;                 // Applies if dot(P, clip.xyz) + clip.w <= 0; (0,0,0,-1) if solid
    Vec4f row0, row1;           // shadowPos = (dot(row0.xyz, P) + row0.w, dot(row1.xyz, P) + row1.w)
};

// Shadows projected on a body this frame, filled by the ModularSystem
struct ReceivedShadows {
    std::vector<ShadowProjection> entries;
    inline void clear() {
        entries.clear();
    }
    inline operator bool() const {
        return !entries.empty();
    }
};

#endif /* end of include guard: SHADOW_PROJECTION_HPP_ */
