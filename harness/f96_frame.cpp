// F96 -- INTENT 11.216.  WHAT DOES THE B17 VIEW OFFSET DO TO
// `Camera::observedToLocalPos`, AND WHAT DOES THE POLE GUARD ANSWER?
//
//   g++ -O0 -std=c++20 -I../../src -o /tmp/f96_frame f96_frame.cpp && /tmp/f96_frame
//   /tmp/f96_frame --state <heading> <alt> <az> <foldLat> <offset> <halfFov>
//        (radians; prints the 3x3 conjugation matrix C for that camera state,
//         so an independently written model can be scored against this one)
//   (-std=c++20 is required: vecmath.hpp uses `requires` clauses -- F40's note.)
//
// This probe is written BEFORE the leg and BEFORE the fix; its predictions are
// the PRED_* strings below and every line of output can contradict them.  It
// uses the project's OWN Mat4f primitives, because the paper derivation of this
// exact composition has been wrong twice already (11.143(b), 11.144(d)).
//
// THE MODEL (11.216, derived from Camera.cpp:139-155 + :173-199 + Camera.hpp:260):
//   viewMat()          = R' . Rv . T(...) . placement       [Camera.cpp:184-197]
//   getObservedPosition() is a copy of that matrix's action, so an observed
//   position o carries R' . Rv . (the true local direction d, scaled).
//   observedToLocalPos = Rv^T . o                           [Camera.hpp:260-262]
//                      = (Rv^T . R' . Rv) . d               == C . d
//   with R' = xrotation(offset * halfFov)                   [Camera.cpp:139-145]
//        Rv = zrotation(heading+pi) . xrotation(pi/2-alt)
//             . zrotation(az-pi/2) . fold()                 [Camera.cpp:93-104]
//        fold() = xrotation(pi/2 - foldLat) when EQUATORIAL and not free
//                                                           [Camera.cpp:86-91]
//
//  (1) PREDICTION -- C is a ROTATION BY EXACTLY `offset*halfFov` ABOUT Rv^T*x,
//      whatever the camera state: max |angle(C) - offset*halfFov| <= 1e-6 deg
//      and max angle(axis(C), Rv^T*x) <= 1e-6 deg over a grid of camera states.
//      At the shipped fov 180 (halfFov = pi/2) and `set zoom_offset 0.3` that
//      angle is 0.3 * 90 deg = 27.000000 deg.
//  (2) PREDICTION -- the FIX (renderViewRotation()^T = Rv^T . R'^T) recovers d
//      exactly: round-trip residual <= 1e-6 deg, at every offset; while the
//      shipped expression is off by up to `offset*halfFov` (27 deg), the error
//      vanishing only for a direction ALONG the axis.
//  (3) PREDICTION -- THE TWO MUTATED MODELS ARE DISTINGUISHABLE FROM THE MODEL:
//        M1 (the rotation the other way)   C1 = Rv^T . R'^T . Rv
//        M2 (the conjugation the other way) C2 = Rv . R' . Rv^T
//      max angle(C.d, C1.d) is ~2*27 deg and max angle(C.d, C2.d) > 1 deg for a
//      generic camera state -- so the leg's per-body table can refute them.
//      (M2 collapses onto M1/M0 only where Rv commutes with R'.)
//  (4) PREDICTION -- THE POLE GUARD.  `observedPosToRaDe` at an exact-pole
//      direction (0,0,+-1): the SHIPPED branch answers RA = +-90 deg, DE = 0;
//      the FIX answers RA = 0, DE = +-90 deg; OLD -- `Utility::rectToSphe` with
//      no branch at all -- answers RA = 0, DE = +-90 deg (atan2(0,0) == 0,
//      asin(+-1) == +-pi/2).  So the fix lands on old and the shipped branch
//      does not.  The SIBLING `observedPosToAltAz` (Camera.hpp:326-328) is
//      CORRECT with the same two lines, because there `.first` is the altitude:
//      it answers alt = +-90 deg, az = 0 both before and after.
#include "tools/vecmath.hpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <string>

static const double DEG = 180.0 / M_PI;

// Utility::rectToSphe(float*,float*,const Vec3f&) transcribed verbatim
// [observed: src/tools/utility.cpp] -- linking utility.cpp would drag the whole
// tools tree in for two lines.  *lng = atan2(y,x), *lat = asin(z/r).
static void rectToSphe(float *lng, float *lat, const Vec3f &v)
{
    const float r = v.length();
    *lat = asinf(v[2] / r);
    *lng = atan2f(v[1], v[0]);
}

static double angdeg(const Vec3f &a, const Vec3f &b)
{
    const Vec3d ad(a[0], a[1], a[2]), bd(b[0], b[1], b[2]);
    const double c = ad.dot(bd) / (ad.length() * bd.length());
    return acos(std::max(-1.0, std::min(1.0, c))) * DEG;
}

