#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/vecmath.hpp"
#include "tools/rotator.hpp"
#include "tools/utility.hpp"
#include "tools/sc_const.hpp"
#include <cmath>

class Renderer;
class ModularBody;
class ModularSystem;

class Camera {
public:
    Camera(ModularBody *reference, float longitude, float latitude, float altitude);
    // Return the distance to the reference in AU
    float distanceToReference() const;
    // Change the reference body without moving
    void switchToBody(ModularBody *dst);
    // Change the reference body
    void warpToBody(ModularBody *dst);
    void update(double jd, float deltaTime);
    void draw(Renderer &renderer);
    void setBoundToSurface(bool b);
    void setFreeMode(bool b);

    void lookTo(const Vec3f &direction, float duration = 1, bool isMaxDuration = false);
    void lookTo(float _alt, float _az, float duration = 1, bool isMaxDuration = false);
    void lookRel(float deltaAlt, float deltaAz, float duration = 1, bool isMaxDuration = false);

    void moveHeading(float deltaHeading);
    void setHeading(float heading);
    inline float getHeading() const {
        return heading;
    }

    void moveRel(const Vec3f &position, float duration = 0, bool calculateDuration = false);
    inline void moveEyeRel(const Vec3f &position, float duration = 0) {
        moveRel(observedToLocalPos(position), duration);
    }
    inline void moveRelLon(float lon, float delay = 0) {
        if (freeMode) {
            lon *= distanceToReference() * 5.f;
            moveEyeRel({lon, 0, 0}, delay);
        } else {
            moveRel({lon, 0, 0}, delay);
        }
    }
    inline void moveRelLat(float lat, float delay = 0) {
        if (freeMode) {
            lat *= distanceToReference() * 5.f;
            moveEyeRel({0, lat, 0}, delay);
        } else {
            moveRel({0, lat, 0}, delay);
        }
    }
    // With alt in meters
    inline void moveRelAlt(double alt, float delay = 0) {
        alt /= 1000*AU;
        if (freeMode) {
            moveEyeRel({0, 0, static_cast<float>(-alt)}, delay);
        } else {
            moveRel({0, 0, static_cast<float>(alt)}, delay);
        }
    }
    inline void multAlt(float coef) {
        coef = distanceToReference()*(coef-1);
        if (freeMode) {
            moveEyeRel({0, 0, coef});
        } else {
            moveRel({0, 0, coef});
        }
    }
    inline void moveTo(const Vec3f &pos, float duration = 0, bool calculateDuration = false) {
        moveRel(pos - (freeMode ? position : Vec3f(longitude, latitude, distanceToReference())), duration, calculateDuration);
    }

    inline Vec3f observedToLocalPos(const Vec3f &observedPos) const {
        return view.getCachedMatrix().transpose().multiplyWithoutTranslation(observedPos);
    }
    inline Vec3f observedToBodyLocalPos(const Vec3f &observedPos) const {
        Vec3f ret = observedToLocalPos(observedPos);
        if (freeMode) {
            ret -= position;
        } else {
            ret.v[2] -= distance;
            ret = Mat4f::yrotation(latitude-M_PI_2).multiplyWithoutTranslation(ret);
            ret = Mat4f::zrotation(-longitude).multiplyWithoutTranslation(ret);
        }
        return ret;
    }
    inline std::pair<float, float> observedPosToRaDe(const Vec3f &observedPos) const {
        Vec3f direction = observedToBodyLocalPos(observedPos);
        std::pair<float, float> ret;
        if ((direction[0] + direction[1]) == 0) {
            ret.second = 0;
            ret.first = std::copysign(M_PI_2, direction[2]);
        } else {
            Utility::rectToSphe(&ret.first, &ret.second, direction);
        }
        return ret;
    }
    inline std::pair<float, float> observedPosToAltAz(const Vec3f &observedPos) const {
        Vec3f direction = observedToLocalPos(observedPos);
        std::pair<float, float> ret;
        if ((direction[0] + direction[1]) == 0) {
            ret.first = 0;
            ret.second = std::copysign(M_PI_2, direction[2]);
        } else {
            Utility::rectToSphe(&ret.second, &ret.first, direction);
        }
        return ret;
    }
    inline void trackBody(nullptr_t) {
        target = nullptr;
    }
    inline void trackBody(ModularBody *body) {
        target = body;
    }
    inline ModularBody *getReferenceBody() const {
        return reference;
    }
    inline ModularSystem *getCurrentSystem() const {
        return system;
    }
    void setHalfFov(float halfFov, float duration = 0.5);

    // Compatibility methods, only work while not in freeMode
    inline void setLongitude(float l) {
        longitude = l*M_PI/180;
    }
    inline void setLatitude(float l) {
        latitude = l*M_PI/180;
    }
    void setAltitude(double altitude);
    inline float getLatitude() const {
        return latitude;
    }
    // This disallow using multiple cameras, but multiple cameras can't be used simultaneously anyway
    static Camera *instance;
private:
    // Calculate intermediate zoom coefficient
    inline float calculateZoomCoef() const {
        float coef = zoomTimer / zoomDuration;
        if (coef > 0.5) {
            coef = 1 - coef;
            coef = 1 - 2*coef*coef;
        } else
            coef *= 2*coef;
        return coef;
    }
    // Calculate zoom coefficient velocity (derivated for one second)
    inline float calculateZoomCoefVelocity() const {
        float coef = zoomTimer / zoomDuration;
        coef = (coef > 0.5) ? 4*(1-coef) : 4*coef;
        return coef / zoomDuration;
    }
    void recomputeAltAzHeading();
    ModularBodyPtr reference;
    ModularBodyPtr target;
    ModularSystem *system; // Determined on construction and update
    Rotator<float> view;
    float alt = 0;
    float az = 0;
    float heading = 0;
    Vec3f position;
    Vec3f deltaPosition;
    float moveDuration = 0;
    float longitude;
    float latitude;
    float distance; // In AU
    float zoomDuration = 0;
    float zoomTimer;
    float srcHalfFov;
    float dstHalfFov;
    static float minHalfFov;
    static float maxHalfFov;
    bool freeMode = false;
    bool boundToSurface = true;
};

#endif /* end of include guard: CAMERA_HPP_ */
