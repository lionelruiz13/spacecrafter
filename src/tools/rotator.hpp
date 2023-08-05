#ifndef ROTATOR_HPP_
#define ROTATOR_HPP_

#include "vecmath.hpp"

template <class T>
class Rotator {
public:
    //! Create a rotator with initial rotation
    Rotator(const Vector4<T> &initial = {1, 0, 0, 0}) {
        setRotation(initial);
    }

    //! Initiate a movement
    inline void moveTo(const Vector4<T> &target, int duration, bool isMaxDuration = false) {
        timer = 0;
        src = current;
        dst = target;
        float cosTheta = src.dot(dst);
        onTransition = true;
        if (cosTheta < 0) {
            dst = -dst;
            cosTheta = src.dot(dst);
        }
        if (cosTheta < 1) {
            theta = acos(cosTheta);
            timerTarget = (isMaxDuration) ? (duration * (theta / M_PI)) : (duration);
            invSinTheta = 1 / sin(theta);
        } else {
            timerTarget = 0;
        }
    }
    //! Initiate a movement relative to the final rotation of the last movement
    inline void moveRel(const Vector4<T> &relTarget, int duration, bool isMaxDuration = false) {
        moveTo(relTarget.combineQuaternions(dst), duration, isMaxDuration);
    }
    //! Set a rotation without any delay
    inline void setRotation(const Vector4<T> &target) {
        dst = current = target;
        mat = Matrix4<T>::fromQuaternion(target);
        outdatedCurrent = outdatedMat = onTransition = false;
    }
    //! Set a rotation relative to the position before applying the rotation of this rotator
    inline void setPreRotation(const Vector4<T> &relTarget) {
        if (onTransition) {
            src = src.combineQuaternions(relTarget);
            dst = dst.combineQuaternions(relTarget);
        } else {
            dst = current = current.combineQuaternions(relTarget);
            outdatedMat = true;
        }
    }
    //! Set a rotation relative to the final rotation of the last movement
    inline void setRelRotation(const Vector4<T> &relTarget) {
        setRotation(relTarget.combineQuaternions(dst));
    }
    //! Return true if currently moving
    inline bool moving() const {
        return onTransition;
    }
    //! Increment the internal counter of deltaTime
    //! @param updateCache should this update perform an update of the cached current quaternion and matrix
    //! @return true if the angle is modified with this update, false otherwise
    inline bool update(float deltaTime, bool updateCache = false) {
        if (!onTransition)
            return false;
        timer += deltaTime;
        if (timer >= timerTarget) {
            setRotation(dst);
        } else {
            outdatedCurrent = true;
            outdatedMat = true;
            if (updateCache)
                getMatrix();
        }
        return true;
    }
    //! Update the internal cached current quaternion if it is outdated, and return it
    inline const Vector4<T> &getQuaternion() {
        if (outdatedCurrent) {
            // Compute slerp
            float t = timer / timerTarget;
            float c0 = sin((1 - t) * theta) * invSinTheta;
            float c1 = sin(t * theta) * invSinTheta;
            current.set(c0*src[0] + c1*dst[0],
                        c0*src[1] + c1*dst[1],
                        c0*src[2] + c1*dst[2],
                        c0*src[3] + c1*dst[3]);
            outdatedCurrent = false;
        }
        return current;
    }
    //! Return the cached quaternion without updating it
    inline const Vector4<T> &getCachedQuaternion() const {
        return current;
    }
    //! Update the internal cached matrix if it is outdated, and return it
    inline const Matrix4<T> &getMatrix() {
        if (outdatedMat) {
            mat = Matrix4<T>::fromQuaternion(getQuaternion());
            outdatedMat = false;
        }
        return mat;
    }
    //! Return the cached matrix without updating it
    inline const Matrix4<T> &getCachedMatrix() const {
        return mat;
    }
    //! Return the time in milliseconds before the end of the movement
    inline int timeBeforeEnd() const {
        return (onTransition) ? (timerTarget - timer) : 0;
    }
    //! Return the quaternion of the end of the movement
    inline const Vector4<T> &getTarget() const {
        return (onTransition) ? current : dst;
    }
private:
    float timer;
    float timerTarget;
    float theta;
    float invSinTheta;
    Vector4<T> src;
    Vector4<T> dst;
    Vector4<T> current;
    Matrix4<T> mat; // Cached matrix
    bool outdatedCurrent;
    bool outdatedMat;
    bool onTransition;
};

#endif /* end of include guard: ROTATOR_HPP_ */
