// F91 -- INTENT 11.213.  WHICH FRAME DOES THE NEW PATH'S RA/DE LAND IN, AND
// WHERE DOES 11.158(f2)'s "-90.0003 deg constant" COME FROM?
//
//   g++ -O0 -std=c++20 -I../../src -o /tmp/f91_frame f91_frame.cpp && /tmp/f91_frame
//   (-std=c++20 is required: vecmath.hpp uses `requires` clauses -- F40's note.)
//
// This probe is written BEFORE the fix and carries its predictions in the
// PRED_* strings below, so every line can fail.  It answers four questions with
// the project's OWN primitives (the paper derivation of this exact composition
// has been wrong twice already -- 11.143(b), 11.144(d)):
//
//  (1) Is the shipped `Camera::observedToBodyLocalPos` the inverse of viewMat?
//      PREDICTION: no -- 133.9041 deg, F34's number (11.144(j)(1)).
//  (2) Is the expression F91 lands the inverse?
//      PREDICTION: yes -- |err| <= 1e-9 AU on the same round trip.
//  (3) Does the corrected inverse land in OLD's equatorial frame?  Old builds
//      mat_local_to_earth_equ = Z((sidereal+lon)*pi/180) . Y((90-lat)*pi/180)
//      [observed: anchor_point_body.cpp:64-65, navigator.cpp:232], and old's
//      LOCAL frame is the new path's local frame turned by +pi/2 about z --
//      that is 11.60's measured azimuth convention read as a frame relation
//      (az_raw_old = pi/2 + az_raw_new, from ModularObject::altAz's
//      `pi/2 - az_raw` [ModularObject.cpp:163] against Body::getAltAz's
//      `3pi - az_raw` [body.cpp:381]).
//      PREDICTION: the two frames COINCIDE -- residual <= 1e-6 deg (double).
//  (4) What is 11.158(f2)'s CONSTANT -90.0003 deg?
//      PREDICTION: it is the `+ M_PI_2` inside ModularBody::getAxisRotation()
//      [ModularBody.hpp:575-577], which the surface fold S carries and which
//      F44's OFFLINE reconstruction could not see, because the dump emits
//      `computeAxisRotation` (the raw axisRotation) [ModularBody.cpp:926] and
//      the reconstruction folds by rz(axisRot) [f44_parity.py:333-334].
//      So: full fold  => 0 deg residual;  fold without the pi/2 => 90.000000 deg.
//      The 0.0003 deg of the measured number is then NOT a frame term at all.
//
// Scene: F44's own measured stamp (11.158(e)) -- camera reference Earth,
// longitude 0.093666002 rad (5.3667 E), latitude 0.755727589 rad (43.3000 N),
// distance 4.2636e-05 AU, freeMode false, boundToSurface true, viewOffsetEff 0,
// Earth axisRot 5.059841 rad.  The round trip additionally uses F34's own
// synthetic place (lon 1.2217, lat 0.5236, dist 4.929e-05, theta 0.7) so its
// number is comparable to the 133.9041 deg on record.
#include "tools/vecmath.hpp"
#include <cstdio>
#include <cmath>
#include <algorithm>

// NB the separation is computed in DOUBLE even for float vectors: a float dot
// product's rounding puts an acos precision floor at ~0.03 deg, which is how a
// 2e-11 AU round trip prints as "0.0198 deg" if you let the floats do it.
static double angdeg(const Vec3f &a, const Vec3f &b)
{
    const Vec3d ad(a[0], a[1], a[2]), bd(b[0], b[1], b[2]);
    const double c = ad.dot(bd) / (ad.length() * bd.length());
    return acos(std::max(-1.0, std::min(1.0, c))) * 180.0 / M_PI;
}
// Utility::rectToSphe(double*,double*,const Vec3d&) transcribed verbatim
// [observed: src/tools/utility.cpp:102-107] -- linking utility.cpp would drag
// the whole tools tree in for two lines.
static void rectToSphe(double *lng, double *lat, const Vec3d &v)
{
    const double r = v.length();
    *lat = asin(v[2] / r);
    *lng = atan2(v[1], v[0]);
}
static double angdeg(const Vec3d &a, const Vec3d &b)
{
    const double c = a.dot(b) / (a.length() * b.length());
    return acos(std::max(-1.0, std::min(1.0, c))) * 180.0 / M_PI;
}

