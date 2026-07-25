#ifndef MODULAR_BODY_HPP_
#define MODULAR_BODY_HPP_

#include "BodyModule.hpp"
#include "Renderer.hpp"
#include "ShadowProjection.hpp"
#include "EntityCore/Tools/Tracer.hpp"
#include "../planetsephems/sideral_time.h"
#include "coreModule/time_mgr.hpp"
#include "bodyModule/rotation_elements.hpp"
#include "bodyModule/orbit.hpp"
#include "ProjectionTransfer.hpp"
#include "EnvironmentModule.hpp"
#include "AsyncHub.hpp"
#include "EntityCore/Executor/ASmooth.hpp"
#include "tools/StringID.hpp"
#include <memory>
#include <iosfwd>
#include <list>
#include <vector>

// (TEXMAP*/TEX big-texture mapping macros moved to tools/s_texture.hpp -
//  they are texture utilities, not body concepts.)

class BodyTesselation; // both-paths tesselation seam (see setTesselation)

// Structural nature of a body. NOT a feature taxonomy (features live in
// BodyModule slots - G1): what remains here is only what modules cannot
// express: STAR = emits light (bit-tested via isStar()); MINOR_BODY =
// mass-instanced small bodies (e.g. upscaled asteroid ring): many visible at
// once, exempt from inter-body shadowing, cluster-optimizable.
// SPHERICAL_BODY/SINGLE_BODY/CUSTOM_BODY: pre-composition remnants - fate
// decided in the second pass (INTENT.md 6.3).
// EARTH / EARTH_MOON RETIRED (B25-emit, §11.73 A3, 2026-07-23): the only
// hard-coded specificity of BodyType::EARTH that still ACTED was apparent
// sidereal time - now the SiderealTimeModel selector below (a capability key,
// `sidereal_time`, A1) - plus Earth's shadow color, which was already the
// existing `shadow_color` key with a name-keyed default (A2). BodyType::EARTH_MOON
// had ZERO consumers [re-verified at delete time, whole-src grep 2026-07-23].
// Both were set only via the applyHardcodedContent name sniff - the identity
// sniff D14 retires for the composed format (§11.79(h)).
enum class BodyType : unsigned char {
    VOID,
    ANCHOR, // Simplest type, just an anchor
    SYSTEM, // A body system
    GALAXY, // A galaxy
    MINOR_BODY, // A body with minor features, usefull for massively instanciated bodies
    SPHERICAL_BODY, // A spherical body, who doesn't use self-shadowing
    SINGLE_BODY, // A body with a single shape
    CUSTOM_BODY, // A body with multiple shapes
    STAR = 0x40, // A body who emit light
};

// The analytic model for a body's spin phase about its polar axis - the
// sidereal-time selector (B27 A1; D10key ratified spelling `sidereal_time`,
// §11.79(e)). A SELECTABLE analytic kind, parallel to coord_func: the data
// picks it, not the body's identity (which is exactly what retires the
// englishName=="Earth" sniff, §5.5). GENERIC = the default (jd-epoch)/period
// spin, every body; EARTH_APPARENT = apparent sidereal time (nutation), Earth's
// model. Legacy loads still reach EARTH_APPARENT through the name sniff
// (applyHardcodedContent, legacy format only - D14); the composed format
// declares it explicitly as the key the twin emits (§11.73 A1).
enum class SiderealTimeModel : unsigned char {
    GENERIC,
    EARTH_APPARENT,
};

// The surface-lighting/tessellation lineage a layered mesh is built on (B27 A6;
// D10key ratified spelling `surface_model`, §11.79(e)). A CAPABILITY, not an
// identity: the old path selected the "moon class" shader family from the body
// being type=Moon, which made the lunar lineage unreachable for any other body
// (a composition capability - §11.73 A6 is the one that BLOCKS arbitrary-body
// surface models). PLANET = the earth/planet row of LayeredMesh::selectShader
// (night/specular/bump combinations); LUNAR = the lunar row (tessellated
// heightmap displacement, no night side). Legacy loads still reach LUNAR through
// the `type = Moon` data string (D9 - frozen forever); the composed format
// declares it explicitly as the key the twin emits (D14 §11.79(h)).
enum class SurfaceModel : unsigned char {
    PLANET,
    LUNAR,
};

// Trail length default when neither `trail_length` nor (legacy) `type` supplies
// one: the old UNKNOWN->ASTEROID->SmallBody value (protosystem.cpp:531-532),
// which is also what every Asteroid/KBO carries today. It is the neutral value
// of the key's domain - the composed format's default when the key is absent
// (type-as-identity retired, D14), and the legacy fallback for an unknown type.
constexpr int TRAIL_LENGTH_DEFAULT = 60;

enum ModularBodyTraits {
    MBT_REPLICATED, // This modular body is heavily replicated (ex : asteroid ring)
    MBT_HALO, // This modular body have a halo
    MBT_CLUSTER, // This modular body must use cluster optimisation (ex : asteroid ring)
};

inline constexpr BodyType operator&(BodyType t1, BodyType t2)
{
    return static_cast<BodyType>(static_cast<uint8_t>(t1) & static_cast<uint8_t>(t2));
}

class ModularSystem;
class Translator;

//! Sentinel createInfo value for datumRadius/groundRadius meaning "UNSET -
//! resolve to the type's class default": `radius` for a plain ModularBody,
//! 0 for a ModularSystem (B10-datum0, §11.75(a) [vixy 2026-07-22] - a system
//! node is navigated INTO, so its altitude/ground reference is its centre).
//! Negative because a real navigation radius is >= 0, so an explicit value -
//! a data key (`datum_radius`/`ground_radius`) or the §11.84 runtime command -
//! is always >= 0 and WINS over the class default (the resolution branches only
//! fire on the sentinel). The loader (ModularSystem::loadBody) never emits the
//! sentinel: it defaults the key to `radius` at the createInfo seam, so plain
//! shipped/scripted bodies stay bit-identical; only the factory system nodes
//! omit the key and take the sentinel.
constexpr float NAV_RADIUS_UNSET = -1.f;

struct ModularBodyCreateInfo {
    std::unique_ptr<Orbit> orbit;
    std::string englishName;
    RotationElements re;
    Vec3f haloColor;
    float albedo;
    float radius;
    // Navigation radii, both in AU, both defaulting to `radius` (B10, §5.2).
    // datumRadius = the radius the observer's ALTITUDE is measured FROM (the
    // nominal surface: altitude/landscape/atmosphere zero-point). groundRadius
    // = the radius the observer cannot descend past in free flight (the ground
    // / floor). Split so an enterable body (datum=ground=0, transparent/fly-into
    // per R4 §11.70) reads altitude-from-centre and descends to the centre,
    // while a terrain-clearance body (datum=radius, ground=radius·1.002) keeps
    // legacy-exact altitudes but stops free descent above the surface. Both
    // equal to `radius` ⇒ bit-identical to a single-reference body. The
    // NAV_RADIUS_UNSET sentinel default means "not authored - resolve to the
    // type's class default" (radius for a plain body, 0 for a ModularSystem):
    // the loader overwrites it with the datum_radius/ground_radius key (default
    // radius) so plain bodies never carry it; the factory system nodes leave it
    // and the ctors resolve it (B10-datum0, §11.75(a)).
    float datumRadius = NAV_RADIUS_UNSET;
    float groundRadius = NAV_RADIUS_UNSET;
    float oblateness; // Not universal - only for pure spherical body modules (so, single-shape body ?) - may provide immense optimisation and quality
    float solLocalDay;
    // New
    Vec3f shadowAbsorbtion;
    float brightness;
    // Spin-phase analytic model (B27 A1, the `sidereal_time` key). Default
    // GENERIC ⇒ an absent key reproduces every body exactly; EARTH_APPARENT is
    // set from the key (composed format) or by the legacy name sniff
    // (applyHardcodedContent). Ordered before bodyType to match the ctor's
    // member-init order.
    SiderealTimeModel siderealTimeModel = SiderealTimeModel::GENERIC;
    // Surface-lighting lineage (B27 A6, the `surface_model` key). Default PLANET
    // ⇒ an absent key reproduces every non-Moon body exactly; LUNAR comes from
    // the key (composed format) or from `type = Moon` (legacy format only, D14).
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    // Trail sample count (B27 A7, the `trail_length` key). Default = the old
    // unknown-type value; the legacy per-class values (Planet/Dwarf 1460, Comet
    // 2920) come from `type` in the legacy format only (D14), from the key in
    // the composed format.
    int trailLength = TRAIL_LENGTH_DEFAULT;
    // Which FORMAT declared this body (D14 §11.79(h) - the retirement of
    // `type`-as-identity is FORMAT-SCOPED, not global). Set by the ONE loader
    // authority (ModularSystem::loadBody) from the system's `composedFile`;
    // false for the factory-built nodes, which carry no `type` data string at
    // all. Consumers ask the body, never the file (I1: a module has no business
    // knowing which parser ran).
    bool composedDeclaration = false;

    // Deprecated
    BodyType bodyType; // Deprecated
    bool isHaloEnabled; // May deprecate
};

//! Minimal size of the system on screen for showing orbiting bodies, in pixels
constexpr int SYSTEM_VISIBILITY_SUBSYSTEM_SIZE = 16;
//! Width (in the same on-screen px units as SYSTEM_VISIBILITY_SUBSYSTEM_SIZE)
//! of the resolved<->dot cross-fade band above the collapse threshold: over
//! [T, T+band) the nested system's interior fades IN while its star-proxy dot
//! fades OUT (B22, INTENT 11.64). TUNABLE — this is in A15's constant set
//! (the collapse threshold family 128/16/0.6/16px), Vixy/tester territory: a
//! tester cannot judge the width without seeing the fade, so this is a
//! DEFENSIBLE DEFAULT (half the threshold ⇒ the fade completes over +50%
//! apparent size, a narrow transition derived from the threshold rather than
//! an independent magic number), NOT a tuned value. Do not tune it here.
constexpr float SYSTEM_COLLAPSE_CROSSFADE_BAND = SYSTEM_VISIBILITY_SUBSYSTEM_SIZE / 2.f;
//! Minimal size of the body (bounding) on screen for showing the outer BodyModule without shadows nor Grounded ModularBody, in pixels
constexpr int BODY_EARLY_VISIBILITY_BOUNDING_SIZE = 2;
//! Minimal size of the body (bounding) on screen for showing the body normally in pixels
constexpr int BODY_FULL_VISIBILITY_BOUNDING_SIZE = 16;
//! Minimal speed while under the area of influence of a body, in body_radius/s
constexpr double MIN_MOVEMENT_SPEED = 0.125;
//! Anti-stuck escape floor for the interactive proximity factor (§5.18 defect,
//! B10 scope iv-b), as a FRACTION OF BODY RADIUS. Radius-relative on purpose:
//! it stays defined when ground_radius == 0 (an enterable body's centre), where
//! a ground_radius-relative epsilon would vanish. Floors the OUTWARD interactive
//! step (Camera::proximityFactor) so height 0 is never a fixed point in any
//! direction (multAlt / moveRelLon / moveRelLat) — the reported "stuck at the
//! surface, can't take off" defect.
//! ---- VALUE SUSPENDED FOR VIXY (B10 carve-out (a)) ----
//! 1e-6·radius ≈ 6.4 m on Earth is a DEFENSIBLE PLACEHOLDER, not a chosen
//! value: it escapes geometrically within ~1-2 s of held input without making
//! surface navigation unusable. It is deliberately NOT MIN_MOVEMENT_SPEED's
//! 0.125 (≈797 km on Earth — an AoI-traversal floor, a different concept and a
//! different unit: a speed, body_radius/s). Open question for Vixy: is the
//! anti-stuck floor the same constant as MIN_MOVEMENT_SPEED (then re-unit and
//! re-value it) or a distinct one (as landed here)? Do not treat 1e-6 as final.
constexpr double ANTISTUCK_ESCAPE_FLOOR = 1e-6;
//! Minimal distance to the center of the body for showing surface BodyModule, in multiple of body radius
constexpr double BODY_SURFACE_HEIGHT = 2;
//! Maximal sizeof a texture to be considered negligible (lazy)
constexpr size_t MAX_LAZY_TEXTURE_SIZE = 4*1024*1024;
constexpr int MAIN_SELF_SHADOWING_RESOLUTION = 8192;
constexpr int SECONDARY_SELF_SHADOWING_RESOLUTION = 2048;

