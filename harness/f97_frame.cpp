// F97 -- INTENT 11.217.  IN WHICH FRAME DOES A CHILD'S
// positionAtTimevInVSOP87Coordinates LAND, AND WHERE DOES `location_orbit`
// PUT A POINT THE AUTHOR SPELLED (orbit_lon, orbit_lat)?
//
//   g++ -O0 -std=c++20 -I../../src -I../../planetsephems -o /tmp/f97_frame \
//       f97_frame.cpp ../../planetsephems/sideral_time.c && /tmp/f97_frame
//   (-std=c++20 is required: vecmath.hpp uses `requires` clauses -- F40's note.)
//
// This probe is written BEFORE any code change and carries its predictions in
// the PRED_* strings below, so every line can fail.  It composes the project's
// OWN primitives (Mat4d/Mat4f/Utility::spheToRect transcribed verbatim) rather
// than a paper re-derivation, because the paper derivation of this exact
// composition has been wrong twice already (11.143(b), 11.144(d)).
//
// SCENE, all values FETCHED from ~/.spacecrafter/ssystem.ini (11.51(d): data
// values are never recalled), [mars] block, lines 156-190:
//   radius = 3397 km, rot_periode = 24.622962 h, rot_rotation_offset = 136.005,
//   rot_pole_ra = 317.6725, rot_pole_de = 52.88212, parent = Sun.
// protosystem.cpp:940-941 turns a pole into elements:
//   rot_obliquity = pi/2 - de,  rot_asc_node = ra + pi/2   (radians)
// protosystem.cpp:958-966: re.period = rot_periode/24 (days), re.epoch = J2000
// (rot_epoch absent), re.precessionRate = 0 (rot_precession_rate absent),
// re.offset = 136.005 (DEGREES).  Mars's parent is the Sun, whose
// rot_local_to_parent is never written (body.cpp:538 `if (parent)`), so
// Mars::getRotEquatorialToVsop87() == Mars's own rot_local_to_parent.
// [earth] block, lines 87-125: rot_periode = 23.9344694 h,
// rot_rotation_offset = 280.5, rot_obliquity = -23.4392803055555555556,
// rot_epoch = 2451545.0, rot_precession_rate = 1.39639 deg/century.
//
// ---------------------------------------------------------------- QUESTIONS
// (1) UNIT OF orbit_lat.  LocationOrbit's ctor converts `lon` by pi/180 and
//     passes `lat` RAW on the same line [orbit.cpp:1091], while the SAME loader
//     converts BOTH keys by pi/180 twelve lines later when it builds the
//     bound_to_surface rotation elements [protosystem.cpp:951-952].
//     PREDICTION P1: an author writing orbit_lat = 45 lands at planetographic
//     latitude asin(sin(45 rad)) = 58.3098861 deg, i.e. 13.3098861 deg north of
//     where he wrote -- the vector is still unit-length, so nothing else can
//     see the error.  Post-fix: 45.0000000 deg, residual <= 1e-12 deg.
//
// (2) THE FRAME `ecliptic_pos` LIVES IN, on the OLD path.
//     Body::get_heliocentric_ecliptic_pos SUMS ecliptic_pos up the chain with
//     no rotation [body.cpp:619-626]; Body::computeDraw's model matrix is
//     T(p_ecl) . T(b_ecl) . R_b . R_p, whose TRANSLATION is p_ecl + b_ecl
//     [body.cpp:992-1011]; and OrbitCreatorEliptic bakes the parent's equator
//     into the provider itself (`rotate_to_vsop87`, orbit.cpp:394-436 from
//     orbit_creator_cor.cpp:52-96) so that its output is already root-aligned.
//     CONCLUSION UNDER TEST: a child's ecliptic_pos is a vector in the ROOT
//     (VSOP87 ecliptic) ORIENTATION, centred on the parent -- never the
//     parent's equatorial frame and never its surface frame.
//     Old's OWN authority for "where is planetographic (lon,lat,alt) on body P"
//     is the observer's: AnchorPointBody::getRotLocalToEquatorial =
//     Z((sidereal+lon)*pi/180) . Y((90-lat)*pi/180) [anchor_point_body.cpp:63-64]
//     composed with P->getRotEquatorialToVsop87() [:69-71].
//     PREDICTION P2: the SHIPPED LocationOrbit output differs from that
//     authority by MORE than 30 deg on Mars at J2000 for (lon 0, lat 0) --
//     it omits getRotEquatorialToVsop87 entirely, so it places the point at
//     ecliptic latitude/longitude instead of planetographic ones.
//     PREDICTION P3: with the equatorial->VSOP87 rotation restored AND the
//     spin read from the parent at evaluation, the residual against old's own
//     authority is <= 1e-12 AU (exact by construction, both ways).
//
// (3) THE SPIN.  LocationOrbit freezes `parentOffset = P->getSiderealTime(0)`
//     and `JDToRotation = 2pi/parentPeriod` at CONSTRUCTION, both through
//     `float` locals [protosystem.cpp:557-558], and extrapolates linearly from
//     JD 0 -- i.e. from -4712-01-01, 2.46 million days back.
//     PREDICTION P4 (Mars, a linear-`re` parent): the frozen model is
//     ALGEBRAICALLY identical to P->getSiderealTime(JD) mod 360, so in DOUBLE
//     the residual would be <= 1e-6 deg; through the shipped `float`
//     parentSideralDay it is >= 1 deg at J2000 (float(1.02595675) differs from
//     the double in the 8th digit and that error is multiplied by 2.45e6 days).
//     PREDICTION P5 (Earth): getSiderealTime is get_apparent_sidereal_time,
//     whose mean rate is 360.98564736629 deg/day plus a quadratic and the
//     equation of the equinoxes; the frozen model's rate is 8640/23.9344694
//     deg/day.  Their difference at J2000 is >= 1 deg.
//
// (4) THE NEW PATH'S TWO SURFACE CONVENTIONS.  ModularBody::getAxisRotation()
//     is `axisRotation + M_PI_2` [ModularBody.hpp:575-577] and the grounded
//     fold is Z(getAxisRotation()) [.hpp:579-581, applied at .hpp:726-727 to
//     the parent's ACCUMULATED EQUATORIAL frame -- ModularBody.cpp:345-358
//     hands grounded children `mat`, the accumulated frame].  So a
//     `surface_point` body authored at orbit_lon = L sits at equatorial
//     longitude axisRotation + pi/2 + L.
//     The CAMERA's own placement is X(lat-pi/2) . Z(-lon) . Z(-getAxisRotation())
//     [Camera.cpp:189-201], whose inverse puts the observer at equatorial
//     longitude axisRotation + lon -- the -pi/2 hidden in X(lat-pi/2) cancels
//     the +pi/2 of the fold.
//     PREDICTION P6: the new path's OBSERVER at (lon, lat) coincides with old's
//     AnchorPointBody direction for the same (lon, lat) to <= 1e-5 deg (float),
//     over 64 (lon, lat, axisRot) states.  [This is what F91/F96 measured live:
//     88 of 90 bodies' alt/az agree to <= 1.7e-5 deg.]
//     PREDICTION P7: the new path's `surface_point` + grounded body authored at
//     the SAME (lon, lat) sits EXACTLY 90.000000 deg away in longitude from
//     that observer, spread 0.000000 deg over the same 64 states, with
//     IDENTICAL latitude.  If P7 holds, `surface_point`'s orbit_lon is NOT the
//     planetographic longitude the rest of the project means by `lon`, and
//     "land where surface_point + grounded lands" and "be exact against old"
//     are MUTUALLY EXCLUSIVE targets for location_orbit.
//     PREDICTION P8 (the mutation that makes P7 evidence): scoring the same
//     pair against a fold WITHOUT the +pi/2 gives 0.000000 deg, so the probe
//     can say which of the two folds it is looking at.
//
// (5) A THIRD-PARTY DEFECT THE ABOVE WALKS THROUGH.  get_nutation caches on
//     `c_JD` but never RESETS `c_longitude`/`c_obliquity` before re-summing the
//     63-term series [planetsephems/sideral_time.c:213-262] -- it writes
//     `c_longitude += ...` into the value left by the previous recompute.
//     PREDICTION P9: calling get_apparent_sidereal_time(J2000) once from a
//     fresh process and again after an intervening call at J2000+10 gives
//     DIFFERENT answers, differing by more than 0.001 deg.
#include "tools/vecmath.hpp"
extern "C" {
#include "sideral_time.h"
}
#include <cstdio>
#include <cmath>
#include <algorithm>

