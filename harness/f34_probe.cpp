// F34 / §5.80 — the predictions of f34_convention.py, recomputed with the
// PROJECT'S OWN matrix primitives (tools/vecmath.hpp, header-only), so that a
// column-major or rotation-handedness slip in the Python cannot survive.
//
//   g++ -O0 -I../../src -o /tmp/f34_probe f34_probe.cpp && /tmp/f34_probe
//
// It rebuilds `Camera::viewMat()` factor for factor (Camera.cpp:172-198) for a
// given pose, extracts the eye the way `getReferenceRelativePosition` does
// (-Rot^T . t, Camera.cpp:455-462), and prints it beside the two closed forms
// the Python predicts with.  Every line that follows is a source transcription,
// with its file:line.
#include "tools/vecmath.hpp"
#include <cstdio>
#include <cmath>

// Utility::spheToRect(float lng, float lat, Vec3f &v)  [tools/utility.cpp:95]
static Vec3f spheToRect(float lng, float lat)
{
    const double cosLat = cos(lat);
    return Vec3f(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

static void show(const char *tag, const Vec3f &v)
{
    printf("%-28s %+.9f %+.9f %+.9f\n", tag, v[0], v[1], v[2]);
}

int main()
{
    const float lon = 1.221730476f;   // 70 deg
    const float lat = 0.523598776f;   // 30 deg
    const float dist = 4.929e-05f;    // ~7378 km, AU
    const float theta = 0.7f;         // getAxisRotation() = axisRotation + pi/2

    // R stands in for renderViewRotation(); any rotation must cancel in -Rot^T.t
    const Mat4f R = Mat4f::zrotation(0.31f).multiplyFast(Mat4f::xrotation(-0.77f));
    const Mat4f S = Mat4f::zrotation(-theta);          // computeSurfaceToBody()

    // ---- anchored branch, Camera.cpp:183-196 -----------------------------
    Mat4f mat{R};
    mat.multiplyTranslation(Vec3f(0, 0, -dist));
    mat = mat.multiplyFast(Mat4f::xrotation(lat - M_PI_2))
             .multiplyFast(Mat4f::zrotation(-lon));
    mat = mat.multiplyFast(S);
    const Vec3f t = mat.getTranslation();
    const Vec3f E(-(mat.r[0]*t[0] + mat.r[1]*t[1] + mat.r[2]*t[2]),
                  -(mat.r[4]*t[0] + mat.r[5]*t[1] + mat.r[6]*t[2]),
                  -(mat.r[8]*t[0] + mat.r[9]*t[1] + mat.r[10]*t[2]));
    show("anchored E = -Rot^T.t", E);
    // the Python's C1: Z(theta) . pB
    const Vec3f pB(dist * cosf(lat) * sinf(lon),
                   -dist * cosf(lat) * cosf(lon),
                   dist * sinf(lat));
    show("  Z(theta).pB", Mat4f::zrotation(theta).multiplyWithoutTranslation(pB));

    // ---- free branch, Camera.cpp:184-185 ---------------------------------
    Vec3f position = spheToRect(-lon, lat) * dist;      // setFreeMode(true):740-741
    Mat4f fmat{R};
    fmat.multiplyTranslation(position);
    fmat = fmat.multiplyFast(S);
    const Vec3f ft = fmat.getTranslation();
    const Vec3f fE(-(fmat.r[0]*ft[0] + fmat.r[1]*ft[1] + fmat.r[2]*ft[2]),
                   -(fmat.r[4]*ft[0] + fmat.r[5]*ft[1] + fmat.r[6]*ft[2]),
                   -(fmat.r[8]*ft[0] + fmat.r[9]*ft[1] + fmat.r[10]*ft[2]));
    show("free E = -Rot^T.t", fE);
    const Vec3f pA(dist * cosf(lat) * cosf(lon),
                   -dist * cosf(lat) * sinf(lon),
                   dist * sinf(lat));
    show("  -Z(theta).pA", Mat4f::zrotation(theta).multiplyWithoutTranslation(pA) * -1.f);
    show("  position", position);
    show("  pA", pA);

    // ---- the swing, closed form vs measured on the two matrices ----------
    const double c = E.dot(fE) / (E.length() * fE.length());
    const double claw = cosf(lat) * cosf(lat) * (1 - sin(2 * lon)) - 1;
    const double cazo = cosf(lat) * cosf(lat) * sin(2 * lon) + sinf(lat) * sinf(lat);
    printf("swing measured %.6f deg | H_A(sign+azimuth) %.6f | azimuth-only %.6f\n",
           acos(c) * 180 / M_PI, acos(claw) * 180 / M_PI, acos(cazo) * 180 / M_PI);
    printf("|E| %.9e  |fE| %.9e  (equal = constant radius)\n", E.length(), fE.length());

    // ---- the rotation that takes the anchored eye to the free one --------
    // predicted: 180 deg about the equatorial-plane axis at azimuth theta+45 deg
    const Vec3f axis(cos(theta + M_PI_4), sin(theta + M_PI_4), 0.f);
    const Vec3f pred = axis * (2.f * axis.dot(E)) - E;   // Rodrigues, angle = pi
    show("Rx(pi about th+45).E", pred);
    printf("residual vs free E: %.3e AU\n", (pred - fE).length());
    return 0;
}