// ModularBody :
// - Relative position (position from orbit, when the body is not visible)
// - [volatile] SystemMatrix (rotation from orbit, feeding back relative position)
// - Axis
// - Rotation
// - Grounded ModularBody
// - Outer orbiting ModularBody
// - Inner ModularBody (shown when inside the AoI)
// - Outer BodyModule (shown when outside the bounding radius)
// - Surface BodyModule (shown instead of Outer BodyModule when the body is the nearest to be bigger than the viewport)
// - Inner BodyModule (shown when inside the body radius but outside the AoI of inner ModularBody)
// - Received shadows (used for caching of the shadow state, allowing to reuse it as long as it's close enough)
// - Grounded EnvironmentModule - Priority over InAoI EnvironmentModule
// - InAoI EnvironmentModule
// - [Loader] All unique instanced BodyModule (by tag)
// - [Loader] All non-unique instanced BodyModule (by name)
// - Body radius
// - Body scaling (Just visual scaling)
// - Bounding radius
// - System Radius
// - Area of Influence radius
// - Separation plane, if any (Mostly used for body rings and milkyway)
// - Brightness (only if it's a star)
// - Projected shadow absorbtion (ex : earth's projected shadow is {0.0, 1.0, 1.0})

// Shadow-projection and depth-buffer strategy: owned by the Renderer - the
// normative blocks moved to Renderer.hpp (they describe render-side buffers,
// not body state). Thread model: see RenderChain.hpp (supersedes the draft
// thread list that lived here).

// ResourcePriority moved to ResourceHub.hpp (resource-layer concept, I2).

// Which of the parent's ownership lists holds a body (the relations block
// above, made live 2026-07-17 - INTENT 11.36): GROUNDED = bound to the
// parent's surface (rover class), ORBITING = standard satellite, INNER =
// inside the parent's volume, shown only while the camera is inside the
// parent's AoI (systems nested in a galaxy). HIDDEN_* = same body parked by
// hide(), still parent-owned, outside every update/draw walk by construction.
enum class BodyRelation {
    HIDDEN_GROUNDED,
    HIDDEN_ORBITING,
    HIDDEN_INNER,
    GROUNDED,
    ORBITING,
    INNER,
};

// hide()/show() translate between a relation and its hidden variant by +-3.
static_assert(static_cast<int>(BodyRelation::GROUNDED) == static_cast<int>(BodyRelation::HIDDEN_GROUNDED) + 3 &&
              static_cast<int>(BodyRelation::ORBITING) == static_cast<int>(BodyRelation::HIDDEN_ORBITING) + 3 &&
              static_cast<int>(BodyRelation::INNER) == static_cast<int>(BodyRelation::HIDDEN_INNER) + 3,
              "hide()/show() map relations by +-3 - keep the enum halves aligned");

enum class BodyModuleType : unsigned char;

class ModularBody {
    // For pointer count and selection modification
    friend class ModularBodyPtr;
    friend class ModularBodySelector;
    // For ModularBody and BodyModule loading
    friend class ModuleLoader;
    // For some ModularBody management
    friend class ModularSystem;
    // For environment chain aggregation (member lists, satellite selection)
    friend class EnvironmentManager;
public:
    // Virtual: children are owned polymorphically (unique_ptr<ModularBody>
    // may hold a nested ModularSystem - G2); destruction through the base
    // pointer requires it. No virtual call sits on any hot walk (draw/update
    // dispatch stays non-virtual) - the cost is one vptr per body.
    virtual ~ModularBody();
    // For emplace_back
    ModularBody(ModularBody *parent, ModularBodyCreateInfo &info);
    // Prevent copy
    ModularBody(const ModularBody &) = delete;
    ModularBody(ModularBody &&) = delete;
    ModularBody &operator=(const ModularBody &) = delete;
    ModularBody &operator=(ModularBody &&) = delete;

    // Create a new child body under the given relation (ownership list).
    // If a body with the same englishName exists, it is replaced by this one.
    // THE registration path (with createChildSystem): pushes into the relation
    // list AND registers into the parent-side owning system's sorted list;
    // ~ModularBody deregisters symmetrically. Only visible relations here -
    // hidden is entered through hide().
    ModularBody *createChild(ModularBodyCreateInfo &info, BodyRelation rel = BodyRelation::ORBITING);
    // Same, constructing a nested ModularSystem (G2: systems are bodies).
    // Default relation INNER: a system lives inside its host's volume and is
    // shown while the camera is inside the host's AoI.
    ModularSystem *createChildSystem(ModularBodyCreateInfo &info, BodyRelation rel = BodyRelation::INNER);
    // The boolean representation of this body is whether it is visible or not
    inline operator bool() const {
        return isVisible & isBodyVisible;
    }
    // The < operator compare the distance to the observer
    inline bool operator<(const ModularBody &other) const {
        return distance < other.distance;
    }
    // The == operator compare the address in memory
    inline bool operator==(const ModularBody &other) const {
        return this == &other;
    }
    // Remove this body, return false on failure
    bool remove(bool recursive = false);
    // Update this body and his childs bodies
    void recursiveUpdate(double jd, const Mat4f &matLocalToBody);
    // Update the cached values
    void updateCache();
    // Recompute the position-derived reach (subsystemRadius + areaOfInfluence)
    // from the CURRENT eclipticPos. Split from updateCache (INTENT 11.62, B15):
    // these track jd EVERY frame (positions move), whereas the module/radius
    // part in updateCache only changes on a radius/scaling/module-load event
    // (the `uncached` latch). Before the split the AoI froze at the first cache
    // (launch jd) -> the reference-transition threshold was stale after any
    // date jump (measured 10.28% drift at the scene jd, §11.62).
    void updateReach();
    // Inform that all childs are no longer visible
    void setChildNoLongerVisible();
    // Minimal updates required to determine if full update is required
    inline void preUpdate(double jd, const Mat4f &preUpdate) {
        const float squaredDistance = preUpdate.r[12] * preUpdate.r[12] + preUpdate.r[13] * preUpdate.r[13] + preUpdate.r[14] * preUpdate.r[14];
        distance = sqrt(squaredDistance);
        if (!hasChildren()) {
            if (distance > boundingRadius) {
                halfAngularSize = atanf(boundingRadius / sqrt(squaredDistance - boundingRadius*boundingRadius));
                const float tmp = cullHalfFov + halfAngularSize;
                isVisible = (tmp > M_PI || (-preUpdate.r[14] >= cos(tmp) * distance));
            } else {
                halfAngularSize = M_PI;
                isVisible = true;
            }
        } else {
            if (distance > subsystemRadius) {
                bool wasChildVisible = isChildVisible;
                isChildVisible = (halfAngularSize > 0.3*halfFov);
                halfAngularSize = atanf(subsystemRadius / sqrt(squaredDistance - subsystemRadius*subsystemRadius));
                const float tmp = cullHalfFov + halfAngularSize;
                if (tmp <= M_PI && (-preUpdate.r[14] < cos(tmp) * distance)) {
                    isVisible = false;
                    if (wasChildVisible)
                        setChildNoLongerVisible();
                    return;
                } else if (wasChildVisible && !isChildVisible)
                    setChildNoLongerVisible();
            } else {
                isChildVisible = true;
            }
            if (distance > boundingRadius) {
                halfAngularSize = atanf(boundingRadius / sqrt(squaredDistance - boundingRadius*boundingRadius));
                const float tmp = cullHalfFov + halfAngularSize;
                isBodyVisible = ((tmp > M_PI) || (-preUpdate.r[14] >= cos(tmp) * distance));
            } else {
                halfAngularSize = M_PI;
                isBodyVisible = true;
            }
            isVisible = isChildVisible | isBodyVisible;
        }
    }
    // Update this body
    inline void update(double jd, const Mat4f &matLocalToBody) {
        screenSize = halfAngularSize/halfFov;
        // Center singularity guard (old-path parity, body.cpp:987-993):
        // at rq→0 the general form is 0/0 — and the TRACKED body sits exactly
        // there once tracking centers it (its halo/hint would ride a NaN
        // screenPos). Small-angle limit: slope0/(distance·halfFov) — 1 for
        // fisheye. Same 1e-5 threshold as the old path; float-acos noise
        // above it stays sub-pixel. (Behind-the-observer rq≈0 keeps the old
        // path's behavior: wrong-but-culled.)
        const float rq = sqrtf(mat.r[12]*mat.r[12] + mat.r[13]*mat.r[13]);
        float f;
        if (projectionMode == ProjectionTransfer::FISHEYE) {
            // The main case (INTENT 11.33): byte-for-byte the historical
            // fast path — non-fisheye modes must not tax it.
            f = (rq > distance * 1e-5f)
                ? acos(-mat.r[14]/distance) / (rq * halfFov)
                : 1.f / (distance * halfFov);
        } else {
            // General radial transfer, same guard structure. CPU must land
            // on the GPU's mapping (custom_project.glsl, spec-const 8) —
            // ProjectionTransfer is the shared authority. The guard branch
            // drops ALLSPHERE's 5.4e-5-NDC constant term (sub-0.1 px,
            // within-guard only; the GPU keeps it).
            f = (rq > distance * 1e-5f)
                ? ProjectionTransfer::radius(projectionMode,
                      acosf(-mat.r[14]/distance) / halfFov, halfFov) / rq
                : ProjectionTransfer::slope0(projectionMode, halfFov)
                      / (distance * halfFov);
        }
        screenPos.first = mat.r[12] * f;
        screenPos.second = mat.r[13] * f;
        axisRotation = computeAxisRotation(jd);
        if (uncached)
            updateCache();       // module/radius part + a fresh updateReach()
        else
            updateReach();       // AoI tracks the current jd every frame (§11.62, B15)
        if (screenSize > 0.004)
            notableBody.push_back(this);
    }

