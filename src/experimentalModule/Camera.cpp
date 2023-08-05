#include "Camera.hpp"

Camera::Camera(ModularBody *reference, float longitude, float latitude, float altitude) :
    reference(reference), longitude(longitude), latitude(latitude), altitude(altitude+reference->getAltitudeReference())
{
}

void Camera::switchToBody(ModularBody *dst)
{
    bool oldFreeMode = freeMode;
    bool oldBoundToSurface = boundToSurface;
    setBoundToSurface(false);
    setFreeMode(true);
    view.setRotation(view.getMatrix().multiplyFast(reference->calculateSwitchCompensation(dst)).toQuaternion());
    reference = dst;
    setFreeMode(oldFreeMode);
    setBoundToSurface(oldBoundToSurface);
}

void Camera::update(float jd, float deltaTime)
{
    view.update(deltaTime);
    Mat4f mat;
    if (freeMode) {
        if (auto newRef = reference->findBetterReference()) {
            switchToBody(newRef);
        }
        mat = view.getMatrix().multiplyTranslation(position);
    } else {
        mat = view.getMatrix()
            .multiplyTranslation(Vec3f(0, 0, distance))
            .multiplyFast(Mat4d::yrotation(M_PI_2-latitude));
            .multiplyFast(Mat4d::zrotation(longitude))
    }
    if (boundToSurface)
        mat = mat.multiplyFast(computeBodyToSurface());
    system = ModularBody::dispatchUpdate(reference, jd, mat);
}

void Camera::draw(Renderer &renderer)
{
    system->drawSystem(renderer);
}

void Camera::setFreeMode(bool b)
{
    if (b == freeMode)
        return;
    // TODO complete this by taking rotation into account
    if (b) {
        Utility::spheToRect(longitude, latitude, pos);
        pos *= distance;
    } else {
        Utility::rectToSphe(longitude, latitude, pos);
        distance = pos.length();
    }
    freeMode = b;
}

void Camera::setBoundToSurface(bool b)
{
    if (b == boundToSurface)
        return;
    if (b) {
        if (freeMode) {
            pos = reference->computeSurfaceToBody().multiplyWithoutTranslation(pos);
        } else {
            longitude -= reference->getAxisRotation();
        }
    } else {
        if (freeMode) {
            pos = reference->computeBodyToSurface().multiplyWithoutTranslation(pos);
        } else {
            longitude += reference->getAxisRotation();
        }
    }
    boundToSurface = b;
}
