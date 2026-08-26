// F40 / INTENT §5.80 — THE PREDICTIONS, COMMITTED BEFORE THE PRODUCT CHANGE.
//
//   g++ -O0 -std=c++20 -I../../src -o /tmp/f40_probe f40_probe.cpp && /tmp/f40_probe
//   (-std=c++20 is REQUIRED: vecmath.hpp uses `requires` clauses; without it the
//    header does not parse and the probe never builds — f34_probe.cpp's recorded
//    command line omits the flag and no longer works on this toolchain.)
//
// F34 (§11.144) established WHERE the free-mode teleport lives: on the
// CONVERTER, the three sites that turn the anchored spherical triple into the
// free cartesian member and back.  §11.151(a) ratified the repair: the
// converter becomes the composer's EXACT inverse.  This probe writes the
// proposed converter out and checks it against `Camera::viewMat()` rebuilt
// factor for factor with the PROJECT's own primitives (tools/vecmath.hpp,
// header-only) — the same instrument shape as `f34_probe.cpp`, for the same
// reason: a column-major or handedness slip in a paper derivation has already
// happened twice in this class (§11.143(b)), so nothing here is transcribed
// from memory.
//
// THE COMPOSER, solved for the eye (this is what must not move):
//   anchored:  mat = R . T(0,0,-d) . X(lat-pi/2) . Z(-lon) [. S]
//              => input-frame eye  E = S^-1 . pB,  pB = Z(lon).X(pi/2-lat).(0,0,d)
//                                                     = d.(cos p sin l, -cos p cos l, sin p)
//   free:      mat = R . T(position) [. S]
//              => input-frame eye  E = -S^-1 . position
//   The fold S cancels: E_free == E_anchored  <=>  position == -pB, for every S.
//
// THE PROPOSED CONVERTER (the code this probe licenses):
//   posePart(lon,lat,d)      -> pB                              [forward]
//   posePartToPose(p)        -> (atan2(p.x,-p.y), asin(p.z/|p|), |p|)   [inverse]
//   setFreeMode(true)        : position = -posePart(longitude,latitude,distance)
//   setFreeMode(false)       : (longitude,latitude,distance) = posePartToPose(-position)
//   moveTo free branch       : dst = -posePart(pos[0],pos[1],altRef+pos[2])
//   getPlace() free branch   : posePartToPose(-position), altitude = |position| - altRef
//   placeAt()                : ALREADY this algebra (Camera.cpp:502-517, F33) —
//                              it is routed through the same pair, unchanged.
#include "tools/vecmath.hpp"
#include <cstdio>
#include <cmath>

static int failures = 0;

static void chk(bool ok, const char *label, const char *fmt = nullptr, double v = 0)
{
    if (!ok) ++failures;
    printf("%s %-62s", ok ? "OK  " : "FAIL", label);
    if (fmt) printf(fmt, v);
    printf("\n");
}