    // Spin phase (rotation about the polar axis) at date jd - THE single
    // authority (I2): consumed both by the per-frame cache (update() above, the
    // visible-body memoization) and by every fresh USE-site readout under the
    // B32 recompute-at-use barrier (D20 §11.79(n), D8 §11.76). Taking jd is what
    // makes it stale-spin-safe (§5.24/B32): a use recomputes from the ROOT-fresh
    // lastJD instead of the visibility-gated axisRotation cache. Use sites:
    // dumpTrace's `axisRot`+`attitude` and dumpHops' `spin` (script-fetch /
    // selection observables); the draw/show channel reads the cache, which is
    // fresh-by-construction there (drawn iff visible iff updated this frame).
    // Branches, each with its reason:
    //   - surfaceLockedAttitude (B24-att, D18 §11.79(l)): a GROUNDED body with
    //     no authored spin is STATIC on the terrain it stands on (a rover sits
    //     still, locked to the surface). No time-varying own spin; only the
    //     fixed rot_rotation_offset phase survives. This is INACTION (no rotation
    //     relative to the surface - D18: "inaction is no rotation"), hence silent
    //     (D12). Explicit rot_periode retires the flag at load and this branch.
    //   - EARTH_APPARENT: apparent sidereal time (B27 A1; the sidereal_time
    //     capability key, no longer englishName=="Earth" / BodyType::EARTH -
    //     §5.5 identity sniff retired, §11.73). Set by the key (composed) or by
    //     the legacy name sniff (applyHardcodedContent, legacy format only, D14).
    //   - default: the generic sidereal spin. re.offset is stored in DEGREES
    //     (shared RotationElements convention, cf old getSiderealTime's degree
    //     formula); adding it raw to a radian formula lagged every non-Earth spin
    //     by offset*(1 - pi/180) (measured on the Moon: 20.7604 deg = 38 deg - 38
    //     rad mod 2pi, §11.16). Absent rot_periode falls to the legacy 24 h
    //     default at load (ModularSystem::loadBody, LOGGED there - D12).
    inline double computeAxisRotation(double jd) const {
        if (surfaceLockedAttitude)
            return re.offset * (M_PI / 180);
        if (siderealTimeModel == SiderealTimeModel::EARTH_APPARENT)
            return get_apparent_sidereal_time(jd) * (M_PI / 180);
        return fmod((jd - re.epoch) / re.period * (2 * M_PI) + re.offset * (M_PI / 180), (2 * M_PI));
    }

    inline float getAxisRotation() const {
        return axisRotation + M_PI_2; // TODO Fix ojml ?
    }

    inline Mat4f computeBodyToSurface() const {
        return Mat4f::zrotation(getAxisRotation());
    }

    inline Mat4f computeSurfaceToBody() const {
        return Mat4f::zrotation(-getAxisRotation());
    }

    // Not inlined because unfrequently called, almost a copy-paste of the loaded test with load-checking before each module draw
    void drawLoaded(Renderer &renderer);
    // Draw this body if it is visible
    inline void draw(Renderer &renderer) {
        if (isVisible & isBodyVisible) {
            if (screenSize > 0.0015) {
                if (loaded) {
                    const auto matrix = mat.multiplyFast(computeBodyToSurface());
                    if (screenSize > 0.008) {
                        if (screenSize < 0.2) {
                            // far (2D behind body) BEFORE clearDepth: the
                            // helper segment carrying them is positioned at
                            // the clearDepth boundary, ahead of this body's
                            // own command buffer - hint circle BEHIND the
                            // disc (old-path parity; measured: the Moon's
                            // hint drew on top of the disc when queued after
                            // the boundary [vixy: 2026-07-12]).
                            for (auto &module : farComponents)
                                module->draw(renderer, this, mat);
                            renderer.clearDepth(distance, boundingRadius);
                            for (auto &module : nearComponents)
                                module->draw(renderer, this, matrix);
                        } else {
                            renderer.clearDepth(distance, boundingRadius);
                            if (distance < scaledRadius) {
                                for (auto &module : inComponents)
                                    module->draw(renderer, this, matrix);
                            } else if (distance < scaledRadius * BODY_SURFACE_HEIGHT) {
                                for (auto &module : groundedComponents)
                                    module->draw(renderer, this, matrix);
                            } else {
                                for (auto &module : nearComponents)
                                    module->draw(renderer, this, matrix);
                            }
                        }
                    } else {
                        for (auto &module : farComponents)
                            module->draw(renderer, this, mat);
                        for (auto &module : nearComponents)
                            module->drawNoDepth(renderer, this, matrix);
                        drawHalo(renderer);
                    }
                } else
                    drawLoaded(renderer);
            } else {
                for (auto &module : farComponents)
                    module->draw(renderer, this, mat);
                drawHalo(renderer);
            }
        }
    }

    // ---- Frame convention of the transform chain (measured against the old
    // path, harness 2026-07-11 - old composition validated exactly) ----------
    // Every ephemeris output (VSOP87, ELP82, ell_orbit with parent rotation
    // BAKED IN at load - see ElipticOrbitLoader) is a vector in the ROOT
    // (ecliptic VSOP87) orientation. Therefore the chain's TRANSLATION frames
    // are all root-aligned ("flat"): hops between bodies are PURE TRANSLATIONS
    // (plus the surface fold for boundToSurface bodies). Violating this
    // (per-hop tilts on translations) rotates every child offset by the
    // parent's obliquity - the confirmed E2 class of the Moon divergence
    // (projection-paths.md C8/C9).
    // ORIENTATIONS are the dual half of the contract (2026-07-17, INTENT
    // 11.34/6.8): a body's `mat` carries its ACCUMULATED equatorial frame
    // (accumulatedBodyPosToBody - the single authority below), NOT its own
    // tilt alone; the camera holds the reference's accumulated frame and
    // dispatchUpdate leaves it exactly once via accumulatedBodyToBodyPos.
    // Translations flat + orientations accumulated is NOT a contradiction:
    // translations are ephemeris vectors (root-aligned by data), orientations
    // are declared rotation elements (nested in the primary's equator by
    // data - the Charon proof). The two halves ride the same chain walk.
    inline void transformParentToBodyPos(double jd, Mat4f &mat_local_to_body) {
        // Light travel time (old-path parity, solarsystem_display.cpp
        // computePositions): the body is seen where it WAS one light-trip ago.
        // Uses the cached observer distance (previous frame) exactly like the
        // old path uses the previous heliocentric positions.
        // distance==distance rejects NaN: a corrupted spatial state must not
        // cross into the time domain - a NaN jd freezes the Kepler solver
        // (elliptic_to_rectangular.c) and with it the whole main loop.
        // The retardation is CLAMPED to its domain of meaning (INTENT 11.36):
        // a galactic-distance observer (reference switched up the hierarchy)
        // retarded solar ephemerides by ~1e9 days - finite, so the NaN
        // barrier passed, but Kepler at absurd dates crawls (measured: the
        // main loop at seconds per frame, bit-identical dumps 150 ms apart).
        // 32 days of light ≈ 5500 AU: beyond that viewing distance the
        // residual retardation error is sub-pixel by construction.
        if (flagLightTravelTime && distance == distance) {
            constexpr double MAX_RETARDATION_DAYS = 32;
            const double retard = distance * (149597870000.0 / (299792458.0 * 86400));
            jd -= (retard < MAX_RETARDATION_DAYS) ? retard : MAX_RETARDATION_DAYS;
        }
        Vec3d tmp;
        if (OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction()) {
            (*oscFunc)(jd,jd,tmp);
        } else {
            orbit->positionAtTimevInVSOP87Coordinates(jd,jd,tmp);
        }
        eclipticPos = tmp;
        lastJD = jd;
        // Surface fold (B24 grounded composition, INTENT 11.78): the PARENT's
        // spin - the walk already hands grounded children the parent's
        // ACCUMULATED equatorial frame (recursiveUpdate frame contract), so
        // composing the parent's axisRotation here completes the parent's
        // SURFACE frame the walk-stop comment above describes ("already
        // carries every ancestor orientation (plus spin)"). The original fold
        // spun by THIS body's own axisRotation - unexercised until the first
        // grounded content, and wrong twice: wrong authority (the camera, the
        // proven surface-standing case, uses the stood-on body's spin -
        // placementRotation), and stale-prone (a grounded child is usually
        // invisible, its own spin state frozen at launch; the parent's spin is
        // fresh whenever this walk runs - its update precedes its children).
        // The child's own rotation elements govern its ATTITUDE only (draw
        // fold) - attitude-default-for-grounded is a recorded design residual.
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(parent->computeBodyToSurface());
        // +ecl: the child sits at +eclipticPos in the parent frame. The chain
        // historically subtracted here (and added on the way up), point-
        // reflecting every body through the reference - measured as
        // eye_root_new == -eye_root_old, the deepest layer of the Moon
        // divergence (harness 2026-07-11).
        mat_local_to_body.multiplyTranslation(eclipticPos);
        // Cache the position frame for the ORBIT pass (row 8): correct for
        // every body regardless of visibility (see the member comment).
        matLocalToBodyPos = mat_local_to_body;
    }

    // Rotation from this body's position-frame (root-aligned) to its own
    // equatorial frame. Matches the old path's rot_local_to_parent exactly
    // (measured: |delta| ~ 4e-8).
    inline Mat4f computeBodyPosToBody(double jd) const {
        return Mat4f::xzrotation(
            re.obliquity,
            re.ascendingNode -re.precessionRate*(jd-re.epoch)
        );
    }

    // Exact inverse of computeBodyPosToBody (zxrotation(z,x) is the transpose
    // of xzrotation(x,z) at identical angles - verified numerically; negating
    // the angles as the previous code did yields the FORWARD tilt in swapped
    // order, not the inverse).
    inline Mat4f computeBodyToBodyPos(double jd) const {
        return Mat4f::zxrotation(
            re.ascendingNode -re.precessionRate*(jd-re.epoch),
            re.obliquity
        );
    }

