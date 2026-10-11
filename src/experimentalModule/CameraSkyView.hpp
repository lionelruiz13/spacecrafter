#ifndef CAMERA_SKY_VIEW_HPP_
#define CAMERA_SKY_VIEW_HPP_

#include "coreModule/skyView.hpp"
#include "CameraProjection.hpp"

class Camera;
class ModularBody;

// The new path's sky view, composed once per frame from the Camera and the current system
class CameraSkyView final : public SkyView {
public:
    CameraSkyView();
    // Call once per frame before any sky layer reads the view
    void update(const Camera &camera, const ModularBody &system);
    bool projectLocal(const Vec3d &v, Vec3d &win) const override;
    bool projectLocalCheck(const Vec3d &v, Vec3d &win) const override;
    bool projectEarthEqu(const Vec3d &v, Vec3d &win) const override;
    bool projectEarthEquFixed(const Vec3d &v, Vec3d &win) const override;
    bool projectEarthEcliptic(const Vec3d &v, Vec3d &win) const override;
    bool projectJ2000(const Vec3d &v, Vec3d &win) const override;
    bool projectJ2000Check(const Vec3d &v, Vec3d &win) const override;
    bool projectJ2000LineCheck(const Vec3d &v1, Vec3d &win1, const Vec3d &v2, Vec3d &win2) const override;
    bool projectJ2000Galactic(const Vec3d &v, Vec3d &win) const override;
    bool projectDomeFixed(const Vec3d &v, Vec3d &win) const override;
    Vec3d getCursorPosEqu(int x, int y) const override;
    double getFov() const override;
    int getViewportWidth() const override;
    int getViewportHeight() const override;
    double getViewportRadius() const override;
    Vec3d getViewportCenter() const override;
    Mat4f getMatProjectionOrtho2D() const override;
    void printGravity180(s_font *font, float x, float y, const std::string &str, Vec4f Color, float xshift, float yshift) const override;
    Mat4f getMatLocalToEye() const override;
    Mat4f getMatJ2000ToEye() const override;
    Mat4f getMatEarthEquToEye() const override;
    Vec3d earthEquToLocal(const Vec3d &v) const override;
    Vec3d localToEarthEqu(const Vec3d &v) const override;
    Vec3d helioToLocal(const Vec3d &v) const override;
    Vec3d helioToEarthEqu(const Vec3d &v) const override;
    const Vec3d &getPrecEquVision() const override;
    CameraProjection law; // From the local frame
    Mat4d j2000ToLocal;
private:
    bool projectFrom(const Mat4d &toEye, const Vec3d &v, Vec3d &win) const {
        return law.projectEye(toEye.multiplyWithoutTranslation(v), win);
    }
    CameraProjection dome; // The fixed 180-degree dome
    Mat4d earthEquToEye, earthEquFixedToEye, eclipticToEye, j2000ToEye, galacticToEye;
    Mat4d earthEquToLocalMat, localToEarthEquMat, helioToLocalMat, helioToEarthEquMat;
    Vec3d precEquVision;
};

#endif /* end of include guard: CAMERA_SKY_VIEW_HPP_ */