// Camera::fold(), Camera.cpp:86-91 (EQUATORIAL mount, not free -- the shipped
// state on every leg here).
static Mat4f fold(float foldLat)
{
    return Mat4f::xrotation(M_PI_2 - foldLat);
}

// Camera::viewRotation(), Camera.cpp:93-104, verbatim.
static Mat4f viewRotation(float heading, float alt, float az, float foldLat)
{
    return Mat4f::zrotation(heading + M_PI)
        .multiplyFast(Mat4f::xrotation(M_PI_2 - alt))
        .multiplyFast(Mat4f::zrotation(az - M_PI_2))
        .multiplyFast(fold(foldLat));
}

// Camera::viewOffsetEyeRotation(), Camera.cpp:139-145, verbatim.
static Mat4f viewOffsetEyeRotation(float o, float halfFov)
{
    if (o == 0.f)
        return Mat4f::identity();
    return Mat4f::xrotation(o * halfFov);
}

// The rotation angle of a 3x3 rotation, from its trace.
static double rotAngleDeg(const Mat4f &m)
{
    const double tr = (double)m.r[0] + (double)m.r[5] + (double)m.r[10];
    return acos(std::max(-1.0, std::min(1.0, (tr - 1.0) / 2.0))) * DEG;
}

// The rotation axis of a 3x3 rotation, from its antisymmetric part.
static Vec3f rotAxis(const Mat4f &m)
{
    // column-major: m.r[C*4+R] is row R, column C.
    Vec3f a(m.r[6] - m.r[9], m.r[8] - m.r[2], m.r[1] - m.r[4]);
    const float n = a.length();
    return (n > 0.f) ? a * (1.f / n) : a;
}

static void printMat3(const char *label, const Mat4f &m)
{
    printf("%s = [", label);
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            printf("%s%.9f", (row || col) ? ", " : "", m.r[col * 4 + row]);
    printf("]\n");
}

