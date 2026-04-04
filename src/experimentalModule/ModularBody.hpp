#ifndef MODULAR_BODY_HPP_
#define MODULAR_BODY_HPP_

#include "BodyModule.hpp"
#include "Renderer.hpp"
#include "EntityCore/Tools/Tracer.hpp"
#include "../planetsephems/sideral_time.h"
#include "coreModule/time_mgr.hpp"
#include "bodyModule/rotation_elements.hpp"
#include "bodyModule/orbit.hpp"
#include "EnvironmentModule.hpp"
#include "AsyncHub.hpp"
#include "EntityCore/Executor/ASmooth.hpp"
#include "tools/StringID.hpp"
#include <memory>
#include <list>
#include <vector>

#define TM(num, name) auto tex##num = name.getBigTexture()
#define TB(num) ((tex##num != nullptr) << num)

// Big texture mapping
#define TEXMAP1(t0) TM(0, t0); const uint16_t texmap = TB(0)
#define TEXMAP2(t0, t1) TM(0, t0); TM(1, t1); const uint16_t texmap = TB(0) | TB(1)
#define TEXMAP3(t0, t1, t2) TM(0, t0); TM(1, t1); TM(2, t2); const uint16_t texmap = TB(0) | TB(1) | TB(2)
#define TEXMAP4(t0, t1, t2, t3) TM(0, t0); TM(1, t1); TM(2, t2); TM(3, t3); const uint16_t texmap = TB(0) | TB(1) | TB(2) | TB(3)
#define TEXMAP5(t0, t1, t2, t3, t4) TM(0, t0); TM(1, t1); TM(2, t2); TM(3, t3); TM(4, t4); const uint16_t texmap = TB(0) | TB(1) | TB(2) | TB(3) | TB(4)

// Get the Texture at the 'num' parameter of TEXMAP which is 'name'
#define TEX(num, name) (tex##num ? *tex##num : name.getTexture())

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

// Shadow projection :
// - With outer orbiting ModularBody
// - Outer : DepthBuffer shared with grounded ModularBody for drawing and shadowing
// - Surface : DepthBuffer split between grounded ModularBody, parent's depth trace is drawn in each DepthBuffer, project shadow with and between grounded ModularBody

// Depth buffers :
// - Orbit depth buffer : Depth bounds calibrated for the smallest system visible [only update bound values] - It may go wrong though
// - Body depth buffer : Cleared for each significant bodies
// - Self-shadowing depth buffer : Large depth buffer for main body, 2k for others
// - Shadow casting stencil buffer

// Requests threads :
// - Main thread (Event + render while updating)
// - Resource loader
// - GPU computing (ex : shadow tracing)
// - Script
// - Texture/resource async loaders
// - Video stream

//! @brief The level of prioritisation of the resources of this body
enum class ResourcePriority {
    UNLOADED, // No resources acquired, either because it was either explicitly unloaded or because it is an inner ModularBody or a child of it and the camera is outside of his area of influence
    LAZY, // Only minimal ressources shall be loaded on background (default)
    BACKGROUND, // High resolution of this ressource will probably been needed (lower resolution in use)
    PRELOAD, // High resolution of this ressource is needed in the near future (preload request)
    ACTIVE, // This ressource is currently needed (ex : missing resolution expected, ressource currently used)
};

enum class BodyRelation {
    HIDDEN_GROUNDED,
    HIDDEN_ORBITING,
    HIDDEN_INNER,
    GROUNDED,
    ORBITING,
    INNER,
};

enum class BodyModuleType : unsigned char;

class ModularBody {
    // For pointer count and selection modification
    friend class ModularBodyPtr;
    friend class ModularBodySelector;
    // For ModularBody and BodyModule loading
    friend class ModuleLoader;
    // For some ModularBody management
    friend class ModularSystem;
public:
    ~ModularBody();
    // For emplace_back
    ModularBody(ModularBody *parent, ModularBodyCreateInfo &info);
    // Prevent copy
    ModularBody(const ModularBody &) = delete;
    ModularBody(ModularBody &&) = delete;
    ModularBody &operator=(const ModularBody &) = delete;
    ModularBody &operator=(ModularBody &&) = delete;