static const char *PRED[] = {
 "P1  shipped orbit_lat=45 lands at 58.3098861 deg (13.3098861 deg high); fixed 45.0 +- 1e-12",
 "P2  shipped LocationOrbit vs old's own authority (Mars, J2000, lon 0 lat 0): > 30 deg",
 "P3  restored form vs old's own authority: <= 1e-12 AU over the whole grid",
 "P4  Mars frozen-vs-live spin: <= 1e-6 deg in double, >= 1 deg through the shipped float",
 "P5  Earth frozen-vs-apparent spin at J2000: >= 1 deg",
 "P6  new-path observer vs old's AnchorPointBody direction: <= 1e-5 deg over 64 states",
 "P7  new-path surface_point+grounded vs that observer: 90.000000 deg, spread 0.000000",
 "P8  same pair scored against the fold WITHOUT +pi/2: 0.000000 deg (the mutation)",
 "P9  get_apparent_sidereal_time(J2000) is not repeatable across an intervening call: > 0.001 deg",
};

static const double AU = 149597870.691;
static const double J2000 = 2451545.0;

// Utility::spheToRect(double,double,Vec3d&) transcribed verbatim
// [observed: src/tools/utility.cpp:88-92].
static void spheToRect(double lng, double lat, Vec3d &v)
{
    const double cosLat = cos(lat);
    v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}