int main()
{
    // ---------------------------------------------------------------- (1)(2)
    // F34's place, so the shipped number is comparable with the one on record.
    const float lon = 1.221730476f, lat = 0.523598776f, dist = 4.929e-05f;
    const float theta = 0.7f;                      // getAxisRotation(), radians
    const Mat4f R = Mat4f::zrotation(0.31f).multiplyFast(Mat4f::xrotation(-0.77f));
    const Mat4f S = Mat4f::zrotation(-theta);      // computeSurfaceToBody()
    const Mat4f St = Mat4f::zrotation(theta);      // computeBodyToSurface() == S^T

    printf("=== F91 frame probe (predictions in the header; every line can fail)\n");
    for (int bound = 1; bound >= 0; --bound) {
        for (int freeMode = 0; freeMode <= 1; ++freeMode) {
            // viewMat(), Camera.cpp:173-199, verbatim
            const Vec3f position = Vec3f(cosf(-lon) * cosf(lat),
                                         sinf(-lon) * cosf(lat), sinf(lat)) * dist;
            Mat4f mat{R};
            if (freeMode) {
                mat.multiplyTranslation(position);
            } else {
                mat.multiplyTranslation(Vec3f(0, 0, -dist));
                mat = mat.multiplyFast(Mat4f::xrotation(lat - M_PI_2))
                         .multiplyFast(Mat4f::zrotation(-lon));
            }
            if (bound)
                mat = mat.multiplyFast(S);

            const Vec3f q(1.3e-04f, -7.0e-05f, 4.1e-05f);   // a point in viewMat's INPUT frame
            const Vec4f o4 = mat * Vec4f(q[0], q[1], q[2], 1.f);
            const Vec3f o(o4[0], o4[1], o4[2]);

            // ---- SHIPPED: Camera.hpp:264-274 verbatim (pre-F91) -------------
            Vec3f sh = R.transpose().multiplyWithoutTranslation(o);   // observedToLocalPos
            if (freeMode) {
                sh -= position;
            } else {
                sh.v[2] -= dist;
                sh = Mat4f::yrotation(lat - M_PI_2).multiplyWithoutTranslation(sh);
                sh = Mat4f::zrotation(-lon).multiplyWithoutTranslation(sh);
            }
            // ---- F91: the exact inverse of the composition above ------------
            Vec3f fx = R.transpose().multiplyWithoutTranslation(o);
            if (freeMode) {
                fx -= position;
            } else {
                fx.v[2] += dist;
                fx = Mat4f::zrotation(lon).multiplyFast(Mat4f::xrotation(M_PI_2 - lat))
                        .multiplyWithoutTranslation(fx);
            }
            if (bound)
                fx = St.multiplyWithoutTranslation(fx);

            printf("  bound=%d free=%d : shipped |err| %.3e AU  angle %8.4f deg"
                   "  |  F91 |err| %.3e AU  angle %.4f deg\n",
                   bound, freeMode, (sh - q).length(), angdeg(sh, q),
                   (fx - q).length(), angdeg(fx, q));
        }
    }

    // -------------------------------------------------------------------- (3)(4)
    // F44's measured scene (11.158(e)), in double, so the residual is the
    // FRAME's and not float32's.
    const double LON = 0.093666002, LAT = 0.755727589, AXIS = 5.059841;
    // old: mat_local_to_earth_equ = Z(sidereal+lon) . Y(90-lat)   [degrees at
    // the site; here already in radians, same value]
    const Mat4d oldLocalToEqu = Mat4d::zrotation(AXIS + LON)
                                    .multiplyFast(Mat4d::yrotation(M_PI_2 - LAT));
    // new, F91's chain, WITHOUT the eye rotation (it cancels: both sides start
    // from the same local vector):  [S^T] . Z(+lon) . X(pi/2-lat)
    const Mat4d newFull = Mat4d::zrotation(AXIS + M_PI_2)
                              .multiplyFast(Mat4d::zrotation(LON))
                              .multiplyFast(Mat4d::xrotation(M_PI_2 - LAT));
    // the COUNTERFACTUAL: the fold F44's reconstruction applied, Z(axisRot)
    const Mat4d newF44 = Mat4d::zrotation(AXIS)
                             .multiplyFast(Mat4d::zrotation(LON))
                             .multiplyFast(Mat4d::xrotation(M_PI_2 - LAT));
    // old's LOCAL frame == the new path's local frame turned by +pi/2 (11.60)
    const Mat4d newLocalToOldLocal = Mat4d::zrotation(M_PI_2);

    double worstFull = 0, worstF44 = 0;
    double raOffMin = 1e9, raOffMax = -1e9, deOffMax = 0;
    for (int i = 0; i < 64; ++i) {
        // a spread of directions in the NEW path's local (zenith) frame
        const double az = (i % 8) * (M_PI / 4) + 0.137, al = ((i / 8) - 3.5) * 0.4;
        const Vec3d lnew(cos(al) * cos(az), cos(al) * sin(az), sin(al));
        const Vec3d eqOld = oldLocalToEqu.multiplyWithoutTranslation(
                                newLocalToOldLocal.multiplyWithoutTranslation(lnew));
        const Vec3d eqNew = newFull.multiplyWithoutTranslation(lnew);
        const Vec3d eq44 = newF44.multiplyWithoutTranslation(lnew);
        worstFull = std::max(worstFull, angdeg(eqOld, eqNew));
        worstF44 = std::max(worstF44, angdeg(eqOld, eq44));
        // 11.158(f2) measured a RIGHT-ASCENSION offset with the declinations
        // already matched, so score it the same way rather than as a separation
        // (a separation is blind to a zero point -- 11.158(i2), the same trap).
        double raA, deA, ra4, de4;
        rectToSphe(&raA, &deA, eqOld);
        rectToSphe(&ra4, &de4, eq44);
        double d = (raA - ra4) * 180.0 / M_PI;
        while (d > 180) d -= 360;
        while (d < -180) d += 360;
        raOffMin = std::min(raOffMin, d);
        raOffMax = std::max(raOffMax, d);
        deOffMax = std::max(deOffMax, fabs(deA - de4) * 180.0 / M_PI);
    }
    printf("  frame identity, 64 directions, double:\n");
    printf("    F91 chain (fold Z(axisRot+pi/2))  vs old's frame : max %.9f deg\n", worstFull);
    printf("    F44 chain (fold Z(axisRot))       vs old's frame : max %.9f deg\n", worstF44);
    printf("    F44 chain, scored as 11.158(f2) does -- RA offset old-minus-f44:\n");
    printf("      [%.9f, %.9f] deg, spread %.9f ; max |dDE| %.9f deg\n",
           raOffMin, raOffMax, raOffMax - raOffMin, deOffMax);
    return 0;
}
