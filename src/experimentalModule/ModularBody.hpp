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

class BodyTesselation;

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

// Picked by the `sidereal_time` key, never by body name
enum class SiderealTimeModel : unsigned char {
    GENERIC, // (jd - epoch) / period spin
    EARTH_APPARENT, // Apparent sidereal time (nutation)
};

enum class SurfaceModel : unsigned char {
    PLANET, // night/specular/bump combinations
    LUNAR, // tessellated heightmap displacement, no night side
};

constexpr int TRAIL_LENGTH_DEFAULT = 60; // In samples

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

//! Resolved by the ctor to `radius` for a body, 0 for a ModularSystem
constexpr float NAV_RADIUS_UNSET = -1.f;

struct ModularBodyCreateInfo {
    std::unique_ptr<Orbit> orbit;
    std::string englishName;
    RotationElements re;
    Vec3f haloColor;
    float albedo;
    float radius;
    float datumRadius = NAV_RADIUS_UNSET; // In AU, where the altitude of the observer is zero
    float groundRadius = NAV_RADIUS_UNSET; // In AU, free flight cannot descend past it, 0 = enterable body
    float oblateness; // Not universal - only for pure spherical body modules (so, single-shape body ?) - may provide immense optimisation and quality
    float solLocalDay;
    // New
    Vec3f shadowAbsorbtion;
    float brightness;
    SiderealTimeModel siderealTimeModel = SiderealTimeModel::GENERIC;
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    int trailLength = TRAIL_LENGTH_DEFAULT;
    bool composedDeclaration = false; // Set by ModularSystem::loadBody only
    bool primary = false; // What its subsystem orbits, shining or not

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
//! Cross-fade width of a nested system with its star proxy, in pixels
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

// Which list of the parent owns the body, HIDDEN_* = parked by hide()
enum class BodyRelation {
    HIDDEN_GROUNDED,
    HIDDEN_ORBITING,
    HIDDEN_INNER,
    GROUNDED,
    ORBITING,
    INNER,
};

constexpr int HIDDEN_SHIFT = 3;
// Extra position solves when a frozen body is used again
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
    // For environment chain aggregation
    friend class EnvironmentManager;
public:
    // Virtual for child ownership only, update/draw stay non-virtual
    virtual ~ModularBody();
    // For emplace_back
    ModularBody(ModularBody *parent, ModularBodyCreateInfo &info);
    // Prevent copy
    ModularBody(const ModularBody &) = delete;
    ModularBody(ModularBody &&) = delete;
    ModularBody &operator=(const ModularBody &) = delete;
    ModularBody &operator=(ModularBody &&) = delete;

