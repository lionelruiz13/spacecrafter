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
// express: STAR = emits light (bit-tested via isStar()); EARTH/EARTH_MOON =
// hard-coded specificities, applied only through applyHardcodedContent when
// the data says hardcoded=true; MINOR_BODY = mass-instanced small bodies
// (e.g. upscaled asteroid ring): many visible at once, exempt from
// inter-body shadowing, cluster-optimizable.
// SPHERICAL_BODY/SINGLE_BODY/CUSTOM_BODY: pre-composition remnants - fate
// decided in the second pass (INTENT.md 6.3).
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
    // Bodies with hard-coded specificities
    EARTH = 0x80,
    EARTH_MOON,
};

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

struct ModularBodyCreateInfo {
    std::unique_ptr<Orbit> orbit;
    std::string englishName;
    RotationElements re;
    Vec3f haloColor;
    float albedo;
    float radius;
    float oblateness; // Not universal - only for pure spherical body modules (so, single-shape body ?) - may provide immense optimisation and quality
    float solLocalDay;
    // New
    Vec3f shadowAbsorbtion;
    float brightness;

    // Deprecated
    BodyType bodyType; // Deprecated
    bool isHaloEnabled; // May deprecate
    bool altitudeRelativeToRadius; // Deprecated
};

//! Minimal size of the system on screen for showing orbiting bodies, in pixels
constexpr int SYSTEM_VISIBILITY_SUBSYSTEM_SIZE = 16;
//! Minimal size of the body (bounding) on screen for showing the outer BodyModule without shadows nor Grounded ModularBody, in pixels
constexpr int BODY_EARLY_VISIBILITY_BOUNDING_SIZE = 2;
//! Minimal size of the body (bounding) on screen for showing the body normally in pixels
constexpr int BODY_FULL_VISIBILITY_BOUNDING_SIZE = 16;
//! Minimal speed while under the area of influence of a body, in body_radius/s
constexpr double MIN_MOVEMENT_SPEED = 0.125;
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
        if (bodyType == BodyType::EARTH) {
            axisRotation = get_apparent_sidereal_time(jd) * (M_PI / 180);
        } else {
            // re.offset is stored in DEGREES (shared RotationElements convention,
            // cf old getSiderealTime's degree formula); adding it raw to a radian
            // formula lagged every non-Earth spin by offset*(1 - pi/180)
            // (measured on the Moon: 20.7604 deg, exactly 38 deg - 38 rad mod 2pi).
            axisRotation = fmod((jd - re.epoch) / re.period * (2 * M_PI) + re.offset * (M_PI / 180), (2 * M_PI));
        }
        if (uncached)
            updateCache();
        if (screenSize > 0.004)
            notableBody.push_back(this);
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
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(computeBodyToSurface());
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
        if (!boundToSurface) {
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
        if (!boundToSurface) {
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
        if (!boundToSurface) {
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
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(computeBodyToSurface());
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
            // Exact inverse of the fold in transformParentToBodyPos:
            // [spin | spin*ecl]^-1 = [spin^-1 | -ecl]
            auto tmp = Mat4f::zrotation(-M_PI_2 - axisRotation);
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
            auto tmp = Mat4f::zrotation(-M_PI_2 - axisRotation);
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
    inline float getRotAscendingnode(void) const {
		return re.ascendingNode;
	}
	inline float getRotObliquity(void) const {
		return re.obliquity;
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
    // Get the distance reference for the altitude
    inline float getAltitudeReference() const {
        return altitudeRelativeToRadius ? scaledRadius : 0;
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
            factor = distance*distance;
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
        if (bodyType==BodyType::EARTH)
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
    float scaledRadius;
    //float scaledInnerRadius;
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
    bool altitudeRelativeToRadius = true; // Determine whether observer's altitude on this body is relative to the radius

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