// Utility::rectToSphe [observed: src/tools/utility.cpp:102-107].
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
// signed difference of two longitudes, in (-180, 180]
static double dlon(double a, double b)
{
    double d = fmod((a - b) * 180.0 / M_PI, 360.0);
    if (d > 180) d -= 360;
    if (d <= -180) d += 360;
    return d;
}

// ---- the parent, exactly as protosystem.cpp builds it ---------------------
struct Parent {
    double periodDays, offsetDeg, epoch, precRate, obliquity, ascNode, radiusAU;
    bool earthApparent = false;
    // Body::getSiderealTime [observed: body.cpp:604-610], DEGREES
    double siderealTime(double jd) const {
        if (earthApparent)
            return get_apparent_sidereal_time(jd);
        return fmod((jd - epoch) / periodDays * 360. + offsetDeg, 360.);
    }
    // Body::compute_trans_matrix [observed: body.cpp:540-541]; the Sun's stays
    // identity, so for a planet this IS getRotEquatorialToVsop87().
    Mat4d rotEquToVsop87(double jd) const {
        return Mat4d::zrotation(ascNode - precRate * (jd - epoch))
             * Mat4d::xrotation(obliquity);
    }
    // ModularBody::computeAxisRotation [observed: ModularBody.hpp:567-573], RADIANS
    double axisRotation(double jd) const {
        if (earthApparent)
            return get_apparent_sidereal_time(jd) * (M_PI / 180);
        return fmod((jd - epoch) / periodDays * (2 * M_PI) + offsetDeg * (M_PI / 180), 2 * M_PI);
    }
};

// OLD's own authority for a surface point, verbatim composition:
// AnchorPointBody::getRotLocalToEquatorial [anchor_point_body.cpp:63-64]
// then AnchorPointBody::getRotEquatorialToVsop87 [:69-71].
static Vec3d oldAuthorityDir(const Parent &p, double jd, double lonDeg, double latDeg)
{
    const Mat4d m = p.rotEquToVsop87(jd)
                  * Mat4d::zrotation((p.siderealTime(jd) + lonDeg) * (M_PI / 180.))
                  * Mat4d::yrotation((90. - latDeg) * (M_PI / 180.));
    return m.multiplyWithoutTranslation(Vec3d(0, 0, 1));
}
// the same, stopping in the parent's EQUATORIAL frame (no rotEquToVsop87)
static Vec3d oldAuthorityEqu(const Parent &p, double jd, double lonDeg, double latDeg)
{
    const Mat4d m = Mat4d::zrotation((p.siderealTime(jd) + lonDeg) * (M_PI / 180.))
                  * Mat4d::yrotation((90. - latDeg) * (M_PI / 180.));
    return m.multiplyWithoutTranslation(Vec3d(0, 0, 1));
}