    // Create a new child body. If a body with the same englishName exists, it is replaced by this one.
    // rel must be a visible relation
    ModularBody *createChild(ModularBodyCreateInfo &info, BodyRelation rel = BodyRelation::ORBITING);
    ModularSystem *createChildSystem(ModularBodyCreateInfo &info, BodyRelation rel = BodyRelation::INNER);
    // The boolean representation of this body is whether it is visible or not
    inline operator bool() const {
        return isVisible & isBodyVisible & !renderHidden;
    }
    // Return true if this body or an ancestor is hidden
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
    // Update subsystemRadius and areaOfInfluence
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
                // Main case, the other modes must not tax it
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
            updateCache(); // Includes updateReach()
        else
            updateReach();
        if (screenSize > 0.004)
            notableBody.push_back(this);
    }

    // Return the spin phase at jd, in radians
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

    // Evaluate the orbit at jd and hop down to the position frame (root-aligned)
    inline void transformParentToBodyPos(double jd, Mat4f &mat_local_to_body) {
        evaluatedJD = jd;
        if (flagLightTravelTime && distance == distance) { // NaN would freeze the Kepler solver
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
        ++evalCount;
        eclipticPos = tmp;
        lastJD = jd;
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(parent->computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(getDisplayEclipticPos());
        matLocalToBodyPos = mat_local_to_body;
    }

    // Own tilt only, ancestors in accumulatedBodyPosToBody
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

    // Equatorial frame of both observer and mesh, ancestor tilts included
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
    // Same from the lastJD of each node, outside the update walk
    inline Mat4f accumulatedBodyPosToBody() const {
        Mat4f ret = computeBodyPosToBody(lastJD);
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

    // Use cached informations from last update
    inline void transformParentToBody(Mat4f &mat_local_to_body) const {
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(parent->computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(getDisplayEclipticPos());
    }

    // Evaluate the orbit at jd and hop up, inverse of transformParentToBodyPos
    inline void transformBodyToParent(double jd, Mat4f &mat_local_to_body) {
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
        ++evalCount;
        eclipticPos = tmp;
        lastJD = jd;
        if (boundToSurface) {
            // Maybe don't inline this unfrequent case
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

    // Update fully if visible, else only the translations of the subtree
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

    // Keep an invisible subtree positioned
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

    // Publish the frame useNow recomputes the hidden children in
    inline void publishParkedFrame(double jd, const Mat4f &flat) {
        if (hiddenBodies.empty())
            return;
        parkedChildFrame = flat;
        parkedFramePublished = true;
    }
    // NaN != NaN, so a corrupted frame is never memoized
    static inline bool sameFrame(const Mat4f &a, const Mat4f &b) {
        for (int i = 0; i < 16; ++i) {
            if (!(a.r[i] == b.r[i]))
                return false;
        }
        return true;
    }
    // Refresh a hidden body for use, return false on failure
    bool useNow();
    inline void refreshFrameState(double jd) {
        axisRotation = computeAxisRotation(jd);
        updateReach();
    }
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
    //! Return the previous orbit; never write a position beside the orbit
    std::unique_ptr<Orbit> setOrbit(std::unique_ptr<Orbit> newOrbit);
    //! Root frame. Never call from inside a position evaluation
    Vec3d getPositionAtDate(double jd) const;
    inline const Vec3f &getEclipticPos() const {
        return eclipticPos;
    }
    //! Return the position as drawn, scaled for a grounded child
    inline Vec3f getDisplayEclipticPos() const {
        return boundToSurface ? eclipticPos * inheritedScaling : eclipticPos;
    }
    inline Vec3d getCachedRootPosition() const {
        Vec3d p{};
        for (const ModularBody *b = this; b->parent; b = b->parent)
            p += Vec3d(b->eclipticPos[0], b->eclipticPos[1], b->eclipticPos[2]);
        return p;
    }
    //! In days, 0 = no orbit line
    inline double getSiderealPeriod() const {
        return re.sidereal_period;
    }
    inline double getLastJD() const {
        return lastJD;
    }
    inline const Mat4f &getMatLocalToBodyPos() const {
        return matLocalToBodyPos;
    }
    inline float getRadius() const {
        return radius;
    }
    inline float getScaledRadius() const {
        return scaledRadius;
    }
    inline float getBoundingRadius() const {
        return boundingRadius;
    }
    inline const Vec3f &getShadowAbsorbtion() const {
        return shadowAbsorbtion;
    }
    inline SiderealTimeModel getSiderealTimeModel() const {
        return siderealTimeModel;
    }
    // May be stale, gate on ShadowService::enabled too
    inline const ReceivedShadows &getReceivedShadows() const {
        return receivedShadows;
    }
    inline void setRadius(float _radius) {
        radius = _radius;
        uncached = true;
    }
    // In AU, before display scaling
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
    //! Set the scaling with no transition
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
    // Restart the recorded trail from here, not a display override
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
    //! Return the scale as drawn; never multiply by `scaling` directly
    inline float getDisplayScaling() const {
        return static_cast<float>(scaling) * inheritedScaling;
    }
    inline float getInheritedScaling() const { return inheritedScaling; }
    inline float getDatumRadiusRaw() const { return datumRadius; }
    inline float getGroundRadiusRaw() const { return groundRadius; }
    // Return true if the created skin is the one drawn
    inline bool getSkinUse() const {
        bool v = false;
        for (auto &m : components)
            if (m && m->getSkinUse(v))
                return v;
        return false;
    }
    // -1 = follows the master
    inline int getOrbitOverride() const { return firstOverride(orbitComponents); }
    inline int getTrailOverride() const { return firstOverride(trailComponents); }
    inline const std::vector<BodyModule *> &getTrailComponents() const { return trailComponents; }
    // What the data gave this body, baseline of the session diff
    struct AuthoredState {
        Vec3f haloColor {0.f, 0.f, 0.f};
        float datumRadius = 0.f;
        float groundRadius = 0.f;
        bool hidden = false;
    };
    inline const AuthoredState &getAuthored() const { return authoredState; }
    // Call once the loader has finished writing into the body
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
	//! In degrees, display only
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
        ++preloadCount;
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
    // Write one JSON object, no newline (trace harness)
    void dumpTrace(std::ostream &out) const;
    void dumpHops(std::ostream &out) const;
    inline const Vec3f &getHaloColor() const {
        return haloColor;
    }
    // Meaningful only when hasChildren()
    inline float getSubsystemRadius() const {
        return subsystemRadius;
    }
    // Hidden children excluded
    inline bool hasChildren() const {
        return !(groundedBodies.empty() && orbitingBodies.empty() && innerBodies.empty());
    }
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
        return scaledDatumRadius;
    }
    // Return the floor of free flight, 0 = enterable body
    inline float getScaledGroundRadius() const {
        return scaledGroundRadius;
    }
    template<class Function>
    static inline void forEach(Function fn) {
        for (auto &b : bodyReference) {
            fn(*b.second);
        }
    }
    // Find a better reference body, return nullptr if this body is the best one
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
    // Bodies needing a depth bucket this frame, cleared by dispatchUpdate
    static inline std::vector<ModularBody *> &drainNotableBodies() {
        return notableBody;
    }
    // This value should be set before calling update, through setHalfFov
    static float halfFov;
    static int projectionMode; // ProjectionTransfer mode
    static float cullHalfFov; // Angle projected at the edge of the screen disc, for culling
    static void setHalfFov(float hf) {
        halfFov = hf;
        cullHalfFov = hf * ProjectionTransfer::edgeAngleNorm(projectionMode, hf);
    }
    static void setProjectionMode(int mode) {
        projectionMode = mode;
        setHalfFov(halfFov);
    }
    // Set through SSystemFactory
    static bool flagLightTravelTime;
    static float haloScale;
    static float haloSizeLimit;
    static float drawAlpha; // Halo brightness multiplier, owned by ModularSystem::drawNested
    // Keep this body alive for a task. Non-atomic: render-chain tasks only
    inline void pin() {
        ++pins;
    }
    // May destroy this body if it was removed while pinned
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
    // For the trace harness
    inline float getAreaOfInfluence() const {
        return areaOfInfluence;
    }
    inline bool isCacheFresh() const {
        return !uncached;
    }
    // Return true if this body has the STAR bit set, meaning it emit light.
    inline bool isStar() const {
        return (bodyType & BodyType::STAR) == BodyType::STAR;
    }
    inline bool isPrimary() const {
        return primary;
    }
    // Minor bodies are exempt from inter-body shadowing
    inline bool isMinorBody() const {
        return bodyType == BodyType::MINOR_BODY;
    }
    inline SurfaceModel getSurfaceModel() const {
        return surfaceModel;
    }
    inline int getTrailLength() const {
        return trailLength;
    }
    inline bool isComposedDeclared() const {
        return composedDeclaration;
    }
    // Return true if this body is a system
    inline bool isSystem() const {
        return !isNotIsolated;
    }
    // Return true if this body is at the center of his system
    inline bool isSystemCentered() const {
        const ModularBody *body = this;
        while (body->isNotIsolated) {
            if (body->eclipticPos.v[0] || body->eclipticPos.v[1] || body->eclipticPos.v[2])
                return false;
            body = body->parent;
        }
        return !body->isNotIsolated;
    }

    // Empty: the enter/leave edges are owned by EnvironmentManager
    inline void enterEnvironment() {
    }
    inline void leaveEnvironment() {
    }
    // grounded = active only while the camera is anchored on this body
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
    // In degrees
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
    // Unroute the replaced module only, the new one must have routed itself
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
    //! Called by the updateCache of the parent on its grounded children
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
    // Draw a halo from given inputs, screen_r = disc diameter in px
    inline void drawHaloCore(Renderer &renderer, const float mag, const float screen_r, const Vec3f &color, const bool satelliteRules) {
        const float fov_deg = halfFov * (360.f / M_PI);
        float fov_q = (fov_deg > 60.f) ? 60.f : fov_deg;
        fov_q = 1.f / (fov_q * fov_q);
        rmag = sqrtf(renderer.adaptLuminance((expf(-0.92103f*(mag + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * ModularBody::haloScale;
        if (satelliteRules) {
            rmag /= (fov_deg > 60.f) ? 25.f : 5.f;
        }
        cmag = 1.f;
        if (rmag < 1.2f) { // Anti-blink
            cmag = (mag > 0.f) ? (rmag*rmag/1.44f) : (rmag/1.2f);
            if (mag > 6.5f)
                cmag *= rmag*rmag/1.44f;
            rmag = 1.2f;
        } else { // Size-limit compression
            const float limit = ModularBody::haloSizeLimit/1.8f;
    		if (rmag > limit) {
    			rmag = limit + sqrtf(rmag-limit)/(limit + 1);
    			if (rmag > ModularBody::haloSizeLimit)
    				rmag = ModularBody::haloSizeLimit;
    		}
        }
        cmag *= 0.5f*rmag/screen_r;
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
        if (rmag < 1.21f && cmag < 0.05f) // Big-but-dim halos are still drawn
            return;
        renderer.drawHalo(screenPos, color * cmag, rmag);
    }
    inline void drawHalo(Renderer &renderer) {
        drawHaloCore(renderer, computeMagnitude(), screenSize * 2.f * viewportRadius,
                     haloColor, isSatellite());
    }
    // Identity
    std::string englishName;
    std::string nameI18;
    // What a save writes back, never edited at runtime; empty = engine-minted body
    std::map<std::string, std::string> declaredParams;
    bool supplemental = false; // Loaded at runtime, dropped by `body action clear`
    uint32_t preloadCount = 0; // Instrument

    // Relations
    ModularBody *parent;
    BodyRelation relation = BodyRelation::ORBITING; // meaningless for parentless roots
    std::vector<std::unique_ptr<ModularBody>> groundedBodies;
    std::vector<std::unique_ptr<ModularBody>> orbitingBodies;
    std::vector<std::unique_ptr<ModularBody>> innerBodies;
    std::vector<std::unique_ptr<ModularBody>> hiddenBodies; // parked by hide(), still owned

    inline std::vector<std::unique_ptr<ModularBody>> &listOf(BodyRelation r) {
        switch (r) {
        case BodyRelation::GROUNDED: return groundedBodies;
        case BodyRelation::ORBITING: return orbitingBodies;
        case BodyRelation::INNER:    return innerBodies;
        default:                     return hiddenBodies;
        }
    }
    // Not for a walk handing a frame down, grounded children get another one
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
    // Only writer of renderHidden and of the membership in the owning system
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
    // Return the list drawn at close range, nullptr = none
    const std::vector<BodyModule *> *closeRangeComponents();
    static inline int firstOverride(const std::vector<BodyModule *> &list) {
        for (auto *m : list) {
            const int v = m ? m->getShownOverride() : -1;
            if (v >= 0)
                return v;
        }
        return -1;
    }
    std::vector<BodyModule *> orbitComponents; // Drawn by ModularSystem::drawOrbits, not a screen-size regime
    std::vector<BodyModule *> trailComponents; // Swept every frame by ModularSystem::drawTrails, even while invisible
    std::vector<BodyModule *> tailComponents; // Swept by ModularSystem::drawTails, batch flushed once
    // std::list<std::shared_ptr<BodyOrbitModule>> orbitalComponents; // Components drawing lines between bodies
    // std::list<std::shared_ptr<EnvironmentModule>> environmentComponents; // Component defining the environment

    // Positionnal
    std::unique_ptr<Orbit> orbit;
    RotationElements re;

    // Cached data (may deprecate)
    Mat4f mat; // Matrix defining this body regarding to the observer
    // Root-aligned position frame, assigned wherever `mat` or its translation is
    Mat4f matLocalToBodyPos = Mat4f::identity();
    Mat4f parkedChildFrame = Mat4f::identity(); // Position frame the hidden children are computed in
    bool parkedFramePublished = false; // Never cleared, false = useNow can't serve the children
    bool unservedLogged = false;
    Vec3f eclipticPos;
    std::pair<float, float> screenPos;
    float halfAngularSize = 0;
    float screenSize = 0; // Ratio of the screen taken by this body
    float distance = 0; // To the observer, 0 = not evaluated
    float axisRotation = 0;
    // X * getDisplayScaling(), written by updateCache only
    float scaledRadius = 0;
    float scaledDatumRadius = 0;
    float scaledGroundRadius = 0;
    float rmag;
    float cmag;
    double lastJD = 0; // jd of the last orbit evaluation, light-travel retarded
    double evaluatedJD = -1; // Un-retarded jd of the last evaluation, -1 = never
    Mat4f evaluatedFrame = Mat4f::identity(); // Parent frame of the last useNow refresh
    uint32_t evalCount = 0; // Instrument: orbit evaluations

    // Halo system
    Vec3f haloColor;
    float albedo;					// Body albedo
    Vec3f shadowAbsorbtion;

    // Navigation and visibility
    ASmooth<AsyncHub, float, 5.f> scaling;
    float scalingTarget = 1.f; // What setScaling was last told
    float inheritedScaling = 1.f; // From the scaled parent it stands on, never folded into `scaling`
    AuthoredState authoredState;
    float radius;
    // Unscaled, in AU, see ModularBodyCreateInfo
    float datumRadius;
    float groundRadius;
    float boundingRadius = 0; // Smallest radius including all groundedComponents and nearComponents
    float subsystemRadius = 0; // Radius including all orbitingBodies
    float areaOfInfluence = 0; // Area under the influence of this body

    // Internal datas, deprecated
    float one_minus_oblateness;
    float solLocalDay;			//time of a sideral day in this planet
    uint8_t pointerCount = 0; // Number of pointer pointing this object
    int pins = 0; // See pin(), separate from pointerCount
    bool parked = false; // Removed from tree while pinned; destroyed at last unpin
    BodyType bodyType;
    SiderealTimeModel siderealTimeModel = SiderealTimeModel::GENERIC;
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    int trailLength = TRAIL_LENGTH_DEFAULT;
    bool composedDeclaration = false;
    bool primary = false; // read through isPrimary(), never directly
    bool renderHidden = false; // Written by propagateRenderHidden only
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
    bool surfaceLockedAttitude = false; // No spin in the surface frame of the parent, set by the loader

    // Global datas
    static Vec3f lightPosition; // Observer-local light source position
    static float lightDistance; // Observer-local light source distance
    static float lightSize;     // Light source radius
    static ModularBody *lastFit;
    static Translator *translator;
    static std::map<std::string, ModularBody *> bodyReference;
    static std::vector<ModularBody *> notableBody;
    static Vec3f defaultHaloColor;
    static std::shared_ptr<BodyTesselation> bodyTesselation;
    static float viewportRadius; // Half the render width in px, written by setViewportRadius only
    // BODY_*_BOUNDING_SIZE in screenSize units
    static float earlyVisibilityScreenSize;
    static float fullVisibilityScreenSize;
    static float bigTextureScreenSize;
public:
    inline static float getViewportRadius() {
        return viewportRadius;
    }
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
    static float deltaTime; // In milliseconds, set by Camera::update before any body update
    static double currentJD; // Written once per frame by dispatchUpdate, 0 before the first frame

    // Animated by its owner: hold the pointer, never copy the values
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