    // ---- Accumulated equatorial frame - THE single authority (I2) ----------
    // The body's equatorial (lat/lon) frame is ONE concept with TWO consumers
    // - the camera/observer declaration frame and the mesh render frame - and
    // they MUST be identical: the mesh is lat/lon-textured and the observer
    // stands on it (an observer at (lat,lon) is above the mesh point at
    // (lat,lon)). Both consumers route through this pair; composing a body
    // orientation from computeBodyPosToBody directly re-splits the concept
    // (the 23.44deg Moon render/observer contradiction, INTENT 11.34/6.8).
    //
    // Semantics (each clause carries its reason):
    // - SELF tilt is UNCONDITIONAL: the mesh is drawn tilted wherever data
    //   declares it (2(a) honors the Sun's 7.25deg), so the observer frame
    //   must tilt with it - even at system center. (The old path is untilted
    //   at BOTH consumers there - self-consistent, but a model-layer
    //   foreclosure; divergence named, 11.34.)
    // - ANCESTOR tilts participate iff the ancestor is NOT system-centered:
    //   satellite rotation data is authored relative to the primary's equator
    //   (the Charon 1.0deg = "1deg from Pluto's equator" proof, 11.34), but
    //   planets' data is ecliptic-referenced because the old path's parentless
    //   guard kept the Sun out of every accumulation ("heliocentric
    //   coordinates are on ecliptic, not solar equator").
    // - The walk STOPS at a boundToSurface seam: a bound body's position
    //   frame is its parent's SURFACE frame, which already carries every
    //   ancestor orientation (plus spin) - continuing the walk would
    //   double-apply them. The seam ancestor's own tilt still participates
    //   (its satellites nest in its equator like any other primary's).
    // Measured (harness/orientation_check.py, 2026-07-17): matches old's
    // observer EXACTLY (getRotEquatorialToVsop87 parity, 0.0000deg incl. the
    // Moon); per-hop pieces match old to float eps (~4e-8).
    //
    // Exit: equatorial frame -> root-aligned. Product self-first:
    // tilt_self^-1 . tilt_parent^-1 ... (exact inverse of the entry walk).
    inline Mat4f accumulatedBodyToBodyPos(double jd) const {
        Mat4f ret = computeBodyToBodyPos(jd);
        // absoluteTiltFrame (B28, §11.67): this body's tilt is authored in the
        // ecliptic ROOT frame (converted from an absolute J2000 pole), so no
        // ancestor tilt participates. Inert on all current data - the 7
        // absolute_pole planets have a system-centered (Sun) parent, whose tilt
        // this walk already skipped, so ret is unchanged either way.
        if (!boundToSurface && !re.absoluteTiltFrame) {
            for (const ModularBody *b = parent; b && b->isNotIsolated; b = b->parent) {
                if (!b->isSystemCentered())
                    ret = ret.multiplyFast(b->computeBodyToBodyPos(jd));
                if (b->boundToSurface)
                    break;
            }
        }
        return ret;
    }
    // Entry: root-aligned -> equatorial frame. Product parents-first:
    // ... tilt_parent . tilt_self (correct nesting order - the old path's
    // one-hop right-multiply is the wrong side, survived only where the
    // factors commute; 11.34(iii)).
    inline Mat4f accumulatedBodyPosToBody(double jd) const {
        Mat4f ret = computeBodyPosToBody(jd);
        // absoluteTiltFrame (B28, §11.67): see accumulatedBodyToBodyPos above -
        // root-aligned tilt takes no ancestor factor; inert on current data.
        if (!boundToSurface && !re.absoluteTiltFrame) {
            for (const ModularBody *b = parent; b && b->isNotIsolated; b = b->parent) {
                if (!b->isSystemCentered())
                    ret = b->computeBodyPosToBody(jd).multiplyFast(ret);
                if (b->boundToSurface)
                    break;
            }
        }
        return ret;
    }
    // Cached variant (per-node lastJD): for use OUTSIDE the update walk, where
    // no uniform frame jd exists (reference-switch compensation). Differential
    // vs the jd form = precession over light-travel deltas (~1e-9 rad).
    inline Mat4f accumulatedBodyPosToBody() const {
        Mat4f ret = computeBodyPosToBody(lastJD);
        // absoluteTiltFrame (B28, §11.67): root-aligned tilt, no ancestor
        // factor; inert on current data (see accumulatedBodyToBodyPos).
        if (!boundToSurface && !re.absoluteTiltFrame) {
            for (const ModularBody *b = parent; b && b->isNotIsolated; b = b->parent) {
                if (!b->isSystemCentered())
                    ret = b->computeBodyPosToBody(b->lastJD).multiplyFast(ret);
                if (b->boundToSurface)
                    break;
            }
        }
        return ret;
    }