// The SHIPPED LocationOrbit, verbatim [observed: orbit.cpp:1090-1105], built
// through the shipped `float` locals [protosystem.cpp:557-558, 599-608].
struct ShippedLocationOrbit {
    double lon, lat, alt, JDToRotation;
    ShippedLocationOrbit(double _lon, double _lat, double _altKm,
                         double parentRadiusAU, float parentPeriod, float parentOffset)
        : lon((_lon + parentOffset) * M_PI / 180), lat(_lat),
          alt(_altKm / AU + parentRadiusAU), JDToRotation((2 * M_PI) / parentPeriod) {}
    Vec3d at(double JD) const {
        Vec3d tmp;
        spheToRect(lon + JD * JDToRotation, lat, tmp);
        return tmp * alt;
    }
};

int main()
{
    printf("=== F97 frame probe -- predictions in the header, every line can fail\n");
    for (const char *s : PRED) printf("  %s\n", s);
    printf("\n");

    // ---- the parents, from the FETCHED data ------------------------------
    Parent mars;
    mars.periodDays = 24.622962 / 24.;
    mars.offsetDeg  = 136.005;
    mars.epoch      = J2000;
    mars.precRate   = 0.;
    mars.obliquity  = M_PI_2 - 52.88212 * (M_PI / 180.);   // protosystem.cpp:940
    mars.ascNode    = 317.6725 * (M_PI / 180.) + M_PI_2;   // protosystem.cpp:941
    mars.radiusAU   = 3397. / AU;

    Parent earth;
    earth.periodDays = 23.9344694 / 24.;
    earth.offsetDeg  = 280.5;
    earth.epoch      = 2451545.0;
    earth.precRate   = 1.39639 * M_PI / (180 * 36525);     // protosystem.cpp:963
    earth.obliquity  = -23.4392803055555555556 * (M_PI / 180.);
    earth.ascNode    = 0.;
    earth.radiusAU   = 6378.14 / AU;
    earth.earthApparent = true;

    // ================================================================= (1)
    printf("--- (1) THE UNIT OF orbit_lat\n");
    {
        Vec3d v; spheToRect(0.0, 45.0, v);          // the shipped call: 45 RADIANS
        double lng, lat; rectToSphe(&lng, &lat, v);
        printf("  shipped  spheToRect(lon, 45.0)      -> latitude %.7f deg  (error %+.7f)\n",
               lat * 180 / M_PI, lat * 180 / M_PI - 45.0);
        Vec3d w; spheToRect(0.0, 45.0 * M_PI / 180., w);
        double lng2, lat2; rectToSphe(&lng2, &lat2, w);
        printf("  fixed    spheToRect(lon, 45.0*pi/180) -> latitude %.13f deg  (error %+.3e)\n",
               lat2 * 180 / M_PI, lat2 * 180 / M_PI - 45.0);
        printf("  |v| = %.17g (unit either way -- nothing downstream can see the error)\n", v.length());
        // the error as a function of the authored value
        printf("  authored lat ->  landed lat  (deg):");
        for (double a : {0., 10., 30., 45., 60., 90.}) {
            Vec3d t; spheToRect(0.0, a, t);
            double L, B; rectToSphe(&L, &B, t);
            printf("  %g->%.4f", a, B * 180 / M_PI);
        }
        printf("\n\n");
    }

    // ================================================================= (2)
    printf("--- (2) THE FRAME: shipped LocationOrbit vs OLD'S OWN AUTHORITY (Mars)\n");
    {
        const double jd = J2000;
        const float pOff = (float)mars.siderealTime(0.0);
        const float pPer = (float)mars.periodDays;
        double worst = 0, worstFixed = 0;
        for (double lonDeg : {0., 45., 90., 180., 270.}) {
            for (double latDeg : {0., 30., -30., 60.}) {
                ShippedLocationOrbit s(lonDeg, latDeg, 0.0, mars.radiusAU, pPer, pOff);
                Vec3d got = s.at(jd);
                Vec3d want = oldAuthorityDir(mars, jd, lonDeg, latDeg) * mars.radiusAU;
                worst = std::max(worst, angdeg(got, want));
                // the RESTORED form: parent's spin at evaluation, lat in degrees,
                // equatorial -> VSOP87 applied.
                Vec3d d; spheToRect((mars.siderealTime(jd) + lonDeg) * (M_PI / 180.),
                                    latDeg * (M_PI / 180.), d);
                Vec3d fixed = mars.rotEquToVsop87(jd).multiplyWithoutTranslation(d) * mars.radiusAU;
                worstFixed = std::max(worstFixed, (fixed - want).length());
                if (lonDeg == 0 && latDeg == 0)
                    printf("  (lon 0, lat 0): shipped is %.6f deg from old's authority\n",
                           angdeg(got, want));
            }
        }
        printf("  worst over the 20-point grid: shipped %.6f deg  |  restored %.3e AU\n\n",
               worst, worstFixed);
    }

    // ================================================================= (3)
    printf("--- (3) THE SPIN: frozen linear model vs the parent's own authority\n");
    {
        printf("  Mars: rate 360/period = %.11f deg/day (double)\n", 360. / mars.periodDays);
        for (double jd : {J2000, J2000 + 0.5, 2440000.0, 2461234.0}) {
            const double live = fmod(mars.siderealTime(jd) + 360., 360.);
            const double frozenD = fmod(mars.siderealTime(0.0) + jd * 360. / mars.periodDays, 360.);
            const float pOff = (float)mars.siderealTime(0.0);
            const float pPer = (float)mars.periodDays;
            const double frozenF = fmod((double)pOff + jd * (360. / (double)pPer), 360.);
            auto sep = [](double a, double b) { double d = fmod(a - b + 540., 360.) - 180.; return d; };
            printf("    jd %-12.4f live %9.5f | frozen(double) %9.5f (%+.6f) | frozen(float) %9.5f (%+.6f)\n",
                   jd, live, fmod(frozenD + 360., 360.), sep(frozenD, live),
                   fmod(frozenF + 360., 360.), sep(frozenF, live));
        }
        printf("  Earth: rate 8640/23.9344694 = %.11f deg/day vs mean sidereal 360.98564736629\n",
               8640. / 23.9344694);
        for (double jd : {J2000, 2440000.0, 2461234.0}) {
            const double live = fmod(earth.siderealTime(jd) + 360., 360.);
            const double frozenD = fmod(earth.siderealTime(0.0) + jd * 360. / earth.periodDays, 360.);
            auto sep = [](double a, double b) { double d = fmod(a - b + 540., 360.) - 180.; return d; };
            printf("    jd %-12.4f apparent %9.5f | frozen(double) %9.5f (%+.6f deg)\n",
                   jd, live, fmod(frozenD + 360., 360.), sep(frozenD, live));
        }
        printf("\n");
    }

    // ================================================================= (4)
    printf("--- (4) THE NEW PATH'S TWO SURFACE CONVENTIONS (float, the engine's own type)\n");
    {
        double worstObs = 0;
        double minSep = 1e9, maxSep = -1e9, minSepMut = 1e9, maxSepMut = -1e9;
        double worstLat = 0;
        int n = 0;
        for (double axisRot : {0.0, 1.3, 3.9, 5.6}) {
            for (double lonDeg : {0., 37., 121., -85.}) {
                for (double latDeg : {0., 43.3, -12.5, 67.}) {
                    const float lon = (float)(lonDeg * M_PI / 180.);
                    const float lat = (float)(latDeg * M_PI / 180.);
                    const float d = 4.2636e-05f;
                    const float ar = (float)axisRot;            // ModularBody::axisRotation
                    const float full = ar + (float)M_PI_2;      // getAxisRotation()
                    // Camera::viewMat, boundToSurface, freeMode false
                    // [observed: Camera.cpp:186-202]
                    Mat4f vm{Mat4f::identity()};
                    vm.multiplyTranslation(Vec3f(0, 0, -d));
                    vm = vm.multiplyFast(Mat4f::xrotation(lat - M_PI_2))
                           .multiplyFast(Mat4f::zrotation(-lon));
                    vm = vm.multiplyFast(Mat4f::zrotation(-full));  // computeSurfaceToBody()
                    const Mat4f inv = vm.inverse();
                    const Vec3f obs(inv.r[12], inv.r[13], inv.r[14]);  // observer in the equatorial frame
                    // old's authority, same (lon, lat), in the parent's equatorial frame
                    Parent q = mars; q.offsetDeg = axisRot * 180 / M_PI; q.epoch = J2000;
                    const Vec3d oldEqu = oldAuthorityEqu(q, J2000, lonDeg, latDeg);
                    const Vec3d obsD(obs[0], obs[1], obs[2]);
                    worstObs = std::max(worstObs, angdeg(obsD, oldEqu));
                    // surface_point + grounded: spheToRect(lon,lat) in the surface
                    // frame, folded by Z(getAxisRotation())
                    Vec3d sp; spheToRect(lonDeg * M_PI / 180., latDeg * M_PI / 180., sp);
                    const Vec3d spEqu = Mat4d::zrotation((double)full).multiplyWithoutTranslation(sp);
                    const Vec3d spEquMut = Mat4d::zrotation((double)ar).multiplyWithoutTranslation(sp);
                    double l1, b1, l2, b2, l3, b3;
                    rectToSphe(&l1, &b1, obsD);
                    rectToSphe(&l2, &b2, spEqu);
                    rectToSphe(&l3, &b3, spEquMut);
                    const double sep = dlon(l2, l1), sepMut = dlon(l3, l1);
                    minSep = std::min(minSep, sep); maxSep = std::max(maxSep, sep);
                    minSepMut = std::min(minSepMut, sepMut); maxSepMut = std::max(maxSepMut, sepMut);
                    worstLat = std::max(worstLat, fabs(b2 - b1) * 180 / M_PI);
                    ++n;
                }
            }
        }
        printf("  states scored: %d\n", n);
        printf("  P6 observer(new) vs old's AnchorPointBody direction : max %.7f deg\n", worstObs);
        printf("  P7 surface_point+grounded MINUS that observer, longitude: [%.6f, %.6f] deg (spread %.6f)\n",
               minSep, maxSep, maxSep - minSep);
        printf("  P8 the MUTATION, fold WITHOUT +pi/2                  : [%.6f, %.6f] deg\n",
               minSepMut, maxSepMut);
        printf("  latitude difference, surface_point vs observer       : max %.7f deg\n\n", worstLat);
    }

    // ================================================================= (6)
    // ADDENDUM, added 2026-09-06 AFTER the live leg refuted two of the
    // predictions above.  The header's PRED_* strings are the pre-registration
    // and are NOT rewritten; this section states what was wrong and re-runs
    // the corrected algebra.  Two errors, both mine, both found by the live
    // measurement disagreeing with the offline one:
    //   (a) section (2)'s "Mars" was NOT Mars.  protosystem.cpp:930-941 turns
    //       rot_pole_ra/de into obliquity/ascendingNode only AFTER rotating the
    //       J2000 pole into VSOP87 by `mat_j2000_to_vsop87`
    //       [observed: navigator.cpp:223-225]; section (2) applied :940-941 to
    //       the RAW J2000 pole, so it modelled a synthetic parent with the
    //       right pole DISTANCE and the wrong pole.  Its 57.750889 deg is that
    //       parent's number; the live leg measures 86.306371 deg on the real
    //       Mars, from the dump's own rotLocalToParent.
    //   (b) P4 assumed re.period is a double.  It is a FLOAT
    //       [observed: rotation_elements.hpp:33-38: period, offset, obliquity,
    //       ascendingNode, precessionRate are all float], so getSiderealTime
    //       and the frozen model read the SAME truncated period and there is no
    //       float cost to measure: on a linear-`re` parent the frozen model is
    //       algebraically identical to the live one, full stop.  Live: -7e-06
    //       deg at J2000 and -1.4e-05 deg 12 h later.  Defect (a) of S5.21 on
    //       such a parent is therefore an I2 defect (the child holds a frozen
    //       COPY of the parent's spin state, so a runtime rot_periode change no
    //       longer reaches it) and NOT a numeric one.  On EARTH it stays
    //       numeric and large -- (3) above, ~8.86 deg.
    printf("--- (6) ADDENDUM: the corrected Mars, and the float re-model\n");
    {
        // mat_j2000_to_vsop87 [observed: navigator.cpp:223-225], verbatim
        const Mat4d j2v = Mat4d::xrotation(-23.4392803055555555556 * (M_PI / 180))
                        * Mat4d::zrotation(0.0000275 * (M_PI / 180));
        Vec3d npole; spheToRect(317.6725 * M_PI / 180., 52.88212 * M_PI / 180., npole);
        const Vec3d vpole = j2v.multiplyWithoutTranslation(npole);
        double ra, de; rectToSphe(&ra, &de, vpole);
        Parent m2 = mars;
        m2.obliquity = (float)(M_PI_2 - de);      // re.obliquity is a FLOAT
        m2.ascNode   = (float)(ra + M_PI_2);      // re.ascendingNode is a FLOAT
        m2.periodDays = (float)(24.622962 / 24.); // re.period is a FLOAT
        m2.offsetDeg  = (float)136.005;           // re.offset is a FLOAT
        printf("  corrected obliquity %.7f deg, ascNode %.7f deg (was %.7f / %.7f)\n",
               m2.obliquity * 180 / M_PI, m2.ascNode * 180 / M_PI,
               mars.obliquity * 180 / M_PI, mars.ascNode * 180 / M_PI);
        const Mat4d R = m2.rotEquToVsop87(J2000);
        printf("  rotEquToVsop87 column 3 (the spin axis in VSOP87): %.9f %.9f %.9f\n",
               R.r[8], R.r[9], R.r[10]);
        printf("  live axisRot at J2000: %.7f deg\n", m2.siderealTime(J2000));
        const float pOff = (float)m2.siderealTime(0.0);
        const float pPer = (float)m2.periodDays;
        for (double lonDeg : {0., 45.}) {
            for (double latDeg : {0., 45.}) {
                ShippedLocationOrbit s(lonDeg, latDeg, 0.0, m2.radiusAU, pPer, pOff);
                const Vec3d got = s.at(J2000);
                const Vec3d want = oldAuthorityDir(m2, J2000, lonDeg, latDeg) * m2.radiusAU;
                Vec3d d; spheToRect((m2.siderealTime(J2000) + lonDeg) * (M_PI / 180.),
                                    latDeg * (M_PI / 180.), d);
                const Vec3d fixed = R.multiplyWithoutTranslation(d) * m2.radiusAU;
                printf("    (lon %5.1f, lat %5.1f): shipped %10.6f deg off | restored %.3e AU\n",
                       lonDeg, latDeg, angdeg(got, want), (fixed - want).length());
            }
        }
        // the frozen model against the live one, both through re's FLOATS
        for (double jd : {J2000, J2000 + 0.5}) {
            const double live = fmod(m2.siderealTime(jd) + 360., 360.);
            const double frozen = fmod((double)pOff + jd * (360. / (double)pPer) + 720., 360.);
            double diff = fmod(frozen - live + 540., 360.) - 180.;
            printf("    jd %.4f  live %10.6f  frozen %10.6f  diff %+.6f deg\n",
                   jd, live, frozen, diff);
        }
        // the LATITUDE half, which is what this task delivers, on this parent
        ShippedLocationOrbit pre(0., 45., 0., m2.radiusAU, pPer, pOff);
        double L, B; rectToSphe(&L, &B, pre.at(J2000));
        printf("  the delivered half: authored orbit_lat 45 emits latitude %.5f deg (pre)\n",
               B * 180 / M_PI);
        printf("                      with lat*pi/180 it emits             %.5f deg (post)\n", 45.0);
    }
    printf("\n");

    // ================================================================= (5)
    printf("--- (5) get_nutation's cache never resets its accumulators\n");
    {
        const double a = get_apparent_sidereal_time(J2000);
        const double b = get_apparent_sidereal_time(J2000 + 10.0);
        const double c = get_apparent_sidereal_time(J2000);
        printf("  first call at J2000      : %.9f deg\n", a);
        printf("  then J2000+10            : %.9f deg\n", b);
        printf("  then J2000 AGAIN         : %.9f deg   (delta %+.9f deg)\n", c, c - a);
        double d = get_apparent_sidereal_time(J2000);
        for (int i = 0; i < 20; ++i) { get_apparent_sidereal_time(J2000 + 10.0); d = get_apparent_sidereal_time(J2000); }
        printf("  after 20 more round trips: %.9f deg   (delta %+.9f deg from the first)\n", d, d - a);
        printf("  mean sidereal at J2000   : %.9f deg (the uncontaminated reference)\n",
               get_mean_sidereal_time(J2000));
    }
    return 0;
}
