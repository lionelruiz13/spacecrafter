// F104 - THE SOLVER, MEASURED OUTSIDE THE ENGINE (INTENT 5.145 / 11.223(b)).
//
// Two questions the engine cannot answer cheaply, both about
// EllipticalOrbit::eccentricAnomaly and IterativeEll/IterativeHyp::operator():
//
//   COST   how many nanoseconds is ONE Newton step?  The frame cadence is
//          saturated at the config cap (11.159(k7)), so fps cannot resolve it;
//          the D11 denominator needs the step's own time.
//   REACH  from a STALE seed, how many one-step calls does each eccentricity in
//          the field file need to converge, and what does taking two steps per
//          call do to that count?  That is the whole of 5.145, per body, with
//          no launch.
//
// The solver is NOT copied here.  `f104_solver.py` slices
// EllipticalOrbit::eccentricAnomaly out of src/bodyModule/orbit.cpp at build
// time into `f104_ecc.inc`, records the slice's md5, and re-slices on every
// run -- so this harness measures the tree it was built from and says which
// one (the f102 shape, taken one step further: extracted, not mirrored).
// IterativeEll/IterativeHyp are included FOR REAL from the tree's header.
//
// Build (f104_solver.py does this):
//   g++ -O2 -std=c++20 -I<src> -I<gendir> -o f104_solver f104_solver.cpp
//
// Modes
//   --steps                       how many steps the built-in slice takes per
//                                 call, measured (1 pre-fix, 2 post-fix)
//   --time E N                    ns per eccentricAnomaly call at eccentricity E
//   --time-ell E N                ns per IterativeEll::operator() call
//   --time-hyp E N                ns per IterativeHyp::operator() call
//   --converge E CALLS [SEEDOFF]  |E_k - E_exact| after each of CALLS calls,
//                                 from a seed left SEEDOFF radians away
//   --corpus FILE CALLS           one line per `name<TAB>e<TAB>branch` row
#include <functional>
#include <algorithm>
#include <math.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>

#include "tools/vecmath.hpp"
#include "bodyModule/iterative_orbits.hpp"

// The slice: `static double sign(...)` and the whole body of
// EllipticalOrbit::eccentricAnomaly, re-extracted from orbit.cpp at build time
// and re-hashed on every run.
#include "f104_ecc.inc"

typedef std::chrono::steady_clock clk;

// The reference answer: Kepler's equation solved to double convergence by a
// method that is NOT the one under test (bisection-guarded Newton run to a
// fixed point), so a shared bug cannot cancel.
static double kepler_exact(double e, double M)
{
    double lo = M - 1.0 - e, hi = M + 1.0 + e;
    double x = M;
    for (int i = 0; i < 400; ++i) {
        const double f = x - e * sin(x) - M;
        if (f > 0) hi = x; else lo = x;
        const double d = 1.0 - e * cos(x);
        double nx = (d != 0.0) ? x - f / d : 0.5 * (lo + hi);
        if (!(nx > lo && nx < hi))
            nx = 0.5 * (lo + hi);
        if (nx == x)
            break;
        x = nx;
    }
    return x;
}

// How many advancing steps does the built-in slice take per call?  Measured,
// not assumed: run the e < 0.2 fixed-point branch one call from a seed and
// compare against one and two hand-applied steps of the same map.
static int measured_steps_per_call()
{
    const double e = 0.1, M = 1.0;
    double lastE = 2.0;                     // non-zero: no reseeding
    f104_eccentricAnomaly(e, M, lastE);
    double one = M + e * sin(2.0);
    double two = M + e * sin(one);
    double three = M + e * sin(two);
    if (lastE == one) return 1;
    if (lastE == two) return 2;
    if (lastE == three) return 3;
    return -1;
}

static double timed_ecc(double e, long n, double *sink)
{
    double lastE = kepler_exact(e, 1.0);
    const auto t0 = clk::now();
    double acc = 0;
    for (long i = 0; i < n; ++i)
        acc += f104_eccentricAnomaly(e, 1.0 + 1e-12 * i, lastE);
    const auto t1 = clk::now();
    *sink = acc + lastE;
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}

static double timed_ell(double e, long n, double *sink)
{
    IterativeEll it(1.0, 1.0, e);
    Vec3d d1(1, 0, 0), d2(0, 1, 0);
    it.warp(1.0);
    const auto t0 = clk::now();
    double acc = 0;
    for (long i = 0; i < n; ++i)
        acc += it(1.0 + 1e-12 * i, d1, d2)[0];
    const auto t1 = clk::now();
    *sink = acc;
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}

static double timed_hyp(double e, long n, double *sink)
{
    IterativeHyp it(1.0, 1.0, e);
    Vec3d d1(1, 0, 0), d2(0, 1, 0);
    it.warp(1.0);
    const auto t0 = clk::now();
    double acc = 0;
    for (long i = 0; i < n; ++i)
        acc += it(1.0 + 1e-12 * i, d1, d2)[0];
    const auto t1 = clk::now();
    *sink = acc;
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / n;
}

// A STALE SEED, modelled the way 5.145 describes it: the body was last
// evaluated at another date, so `lastE` holds the eccentric anomaly of THAT
// date and is not zero -- the `if (lastE == 0)` reseeding never fires.  The
// staleness is expressed as an offset in eccentric anomaly.
static void converge(double e, double M, int calls, double seedoff,
                     std::vector<double> &err, std::vector<double> *val = nullptr)
{
    const double exact = kepler_exact(e, M);
    double lastE = exact + seedoff;
    if (lastE == 0.0)
        lastE = 1e-300;                     // never trip the reseeding branch
    err.clear();
    if (val) val->clear();
    for (int i = 0; i < calls; ++i) {
        f104_eccentricAnomaly(e, M, lastE);
        err.push_back(fabs(lastE - exact));
        if (val) val->push_back(lastE);
    }
}