    // Use cached informations from last update. Flat hop: translation only
    // (surface fold for bound bodies) - exact inverse of the cached
    // transformBodyToParent below.
    inline void transformParentToBody(Mat4f &mat_local_to_body) const {
        if (boundToSurface) // PARENT spin - see transformParentToBodyPos
            mat_local_to_body = mat_local_to_body.multiplyFast(parent->computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(eclipticPos);
    }

    inline void transformBodyToParent(double jd, Mat4f &mat_local_to_body) {
        // Same retardation + NaN barrier + absurd-date clamp as
        // transformParentToBodyPos (the up-hop must mirror the down-hop).
        if (flagLightTravelTime && distance == distance) {
            constexpr double MAX_RETARDATION_DAYS = 32;
            const double retard = distance * (149597870000.0 / (299792458.0 * 86400));
            jd -= (retard < MAX_RETARDATION_DAYS) ? retard : MAX_RETARDATION_DAYS;
        }
        Vec3d tmp;
        if (OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction()) {
            (*oscFunc)(jd,jd,tmp);
        } else {
            orbit->positionAtTimevInVSOP87Coordinates(jd,jd,tmp);
        }
        eclipticPos = tmp;
        lastJD = jd;
        if (boundToSurface) {
            // Maybe don't inline this unfrequent case
            // Exact inverse of the fold in transformParentToBodyPos
            // (PARENT spin, see there): [spin | spin*ecl]^-1 = [spin^-1 | -ecl]
            auto tmp = parent->computeSurfaceToBody();
            tmp.r[12] -= eclipticPos[0];
            tmp.r[13] -= eclipticPos[1];
            tmp.r[14] -= eclipticPos[2];
            mat_local_to_body = mat_local_to_body.multiplyFast(tmp);
        } else {
            mat_local_to_body.multiplyTranslation(-eclipticPos);
        }
    }

    // Use cached informations from last update
    inline void transformBodyToParent(Mat4f &mat_local_to_body) const {
        if (boundToSurface) {
            // Maybe don't inline this unfrequent case
            // PARENT spin - see transformParentToBodyPos (B24 fold fix)
            auto tmp = parent->computeSurfaceToBody();
            tmp.r[12] -= eclipticPos[0];
            tmp.r[13] -= eclipticPos[1];
            tmp.r[14] -= eclipticPos[2];
            mat_local_to_body = mat_local_to_body.multiplyFast(tmp);
        } else {
            mat_local_to_body.multiplyTranslation(-eclipticPos);
        }
    }

    inline void selectiveUpdate(double jd, Mat4f mat_local_to_parent) {
        transformParentToBodyPos(jd, mat_local_to_parent);
        preUpdate(jd, mat_local_to_parent);
        if (isVisible) {
            // mat_local_to_parent is now this body's position-frame (flat);
            // recursiveUpdate folds the ACCUMULATED equatorial frame into
            // `mat` only (frame contract above), children keep the flat frame.
            recursiveUpdate(jd, mat_local_to_parent);
        } else {
            mat.r[12] = mat_local_to_parent.r[12];
            mat.r[13] = mat_local_to_parent.r[13];
            mat.r[14] = mat_local_to_parent.r[14];
            // Fulfill the matLocalToBodyPos contract (member doc: "Set on EVERY
            // position update, visible or not"): mat_local_to_parent IS this
            // body's flat position frame here (transformParentToBodyPos ran
            // above) - the same value recursiveUpdate would cache. Without this,
            // an off-screen body's cached frame stayed stale, so the system-level
            // ORBIT/TRAIL passes (which draw every evaluated body, incl. the
            // off-screen ones - row 9 invisible-tick) projected them at their
            // last-visible frame (§11.39's "fully off-screen cache is stale"
            // limitation). Render-frame cache only - zero position-parity impact.
            matLocalToBodyPos = mat_local_to_parent;
            // Old-path parity: positions of non-drawn bodies stay queryable and
            // sortable (the old path updates every body every frame). Refresh
            // the subtree's translations; rotations/visibility stay gated (G4).
            // Bound children live in the surface frame: fold the ACCUMULATED
            // equatorial frame (single authority - same frame the visible
            // branch passes via `mat`), never the own tilt alone.
            if (!groundedBodies.empty()) {
                const Mat4f surfaceFrame = mat_local_to_parent.multiplyFast(accumulatedBodyPosToBody(jd));
                for (auto &c : groundedBodies)
                    c->recursiveTranslationUpdate(jd, surfaceFrame);
            }
            for (auto &c : orbitingBodies)
                c->recursiveTranslationUpdate(jd, mat_local_to_parent);
            for (auto &c : innerBodies)
                c->recursiveTranslationUpdate(jd, mat_local_to_parent);
            updateHiddenBodies(jd, mat_local_to_parent);
        }
    }

    // Translation-only refresh (invisible subtrees): keeps eclipticPos, mat
    // translation and distance current without paying rotations, visibility
    // classification or module updates. See selectiveUpdate else-branch.
    inline void recursiveTranslationUpdate(double jd, Mat4f frame) {
        transformParentToBodyPos(jd, frame);
        mat.r[12] = frame.r[12];
        mat.r[13] = frame.r[13];
        mat.r[14] = frame.r[14];
        // matLocalToBodyPos contract (visible or not): `frame` is this body's
        // flat position frame post-transform - keep the render-frame cache fresh
        // for the system ORBIT/TRAIL passes on deep invisible subtrees. See the
        // selectiveUpdate else-branch note. Render cache only, no parity impact.
        matLocalToBodyPos = frame;
        distance = frame.getTranslation().length();
        if (!groundedBodies.empty()) {
            const Mat4f surfaceFrame = frame.multiplyFast(accumulatedBodyPosToBody(jd));
            for (auto &c : groundedBodies)
                c->recursiveTranslationUpdate(jd, surfaceFrame);
        }
        for (auto &c : orbitingBodies)
            c->recursiveTranslationUpdate(jd, frame);
        for (auto &c : innerBodies)
            c->recursiveTranslationUpdate(jd, frame);
        updateHiddenBodies(jd, frame);
    }

    // Hidden bodies are NOT frozen [vixy 2026-07-21, USER_QUESTIONS Q13
    // verbatim: "It should be where it is now"; INTENT 11.48(a) A10 -> 11.54,
    // which closes the 11.15b(b) suspension]. hide() parks a body in
    // hiddenBodies, a list NO walk visited, so the body kept whatever
    // eclipticPos it last had - for the bodies shipped `hidden = true` that is
    // the position the CONSTRUCTOR evaluated at the parent's then-lastJD.
    // A hidden body is just another NON-DRAWN body, and the requirement for
    // that class was already stated in the selectiveUpdate else-branch
    // ("positions of non-drawn bodies stay queryable and sortable") - so it
    // gets the same mechanism, translation-only: eclipticPos / mat translation
    // / distance stay current, no rotations, no visibility classification, no
    // module updates (a hidden body draws nothing; trail RECORDING while
    // hidden is B11's question, not this one).
    // The relation still names the origin list, hence the per-relation frame -
    // identical to the visible dispatch: HIDDEN_GROUNDED rides the accumulated
    // surface frame, HIDDEN_ORBITING/HIDDEN_INNER the flat position frame.
    // `surface` may be null: it is then derived on demand, i.e. only when a
    // hidden GROUNDED child actually exists. `skip` excludes the body the
    // dispatch came up from (a hidden body can still be the camera reference,
    // and it has already been updated as the walk's root).
    inline void updateHiddenBodies(double jd, const Mat4f &flat,
                                   const Mat4f *surface = nullptr,
                                   const ModularBody *skip = nullptr) {
        if (hiddenBodies.empty())
            return;
        Mat4f derivedSurface;
        for (auto &c : hiddenBodies) {
            if (c.get() == skip)
                continue;
            if (c->relation == BodyRelation::HIDDEN_GROUNDED) {
                if (!surface) {
                    derivedSurface = flat.multiplyFast(accumulatedBodyPosToBody(jd));
                    surface = &derivedSurface;
                }
                c->recursiveTranslationUpdate(jd, *surface);
            } else {
                c->recursiveTranslationUpdate(jd, flat);
            }
        }
    }

    // Update the body system from a given body, return the active system
    static ModularSystem *dispatchUpdate(ModularBody *body, double jd, Mat4f mat_local_to_body);
    inline const std::string &getEnglishName(void) const {
        return englishName;
    }
    inline const std::string &getNameI18n(void) const {
        return nameI18;
    }
    inline ModularBody *getParent() {
        return parent;
    }
    //! Subtree membership: true when `ancestor` is this body or any of its
    //! ancestors. The question "which system does this body belong to?" has no
    //! other answer in this type - `bodyReference` is a GLOBAL name->body map
    //! (findBody/findBodyOnce), so a name lookup alone cannot tell a body of
    //! THIS system from a same-named body another system loaded first (I4: the
    //! membership answer belongs to the tree, not to the name registry).
    //! Client: ModularSystem::generateComposedTwin (INTENT §11.109(c)).
    //! O(depth); the shipped trees are <= 5 deep.
    inline bool isInSubtreeOf(const ModularBody *ancestor) const {
        for (const ModularBody *b = this; b; b = b->parent)
            if (b == ancestor)
                return true;
        return false;
    }
    // Old-path satellite classification (body.cpp:107-124: parent of type
    // CENTER/SUN/STAR => not a satellite; new: SYSTEM/STAR-bit parents).
    // Client: the ORBIT module (planet vs satellite master-flag routing).
    inline bool isSatellite() const {
        return parent && !(parent->isStar() || parent->isSystem());
    }
    //! Read access to the orbit for ephemeris-at-date queries (client:
    //! EnvironmentManager's zodiacal ecliptic-normal sampling, the old
    //! Body::getPositionAtDate form). Null for system-centered bodies.
    inline const Orbit *getOrbit() const {
        return orbit.get();
    }
    //! Current parent-relative position (root-aligned VSOP87). Client: the
    //! ORBIT module's center-notch (old Body::get_ecliptic_pos()).
    inline const Vec3f &getEclipticPos() const {
        return eclipticPos;
    }
    //! Orbit visualization period in days (old re.sidereal_period, the
    //! orbit-line draw gate); 0 = still orbit (no orbit line).
    inline double getSiderealPeriod() const {
        return re.sidereal_period;
    }
    //! Last evaluation jd of this body (client: ORBIT sampling epoch).
    inline double getLastJD() const {
        return lastJD;
    }
    //! This body's POSITION frame (root-aligned; see member). Client: the
    //! ORBIT pass, to reconstruct the PARENT frame the orbit points live in.
    inline const Mat4f &getMatLocalToBodyPos() const {
        return matLocalToBodyPos;
    }
    inline float getRadius() const {
        return radius;
    }
    inline float getScaledRadius() const {
        return scaledRadius;
    }
    // Smallest radius including all groundedComponents and nearComponents
    inline float getBoundingRadius() const {
        return boundingRadius;
    }
    inline const Vec3f &getShadowAbsorbtion() const {
        return shadowAbsorbtion;
    }
    //! The body's spin-phase analytic model (B27 A1). Read by the twin generator
    //! (generateComposedTwin) to materialize the sidereal_time capability key.
    inline SiderealTimeModel getSiderealTimeModel() const {
        return siderealTimeModel;
    }
    // Received-shadow state of this frame (empty when not a receiver).
    // Consumers must also gate on ShadowService::enabled - entries may be
    // stale from the frame the flag was switched off.
    inline const ReceivedShadows &getReceivedShadows() const {
        return receivedShadows;
    }
    inline void setRadius(float _radius) {
        radius = _radius;
        uncached = true;
    }
    // Runtime navigation-radius seams (B10 §5.2 / §11.79(e) D9key). Both in AU
    // (like the `radius` member), the SAME two scalars the ssystem.ini
    // `datum_radius`/`ground_radius` keys land through the loader
    // (ModularSystem.cpp:842-843) - this is the RUNTIME channel for them,
    // reached by `body name <X> datum_radius|ground_radius <km>`. Only the raw
    // member is written; scaling to scaledDatumRadius/scaledGroundRadius is
    // deferred to updateCache (uncached=true) - the SINGLE scaling authority
    // both the load path and this command share (I2, no parallel scaling logic).
    // The scaled value refreshes on this body's next update() (it is the camera
    // reference / a visible body wherever it matters), exactly like setRadius.
    inline void setDatumRadius(float _datumRadius) {
        datumRadius = _datumRadius;
        uncached = true;
    }
    inline void setGroundRadius(float _groundRadius) {
        groundRadius = _groundRadius;
        uncached = true;
    }
    // Halo emission is body state (create-info flagHalo); modules/loaders
    // adjust it through this accessor, never by friendship (BodyModule.hpp
    // rule). First client: OjmLoader - the old Artificial suppressed its
    // halo by overriding drawHalo to nothing (body_artificial.hpp:70-72).
    inline void setHaloEnabled(bool enabled) {
        isHaloEnabled = enabled;
    }
    inline void setScaling(float _scale) {
        scaling = _scale;
        uncached = true;
    }
    // Per-name orbit toggle seam (old Body::setFlagOrbit). Routes to this
    // body's ORBIT module(s) via the dedicated list; setShown is the base
    // no-op for every other module type, so no type knowledge leaks here.
    inline void setFlagOrbit(bool b) {
        for (auto *m : orbitComponents)
            m->setShown(b);
    }
    // Per-name trail toggle seam (old Body::setFlagTrail). Routes to this body's
    // TRAIL module(s) via the dedicated list; setShown is a no-op for every
    // other module type (no name/type sniffing).
    inline void setFlagTrail(bool b) {
        for (auto *m : trailComponents)
            m->setShown(b);
    }
    // Skin-texture seam (old SolarSystemTex -> Body::createTexSkin/switchMapSkin;
    // S6 Textures row, INTENT 9). Broadcast to every module slot; consumers
    // self-select (BodyModule default no-op - no type sniffing here).
    inline void createTexSkin(const std::string &texName) {
        for (auto &m : components)
            if (m)
                m->createTexSkin(texName);
    }
    inline void switchTexSkin(bool use) {
        for (auto &m : components)
            if (m)
                m->switchTexSkin(use);
    }
    // Runtime per-body color seam (old SolarSystemColor::setBodyColor ->
    // Body::setColor; command `body name X color <channel> value r,g,b`, INTENT
    // §11.65). HALO is body-owned (haloColor, consumed by drawHalo); the
    // LABEL/ORBIT/TRAIL channels live on the HINT/ORBIT/TRAIL modules, which
    // self-select on the channel (BodyModule default no-op - the createTexSkin
    // broadcast precedent, no type sniffing). Takes effect on the next drawn
    // frame (draw reads the per-instance member), exactly like old.
    inline void setColor(BodyColorType type, const Vec3f &c) {
        if (type == BodyColorType::HALO || type == BodyColorType::ALL)
            haloColor = c;
        for (auto &m : components)
            if (m)
                m->setColor(type, c);
    }
    // Default halo color seam (old BodyColor::defaultHalo). The LABEL/ORBIT/
    // TRAIL module defaults are module statics set at the same factory seam.
    static inline void setDefaultHaloColor(const Vec3f &c) { defaultHaloColor = c; }
    inline float getRotAscendingnode(void) const {
		return re.ascendingNode;
	}
	inline float getRotObliquity(void) const {
		return re.obliquity;
	}
	//! Axial tilt (obliquity of rotation axis w.r.t. the orbit), in DEGREES, from
	//! the ssystem.ini `axial_tilt` key (RotationElements::axialTilt; unset -> 0).
	//! It is the display-only value the old path drew the planet-grid tropic /
	//! polar circles at: tropic latitude = +/-axialTilt, polar-circle latitude =
	//! +/-(90 - axialTilt). Client: the GRID module (PlanetGridModule). Distinct
	//! from getRotObliquity (that one is radians, pole-derived, drives rotation).
	inline float getAxialTilt(void) const {
		return re.axialTilt;
	}
    // Invalidate the internal state cache
    inline void invalidateCachedState() {
        uncached = true;
    }
    // Calculate a matrix to apply to the observer for a seemless change of body reference
    Mat4f calculateSwitchCompensation(const ModularBody *to) const;
    // Find the nearest common parent between two bodies
    inline const ModularBody *findCommonParent(const ModularBody *with) const {
        for (const ModularBody *common = with; common; common = common->parent) {
            for (const ModularBody *p = this; p; p = p->parent) {
                if (common == p)
                    return common;
            }
        }
        return nullptr;
    }
    // Hide this body, return true if it was shown before this call
    bool hide();
    // Show this body, return true if it was hidden before this call
    bool show();
    inline void preload() {
        for (auto &module : nearComponents) {
            module->preload(this);
        }
        for (auto &module : inComponents) {
            module->preload(this);
        }
    }
    inline const std::pair<float, float> &getScreenPos() const {
        return screenPos;
    }
    // Dual-path trace harness (INTENT.md 11.14): serialize this body's
    // NEW-path transform state as one JSON object (no newline). Read-only.
    void dumpTrace(std::ostream &out) const;
    // Harness: emit the per-hop construction pieces from this body up to the
    // isolated root - for each node: cached ecl/lastJD and the four matrices
    // the chain composes (up = transformBodyToParent, down =
    // transformParentToBody, tilt = computeBodyPosToBody(lastJD), spin =
    // computeBodyToSurface), each applied to identity. Uses the same cached
    // state and the same const methods the real update composes with -
    // reconstruction, labeled as such (fresh regardless of visibility).
    void dumpHops(std::ostream &out) const;
    inline const Vec3f &getHaloColor() const {
        return haloColor;
    }
    // Radius including all orbiting bodies; meaningful only when hasChildren()
    inline float getSubsystemRadius() const {
        return subsystemRadius;
    }
    // Visible (non-hidden) children exist - the update/draw walks' predicate.
    inline bool hasChildren() const {
        return !(groundedBodies.empty() && orbitingBodies.empty() && innerBodies.empty());
    }
    // ANY owned child incl. hidden - the removal-safety predicate (destroying
    // a body destroys its hidden children too; non-recursive remove refuses).
    inline bool ownsAnyChild() const {
        return hasChildren() || !hiddenBodies.empty();
    }
    inline float getScreenSize() const {
        return screenSize;
    }
    inline float getHalfAngularSize() const {
        return halfAngularSize;
    }
    inline Vec3f getObservedPosition() const {
        return mat.getTranslation();
    }
    // Update this body as being the light source
    inline void updateAsLightSource() const {
        lightPosition = mat.getTranslation();
        lightDistance = lightPosition.length();
        lightSize = scaledRadius;
    }
    // Get the distance reference for the altitude (datum_radius, scaled). ONE
    // value, no branch (B10 §5.2): feeds `moveto altitude`, the altitude
    // readout, and the landscape/atmosphere thresholds. == scaledRadius for
    // every default (datum_radius == radius) body.
    inline float getAltitudeReference() const {
        return scaledDatumRadius;
    }
    // The radius the observer cannot descend past in free flight (ground_radius,
    // scaled). == scaledRadius for every default body; 0 for an enterable body.
    inline float getScaledGroundRadius() const {
        return scaledGroundRadius;
    }
    template<class Function>
    static inline void forEach(Function fn) {
        for (auto &b : bodyReference) {
            fn(*b.second);
        }
    }
    // Find a better reference body, return nullptr if this body is the best
    // one. `observerDistance` is the CALLER's (camera's) own distance to this
    // body - the decision must NOT read this->distance: that member belongs
    // to the visibility bookkeeping, which legitimately zeroes it through
    // ancestor transitions (setChildNoLongerVisible) in the same dispatch
    // that precedes the decision - measured as a reference pinned at
    // refDist==0 while the camera sat 5e11 AU out (INTENT 11.36 scene E).
    // The child-capture scan below still reads the children's members
    // (sentinel-guarded): a clobbered frame just retries next frame.
    // Guard: an uncached body (updateCache never ran) has areaOfInfluence 0 -
    // deciding a transition on it would escalate every reference on its first
    // frame; stale cache never drives a switch.
    inline ModularBody *findBetterReference(const float observerDistance) {
        if (uncached)
            return nullptr;
        if (observerDistance > areaOfInfluence)
            return parent;
        for (auto &c : groundedBodies) {
            if (c->isInAreaOfInfluence())
                return c.get();
        }
        for (auto &c : orbitingBodies) {
            if (c->isInAreaOfInfluence())
                return c.get();
        }
        for (auto &c : innerBodies) {
            if (c->isInAreaOfInfluence())
                return c.get();
        }
        return nullptr;
    }
    // Check if a given body exists
    static inline bool exists(const std::string &englishName) {
        return bodyReference.count(englishName);
    }
    // Find a body by name, nullptr if it doesn't exist.
    // Misses are exceptional by design: callers are expected to look up names which exist
    // (scripts, UI selection), so the miss path may pay the exception cost.
    static inline ModularBody *findBody(const std::string &englishName) {
        if (!(lastFit && lastFit->englishName == englishName)) {
            try {
                lastFit = bodyReference.at(englishName); // Don't create null entry if not found
            } catch (...) {
                return nullptr;
            }
        }
        return lastFit;
    }
    // Avoid flushing cache for unique searches, like initial system creation
    static inline ModularBody *findBodyOnce(const std::string &englishName) {
        try {
            return bodyReference.at(englishName); // Don't create null entry if not found
        } catch (...) {
            return nullptr;
        }
    }
    // Slow (O(n) complexity over body count)
    static ModularBody *findBodyNameI18n(const std::string &nameI18);
    // Read access to the per-frame notableBody list (contract below) for the
    // Renderer's depth-bucket partitioning. Lifecycle: cleared at update
    // start (dispatchUpdate - every frame, whichever path draws), filled by
    // update(), read between this update and the next.
    static inline std::vector<ModularBody *> &drainNotableBodies() {
        return notableBody;
    }
    // PRECONDITION of every update/preUpdate in the frame: halfFov must hold
    // the current half field-of-view BEFORE the update pass runs - visibility
    // classification and screenSize derive from it. Set by the frame task
    // (Camera) THROUGH setHalfFov (which maintains cullHalfFov), read
    // everywhere; stale halfFov = silently wrong culling.
    static float halfFov;
    // Projection transfer mode (INTENT 11.33): mirror of the launch-constant
    // Context::projectionType, set once at SSystemFactory construction
    // (post-config). FISHEYE (0) keeps the historical fast path in update();
    // preUpdate's cone tests cull against cullHalfFov = halfFov *
    // edgeAngleNorm(mode) - the angle whose PROJECTED radius is the screen-
    // disc edge: equal to halfFov for FISHEYE/EKISOLID/ASPHERIC, 3.3%
    // tighter for ALLSPHERE. screenSize and screenPos normalization stay on
    // halfFov (angular semantics - regime thresholds keep their meaning).
    static int projectionMode;
    static float cullHalfFov;
    static void setHalfFov(float hf) {
        halfFov = hf;
        cullHalfFov = hf * ProjectionTransfer::edgeAngleNorm(projectionMode, hf);
    }
    static void setProjectionMode(int mode) {
        projectionMode = mode;
        setHalfFov(halfFov);
    }
    // Apply light-travel-time retardation to orbit evaluation (old-path
    // parity). Routed from config/scripts through
    // SSystemFactory::setFlagLightTravelTime, which sets BOTH paths.
    static bool flagLightTravelTime;
    // Halo photometry inputs, old-path parity (Body::object_scale /
    // object_size_limit). Set through SSystemFactory::setScale/setSizeLimit -
    // both-paths seams, like flagLightTravelTime.
    static float haloScale;
    static float haloSizeLimit;
    // System-collapse cross-fade brightness multiplier (B22, INTENT 11.64):
    // 1.0 everywhere except inside ModularSystem::drawNested's transition band.
    // Multiplied into every halo's cmag (drawHaloCore) so the resolved subtree
    // and the star-proxy dot cross-fade instead of hard-switching at the ~16px
    // collapse threshold. Owned/scoped by drawNested (save/set/restore, like
    // lightPosition); default 1.0 ⇒ x1.0f identity for every exercised halo.
    static float drawAlpha;
    // --- Work-domain pin (C2) -----------------------------------------------
    // Pins keep this body's memory alive while work-domain tasks (loading,
    // building) reference it. Plain non-atomic int BY DESIGN: pin() and
    // unpin() are legal ONLY inside render-chain tasks (see RenderChain.hpp) -
    // the chain serializes them. Worker code cannot release: it transfers its
    // hold into the publish task, which unpins on the chain.
    inline void pin() {
        ++pins;
    }
    // Unpin; if this body was parked for destruction (removed from the tree
    // while pinned) and this was the last pin, destruction happens now.
    void unpin();
    // distance == 0 is the NOT-EVALUATED sentinel (setChildNoLongerVisible
    // zeroes it on visibility transitions; drawSystem's break rule reads it) -
    // it must never read as "at zero distance": an unevaluated child captured
    // the camera reference through 0 <= AoI (measured: the Moon captured from
    // 2.4e-3 AU away; Sun<->SolarSystem reference livelock after a transition
    // zeroed the ex-reference - INTENT 11.36 scene E probes).
    inline bool isInAreaOfInfluence() const {
        return distance > 0 && distance <= areaOfInfluence;
    }
    // Must be called after update as it depends on updated values
    inline float getPhase() const {
        const Vec3f heliopos = mat.getTranslation() - lightPosition;
        const float Rq = heliopos.lengthSquared();
        const float pq = distance*distance;
        const float cos_chi = (pq + Rq - lightDistance*lightDistance)/(2.0*distance*sqrt(Rq));
        return ((M_PI - acos(cos_chi)) * cos_chi + sqrt(1.0 - cos_chi*cos_chi)) / M_PI;
    }
    // Must be called after update as it depends on updated values
    inline float computeMagnitude() const {
        float factor;
        if (isStar()) {
            // Dims with distance: old path body_sun.cpp:86 is
            // -26.73 + 2.5*log10(d^2), i.e. factor = 1/d^2 in the shared
            // -2.5*log10(factor) tail below. Was factor = d^2 - inverted,
            // stars BRIGHTENED with distance; exact at 1 AU where the factor
            // is 1, which is why every near-Earth parity scene passed over it
            // (found at the first far-star live surface, INTENT 11.80).
            factor = 1.f / (distance*distance);
        } else {
        	const Vec3f heliopos = mat.getTranslation() - lightPosition;
        	const float Rq = heliopos.lengthSquared();
        	const float pq = distance*distance;
        	const float cos_chi = (pq + Rq - lightDistance*lightDistance)/(2.0*distance*sqrt(Rq));
            const float phase = ((M_PI - acos(cos_chi)) * cos_chi + sqrt(1.0 - cos_chi*cos_chi)) / M_PI;
            factor = 2 * albedo * scaledRadius * scaledRadius * phase / (3 * pq * Rq);
        }
        return -26.73f - 2.5f*log10f(factor);
    }
    static void setTranslator(Translator &_translator);
    inline float getDistanceToObserver() const {
        return distance;
    }
    // Harness/diagnostic accessors (INTENT 11.36 scene E instrument): the
    // reference-transition inputs, observable from the camera dump.
    inline float getAreaOfInfluence() const {
        return areaOfInfluence;
    }
    inline bool isCacheFresh() const {
        return !uncached;
    }
    // Return true if this body has the STAR bit set, meaning it emit light
    inline bool isStar() const {
        return (bodyType & BodyType::STAR) == BodyType::STAR;
    }
    // Return true if this body is a MINOR_BODY: mass-instanced small body,
    // EXEMPT from inter-body shadowing (D3, §2.0). The capability the three
    // shadow-caster/receiver sweeps actually ask for - asking it here instead of
    // comparing the enum inline keeps ONE authority for "what MINOR_BODY means"
    // (I2/I4) and is what the composed `shadow_exempt` key now sets (B27 Tier B).
    inline bool isMinorBody() const {
        return bodyType == BodyType::MINOR_BODY;
    }
    // The declared surface-lighting lineage (B27 A6, `surface_model`).
    inline SurfaceModel getSurfaceModel() const {
        return surfaceModel;
    }
    // The declared trail sample count (B27 A7, `trail_length`).
    inline int getTrailLength() const {
        return trailLength;
    }
    // True iff this body was declared in the COMPOSED format (D14 §11.79(h)):
    // the format in which `type`-as-identity is retired. A module loader asks
    // this - never which file was parsed (I1).
    inline bool isComposedDeclared() const {
        return composedDeclaration;
    }
    // Return true if this body is a system - STRUCTURAL test (isolation
    // root), not the enum: a ModularSystem is a system whatever its bodyType
    // says (SYSTEM, GALAXY milkyway, future protosystem types). Asking the
    // capability instead of the name (the enum stays descriptive data).
    inline bool isSystem() const {
        return !isNotIsolated;
    }
    // Return true if this body is at the center of his system
    // (2026-07-17 fix: the loop tested THIS body's eclipticPos at every
    // level - loop-invariant, ancestors never examined - so an ecl==0 body
    // parented to an OFF-CENTER parent (script-reachable: a pedagogical
    // construct parked at a planet center, the R1 generality 2 relies on)
    // read as system-centered and lost its ancestors' tilt participation.)
    inline bool isSystemCentered() const {
        const ModularBody *body = this;
        while (body->isNotIsolated) {
            if (body->eclipticPos.v[0] || body->eclipticPos.v[1] || body->eclipticPos.v[2])
                return false;
            body = body->parent;
        }
        // Structural top test (matches isSystem): the loop exits at the
        // isolated root, which IS a system whatever its enum (the previous
        // enum==SYSTEM test read GALAXY/universe roots as false - a star at
        // a galaxy's center would have lost its ancestors' participation).
        return !body->isNotIsolated;
    }

    // Camera reference transition wiring points (Camera ctor/switchToBody/
    // warpToBody). The edge SEMANTICS (enter/leave on the environment
    // members) are owned by EnvironmentManager's per-frame chain diff -
    // single authority, so endpoint switches and deep warps produce
    // identical edges and shared ancestors never see spurious leave/enter
    // pairs. These hooks stay as the structural wiring for a future
    // edge-driven (non-polled) form.
    inline void enterEnvironment() {
    }
    inline void leaveEnvironment() {
    }
    // Install an environment member (EnvironmentModule.hpp). grounded =
    // active only while the camera is anchored on this body (landscape
    // class); otherwise active whenever this body is on the camera's
    // reference chain (InAoI class: atmosphere, milkyway).
    inline void addEnvironment(std::unique_ptr<EnvironmentModule> &&module, bool grounded) {
        (grounded ? groundedEnvironment : environment).push_back(std::move(module));
    }
    inline const Mat4f &getMat() const {
        return mat;
    }
    // Per-body environment data (old AtmosphereParams thresholds - see
    // BodyEnvironmentParams; parsed by ModularSystem::loadBody from the
    // same ssystem.ini keys, same defaults).
    BodyEnvironmentParams envParams;
    static inline const Vec3f &getLightPosition() {
        return lightPosition;
    }
    inline float getOneMinusOblateness() const {
        return one_minus_oblateness;
    }
    inline float getLightHalfAngle() const {
        return atan(lightSize/(lightPosition-mat.getTranslation()).length());
    }
    double getSiderealDay(void) const {
        return re.period;
    }
    double getSiderealTime(double jd) const {
        if (siderealTimeModel == SiderealTimeModel::EARTH_APPARENT)
            return get_apparent_sidereal_time(jd);
    	return fmod((jd - re.epoch) / re.period * 360. + re.offset, 360);
    }
    //! get BodyModule by name
    BodyModule *slot(StringID slot)
    {
        if (components.size() <= slot.id)
            return nullptr;
        return components[slot.id].get();
    }
    //! get BodyModule by name
    const BodyModule *slot(StringID slot) const
    {
        if (components.size() <= slot.id)
            return nullptr;
        return components[slot.id].get();
    }
    // Install a module into a slot, taking ownership.
    // Erases the *replaced* module's routing (far/near/grounded/in) only - the new module
    // is expected to have routed itself during load (see ModuleLoader loading contract).
    void slot(StringID slotID, std::unique_ptr<BodyModule> &&module)
    {
        if (components.size() <= slotID.id) {
            components.resize(slotID.id + 1);
        } else if (auto *slot = components[slotID.id].get()) {
            std::erase(farComponents, slot);
            std::erase(nearComponents, slot);
            std::erase(groundedComponents, slot);
            std::erase(inComponents, slot);
            std::erase(orbitComponents, slot);
            std::erase(trailComponents, slot);
            std::erase(tailComponents, slot);
        }
        components[slotID.id] = std::move(module);
    }
    static StringIDCluster slotID;
    static Tracer tracer;
private:
    // Deduce which modules are to be bound to this body from the parameters
    std::vector<BodyModuleType> deduceBodyModuleList(std::map<std::string, std::string> &param);
    void select();
    void deselect();
    // FAITHFUL PORT of the old Halo::computeHalo + drawHalo (halo.cpp:82-167):
    // identical adaptLuminance ARGUMENT and identical outer multiplier - NOT a
    // re-derivation. adaptLuminance is a state-dependent power law
    // (pow(w·π·1e-4, alpha_wa/alpha_da) - tone_reproductor.hpp:93), so
    // re-derived constants can only agree at ONE adaptation state: the
    // previous form drew Saturn's halo visibly bigger and Neptune's brighter
    // than the old path (A/B captures, 2026-07-12, INTENT 11.19a). Every rule
    // below carries its old-path line reference; divergences here are visual
    // divergences between paths by construction.
    // Core of the faithful port with the INPUTS parameterized (magnitude,
    // px disc floor, color, satellite rules): self-draw passes its own state
    // (drawHalo below - values byte-identical to the pre-factoring form);
    // the far-system star proxy (ModularSystem::drawStarProxy) passes its
    // star's photometry at the system node's position (INTENT 11.36).
    inline void drawHaloCore(Renderer &renderer, const float mag, const float screen_r, const Vec3f &color, const bool satelliteRules) {
        const float fov_deg = halfFov * (360.f / M_PI); // = old prj->getFov()
        float fov_q = (fov_deg > 60.f) ? 60.f : fov_deg; // halo.cpp:111-113
        fov_q = 1.f / (fov_q * fov_q);
        rmag = sqrtf(renderer.adaptLuminance((expf(-0.92103f*(mag + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * ModularBody::haloScale;
        if (satelliteRules) { // halo.cpp:117-120 (satellites only, NOT all non-stars)
            rmag /= (fov_deg > 60.f) ? 25.f : 5.f;
        }
        cmag = 1.f;
        if (rmag < 1.2f) { // halo.cpp:125-131 (anti-blink)
            cmag = (mag > 0.f) ? (rmag*rmag/1.44f) : (rmag/1.2f);
            if (mag > 6.5f)
                cmag *= rmag*rmag/1.44f;
            rmag = 1.2f;
        } else { // halo.cpp:134-141 (size-limit compression)
            const float limit = ModularBody::haloSizeLimit/1.8f;
    		if (rmag > limit) {
    			rmag = limit + sqrtf(rmag-limit)/(limit + 1);
    			if (rmag > ModularBody::haloSizeLimit)
    				rmag = ModularBody::haloSizeLimit;
    		}
        }
        cmag *= 0.5f*rmag/screen_r; // halo.cpp:144-151
        if (cmag > 1.f)
            cmag = 1.f;
        if (rmag < screen_r) {
    		cmag *= rmag/screen_r;
    		rmag = screen_r;
    	}
        if (satelliteRules) {
            // Eclipse-behind-parent rule, satellites ONLY (halo.cpp:153-163).
            // For planets the parent IS the light source: OP=0 -> 0/0 (the
            // previous all-non-star form survived on NaN-compares-false).
            const Vec3f _planet = parent->mat.getTranslation() - lightPosition;
            const Vec3f _satellite = mat.getTranslation() - lightPosition;
            const double c = _planet.dot(_satellite);
            const double OP = _planet.length();
    		const double OS = _satellite.length();
            if (c > 0 && OP < OS && fabs(acos(c/(OP*OS))) < atan(parent->radius/OP)) {
                cmag = 0.0;
            }
        }
        // System-collapse cross-fade (B22): scale brightness by the active
        // fade alpha. 1.0 in every exercised path (x1.0f is exact), non-1 only
        // inside ModularSystem::drawNested's band. Applied to cmag (brightness),
        // not rmag (size): the halo dims to nothing rather than shrinking, so a
        // fading dot and a fading-in interior cross-dissolve at the same screen
        // position. Placed before the skip rule so a fully-faded halo (alpha→0)
        // drops out consistently.
        cmag *= drawAlpha;
        if (rmag < 1.21f && cmag < 0.05f) // halo.cpp:86 skip rule (old draws
            return; // big-but-dim halos; the previous cmag-only gate dropped them)
        renderer.drawHalo(screenPos, color * cmag, rmag);
    }
    inline void drawHalo(Renderer &renderer) {
        // screen_r: old getOnScreenSize (body.cpp:668-671) = FULL angular
        // diameter over the viewport HEIGHT = screenSize·2·viewportRadius
        // (the ·viewportRadius form halved every disc-floored halo).
        drawHaloCore(renderer, computeMagnitude(), screenSize * 2.f * viewportRadius,
                     haloColor, isSatellite());
    }
    // Identity
    std::string englishName;
    std::string nameI18;

    // Relations - ownership by relation (single authority: `relation` says
    // which list of the parent owns this body; boundToSurface is its hot-path
    // cache, written only at createChild*). Children are polymorphic
    // (unique_ptr): a ModularSystem nests in a system like any body (G2).
    // Registration contract: the base ctor does NOT register anywhere -
    // createChild/createChildSystem are the only entry (list push + owning-
    // system addBody); ~ModularBody deregisters symmetrically.
    ModularBody *parent;
    BodyRelation relation = BodyRelation::ORBITING; // meaningless for parentless roots
    std::vector<std::unique_ptr<ModularBody>> groundedBodies;
    std::vector<std::unique_ptr<ModularBody>> orbitingBodies;
    std::vector<std::unique_ptr<ModularBody>> innerBodies;
    std::vector<std::unique_ptr<ModularBody>> hiddenBodies; // parked by hide(), still owned

    // The ownership list a relation designates (hidden variants -> hiddenBodies).
    inline std::vector<std::unique_ptr<ModularBody>> &listOf(BodyRelation r) {
        switch (r) {
        case BodyRelation::GROUNDED: return groundedBodies;
        case BodyRelation::ORBITING: return orbitingBodies;
        case BodyRelation::INNER:    return innerBodies;
        default:                     return hiddenBodies;
        }
    }
    // Iterate the visible children (grounded+orbiting+inner; hidden excluded
    // by construction). Frame-sensitive walks iterate the lists directly
    // (grounded receive the surface frame per-list); this helper serves the
    // frame-insensitive sites.
    template<class F>
    inline void forEachVisibleChild(F &&fn) {
        for (auto &c : groundedBodies) fn(*c);
        for (auto &c : orbitingBodies) fn(*c);
        for (auto &c : innerBodies) fn(*c);
    }
    inline void clearChildren() {
        groundedBodies.clear();
        orbitingBodies.clear();
        innerBodies.clear();
        hiddenBodies.clear();
    }
    // Register a freshly-created child into the owning system's sorted list
    // (walk from THIS body: the parent side - a nested system registers in
    // its host's system, its own content registers in itself).
    void registerToSystem(ModularBody *child);
    std::vector<std::unique_ptr<EnvironmentModule>> groundedEnvironment;
    std::vector<std::unique_ptr<EnvironmentModule>> environment;
    // The "Received shadows" relation realized (S5/G7): per-frame received-
    // shadow state, produced by the ModularSystem orchestration, consumed by
    // drawing modules (contract: ShadowProjection.hpp).
    ReceivedShadows receivedShadows;

    // TODO create an optimized std::string for limited set
    std::vector<std::unique_ptr<BodyModule>> components; // Reference every BodyModule of this ModularBody by name

    std::vector<BodyModule *> farComponents; // 2D behind body, SKIP when screenSize > 20%, update NEVER called
    std::vector<BodyModule *> nearComponents; // Drawn if screenSize >= 0.15% and distance > scaledRadius * BODY_SURFACE_HEIGHT
    std::vector<BodyModule *> groundedComponents; // Drawn if distance <= scaledRadius * BODY_SURFACE_HEIGHT
    std::vector<BodyModule *> inComponents; // Draw if distance <= scaledRadius
    std::vector<BodyModule *> orbitComponents; // Orbit lines (row 8): drawn in the system-level orbit pass (ModularSystem::drawOrbits), not a screen-size regime
    std::vector<BodyModule *> trailComponents; // Trail lines (row 9): swept every frame by the system-level trail pass (ModularSystem::drawTrails) so accumulation continues while invisible, not a screen-size regime
    std::vector<BodyModule *> tailComponents; // Comet tails (row 12): instanced batch swept as a system phase (ModularSystem::drawTails) so update() ticks and the batch flushes once, not a screen-size regime
    // std::list<std::shared_ptr<BodyOrbitModule>> orbitalComponents; // Components drawing lines between bodies
    // std::list<std::shared_ptr<EnvironmentModule>> environmentComponents; // Component defining the environment

    // Positionnal
    std::unique_ptr<Orbit> orbit;
    RotationElements re;

    // Cached data (may deprecate)
    Mat4f mat; // Matrix defining this body regarding to the observer
    // This body's POSITION frame (root-aligned, "flat"): parent position frame
    // . translation(eclipticPos). Its ROTATION is the reference eye frame
    // (identical for every body), its TRANSLATION is this body's eye position.
    // Set on EVERY position update (visible or not - unlike `mat`, whose
    // rotation is stale for out-of-cone bodies), so the ORBIT pass can place a
    // child's orbit in its parent's frame reliably (row 8). Identity until the
    // first update.
    Mat4f matLocalToBodyPos = Mat4f::identity();
    Vec3f eclipticPos;
    std::pair<float, float> screenPos;
    float halfAngularSize = 0; // 0 until first update (uninit class, INTENT 5.16/11.28c/11.32)
    float screenSize = 0; // Ratio of the screen taken by this body; 0 until
                          // first update - hidden/frozen bodies never update
                          // and dumped garbage otherwise (-nan broke the
                          // harness JSON, INTENT 11.32)
    // = 0 until first evaluated: uninitialized, it fed garbage into the
    // light-travel retardation (jd - garbage -> non-converging Kepler solve,
    // frozen main loop) - first-frame value 0 matches the old path's
    // zero-initialized previous positions.
    float distance = 0;
    // = 0 until first update for the same reason as distance (the sibling of
    // that class): never-updated bodies (invisible, dist == 0) dump/read raw
    // heap garbage otherwise - surfaced 2026-07-16 as a heap-layout-dependent
    // "-nan" in dual_dump JSON (invalid token, predict.py hard-stop).
    float axisRotation = 0;
    // = 0 until the first updateCache(), same read-before-write class as
    // distance/axisRotation above and as §5.9's boundingRadius (§11.101(h)):
    // written ONLY by updateCache(), so an uninitialized read returns heap
    // garbage. Today no such read happens - loadBody runs updateCache() at the
    // end of every body load, so a grounded child reading its parent's
    // getAltitudeReference() finds it written - but that is protection by CALL
    // ORDERING, held by callers; zero-init moves it to CONSTRUCTION, where the
    // type holds it and no future caller order can lose it.
    float scaledRadius = 0;
    // Scaled navigation radii (raw datum/ground * scaling, recomputed with
    // scaledRadius in updateCache — B10 §5.2). scaledDatumRadius is the
    // successor of the commented-out `scaledInnerRadius` drafting residue.
    float scaledDatumRadius = 0;
    float scaledGroundRadius = 0;
    float rmag;
    float cmag;
    double lastJD = 0;

    // Halo system
    Vec3f haloColor;
    float albedo;					// Body albedo

    // Per-channel absorption of the shadow this body PROJECTS (shadow_color
    // param; Earth {0,1,1}: absorbs G/B, not R -> red umbra - the atmosphere
    // diffraction emulation [vixy: 2026-07-12], shadow-paths.md B5). Applied
    // at receive time by the receiver's shader.
    Vec3f shadowAbsorbtion;

    // Navigation and visibility
    ASmooth<AsyncHub, float, 5.f> scaling;
    float radius;
    // Raw (unscaled) navigation radii, both defaulting to `radius` (B10 §5.2).
    // See ModularBodyCreateInfo for the datum/ground roles.
    float datumRadius;
    float groundRadius;
    float boundingRadius; // Smallest radius including all groundedComponents and nearComponents
    float subsystemRadius; // Radius including all orbitingBodies
    float areaOfInfluence; // Area under the influence of this body

    // Internal datas, deprecated
    float one_minus_oblateness;
    float solLocalDay;			//time of a sideral day in this planet
    uint8_t pointerCount = 0; // Number of pointer pointing this object
    // Work-domain pin count (C2). Deliberately separate from pointerCount:
    // pointerCount is a render-local UI concept (ModularBodyPtr), pins guard
    // memory against in-flight work tasks. Both are chain/render-serialized,
    // neither needs atomics. (I2: one info, one domain.)
    int pins = 0;
    bool parked = false; // Removed from tree while pinned; destroyed at last unpin
    BodyType bodyType;
    // Spin-phase analytic model (B27 A1, `sidereal_time` key). GENERIC for every
    // body except the apparent-sidereal-time model (Earth); consumed by
    // computeAxisRotation / getSiderealTime. Copied from createInfo in the ctor.
    SiderealTimeModel siderealTimeModel = SiderealTimeModel::GENERIC;
    // Surface-lighting lineage (B27 A6, `surface_model` key), consumed by
    // LayeredMeshLoader. Resolved once at load (ModularSystem::loadBody, the one
    // data->capability authority) so no module ever re-reads the `type` string.
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    // Trail sample count (B27 A7, `trail_length` key), consumed by TrailLoader.
    int trailLength = TRAIL_LENGTH_DEFAULT;
    // Declaring format (D14 §11.79(h)). See ModularBodyCreateInfo.
    bool composedDeclaration = false;
    bool isHaloEnabled;
    bool isVisible = false;
    bool isBodyVisible = true;
    bool isChildVisible = false;
    bool isNotIsolated = true; // If false, update and draw are not transmitted to the parent body
    bool isSelected = false;
    bool isCenterOfInterest = false;
    bool inParent = false; // Determine whether this body is potentially inside his parent
    bool uncached = true; // Determine whether this body require any update
    bool loaded = false; // Determine whether all nearComponents and inComponents are fully loaded
    bool boundToSurface = false; // Hot-path cache of (relation == GROUNDED) - written by createChild* only
    // Attitude default (B24-att, D18 §11.79(l)): when set, this body's mesh is
    // STATIC in its (parent-surface) frame - computeAxisRotation drops the
    // time-varying own spin, keeping only the fixed rot_rotation_offset phase.
    // Resolved once at load (ModularSystem::loadBody: grounded AND no authored
    // rot_periode); false everywhere else, so every non-grounded / explicit-spin
    // body is bit-identical. Owner = the loader (attitude resolution site, I2/I4);
    // no per-draw sniffing - all consumers ride computeAxisRotation's constant.
    bool surfaceLockedAttitude = false;

    // Global datas
    // Light state is CURRENT-SYSTEM-scoped: updateSystem writes the system's
    // star; a nested system draw (ModularSystem::drawNested) saves it, runs
    // with ITS star, and restores before the enclosing system's remaining
    // bodies. Starless systems leave it untouched (their bodies don't read it
    // outside the delegation path).
    static Vec3f lightPosition; // Observer-local light source position
    static float lightDistance; // Observer-local light source distance
    static float lightSize;     // Light source radius
    static ModularBody *lastFit;
    static Translator *translator;
    static std::map<std::string, ModularBody *> bodyReference;
    // Bodies sufficiently large on screen to need a depth bucket this frame.
    // Producer: update() (pushes when screenSize > threshold). Consumer: the
    // Renderer's depth-range partitioning (splits the full depth range between
    // these bodies according to their needs - no single range can hold both
    // parent-scale distances and child-scale detail, D1/D2). CONTRACT: the
    // consumer drains (clears) this list every frame - an undrained frame
    // leaks entries and corrupts the next frame's partitioning.
    static std::vector<ModularBody *> notableBody;
    static Vec3f defaultHaloColor;
    static std::shared_ptr<BodyTesselation> bodyTesselation; // both-paths seam (setTesselation)
public:
    // Frame geometry, same public-precondition class as halfFov (set by
    // dispatchUpdate from the VulkanMgr scissor; consumed by drawHalo's px
    // conversion, the pointer service and drawSystem's px conversion).
    static float viewportRadius;
    // Frame clock in MILLISECONDS (the old-path fader/animation convention -
    // LinearFader::update takes ms ticks). Same precondition class as halfFov:
    // written ONCE per frame by Camera::update before any body update runs;
    // consumers are per-frame animations (module faders, pointer breathing).
    // NOT a physics/simulation dt - orbital time comes from jd only.
    static float deltaTime;

    // Both-paths tesselation seam (row 2/13, 2026-07-15): the SAME shared
    // BodyTesselation object the old path reads, injected where the old path
    // injects its own (SolarSystemTex ctor -> Body::setTesselation mirror).
    // Sharing the OBJECT (not copying values) makes animated `body
    // tesselation` transitions identical by construction (values are
    // Scalable, ticked by the old-path update - relocate that tick when the
    // old path is retired, INTENT retirement map). Consumers: LayeredMesh
    // (TesParam + altimetry level), AtmExtModule (min/max TesParam).
    static void setTesselation(std::shared_ptr<BodyTesselation> t) {
        bodyTesselation = std::move(t);
    }
    static const std::shared_ptr<BodyTesselation> &getTesselation() {
        return bodyTesselation;
    }

    // The selected body, nullptr when none - the aggregate view of the
    // per-body isSelected flag (single authority: maintained by
    // select()/deselect(), which only ModularBodySelector may call; redirect()
    // keeps it valid across body replacement like every ModularBodyPtr).
    static inline ModularBody *getSelected() {
        return selectedBody;
    }
private:
    static ModularBody *selectedBody;
};

#endif /* end of include guard: MODULAR_BODY_HPP_ */
