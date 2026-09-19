#ifndef SHADOW_PROJECTION_HPP_
#define SHADOW_PROJECTION_HPP_

#include "tools/vecmath.hpp"
#include <vector>
#include <utility>

class ModularBody;
class BodyModule;

struct ShadowProjection {
    ModularBody *caster;        // caster BODY - receiving modules of the same body test it for self-exclusion
    const BodyModule *source;   // the PROJECTING module - the other half of the self-exclusion key
    std::pair<float, float> pos; // caster center in the entry's sun-frame xy, relative to the receiver center (eye-space units)
    float size;                 // module castRadius + smoothRadius (penumbra growth) - the shadow-map disc radius, same units
    uint8_t layerIdx;           // layer in the ShadowService layer array
    Vec3f absorbtion;
    Vec3f glow;
    Vec4f clip;                 // eye-space half-space gate (header block);
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
