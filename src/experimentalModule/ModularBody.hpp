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
#include <map>
#include <vector>

// (TEXMAP*/TEX big-texture mapping macros moved to tools/s_texture.hpp -
//  they are texture utilities, not body concepts.)

class BodyTesselation; // both-paths tesselation seam (see setTesselation)

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

enum class SiderealTimeModel : unsigned char {
    GENERIC,
    EARTH_APPARENT,
};

enum class SurfaceModel : unsigned char {
    PLANET,
    LUNAR,
};

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

constexpr float NAV_RADIUS_UNSET = -1.f;

struct ModularBodyCreateInfo {
    std::unique_ptr<Orbit> orbit;
    std::string englishName;
    RotationElements re;
    Vec3f haloColor;
    float albedo;
    float radius;
    float datumRadius = NAV_RADIUS_UNSET;
    float groundRadius = NAV_RADIUS_UNSET;
    float oblateness; // Not universal - only for pure spherical body modules (so, single-shape body ?) - may provide immense optimisation and quality
    float solLocalDay;
    // New
    Vec3f shadowAbsorbtion;
    float brightness;
    SiderealTimeModel siderealTimeModel = SiderealTimeModel::GENERIC;
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    int trailLength = TRAIL_LENGTH_DEFAULT;
    bool composedDeclaration = false;
    bool primary = false;

    // Deprecated
    BodyType bodyType; // Deprecated
    bool isHaloEnabled; // May deprecate
};

//! Minimal size of the system on screen for showing orbiting bodies, in pixels
constexpr int SYSTEM_VISIBILITY_SUBSYSTEM_SIZE = 16;
//! Minimal size of the body (bounding) on screen for showing the outer BodyModule without full shadowing (only fast/approximate shadowing) nor Grounded ModularBody, in pixels
constexpr int BODY_EARLY_VISIBILITY_BOUNDING_SIZE = 2;
//! Minimal size of the body (bounding) on screen for showing the body normally in pixels
constexpr int BODY_FULL_VISIBILITY_BOUNDING_SIZE = 16;
//! Minimal size of the body (bounding) on screen for considering higher-resolution resources (texture above 1 Mio, high-resolution ojm/ojml)
constexpr int BODY_BIG_TEXTURE_BOUNDING_SIZE = 256;
//! Minimal speed while under the area of influence of a body, in body_radius/s
constexpr double MIN_MOVEMENT_SPEED = 0.125;
//! Minimal distance to the center of the body for showing surface BodyModule, in multiple of body radius
//! Surface BodyModule are designed on the assumption that proximity reduce the visible surface and change several assumptions
constexpr double BODY_SURFACE_HEIGHT = 2;

constexpr float SYSTEM_COLLAPSE_CROSSFADE_BAND = SYSTEM_VISIBILITY_SUBSYSTEM_SIZE / 2.f;

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

// ResourcePriority moved to ResourceHub.hpp (resource-layer concept, I2).

enum class BodyRelation {
    HIDDEN_GROUNDED,
    HIDDEN_ORBITING,
    HIDDEN_INNER,
    GROUNDED,
    ORBITING,
    INNER,
};

constexpr int HIDDEN_SHIFT = 3;

constexpr int RESUME_EXTRA_ITERATIONS = 4;

