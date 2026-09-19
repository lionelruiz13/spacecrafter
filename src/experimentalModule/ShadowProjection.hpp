#ifndef SHADOW_PROJECTION_HPP_
#define SHADOW_PROJECTION_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <utility>

class ModularBody;
class BodyModule;

// One (caster body, projecting module) entry shading a receiver, in observer-local eye space.
// A receiving module skips its own entries (caster == its body && source == itself).
// Transmission of an entry: 1 - coverage * absorbtion + umbra * glow; entries multiply.
struct ShadowProjection {
    ModularBody *caster;       // caster BODY - receiving modules of the same body test it for self-exclusion
    const BodyModule *source;   // the PROJECTING module - the other half of the self-exclusion key
    std::pair<float, float> pos; // caster center in the entry's sun-frame xy, relative to the receiver center (eye-space units)
    float size;                 // module castRadius + smoothRadius (penumbra growth) - the shadow-map disc radius, same units
    uint8_t layerIdx;           // layer in the ShadowService layer array
    Vec3f absorbtion;           // share of the covered direct light the caster's material blocks; {1,1,1} if solid
    Vec3f glow;                 // light the caster's atmosphere refracts into the true umbra; zero without atmosphere
    Vec4f clip;                 // applies iff dot(P, clip.xyz) + clip.w <= 0 (planar casters); (0,0,0,-1) if solid
    // shadowPos = (dot(row0.xyz, P) + row0.w, dot(row1.xyz, P) + row1.w): sun frame of the light casting this entry,
    // folded on the receiver center; its x axis is tied to the receiver's spin axis, never to the eye-space up
    Vec4f row0, row1;
};

// What a receiving module samples the shadows projected on its body with, this frame; filled by the ModularSystem
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
