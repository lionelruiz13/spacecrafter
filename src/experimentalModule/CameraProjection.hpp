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
        return projectEye(localToEye.multiplyWithoutTranslation(local), win);
    }
    // Same from eye space; win[2] = 0, the CPU sky draws 2D overlays without depth
    bool projectEye(const Vec3d &eye, Vec3d &win) const {
        const double depth = eye.length();
        const double rq = std::sqrt(eye[0] * eye[0] + eye[1] * eye[1]);
        if (rq == 0) {
            win.set(cx, cy, 0);
            return eye[2] < 0;
        }
        const float r = ProjectionTransfer::radius(mode, std::acos(-eye[2] / depth) / halfFov, halfFov);
        const double f = r * radius / rq;
        win.set(cx + eye[0] * f, cy + eye[1] * f, 0);
        return r <= 1;
    }
    // Return the eye direction of the screen disc point (x, y), in disc radii from the centre
    Vec3d direction(double x, double y) const {
        const double len = std::sqrt(x * x + y * y);
        if (len == 0)
            return Vec3d(0, 0, -1);
        const double theta = ProjectionTransfer::angleNorm(mode, len, halfFov) * halfFov;
        const double s = std::sin(theta) / len;
        return Vec3d(x * s, y * s, -std::cos(theta));
    }
};

#endif /* end of include guard: CAMERA_PROJECTION_HPP_ */