// hide()/show() translate between a relation and its hidden variant by +-HIDDEN_SHIFT.
static_assert(static_cast<int>(BodyRelation::GROUNDED) == static_cast<int>(BodyRelation::HIDDEN_GROUNDED) + HIDDEN_SHIFT &&
              static_cast<int>(BodyRelation::ORBITING) == static_cast<int>(BodyRelation::HIDDEN_ORBITING) + HIDDEN_SHIFT &&
              static_cast<int>(BodyRelation::INNER) == static_cast<int>(BodyRelation::HIDDEN_INNER) + HIDDEN_SHIFT,
              "hide()/show() map relations by +-HIDDEN_SHIFT - keep the enum halves aligned");

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
    virtual ~ModularBody();
    // For emplace_back
    ModularBody(ModularBody *parent, ModularBodyCreateInfo &info);
    // Prevent copy
    ModularBody(const ModularBody &) = delete;
    ModularBody(ModularBody &&) = delete;
    ModularBody &operator=(const ModularBody &) = delete;
    ModularBody &operator=(ModularBody &&) = delete;

    ModularBody *createChild(ModularBodyCreateInfo &info, BodyRelation rel = BodyRelation::ORBITING);
    ModularSystem *createChildSystem(ModularBodyCreateInfo &info, BodyRelation rel = BodyRelation::INNER);
    // The boolean representation of this body is whether it is visible or not -
    // i.e. whether it belongs to the drawn/pickable surface at all. HIDDEN is
    // part of that answer (B4, S11.111): the BodyRelation block above states
    // that a hidden body is "outside every update/draw walk by construction",
    // and that held only through isVisible, which dispatchUpdate's preUpdate
    // OVERWRITES for the camera's own reference body - a hidden body CAN be the
    // reference (INTENT 11.36), and then it re-entered both the draw sweep and
    // the pick sweep. Unreachable before B4 (nothing hid the reference);
    // reachable now that an anchor point IS a hidden reference body - and a
    // radius-0 reference produces a NaN halo (drawHaloCore's cmag *=
    // 0.5*rmag/screen_r with screen_r == 0) and a zero-size pick candidate at
    // the screen centre.
    // B39 (S11.117) asks `renderHidden` instead of `relation`: `relation` is the
    // DECLARED value and answers only for the node that was hidden, while the
    // question here is the EFFECTIVE one - a body hidden by NESTING (D23:
    // "hiding a body implicitly hide his child body as a side effect of
    // nesting") is equally outside the rendered universe while its own declared
    // flag must not move. Measured pre-fix: the selection pointer on Io under a
    // hidden Jupiter drew 137 px, against 138 px with the parent shown.
    inline operator bool() const {
        return isVisible & isBodyVisible & !renderHidden;
    }
    inline bool isRenderHidden() const {
        return renderHidden;
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
        const float rq = sqrtf(mat.r[12]*mat.r[12] + mat.r[13]*mat.r[13]);
        if (distance == 0.f) {
            screenPos.first = screenPos.second = 0.f;
        } else {
            float f;
            if (projectionMode == ProjectionTransfer::FISHEYE) {
                // The main case (INTENT 11.33): byte-for-byte the historical
                // fast path -- non-fisheye modes must not tax it.
                f = (rq > distance * 1e-5f)
                    ? acos(-mat.r[14]/distance) / (rq * halfFov)
                    : 1.f / (distance * halfFov);
            } else {
                f = (rq > distance * 1e-5f)
                    ? ProjectionTransfer::radius(projectionMode,
                          acosf(-mat.r[14]/distance) / halfFov, halfFov) / rq
                    : ProjectionTransfer::slope0(projectionMode, halfFov)
                          / (distance * halfFov);
            }
            screenPos.first = mat.r[12] * f;
            screenPos.second = mat.r[13] * f;
        }
        axisRotation = computeAxisRotation(jd);
        if (uncached)
            updateCache();       // module/radius part + a fresh updateReach()
        else
            updateReach();       // AoI tracks the current jd every frame (S11.62, B15)
        if (screenSize > 0.004)
            notableBody.push_back(this);
    }

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
    inline void draw(Renderer &renderer) {
        if (*this) {
            if (screenSize > earlyVisibilityGate()) {
                if (loaded) {
                    const auto matrix = mat.multiplyFast(computeBodyToSurface());
                    if (screenSize > fullVisibilityGate()) {
                        if (screenSize < 0.2) {
                            for (auto &module : farComponents)
                                module->draw(renderer, this, mat);
                            renderer.clearDepth(distance, boundingRadius);
                            for (auto &module : nearComponents)
                                module->draw(renderer, this, matrix);
                        } else {
                            renderer.clearDepth(distance, boundingRadius);
                            if (const auto *components = closeRangeComponents())
                                for (auto *module : *components)
                                    module->draw(renderer, this, matrix);
                        }
                    } else {
                        for (auto &module : farComponents)
                            module->draw(renderer, this, mat);
                        renderer.enterDepthlessSlice(distance, boundingRadius);
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

    inline void transformParentToBodyPos(double jd, Mat4f &mat_local_to_body) {
        evaluatedJD = jd;
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
        ++evalCount; // instrument (B39): "the position code RAN", see the member
        eclipticPos = tmp;
        lastJD = jd;
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(parent->computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(getDisplayEclipticPos());
        // Cache the position frame for the ORBIT pass (row 8): correct for
        // every body regardless of visibility (see the member comment).
        matLocalToBodyPos = mat_local_to_body;
    }

    inline Mat4f computeBodyPosToBody(double jd) const {
        return Mat4f::xzrotation(
            re.obliquity,
            re.ascendingNode -re.precessionRate*(jd-re.epoch)
        );
    }

    inline Mat4f computeBodyToBodyPos(double jd) const {
        return Mat4f::zxrotation(
            re.ascendingNode -re.precessionRate*(jd-re.epoch),
            re.obliquity
        );
    }

    inline Mat4f accumulatedBodyToBodyPos(double jd) const {
        Mat4f ret = computeBodyToBodyPos(jd);
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
    inline Mat4f accumulatedBodyPosToBody(double jd) const {
        Mat4f ret = computeBodyPosToBody(jd);
        // absoluteTiltFrame (B28, S11.67): see accumulatedBodyToBodyPos above -
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
    inline Mat4f accumulatedBodyPosToBody() const {
        Mat4f ret = computeBodyPosToBody(lastJD);
        // absoluteTiltFrame (B28, S11.67): root-aligned tilt, no ancestor
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

    inline void transformParentToBody(Mat4f &mat_local_to_body) const {
        if (boundToSurface) // PARENT spin - see transformParentToBodyPos
            mat_local_to_body = mat_local_to_body.multiplyFast(parent->computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(getDisplayEclipticPos()); // D21, see there
    }

    inline void transformBodyToParent(double jd, Mat4f &mat_local_to_body) {
        evaluatedJD = jd; // see transformParentToBodyPos (B39 barrier stamp)
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
        ++evalCount; // instrument (B39): "the position code RAN", see the member
        eclipticPos = tmp;
        lastJD = jd;
        if (boundToSurface) {
            // Maybe don't inline this unfrequent case
            // Exact inverse of the fold in transformParentToBodyPos
            // (PARENT spin, see there): [spin | spin*ecl]^-1 = [spin^-1 | -ecl]
            // `ecl` is the DRAWN offset (D21) - the down-hop applies that one,
            // so the up-hop must undo that one.
            const Vec3f ecl = getDisplayEclipticPos();
            auto tmp = parent->computeSurfaceToBody();
            tmp.r[12] -= ecl[0];
            tmp.r[13] -= ecl[1];
            tmp.r[14] -= ecl[2];
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
            const Vec3f ecl = getDisplayEclipticPos(); // D21, see the fresh twin
            auto tmp = parent->computeSurfaceToBody();
            tmp.r[12] -= ecl[0];
            tmp.r[13] -= ecl[1];
            tmp.r[14] -= ecl[2];
            mat_local_to_body = mat_local_to_body.multiplyFast(tmp);
        } else {
            mat_local_to_body.multiplyTranslation(-eclipticPos);
        }
    }

    inline void selectiveUpdate(double jd, Mat4f mat_local_to_parent) {
        transformParentToBodyPos(jd, mat_local_to_parent);
        preUpdate(jd, mat_local_to_parent);
        if (isVisible) {
            recursiveUpdate(jd, mat_local_to_parent);
        } else {
            mat.r[12] = mat_local_to_parent.r[12];
            mat.r[13] = mat_local_to_parent.r[13];
            mat.r[14] = mat_local_to_parent.r[14];
            matLocalToBodyPos = mat_local_to_parent;
            if (!groundedBodies.empty()) {
                const Mat4f surfaceFrame = mat_local_to_parent.multiplyFast(accumulatedBodyPosToBody(jd));
                for (auto &c : groundedBodies)
                    c->recursiveTranslationUpdate(jd, surfaceFrame);
            }
            for (auto &c : orbitingBodies)
                c->recursiveTranslationUpdate(jd, mat_local_to_parent);
            for (auto &c : innerBodies)
                c->recursiveTranslationUpdate(jd, mat_local_to_parent);
            publishParkedFrame(jd, mat_local_to_parent);
        }
    }

    inline void recursiveTranslationUpdate(double jd, Mat4f frame) {
        transformParentToBodyPos(jd, frame);
        mat.r[12] = frame.r[12];
        mat.r[13] = frame.r[13];
        mat.r[14] = frame.r[14];
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
        publishParkedFrame(jd, frame);
    }

    inline void publishParkedFrame(double jd, const Mat4f &flat) {
        if (hiddenBodies.empty())
            return;
        parkedChildFrame = flat;
        parkedFramePublished = true;
    }
    static inline bool sameFrame(const Mat4f &a, const Mat4f &b) {
        for (int i = 0; i < 16; ++i) {
            if (!(a.r[i] == b.r[i]))
                return false;
        }
        return true;
    }
    bool useNow();
    inline void refreshFrameState(double jd) {
        axisRotation = computeAxisRotation(jd);
        updateReach();
    }
    // Deliver the unhide edge to every module of this subtree (see the .cpp).
    void resumeModulesAfterHidden();

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
    inline bool isInSubtreeOf(const ModularBody *ancestor) const {
        for (const ModularBody *b = this; b; b = b->parent)
            if (b == ancestor)
                return true;
        return false;
    }
    inline bool isSatellite() const {
        return parent && !(parent->isPrimary() || parent->isSystem());
    }
    inline const Orbit *getOrbit() const {
        return orbit.get();
    }
    std::unique_ptr<Orbit> setOrbit(std::unique_ptr<Orbit> newOrbit);
    Vec3d getPositionAtDate(double jd) const;
    //! Current parent-relative position (root-aligned VSOP87). Client: the
    //! ORBIT module's center-notch (old Body::get_ecliptic_pos()).
    inline const Vec3f &getEclipticPos() const {
        return eclipticPos;
    }
    inline Vec3f getDisplayEclipticPos() const {
        return boundToSurface ? eclipticPos * inheritedScaling : eclipticPos;
    }
    inline Vec3d getCachedRootPosition() const {
        Vec3d p{};
        for (const ModularBody *b = this; b->parent; b = b->parent)
            p += Vec3d(b->eclipticPos[0], b->eclipticPos[1], b->eclipticPos[2]);
        return p;
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
    inline const ReceivedShadows &getReceivedShadows() const {
        return receivedShadows;
    }
    inline void setRadius(float _radius) {
        radius = _radius;
        uncached = true;
    }
    inline void setDatumRadius(float _datumRadius) {
        datumRadius = _datumRadius;
        uncached = true;
    }
    inline void setGroundRadius(float _groundRadius) {
        groundRadius = _groundRadius;
        uncached = true;
    }
    inline void setHaloEnabled(bool enabled) {
        isHaloEnabled = enabled;
    }
    inline void setScaling(float _scale) {
        scalingTarget = _scale;
        scaling = _scale;
        uncached = true;
    }
    inline void restoreScaling(float _scale) {
        scalingTarget = _scale;
        scaling.set(_scale, 0.f);
        uncached = true;
        updateCache();
    }
    inline void setFlagOrbit(bool b) {
        for (auto *m : orbitComponents)
            m->setShown(b);
    }
    inline void setFlagTrail(bool b) {
        for (auto *m : trailComponents)
            m->setShown(b);
    }
    void startTrail(bool record);
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
    static inline const Vec3f &getDefaultHaloColor() { return defaultHaloColor; }
    inline bool getColor(BodyColorType type, Vec3f &out) const {
        if (type == BodyColorType::HALO) {
            out = haloColor;
            return true;
        }
        for (auto &m : components)
            if (m && m->getColor(type, out))
                return true;
        return false;
    }
    inline bool getAuthoredColor(BodyColorType type, Vec3f &out) const {
        if (type == BodyColorType::HALO) {
            out = authoredState.haloColor;
            return true;
        }
        for (auto &m : components)
            if (m && m->getAuthoredColor(type, out))
                return true;
        return false;
    }
    inline bool isHiddenDeclared() const { return relation < BodyRelation::GROUNDED; }
    inline float getScalingTarget() const { return scalingTarget; }
    inline float getDisplayScaling() const {
        return static_cast<float>(scaling) * inheritedScaling;
    }
    //! The dilation this body inherits from its parent (1 unless it is a
    //! grounded child of a display-scaled body). Instrument + ledger channel.
    inline float getInheritedScaling() const { return inheritedScaling; }
    // The RAW nav radii, in AU, before `scaling` multiplies them. The scaled
    // products already had getters; the ledger needs what the operator set.
    inline float getDatumRadiusRaw() const { return datumRadius; }
    inline float getGroundRadiusRaw() const { return groundRadius; }
    // Is the created skin the one being drawn (S2 row D7's scalar half)? False
    // when no module in this body owns a skin at all.
    inline bool getSkinUse() const {
        bool v = false;
        for (auto &m : components)
            if (m && m->getSkinUse(v))
                return v;
        return false;
    }
    // The per-body ORBIT / TRAIL visibility override: -1 = follows the master.
    inline int getOrbitOverride() const { return firstOverride(orbitComponents); }
    inline int getTrailOverride() const { return firstOverride(trailComponents); }
    //! The TRAIL modules of this body, for the one consumer that needs the
    //! accumulated points themselves (S2 row D10's carve-out).
    inline const std::vector<BodyModule *> &getTrailComponents() const { return trailComponents; }
    struct AuthoredState {
        Vec3f haloColor {0.f, 0.f, 0.f};
        float datumRadius = 0.f;
        float groundRadius = 0.f;
        bool hidden = false;
    };
    inline const AuthoredState &getAuthored() const { return authoredState; }
    inline void captureAuthoredState() {
        authoredState.haloColor = haloColor;
        authoredState.datumRadius = datumRadius;
        authoredState.groundRadius = groundRadius;
        authoredState.hidden = isHiddenDeclared();
        for (auto &m : components)
            if (m)
                m->captureAuthored();
    }
    inline float getRotAscendingnode(void) const {
		return re.ascendingNode;
	}
	inline float getRotObliquity(void) const {
		return re.obliquity;
	}
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
    inline void preload(int keepFrames) {
        ++preloadCount; // instrument: "the seam reached THIS body" (see the member)
        for (auto &module : nearComponents) {
            module->preload(this, keepFrames);
        }
        for (auto &module : inComponents) {
            module->preload(this, keepFrames);
        }
    }
    inline const std::pair<float, float> &getScreenPos() const {
        return screenPos;
    }
    // Dual-path trace harness (INTENT.md 11.14): serialize this body's
    // NEW-path transform state as one JSON object (no newline). Read-only.
    void dumpTrace(std::ostream &out) const;
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
    static inline std::vector<ModularBody *> &drainNotableBodies() {
        return notableBody;
    }
    static float halfFov;
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
    static bool flagLightTravelTime;
    static float haloScale;
    static float haloSizeLimit;
    static float drawAlpha;
    inline void pin() {
        ++pins;
    }
    // Unpin; if this body was parked for destruction (removed from the tree
    // while pinned) and this was the last pin, destruction happens now.
    void unpin();
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
    // Return true if this body has the STAR bit set, meaning it emit light.
    // ILLUMINATION ONLY since the D27 split (S11.113(f), [vixy]: "Split
    // light_source and primary flags, the first one for light purpose and the
    // second one for isStar() purpose minus light source") - every consumer that
    // asks a STRUCTURAL question about the body now asks isPrimary() instead.
    inline bool isStar() const {
        return (bodyType & BodyType::STAR) == BodyType::STAR;
    }
    inline bool isPrimary() const {
        return primary;
    }
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
    inline bool isComposedDeclared() const {
        return composedDeclaration;
    }
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
        return !body->isNotIsolated;
    }

    inline void enterEnvironment() {
    }
    inline void leaveEnvironment() {
    }
    inline void addEnvironment(std::unique_ptr<EnvironmentModule> &&module, bool grounded) {
        (grounded ? groundedEnvironment : environment).push_back(std::move(module));
    }
    inline const Mat4f &getMat() const {
        return mat;
    }
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
    inline void setInheritedScaling(float f) {
        if (inheritedScaling != f) {
            inheritedScaling = f;
            uncached = true;
        }
    }
    // Deduce which modules are to be bound to this body from the parameters
    std::vector<BodyModuleType> deduceBodyModuleList(std::map<std::string, std::string> &param);
    void select();
    void deselect();
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
            const Vec3f _planet = parent->mat.getTranslation() - lightPosition;
            const Vec3f _satellite = mat.getTranslation() - lightPosition;
            const double c = _planet.dot(_satellite);
            const double OP = _planet.length();
    		const double OS = _satellite.length();
            if (c > 0 && OP < OS && fabs(acos(c/(OP*OS))) < atan(parent->radius/OP)) {
                cmag = 0.0;
            }
        }
        cmag *= drawAlpha;
        if (rmag < 1.21f && cmag < 0.05f) // halo.cpp:86 skip rule (old draws
            return; // big-but-dim halos; the previous cmag-only gate dropped them)
        renderer.drawHalo(screenPos, color * cmag, rmag);
    }
    inline void drawHalo(Renderer &renderer) {
        drawHaloCore(renderer, computeMagnitude(), screenSize * 2.f * viewportRadius,
                     haloColor, isSatellite());
    }
    // Identity
    std::string englishName;
    std::string nameI18;

    std::map<std::string, std::string> declaredParams;

    bool supplemental = false;

    uint32_t preloadCount = 0;

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
    void registerToSystem(ModularBody *child);
    ModularSystem *owningSystem() const;
    void propagateRenderHidden(bool ancestorHidden);
    std::vector<std::unique_ptr<EnvironmentModule>> groundedEnvironment;
    std::vector<std::unique_ptr<EnvironmentModule>> environment;
    ReceivedShadows receivedShadows;

    // TODO create an optimized std::string for limited set
    std::vector<std::unique_ptr<BodyModule>> components; // Reference every BodyModule of this ModularBody by name

    std::vector<BodyModule *> farComponents; // 2D behind body, SKIP above screenSize 0.2, update NEVER called
    std::vector<BodyModule *> nearComponents; // Drawn above BODY_EARLY_VISIBILITY_BOUNDING_SIZE and distance > scaledRadius * BODY_SURFACE_HEIGHT
    std::vector<BodyModule *> groundedComponents; // Drawn if distance <= scaledRadius * BODY_SURFACE_HEIGHT and a surface is loaded
    std::vector<BodyModule *> inComponents; // Draw if distance <= scaledRadius
    const std::vector<BodyModule *> *closeRangeComponents();
    // -1 when no module in the list carries a live override.
    static inline int firstOverride(const std::vector<BodyModule *> &list) {
        for (auto *m : list) {
            const int v = m ? m->getShownOverride() : -1;
            if (v >= 0)
                return v;
        }
        return -1;
    }
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
    Mat4f matLocalToBodyPos = Mat4f::identity();
    Mat4f parkedChildFrame = Mat4f::identity();
    bool parkedFramePublished = false;
    bool unservedLogged = false;
    Vec3f eclipticPos;
    std::pair<float, float> screenPos;
    float halfAngularSize = 0; // 0 until first update (uninit class, INTENT 5.16/11.28c/11.32)
    float screenSize = 0; // Ratio of the screen taken by this body; 0 until
    float distance = 0;
    float axisRotation = 0;
    float scaledRadius = 0;
    float scaledDatumRadius = 0;
    float scaledGroundRadius = 0;
    float rmag;
    float cmag;
    double lastJD = 0;
    double evaluatedJD = -1;
    Mat4f evaluatedFrame = Mat4f::identity();
    uint32_t evalCount = 0;

    // Halo system
    Vec3f haloColor;
    float albedo;					// Body albedo

    Vec3f shadowAbsorbtion;

    // Navigation and visibility
    ASmooth<AsyncHub, float, 5.f> scaling;
    float scalingTarget = 1.f;   // what setScaling was last told (D32's settled value)
    float inheritedScaling = 1.f;
    AuthoredState authoredState; // what the DATA gave this body (D30's delta baseline)
    float radius;
    // Raw (unscaled) navigation radii, both defaulting to `radius` (B10 S5.2).
    // See ModularBodyCreateInfo for the datum/ground roles.
    float datumRadius;
    float groundRadius;
    float boundingRadius = 0; // Smallest radius including all groundedComponents and nearComponents
    float subsystemRadius = 0; // Radius including all orbitingBodies
    float areaOfInfluence = 0; // Area under the influence of this body

    // Internal datas, deprecated
    float one_minus_oblateness;
    float solLocalDay;			//time of a sideral day in this planet
    uint8_t pointerCount = 0; // Number of pointer pointing this object
    int pins = 0;
    bool parked = false; // Removed from tree while pinned; destroyed at last unpin
    BodyType bodyType;
    SiderealTimeModel siderealTimeModel = SiderealTimeModel::GENERIC;
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    // Trail sample count (B27 A7, `trail_length` key), consumed by TrailLoader.
    int trailLength = TRAIL_LENGTH_DEFAULT;
    // Declaring format (D14 S11.79(h)). See ModularBodyCreateInfo.
    bool composedDeclaration = false;
    // Structural primacy (B27 Tier B, `primary` key). See ModularBodyCreateInfo;
    // read through isPrimary(), never directly.
    bool primary = false;
    bool renderHidden = false;
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
    static std::vector<ModularBody *> notableBody;
    static Vec3f defaultHaloColor;
    static std::shared_ptr<BodyTesselation> bodyTesselation; // both-paths seam (setTesselation)
    static float viewportRadius;
    static float earlyVisibilityScreenSize;
    static float fullVisibilityScreenSize;
    static float bigTextureScreenSize;
public:
    inline static float getViewportRadius() {
        return viewportRadius;
    }
    // The ONE writer of the viewport radius (I3): it recomputes every derived
    // gate, so there is no state in which the gates disagree with the viewport.
    static void setViewportRadius(float halfRenderWidthPx);
    inline static float earlyVisibilityGate() {
        return earlyVisibilityScreenSize;
    }
    inline static float fullVisibilityGate() {
        return fullVisibilityScreenSize;
    }
    inline static float bigTextureGate() {
        return bigTextureScreenSize;
    }
    static float deltaTime;
    static double currentJD;

    static void setTesselation(std::shared_ptr<BodyTesselation> t) {
        bodyTesselation = std::move(t);
    }
    static const std::shared_ptr<BodyTesselation> &getTesselation() {
        return bodyTesselation;
    }

    static inline ModularBody *getSelected() {
        return selectedBody;
    }
private:
    static ModularBody *selectedBody;
};

#endif /* end of include guard: MODULAR_BODY_HPP_ */
