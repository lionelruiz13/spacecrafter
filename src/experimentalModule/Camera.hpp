#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/vecmath.hpp"
#include "tools/rotator.hpp"

class Renderer;
class ModularBody;
class ModularSystem;

class Camera {
public:
    Camera(ModularBody *reference, float longitude, float latitude, float altitude);
    float distanceToReference() const {
        return distance - reference->getAltitudeReference();
    }
    // Change the reference body
    void switchToBody(ModularBody *dst);
    void update(float deltaTime);
    void draw(Renderer &renderer);
    void setBoundToSurface(bool b);
    void setFreeMode(bool b);
private:
    ModularBodyPtr reference;
    ModularSystem *system; // Determined on update
    Rotator<float> view;
    Vec3f viewDirection;
    Vec3f position;
    float longitude;
    float latitude;
    float distance;
    bool freeMode = false;
    bool boundToSurface = true;
};

#endif /* end of include guard: CAMERA_HPP_ */
