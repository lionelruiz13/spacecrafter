/* Schwarzschild null trajectories, in units of the Schwarzschild radius.
 * See E. Bruneton, Real-time High-Quality Rendering of Non-Rotating Black Holes
 * (2020), for precomputed trajectory/intersection rendering.
 */
#ifndef BLACKHOLE_TRAJECTORY_LUT_HPP
#define BLACKHOLE_TRAJECTORY_LUT_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace blackhole {
constexpr double pi = 3.14159265358979323846;
constexpr double criticalImpact = 2.598076211353316;
// The common scene's visual radius is not a Schwarzschild radius. Calibrate
// the disk trajectories so their critical image approaches that same contour.
constexpr double diskRadiusCalibration = 1.0 / criticalImpact;
constexpr double minImpact = 0.0001;
constexpr double coreImpact = 128.0;
constexpr double impactScale = 0.002;
constexpr double maxPhi = 3.0 * pi;
constexpr int coreWidth = 1024;
constexpr int tailWidth = 257;
constexpr int lutWidth = coreWidth + tailWidth;
constexpr int lutHeight = 1024;
constexpr int lutChannels = 4;

// Signed logarithmic spacing resolves both sides of the capture boundary.
inline double impactAt(double x)
{
    const double column = x * (lutWidth - 1);
    if (column >= coreWidth) {
        const double q = 1.0 - (column - coreWidth) / (tailWidth - 1);
        return q > 0.0 ? coreImpact / q : std::numeric_limits<double>::infinity();
    }
    x = column / (coreWidth - 1);
    if (x < 0.5) {
        const double lo = std::log(minImpact / (criticalImpact - minImpact));
        const double hi = std::log((criticalImpact - 1e-6) / 1e-6);
        return criticalImpact / (1.0 + std::exp(-(lo + x * 2.0 * (hi - lo))));
    }
    const double q = x * 2.0 - 1.0;
    const double span = q < 0.0 ? criticalImpact - minImpact : coreImpact - criticalImpact;
    return criticalImpact + std::copysign(impactScale * std::expm1(std::abs(q) * std::log1p(span / impactScale)), q);
}

inline double impactCoordinate(double b)
{
    if (b > coreImpact)
        return (coreWidth + (tailWidth - 1) * (1.0 - coreImpact / b)) / (lutWidth - 1);
    constexpr double coreScale = double(coreWidth - 1) / (lutWidth - 1);
    if (b < criticalImpact) {
        const double lo = std::log(minImpact / (criticalImpact - minImpact));
        const double hi = std::log((criticalImpact - 1e-6) / 1e-6);
        return coreScale * 0.5 * (std::log(b / (criticalImpact - b)) - lo) / (hi - lo);
    }
    const double d = b - criticalImpact;
    const double span = d < 0.0 ? criticalImpact - minImpact : coreImpact - criticalImpact;
    return coreScale * (0.5 + 0.5 * std::copysign(std::log1p(std::abs(d) / impactScale) / std::log1p(span / impactScale), d));
}

// v = b/r remains finite at infinite impact; v'' = -v + 3v^2/(2b).
inline double scaledApsis(double inverseImpact)
{
    double v = 1.0;
    for (int i = 0; i < 8; ++i)
        v = 1.0 / std::sqrt(1.0 - inverseImpact * v);
    return v;
}

struct Orbit {
    double u;
    double du;
};

inline double apsisInverseRadius(double b, bool inner)
{
    if (!inner && b > coreImpact)
        return scaledApsis(1.0 / b) / b;
    const double angle = std::acos(std::clamp(1.0 - 2.0 * criticalImpact * criticalImpact / (b * b), -1.0, 1.0));
    return (1.0 + 2.0 * std::cos((angle + (inner ? 0.0 : 4.0 * pi)) / 3.0)) / 3.0;
}

inline double inversePhaseCoordinate(double b, double u, bool inner)
{
    if (b < criticalImpact)
        return inner ? 3.0 * (1.0 - u) : u;
    const double apsis = apsisInverseRadius(b, inner);
    const double remaining = inner ? (u - apsis) / (1.0 - apsis) : 1.0 - u / apsis;
    return 1.0 - std::sqrt(std::clamp(remaining, 0.0, 1.0));
}