// The same, for IterativeEll: its state is (H, c, s) and its "stale seed" is a
// warp at another time.  n = 1, so dt IS the mean anomaly.
static void converge_ell(double e, double M, int calls, double seedoff,
                         std::vector<double> &err)
{
    const double exact = kepler_exact(e, M);
    IterativeEll it(1.0, 1.0, e);
    it.warp(M + seedoff);                   // converged at the STALE date
    Vec3d d1(1, 0, 0), d2(0, 1, 0);
    err.clear();
    for (int i = 0; i < calls; ++i) {
        const Vec3d p = it(M, d1, d2);
        // recover H from the returned point: x = a*(c-e) with a = q/(1-e) and
        // y = h1*s with h1 = q*sqrt((1+e)/(1-e)), q = 1 (iterative_orbits.hpp:60)
        const double c = p[0] * (1.0 - e) + e;
        const double s = p[1] / sqrt((1.0 + e) / (1.0 - e));
        err.push_back(fabs(atan2(s, c) - exact));
    }
}

static int usage()
{
    fprintf(stderr, "usage: f104_solver --steps | --time E N | --time-ell E N |"
                    " --time-hyp E N | --converge E CALLS [SEEDOFF] |"
                    " --corpus FILE CALLS\n");
    return 2;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return usage();
    const std::string mode = argv[1];
    printf("# f104_solver  slice=%s  steps_per_call=%d\n",
           F104_ECC_MD5, measured_steps_per_call());
    if (mode == "--steps")
        return 0;
    if (mode == "--time" || mode == "--time-ell" || mode == "--time-hyp") {
        if (argc < 4) return usage();
        const double e = atof(argv[2]);
        const long n = atol(argv[3]);
        double sink = 0;
        // one warm-up pass, then three timed passes: the median is reported and
        // the spread is printed, so a noisy host is visible rather than hidden
        double v[4];
        for (int i = 0; i < 4; ++i)
            v[i] = (mode == "--time") ? timed_ecc(e, n, &sink)
                 : (mode == "--time-ell") ? timed_ell(e, n, &sink)
                                          : timed_hyp(e, n, &sink);
        std::sort(v + 1, v + 4);
        printf("%s e=%g n=%ld ns_per_call=%.4f  passes=%.4f,%.4f,%.4f  sink=%.17g\n",
               mode.c_str(), e, n, v[2], v[1], v[2], v[3], sink);
        return 0;
    }
    if (mode == "--converge") {
        if (argc < 4) return usage();
        const double e = atof(argv[2]);
        const int calls = atoi(argv[3]);
        const double off = (argc > 4) ? atof(argv[4]) : M_PI;
        std::vector<double> err, val;
        converge(e, 1.0, calls, off, err, &val);
        // The VALUE is printed, not only the error: the whole structural claim
        // of this fix is that doubling the step does not change the iterate
        // SEQUENCE, only which element a call returns -- so the post binary's
        // value after k calls must be bit-identical to the pre binary's after
        // 2k calls.  That is checkable only if the bits are on the page.
        for (size_t i = 0; i < err.size(); ++i)
            printf("call %2zu lastE %.17g err %.17g\n", i + 1, val[i], err[i]);
        return 0;
    }
    if (mode == "--sweep") {
        // For eccentricity E, how does the residual after k calls depend on how
        // STALE the seed was?  5.145's magnitude is a function of that, and the
        // staleness of a parked body's seed is not observable from outside.
        if (argc < 3) return usage();
        const double e = atof(argv[2]);
        const int calls = (argc > 3) ? atoi(argv[3]) : 20;
        printf("seedoff\t");
        for (int k = 1; k <= calls; ++k)
            printf("err%d%s", k, k == calls ? "\n" : "\t");
        for (double off = 0.25; off <= 64.0; off *= 2.0) {
            for (int sgn = 1; sgn >= -1; sgn -= 2) {
                std::vector<double> err;
                converge(e, 1.0, calls, sgn * off, err);
                printf("%+g\t", sgn * off);
                for (int k = 0; k < calls; ++k)
                    printf("%.6g%s", err[k], k == calls - 1 ? "\n" : "\t");
            }
        }
        return 0;
    }
    if (mode == "--corpus") {
        if (argc < 4) return usage();
        FILE *f = fopen(argv[2], "r");
        if (!f) { perror(argv[2]); return 3; }
        const int calls = atoi(argv[3]);
        char line[512];
        printf("name\te\tbranch\tcalls_to_1e-9\terr_after_5_calls\t"
               "err_after_%d_calls\n", calls);
        while (fgets(line, sizeof line, f)) {
            char name[128] = {0}, branch[64] = {0};
            double e = 0;
            if (sscanf(line, "%127[^\t]\t%lf\t%63[^\t\n]", name, &e, branch) != 3)
                continue;
            std::vector<double> err;
            const bool comet = (strncmp(branch, "comet", 5) == 0);
            // the WORST stale seed the map can be given: half a revolution of
            // eccentric anomaly away from the answer
            if (comet)
                converge_ell(e, 1.0, calls, M_PI, err);
            else
                converge(e, 1.0, calls, M_PI, err);
            int k = -1;
            for (size_t i = 0; i < err.size(); ++i)
                if (err[i] < 1e-9) { k = (int)i + 1; break; }
            printf("%s\t%.17g\t%s\t%d\t%.6g\t%.6g\n", name, e, branch, k,
                   err.size() > 4 ? err[4] : NAN, err.back());
        }
        fclose(f);
        return 0;
    }
    return usage();
}
