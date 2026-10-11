#ifndef CAMERA_PROJECTION_HPP_
#define CAMERA_PROJECTION_HPP_

#include "tools/vecmath.hpp"
#include "ProjectionTransfer.hpp"
#include <cmath>

// The new path's CPU projection law, fed by the Camera; must equal shaders/src/custom_project.glsl
struct CameraProjection {
    Mat4d localToEye; // Rotation only
    float halfFov = 0; // In radians
    int mode = ProjectionTransfer::FISHEYE;
    float radius = 0, cx = 0, cy = 0; // Screen disc, in render px
    // Return false when the point falls outside the screen disc
    bool project(const Vec3d &local, Vec3d &win) const {
        win = localToEye.multiplyWithoutTranslation(local);
        const double depth = win.length();
        const double rq = std::sqrt(win[0] * win[0] + win[1] * win[1]);
        if (rq == 0) {
            const bool front = win[2] < 0;
            win.set(cx, cy, depth);
            return front;
        }
        const float r = ProjectionTransfer::radius(mode, std::acos(-win[2] / depth) / halfFov, halfFov);
        const double f = r * radius / rq;
        win.set(cx + win[0] * f, cy + win[1] * f, depth);
        return r <= 1;
    }
};

#endif /* end of include guard: CAMERA_PROJECTION_HPP_ */