inline Orbit advance(Orbit s, double h, double gravity = 1.0)
{
    const auto acceleration = [gravity](double u) { return -u + 1.5 * gravity * u * u; };
    const double a = acceleration(s.u);
    const double b = acceleration(s.u + h * s.du * 0.5);
    const double c = acceleration(s.u + h * (s.du + h * a * 0.5) * 0.5);
    const double d = acceleration(s.u + h * (s.du + h * b * 0.5));
    return {s.u + h * (s.du + h * (a + b + c) / 6.0),
            s.du + h * (a + 2.0 * b + 2.0 * c + d) / 6.0};
}

// R: inverse radius u(phi,b); the tail stores b*u, including b=infinity.
// G: inbound phi(u,b); for turning rays the last row is exactly the apsis.
// B/A: radius and inverse phase for the inner branch starting at the horizon.
// This branch covers observers below the photon sphere with b > criticalImpact.
// No integration is performed by the rendering shader.
inline std::vector<float> makeTrajectoryLut()
{
    std::vector<float> data(lutWidth * lutHeight * lutChannels);
    constexpr int substeps = 4;
    for (int x = 0; x < lutWidth; ++x) {
      const double b = impactAt(double(x) / (lutWidth - 1));
      for (int branch = 0; branch < 2; ++branch) {
        const int channel = branch * 2;
        const bool scaled = x >= coreWidth && branch == 0;
        const double gravity = scaled ? 1.0 / b : 1.0;
        const double direction = branch ? -1.0 : 1.0;
        Orbit state{branch ? 1.0 : 0.0, scaled ? 1.0 : direction / b};
        const bool turning = b >= criticalImpact;
        const double apsis = scaled ? scaledApsis(gravity) : (turning ? apsisInverseRadius(b, branch != 0) : 0.0);
        bool finished = false;
        bool inbound = true;
        int inverseRow = 1;
        double lastInboundPhi = 0.0;
        data[lutChannels * x + channel + 1] = 0.f;
        for (int y = 0; y < lutHeight; ++y) {
            const double rowPhi = maxPhi * std::pow(double(y) / (lutHeight - 1), 2);
            const double nextPhi = maxPhi * std::pow(double(y + 1) / (lutHeight - 1), 2);
            const double h = (nextPhi - rowPhi) / substeps;
            data[lutChannels * (y * lutWidth + x) + channel] = static_cast<float>(state.u);
            for (int step = 0; step < substeps && y + 1 < lutHeight && !finished; ++step) {
                const double phi = rowPhi + step * h;
                const Orbit next = advance(state, h, gravity);
                if (inbound) {
                    while (inverseRow < lutHeight) {
                        const double tRadius = double(inverseRow) / (lutHeight - 1);
                        // Concentrate samples at the turning point, where the
                        // inverse phase has an infinite radial derivative.
                        const double quadratic = tRadius * (2.0 - tRadius);
                        const double target = turning
                            ? (branch ? 1.0 - (1.0 - apsis) * quadratic : apsis * quadratic)
                            : (branch ? 1.0 - tRadius / 3.0 : tRadius);
                        if ((target - next.u) * direction > 0.0 || (next.u - state.u) * direction <= 0.0)
                            break;
                        const double t = (target - state.u) / (next.u - state.u);
                        data[lutChannels * (inverseRow++ * lutWidth + x) + channel + 1] = static_cast<float>(phi + t * h);
                    }
                    lastInboundPhi = phi + h;
                    if (next.du * direction <= 0.0) {
                        lastInboundPhi = phi + h * state.du / (state.du - next.du);
                        inbound = false;
                    }
                }
                state = next;
                if ((!scaled && state.u >= 1.0) || state.u <= 0.0) {
                    state.u = state.u <= 0.0 ? 0.0 : 1.0;
                    finished = true;
                }
            }
        }
        // Unreachable radii saturate at the apsis, never interpolate a sentinel.
        for (; inverseRow < lutHeight; ++inverseRow)
            data[lutChannels * (inverseRow * lutWidth + x) + channel + 1] = static_cast<float>(lastInboundPhi);
      }
    }
    return data;
}
}
#endif