int main(int argc, char **argv)
{
    if (argc >= 8 && std::string(argv[1]) == "--state") {
        const float heading = atof(argv[2]), alt = atof(argv[3]);
        const float az = atof(argv[4]), foldLat = atof(argv[5]);
        const float o = atof(argv[6]), hf = atof(argv[7]);
        const Mat4f Rv = viewRotation(heading, alt, az, foldLat);
        const Mat4f Rp = viewOffsetEyeRotation(o, hf);
        const Mat4f C = Rv.transpose().multiplyFast(Rp).multiplyFast(Rv);
        printf("# state heading=%.9f alt=%.9f az=%.9f foldLat=%.9f offset=%.9f "
               "halfFov=%.9f\n", heading, alt, az, foldLat, o, hf);
        printMat3("C", C);
        printMat3("Rv", Rv);
        const Vec3f axis = Rv.transpose().multiplyWithoutTranslation(Vec3f(1, 0, 0));
        printf("axis_model = [%.9f, %.9f, %.9f]\n", axis[0], axis[1], axis[2]);
        printf("angle_deg = %.9f\n", rotAngleDeg(C));
        return 0;
    }

    printf("=== F96 offset-frame probe (predictions in the header; every line can fail)\n");

    // ------------------------------------------------------------------ (1)(3)
    // A grid of camera states x offsets.  The shipped `set zoom_offset 0.3` at
    // the shipped fov 180 is the (o=0.3, halfFov=pi/2) cell.
    const float headings[] = {0.f, 0.7f, -2.1f};
    const float alts[]     = {0.f, 0.9f, -0.4f};
    const float azs[]      = {0.f, 2.3f, 5.1f};
    const float foldLats[] = {0.755727589f, 0.f, -1.1f};
    const float offsets[]  = {0.3f, 0.1f, -0.5f};
    const float halfFov    = M_PI_2;                 // fov 180, the shipped one

    double worstAngle = 0, worstAxis = 0, worstM1 = 0, worstM2 = 0, minM2 = 1e9;
    double worstShipped = 0, worstFixed = 0;
    for (float heading : headings)
      for (float a : alts)
        for (float z : azs)
          for (float fl : foldLats)
            for (float o : offsets) {
                const Mat4f Rv = viewRotation(heading, a, z, fl);
                const Mat4f Rp = viewOffsetEyeRotation(o, halfFov);
                const Mat4f C  = Rv.transpose().multiplyFast(Rp).multiplyFast(Rv);
                const Mat4f C1 = Rv.transpose().multiplyFast(Rp.transpose())
                                     .multiplyFast(Rv);                  // M1
                const Mat4f C2 = Rv.multiplyFast(Rp).multiplyFast(Rv.transpose()); // M2
                const double want = fabs(o) * halfFov * DEG;
                worstAngle = std::max(worstAngle, fabs(rotAngleDeg(C) - want));
                Vec3f model = Rv.transpose().multiplyWithoutTranslation(Vec3f(1, 0, 0));
                if (o < 0) model = model * -1.f;   // the trace axis is unsigned-consistent
                worstAxis = std::max(worstAxis, std::min(angdeg(rotAxis(C), model),
                                                         angdeg(rotAxis(C), model * -1.f)));
                double m2here = 0;
                for (int i = 0; i < 64; ++i) {
                    const double azd = (i % 8) * (M_PI / 4) + 0.137;
                    const double ald = ((i / 8) - 3.5) * 0.4;
                    const Vec3f d(cos(ald) * cos(azd), cos(ald) * sin(azd), sin(ald));
                    const Vec3f m0 = C.multiplyWithoutTranslation(d);
                    worstM1 = std::max(worstM1, angdeg(m0, C1.multiplyWithoutTranslation(d)));
                    m2here = std::max(m2here, angdeg(m0, C2.multiplyWithoutTranslation(d)));
                    // the two expressions of observedToLocalPos, on the SAME
                    // observed position: o_obs = R' . Rv . d
                    const Vec3f obs = Rp.multiplyFast(Rv).multiplyWithoutTranslation(d);
                    const Vec3f shipped = Rv.transpose().multiplyWithoutTranslation(obs);
                    const Vec3f fixed = Rp.multiplyFast(Rv).transpose()
                                            .multiplyWithoutTranslation(obs);
                    worstShipped = std::max(worstShipped, angdeg(shipped, d));
                    worstFixed = std::max(worstFixed, angdeg(fixed, d));
                }
                worstM2 = std::max(worstM2, m2here);
                minM2 = std::min(minM2, m2here);
            }
    printf("  (1) C == rotation(offset*halfFov, Rv^T*x) over 243 camera states:\n");
    printf("      max |angle(C) - |offset|*halfFov|  = %.9f deg\n", worstAngle);
    printf("      max angle(axis(C), Rv^T*x)         = %.9f deg\n", worstAxis);
    printf("  (2) observedToLocalPos on an offset-carrying observed position, 64 dirs:\n");
    printf("      SHIPPED (Rv^T)              max error = %.9f deg\n", worstShipped);
    printf("      FIX     ((R'.Rv)^T)         max error = %.9f deg\n", worstFixed);
    printf("  (3) the mutated models, scored against the model on the same dirs:\n");
    printf("      M1 (rotation the other way) max sep  = %.9f deg\n", worstM1);
    printf("      M2 (conjugation swapped)    max sep  = %.9f deg (min over states %.9f)\n",
           worstM2, minM2);

    // The shipped cell, named on its own: fov 180, `set zoom_offset 0.3`.
    {
        const Mat4f Rv = viewRotation(0.f, 0.3f, 1.1f, 0.755727589f);
        const Mat4f Rp = viewOffsetEyeRotation(0.3f, M_PI_2);
        const Mat4f C = Rv.transpose().multiplyFast(Rp).multiplyFast(Rv);
        printf("  the shipped cell (fov 180, set zoom_offset 0.3): angle(C) = %.9f deg\n",
               rotAngleDeg(C));
    }

    // -------------------------------------------------------------------- (4)
    printf("  (4) the pole guard, direction (0,0,+-1):\n");
    for (int s = 1; s >= -1; s -= 2) {
        const Vec3f dir(0.f, 0.f, (float)s);
        // SHIPPED observedPosToRaDe branch [Camera.cpp:256-258]
        std::pair<float, float> shipped;
        shipped.second = 0;
        shipped.first = std::copysign(M_PI_2, dir[2]);
        // THE FIX: the +-pi/2 belongs in .second (declination)
        std::pair<float, float> fixed;
        fixed.first = 0;
        fixed.second = std::copysign(M_PI_2, dir[2]);
        // OLD: Utility::rectToSphe, no branch at all [body.cpp getRaDeValue]
        float ra_old, de_old;
        rectToSphe(&ra_old, &de_old, dir);
        // THE SIBLING observedPosToAltAz [Camera.hpp:326-328]: .first is ALT
        std::pair<float, float> altaz;
        altaz.first = std::copysign(M_PI_2, dir[2]);
        altaz.second = 0;
        printf("      z=%+d : SHIPPED RA %+8.3f DE %+8.3f | FIX RA %+8.3f DE %+8.3f"
               " | OLD RA %+8.3f DE %+8.3f | sibling alt %+8.3f az %+8.3f\n",
               s, shipped.first * DEG, shipped.second * DEG,
               fixed.first * DEG, fixed.second * DEG,
               ra_old * DEG, de_old * DEG,
               altaz.first * DEG, altaz.second * DEG);
    }
    // and the guard's own reachability: what the branch tests
    {
        const Vec3f d(0.f, 0.f, 1.f);
        printf("      branch condition (x==0 && y==0) on (0,0,1) : %s\n",
               (d[0] == 0 && d[1] == 0) ? "TAKEN" : "not taken");
    }
    return 0;
}