// Utility::spheToRect(float lng, float lat, Vec3f &v)  [tools/utility.cpp:95]
static Vec3f spheToRect(float lng, float lat)
{
    const double cosLat = cos(lat);
    return Vec3f(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

// ---- THE PROPOSED PAIR, verbatim as it will be written in Camera --------
static Vec3f posePart(float lon, float lat, float dist)
{
    const float cl = std::cos(lat);
    return Vec3f(dist * cl * std::sin(lon), -dist * cl * std::cos(lon),
                 dist * std::sin(lat));
}

static Vec3f posePartToPose(const Vec3f &p, float lonAt0, float latAt0)
{
    const float d = p.length();
    if (d == 0.f)
        return Vec3f(lonAt0, latAt0, 0.f);
    return Vec3f(std::atan2(p[0], -p[1]), std::asin(p[2] / d), d);
}

// ---- the composer, rebuilt factor for factor [Camera.cpp:183-198] -------
static Vec3f eyeOf(const Mat4f &mat)
{
    const Vec3f t = mat.getTranslation();
    return Vec3f(-(mat.r[0]*t[0] + mat.r[1]*t[1] + mat.r[2]*t[2]),
                 -(mat.r[4]*t[0] + mat.r[5]*t[1] + mat.r[6]*t[2]),
                 -(mat.r[8]*t[0] + mat.r[9]*t[1] + mat.r[10]*t[2]));
}

static Vec3f anchoredEye(const Mat4f &R, float lon, float lat, float d,
                         bool bound, float theta)
{
    Mat4f mat{R};
    mat.multiplyTranslation(Vec3f(0, 0, -d));
    mat = mat.multiplyFast(Mat4f::xrotation(lat - M_PI_2))
             .multiplyFast(Mat4f::zrotation(-lon));
    if (bound)
        mat = mat.multiplyFast(Mat4f::zrotation(-theta)); // computeSurfaceToBody()
    return eyeOf(mat);
}

static Vec3f freeEye(const Mat4f &R, const Vec3f &position, bool bound, float theta)
{
    Mat4f mat{R};
    mat.multiplyTranslation(position);
    if (bound)
        mat = mat.multiplyFast(Mat4f::zrotation(-theta));
    return eyeOf(mat);
}

struct Pose { const char *name; float lon, lat, d; };

int main()
{
    const Mat4f R = Mat4f::zrotation(0.31f).multiplyFast(Mat4f::xrotation(-0.77f));
    const float theta = 0.7f;                 // getAxisRotation() = axisRotation + pi/2

    // Places: the F34 discriminating pose, the SHIPPED place §5.80 was minted
    // on, the F39 free discriminator, both poles, the date line, the centre.
    const Pose poses[] = {
        {"F34 lam 70 phi 30, 7378 km", 1.221730476f, 0.523598776f, 4.929e-05f},
        {"shipped 5d22'E 43d18'N 75 m", 0.093724f,   0.755941f,    4.2635e-05f},
        {"F39 discriminator lon 60",    1.047197551f, 0.0f,        4.2634e-05f},
        {"north pole",                  0.4f,         1.5707963f,  4.3e-05f},
        {"south pole",                 -2.1f,        -1.5707963f,  4.3e-05f},
        {"date line lon 180",           3.14159265f,  0.2f,        4.3e-05f},
        {"lon -180",                   -3.14159265f, -0.9f,        1.0e-03f},
        {"far, 1 AU",                   2.0f,         0.3f,        1.0f},
    };

    for (int bound = 0; bound < 2; ++bound) {
        printf("\n=== boundToSurface = %d ===\n", bound);
        for (const Pose &q : poses) {
            const Vec3f Ea = anchoredEye(R, q.lon, q.lat, q.d, bound, theta);

            // (1) THE FIX: the eye does not move across the toggle.
            const Vec3f posFix = posePart(q.lon, q.lat, q.d) * -1.f;
            const Vec3f Ef = freeEye(R, posFix, bound, theta);
            const double resFix = (Ef - Ea).length();
            // (2) TODAY: position = spheToRect(-lon,lat)*distance  [Camera.cpp:740]
            const Vec3f posNow = spheToRect(-q.lon, q.lat) * q.d;
            const Vec3f EfNow = freeEye(R, posNow, bound, theta);
            const double resNow = (EfNow - Ea).length();
            printf("  %-30s  fixed %.3e AU | today %.3e AU (%8.4f deg, %9.1f km)\n",
                   q.name, resFix, resNow,
                   acos(std::max(-1.0, std::min(1.0,
                        (double)Ea.dot(EfNow) / (Ea.length() * EfNow.length()))))
                   * 180 / M_PI,
                   resNow * 149597870.691);
            // FLOOR: float32 eps is 2^-24 = 5.96e-08 RELATIVE, and both eyes
            // are three-factor products at the scale of d, so the residual is
            // relative to d — not absolute.  (The first spelling used an
            // absolute 1e-09 AU floor and failed ONLY the 1 AU pose, at
            // 6.7e-08 AU = 1.1 ulp of float32 at 1 AU: a gate error, attributed
            // before anything was changed, F34's own rule.)
            chk(resFix < 1e-6 * q.d, "    the fixed converter holds the eye",
                "  %.3e AU", resFix);
            chk(resNow > resFix * 1e3 || q.d == 0, "    ... and today's does not (two-sided)",
                "  %.3e AU", resNow);

            // (3) the inverse recovers the triple that generated it
            const Vec3f back = posePartToPose(posFix * -1.f, 0.f, 0.f);
            double dl = std::fmod(back[0] - q.lon + 3 * M_PI, 2 * M_PI) - M_PI;
            if (std::cos(q.lat) < 1e-6) dl = 0;   // at a pole longitude is free
            chk(std::abs(dl) < 1e-5 && std::abs(back[1] - q.lat) < 1e-5
                && std::abs(back[2] - q.d) < 1e-6 * q.d,
                "    triple -> position -> triple round trip", "  dlon %.3e rad", dl);

            // (4) the recovered triple's anchored eye IS the free eye
            const Vec3f Eb = anchoredEye(R, back[0], back[1], back[2], bound, theta);
            chk((Eb - Ef).length() < 1e-6 * q.d,
                "    exit(entry(pose)) draws the same eye", "  %.3e AU",
                (Eb - Ef).length());

            // (5) getPlace()'s free answer after entry == the entry triple
            //     (what `get status position` answers today, §11.144(f)) — the
            //     readout's VALUE is preserved by the fix, which is why the
            //     second §11.144 rider is not touched.
            const Vec3f pl = posePartToPose(posFix * -1.f, q.lon, q.lat);
            double dlp = std::fmod(pl[0] - q.lon + 3 * M_PI, 2 * M_PI) - M_PI;
            if (std::cos(q.lat) < 1e-6) dlp = 0;
            chk(std::abs(dlp) < 1e-5 && std::abs(pl[1] - q.lat) < 1e-5,
                "    getPlace() after entry == the entry triple", "  dlon %.3e rad", dlp);
        }
    }

    // ---- the degenerate member: the eye AT the reference's centre ----------
    {
        const Vec3f zero(0, 0, 0);
        const Vec3f pl = posePartToPose(zero, 1.2f, -0.4f);
        chk(pl[0] == 1.2f && pl[1] == -0.4f && pl[2] == 0.f,
            "centre: the angles parametrize nothing, the pair is kept (placeAt's rule)");
    }

    // ---- placeAt's existing algebra IS this pair ---------------------------
    // Camera.cpp:508-517 computes d=|p|, latitude=asin(p[2]/d),
    // longitude=atan2(p[0],-p[1]) — routing it through posePartToPose must be
    // bit-identical, which is checkable here rather than by reading.
    {
        const Vec3f p(3.1e-05f, -2.7e-05f, 1.9e-05f);
        const float d = p.length();
        const float lat = std::asin(p[2] / d), lon = std::atan2(p[0], -p[1]);
        const Vec3f v = posePartToPose(p, 0, 0);
        chk(v[0] == lon && v[1] == lat && v[2] == d,
            "placeAt's anchored inverse == posePartToPose, bit for bit");
    }

    // ---- the free discriminator §11.152(o), in closed form -----------------
    // A rover authored at `orbit_lon L` sits at azimuth L in the parent's
    // surface frame (SurfacePointOrbitLoader.hpp:145, spheToRect(lon,lat)).
    // The observer's pose part pB has azimuth lon - pi/2 — MEASURED as the
    // 90.0 deg surface-mode reading of §11.152(o).  Today's free converter
    // writes pA, whose eye -pA has azimuth pi - lon, i.e. 60.0 deg away at
    // lon = 60: that is the same measurement's FREE reading, and it is the
    // defect, not the target.  After the fix both modes read the surface
    // number, and the TOGGLE moves nothing — which is the transparency
    // §11.151(a) ratified.  [The 90 deg itself is §5.49's channel, untouched.]
    {
        const float L = 1.047197551f;                    // 60 deg
        const Vec3f rover = spheToRect(L, 0.f);
        const Vec3f obsNow = spheToRect(-L, 0.f) * -1.f; // eye = -position, today
        const Vec3f obsFix = posePart(L, 0.f, 1.f);      // eye = pB, after
        const double aNow = acos(rover.dot(obsNow)) * 180 / M_PI;
        const double aFix = acos(rover.dot(obsFix)) * 180 / M_PI;
        printf("\nfree discriminator at lon 60: today %.4f deg, after the fix %.4f deg"
               " (surface mode reads %.4f deg in BOTH)\n", aNow, aFix, aFix);
        chk(std::abs(aNow - 60.0) < 1e-3, "  today's free reading is §11.152(o)'s 60.0 deg");
        chk(std::abs(aFix - 90.0) < 1e-3, "  the fixed free reading is the surface 90.0 deg");
    }

    printf("\nf40_probe: %s (%d failures)\n", failures ? "FAILURES" : "all checks OK", failures);
    return failures ? 1 : 0;
}
