#ifndef MODULAR_BODY_HPP_
#define MODULAR_BODY_HPP_

#include "BodyModule.hpp"
#include "Renderer.hpp"

enum class BodyType : unsigned char {
    ANCHOR, // Simplest type, just an anchor
    MINOR_BODY, // A body with minor features, usefull for massively instanciated bodies
    SPHERICAL_BODY, // A spherical body, who doesn't use self-shadowing
    SINGLE_BODY, // A body with a single shape
    CUSTOM_BODY, // A body with multiple shapes
    STAR = 0x40, // A body who emit light
    // Bodies with hard-coded specificities
    EARTH = 0x80,
    EARTH_MOON,
};

#define UPDATE_CADENCY JD_SECOND

class ModularSystem;

struct ModularBodyCreateInfo {
    std::unique_ptr<Orbit> orbit;
    std::string englishName;
    RotationElements re;
    Vec3f haloColor;
    float albedo;
    float radius;
    float innerRadius;
    float oblateness;
    float solLocalDay;
    BodyType bodyType;
    bool isHaloEnabled;
    bool altitudeRelativeToRadius;
};

class ModularBody {
    friend class ModularBodyPtr;
    friend class ModularBodySelector;
    // For the initial loading of a ModularBody
    friend class ModularBodyLoader;
public:
    ~ModularBody();
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
    void updateEclipticPos(Vec3f eclipticPos, double jd, double targetJD);
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
        return axisRotation + M_PI_2;
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
            for (auto &module : farComponents)
                module.draw(renderer, this, mat);
            if (screenSize > 0.0015) {
                if (loaded) {
                    const auto matrix = mat.multiplyFast(computeBodyToSurface()); // TODO Fix ojml ?
                    if (screenSize > 0.008) {
                        renderer.clearDepth();
                        if (screenSize < 0.2) {
                            for (auto &module : nearComponents)
                                module.draw(renderer, this, matrix);
                        } else {
                            if (distance < scaledInnerRadius) {
                                for (auto &module : inComponents)
                                    module.draw(renderer, this, matrix);
                            } else {
                                for (auto &module : nearComponents)
                                    module.draw(renderer, this, matrix);
                            }
                        }
                    } else {
                        for (auto &module : nearComponents)
                            module.drawNoDepth(renderer, this, matrix);
                        drawHalo(renderer);
                    }
                } else
                    drawLoaded(renderer);
            } else {
                drawHalo(renderer);
            }
        }
    }

    inline void transformParentToBodyPos(double jd, Mat4f &mat_local_to_body) {
        const double delta = jd - lastJD;
        lastJD = jd;
        if ((delta > 0) == (jd > computedJD)) {
            eclipticPos = computedEclipticPos + deltaEclipticPos * (jd - computedJD);
            updateEclipticPos(computedEclipticPos, computedJD, (delta > 0) ? (jd + UPDATE_CADENCY) : (jd - UPDATE_CADENCY));
        } else {
            eclipticPos += deltaEclipticPos * delta;
        }
        if (boundToParent)
            mat_local_to_body = mat_local_to_body.multiplyFast(computeBodyToSurface());
        mat_local_to_body.multiplyTranslation(eclipticPos);
    }

    inline Mat4f computeBodyPosToBody() {
        return Mat4f::xzrotation(
            re.obliquity,
            re.ascendingNode -re.precessionRate*(jd-re.epoch)
        );
    }

    inline void transformBodyToParent(double jd, Mat4f &mat_local_to_body) {
        const double delta = jd - lastJD;
        lastJD = jd;
        if ((delta > 0) == (jd > computedJD)) {
            eclipticPos = computedEclipticPos + deltaEclipticPos * (jd - computedJD);
            updateEclipticPos(computedEclipticPos, computedJD, (delta < 0) ? (jd + UPDATE_CADENCY) : (jd - UPDATE_CADENCY));
        } else {
            eclipticPos += deltaEclipticPos * delta;
        }
        mat_local_to_body = mat_local_to_body.multiplyFast(Mat4f::zxrotation(
            re.precessionRate*(jd-re.epoch) - re.ascendingNode,
            -re.obliquity
        ));
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
            recursiveUpdate(jd, mat_local_to_parent.multiplyFast(computeBodyPosToBody()));
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
    // Invalidate the internal state cache
    inline void invalidateCachedState() {
        uncached = true;
    }
    // Calculate a matrix to apply to the observer for a seemless change of body reference
    Mat4f calculateSwitchCompensation(ModularBody *to) const;
    // Find the nearest common parent between two bodies
    inline ModularBody *findCommonParent(ModularBody *with) const {
        for (ModularBody *common = with; common; common = common->parent) {
            for (ModularBody *p = this; p; p = p->parent) {
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
            module.preload(this);
        }
        for (auto &module : inComponents) {
            module.preload(this);
        }
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
    inline ModularBody *findBetterReference() const {
        if (distance > areaOfInfluence)
            return parent;
        for (auto &c : ret->childs) {
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
protected:
    ModularBody(ModularBody *parent, ModularBodyCreateInfo &info);
    void select();
    void deselect();
    inline void drawHalo(Renderer &renderer) {
        const float fov_q = std::min(halfFov, (M_PI/6));
        const float mag = computeMagnitude();

        rmag = sqrtf(renderer.adaptLuminance((expf(-0.92103f*(mag + 12.12331f)) * 8.2295998f) / (fov_q * fov_q))) * 30.f * ModularBody::haloScale;
        if (!(body->parent.bodyType & BodyType::STAR))
            rmag /= (halfFov >= (M_PI/6)) ? 25 : 5;
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
        }
        const float screen_r = screenSize * 1024;
        cmag *= 0.5*rmag/screen_r;
        if (cmag > 1)
            cmag = 1;
        if (rmag < screen_r) {
    		cmag *= rmag/screen_r;
    		rmag = screen_r;
    	}

        if (!(body->parent.bodyType & BodyType::STAR)) {
            const Vec3f _planet = parent->mat.getTranslation() - lightPosition;
            const Vec3f _satellite = mat.getTranslation() - lightPosition;
            double OP = _planet.length();
    		double OS = _satellite.length();
            float c = _planet.dot(_satellite);
            if (c>0 && parent->distance < distance) {
                if (fabs(acos(c/(parent->distance*distance))) < atan(parent->radius/parent->distance)) {
                    cmag = 0.0;
                }
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
    std::list<ModularBody> childs; // Drawn if screenSize >= 10%
    std::list<std::shared_ptr<BodyModule>> farComponents; // 2D behind body, SKIP when screenSize > 20%, update NEVER called
    std::list<std::shared_ptr<BodyModule>> nearComponents; // Drawn if screenSize >= 0.15% and distance > innerRadius
    std::list<std::shared_ptr<BodyModule>> inComponents; // Draw if distance <= innerRadius
    std::list<std::shared_ptr<BodyOrbitModule>> orbitalComponents; // Components drawing lines between bodies
    // std::list<std::shared_ptr<EnvironmentModule>> environmentComponents; // Component defining the environment

    // Positionnal
    std::unique_ptr<Orbit> orbit;
    RotationElements re;

    // Cached data
    Mat4f mat; // Matrix defining this body regarding to the observer
    Vec3f eclipticPos;
    std::pair<float, float> screenPos;
    float halfAngularSize;
    float screenSize; // Ratio of the screen taken by this body
    float distance;
    float axisRotation;
    float scaledRadius;
    float scaledInnerRadius;
    float rmag;
    float cmag;

    // Asynchronous internal datas
    double lastJD = 0; // Last JD
    double computedJD = 0; // Computed JD
    Vec3f computedEclipticPos; // computed position reached in the future, relative to the parent body
    Vec3f deltaEclipticPos; // Displacement from the previous eclipticPos to the computedEclipticPos

    // Halo system
    Vec3f haloColor;
    float albedo;					// Body albedo

    // Navigation and visibility
    float radius;
    float scaling = 1;
    float innerRadius; // Radius of the area in which inComponents are drawn
    float boundingRadius; // Smallest radius including all nearComponents
    float subsystemRadius; // Radius including all child bodies
    float areaOfInfluence; // Area under the influence of this body

    // Internal datas
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
    static ModularBody *lastFit;
    static std::map<std::string, ModularBody *> bodyReference;
    static std::list<ModularBody> hidden;
    static std::vector<ModularBody *> notableBody; // List of bodies sufficiently large to need a depth bucket
    static Vec3f defaultHaloColor;
    static float haloScale;
    static float haloSizeLimit;
};

#endif /* end of include guard: MODULAR_BODY_HPP_ */
