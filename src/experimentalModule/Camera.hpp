#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/vecmath.hpp"
#include "tools/rotator.hpp"
#include "tools/utility.hpp"

class Renderer;
class ModularBody;
class ModularSystem;

class Camera {
public:
    Camera(ModularBody *reference, float longitude, float latitude, float altitude);
    float distanceToReference() const;
    // Change the reference body
    void switchToBody(ModularBody *dst);
    void update(float jd, float deltaTime);
    void draw(Renderer &renderer);
    void setBoundToSurface(bool b);
    void setFreeMode(bool b);
    void moveTo(const Vec3f &direction, float duration = 1, bool isMaxDuration = false);
    void moveTo(float _alt, float _az, float duration = 1, bool isMaxDuration = false);
    void moveRel(float deltaAlt, float deltaAz, float duration = 1, bool isMaxDuration = false);
    inline Vec3f observedToLocalPos(const Vec3f &observedPos) const {
        return view.getCachedMatrix().transpose().multiplyWithoutTranslation(observedPos);
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
    inline void trackBody(ModularBody *body) {
        target = body;
    }
private:
    void recomputeAltAzHeading();
    ModularBodyPtr reference;
    ModularBodyPtr target;
    ModularSystem *system; // Determined on update
    Rotator<float> view;
    float alt = 0;
    float az = 0;
    float heading = 0;
    Vec3f position;
    float longitude;
    float latitude;
    float distance;
    bool freeMode = false;
    bool boundToSurface = true;
};

#endif /* end of include guard: CAMERA_HPP_ */