    // Create a new child body. If a body with the same englishName exists, it is replaced by this one.
    ModularBody *createChild(ModularBodyCreateInfo &info);
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
        if (bodyType == BodyType::EARTH_MOON)
        {
            std::cout << "\rGOT : " << preUpdate.getTranslation() << std::flush;
        }
        const float squaredDistance = preUpdate.r[12] * preUpdate.r[12] + preUpdate.r[13] * preUpdate.r[13] + preUpdate.r[14] * preUpdate.r[14];
        distance = sqrt(squaredDistance);
        if (childs.empty()) {
            if (distance > boundingRadius) {
                halfAngularSize = atanf(boundingRadius / sqrt(squaredDistance - boundingRadius*boundingRadius));
                const float tmp = halfFov + halfAngularSize;
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
                const float tmp = halfFov + halfAngularSize;
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
                const float tmp = halfFov + halfAngularSize;
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
        const float f = acos(-mat.r[14]/distance) / (sqrt(mat.r[12]*mat.r[12] + mat.r[13]*mat.r[13]) * halfFov);
        screenPos.first = mat.r[12] * f;
        screenPos.second = mat.r[13] * f;
        if (bodyType == BodyType::EARTH) {
            axisRotation = get_apparent_sidereal_time(jd) * (M_PI / 180);
        } else {
            axisRotation = fmod((jd - re.epoch) / re.period * (2 * M_PI) + re.offset, (2 * M_PI));
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
                        renderer.clearDepth(distance, boundingRadius);
                        if (screenSize < 0.2) {
                            for (auto &module : farComponents)
                                module->draw(renderer, this, mat);
                            for (auto &module : nearComponents)
                                module->draw(renderer, this, matrix);
                        } else {
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

    inline void transformParentToBodyPos(double jd, Mat4f &mat_local_to_body) {
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
        mat_local_to_body.multiplyTranslation(-eclipticPos);
    }

    inline Mat4f computeBodyPosToBody(double jd) {
        return Mat4f::xzrotation(
            re.obliquity,
            re.ascendingNode -re.precessionRate*(jd-re.epoch)
        );
    }

    // Use cached informations from last update
    inline void transformParentToBody(Mat4f &mat_local_to_body) const {
        if (boundToSurface)
            mat_local_to_body = mat_local_to_body.multiplyFast(computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(-eclipticPos);
        mat_local_to_body.multiplyFast(Mat4f::xzrotation(
            re.obliquity,
            re.ascendingNode -re.precessionRate*(lastJD-re.epoch)
        ));
    }

    inline void transformBodyToParent(double jd, Mat4f &mat_local_to_body) {
        Vec3d tmp;
        if (OsculatingFunctionType *oscFunc = orbit->getOsculatingFunction()) {
            (*oscFunc)(jd,jd,tmp);
        } else {
            orbit->positionAtTimevInVSOP87Coordinates(jd,jd,tmp);
        }
        eclipticPos = tmp;
        lastJD = jd;
        mat_local_to_body = mat_local_to_body.multiplyFast(Mat4f::zxrotation(
            re.precessionRate*(jd-re.epoch) - re.ascendingNode,
            -re.obliquity
        ));
        if (boundToSurface) {
            // Maybe don't inline this unfrequent case
            auto tmp = Mat4f::zrotation(-M_PI_2 - axisRotation);
            tmp.r[12] += eclipticPos[0];
            tmp.r[13] += eclipticPos[1];
            tmp.r[14] += eclipticPos[2];
            mat_local_to_body = mat_local_to_body.multiplyFast(tmp);
        } else {
            mat_local_to_body.multiplyTranslation(eclipticPos);
        }
    }

    // Use cached informations from last update
    inline void transformBodyToParent(Mat4f &mat_local_to_body) const {
        mat_local_to_body = mat_local_to_body.multiplyFast(Mat4f::zxrotation(
            re.precessionRate*(lastJD-re.epoch) - re.ascendingNode,
            -re.obliquity
        ));
        if (boundToSurface) {
            // Maybe don't inline this unfrequent case
            auto tmp = Mat4f::zrotation(-M_PI_2 - axisRotation);
            tmp.r[12] += eclipticPos[0];
            tmp.r[13] += eclipticPos[1];
            tmp.r[14] += eclipticPos[2];
            mat_local_to_body = mat_local_to_body.multiplyFast(tmp);
        } else {
            mat_local_to_body.multiplyTranslation(eclipticPos);
        }
    }

    inline void selectiveUpdate(double jd, Mat4f mat_local_to_parent) {
        transformParentToBodyPos(jd, mat_local_to_parent);
        preUpdate(jd, mat_local_to_parent);
        // std::cout << englishName << " : " << isVisible << " : " << mat_local_to_parent.getTranslation() << std::endl;
        if (isVisible) {
            recursiveUpdate(jd, mat_local_to_parent.multiplyFast(computeBodyPosToBody(jd)));
        } else {
            mat.r[12] = mat_local_to_parent.r[12];
            mat.r[13] = mat_local_to_parent.r[13];
            mat.r[14] = mat_local_to_parent.r[14];
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
    inline float getRadius() const {
        return radius;
    }
    inline float getScaledRadius() const {
        return scaledRadius;
    }
    inline void setRadius(float _radius) {
        radius = _radius;
        uncached = true;
    }
    inline void setScaling(float _scale) {
        scaling = _scale;
        uncached = true;
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
    // Find a better reference body, return nullptr if this body is the best one
    inline ModularBody *findBetterReference() {
        if (distance > areaOfInfluence)
            return parent;
        for (auto &c : childs) {
            if (c.isInAreaOfInfluence())
                return &c;
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
    // This value should be set before calling update
    static float halfFov;
    inline bool isInAreaOfInfluence() const {
        return distance <= areaOfInfluence;
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
        return -26.73f + 2.5f*log10f(factor);
    }
    static void setTranslator(Translator &_translator);
    inline float getDistanceToObserver() const {
        return distance;
    }
    // Return true if this body has the STAR bit set, meaning it emit light
    inline bool isStar() const {
        return (bodyType & BodyType::STAR) == BodyType::STAR;
    }
    // Return true if this body is a system
    inline bool isSystem() const {
        return bodyType == BodyType::SYSTEM;
    }
    // Return true if this body is at the center of his system
    inline bool isSystemCentered() const {
        const ModularBody *body = this;
        while (body->isNotIsolated) {
            if (eclipticPos.v[0] || eclipticPos.v[1] || eclipticPos.v[2])
                return false;
            body = body->parent;
        }
        return (body->bodyType == BodyType::SYSTEM);
    }

    inline void enterEnvironment() {
    }
    inline void leaveEnvironment() {
    }
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
    void slot(StringID slotID, std::unique_ptr<BodyModule> &&module)
    {
        if (components.size() <= slotID.id) {
            components.resize(slotID.id + 1);
        } else if (auto *slot = components[slotID.id].get()) {
            std::erase(farComponents, slot);
            std::erase(nearComponents, slot);
            std::erase(groundedComponents, slot);
            std::erase(inComponents, slot);
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
    inline void drawHalo(Renderer &renderer) {
        const float fov_q = std::min(halfFov, static_cast<float>(M_PI)/6.f);
        const float mag = computeMagnitude();

        rmag = sqrtf(renderer.adaptLuminance(expf(-0.92103f*mag)*1.5289075617276093f / (fov_q * fov_q))) * 6.f * ModularBody::haloScale;
        if (!isStar() && (halfFov >= (M_PI/6)))
            rmag /= 5;
        if (rmag < 1.2f) {
            cmag = (mag > 0) ? (rmag*rmag/1.44f) : (rmag/1.2f);
            if (mag > 6.5f)
                cmag *= rmag*rmag/1.44f;
            rmag = 1.2f;
        } else {
            float limit = ModularBody::haloSizeLimit/1.8;
    		if (rmag > limit) {
    			rmag = limit + sqrt(rmag-limit)/(limit + 1);
    			if (rmag > ModularBody::haloSizeLimit)
    				rmag = ModularBody::haloSizeLimit;
    		}
            cmag = 1.f;
        }
        const float screen_r = screenSize * 1024;
        cmag *= 0.5*rmag/screen_r;
        if (cmag > 1)
            cmag = 1;
        if (rmag < screen_r) {
    		cmag *= rmag/screen_r;
    		rmag = screen_r;
    	}

        if (!isStar()) {
            const Vec3f _planet = parent->mat.getTranslation() - lightPosition;
            const Vec3f _satellite = mat.getTranslation() - lightPosition;
            double OP = _planet.length();
    		double OS = _satellite.length();
            if (OP < OS && fabs(acos(_planet.dot(_satellite)/(OP*OS))) < atan(parent->radius/OP)) {
                cmag = 0.0;
            }
        }

        if (cmag > 0.05)
            renderer.drawHalo(screenPos, haloColor * cmag, rmag);
    }
    // Identity
    std::string englishName;
    std::string nameI18;

    // Relations
    ModularBody *parent;
    BodyRelation relation;
    std::vector<std::unique_ptr<ModularBody>> groundedBodies;
    std::vector<std::unique_ptr<ModularBody>> orbitingBodies;
    std::vector<std::unique_ptr<ModularBody>> innerBodies;
    std::vector<std::unique_ptr<ModularBody>> hiddenBodies;
    std::vector<std::unique_ptr<EnvironmentModule>> groundedEnvironment;
    std::vector<std::unique_ptr<EnvironmentModule>> environment;
    // std::vector<std::unique_ptr<ShadowProjection>> shadows;

    // TODO create an optimized std::string for limited set
    std::vector<std::unique_ptr<BodyModule>> components; // Reference every BodyModule of this ModularBody by name

    // TODO use *Bodies instead
    std::list<ModularBody> childs; // Drawn if screenSize >= 10%

    std::vector<BodyModule *> farComponents; // 2D behind body, SKIP when screenSize > 20%, update NEVER called
    std::vector<BodyModule *> nearComponents; // Drawn if screenSize >= 0.15% and distance > scaledRadius * BODY_SURFACE_HEIGHT
    std::vector<BodyModule *> groundedComponents; // Drawn if distance <= scaledRadius * BODY_SURFACE_HEIGHT
    std::vector<BodyModule *> inComponents; // Draw if distance <= scaledRadius
    // std::list<std::shared_ptr<BodyOrbitModule>> orbitalComponents; // Components drawing lines between bodies
    // std::list<std::shared_ptr<EnvironmentModule>> environmentComponents; // Component defining the environment

    // Positionnal
    std::unique_ptr<Orbit> orbit;
    RotationElements re;

    // Cached data (may deprecate)
    Mat4f mat; // Matrix defining this body regarding to the observer
    Vec3f eclipticPos;
    std::pair<float, float> screenPos;
    float halfAngularSize;
    float screenSize; // Ratio of the screen taken by this body
    float distance;
    float axisRotation;
    float scaledRadius;
    //float scaledInnerRadius;
    float rmag;
    float cmag;
    double lastJD = 0;

    // Halo system
    Vec3f haloColor;
    float albedo;					// Body albedo

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
    bool boundToSurface = false; // Determine whether this body is bound to the surface of his parent
    bool altitudeRelativeToRadius = true; // Determine whether observer's altitude on this body is relative to the radius

    // Global datas
    static Vec3f lightPosition; // Observer-local light source position
    static float lightDistance; // Observer-local light source distance
    static float lightSize;     // Light source radius
    static ModularBody *lastFit;
    static Translator *translator;
    static std::map<std::string, ModularBody *> bodyReference;
    static std::list<ModularBody> hidden;
    static std::vector<ModularBody *> notableBody; // List of bodies sufficiently large to need a depth bucket
    static Vec3f defaultHaloColor;
    static float haloScale;
    static float haloSizeLimit;
};

#endif /* end of include guard: MODULAR_BODY_HPP_ */
