#ifndef PROJECTION_TRANSFER_HPP_
#define PROJECTION_TRANSFER_HPP_

#include <cmath>

// Radial projection transfer on the CPU, must equal shaders/src/custom_project.glsl
// r = NDC radius, thn = theta/halfFov, theta = angle from the view axis in radians
namespace ProjectionTransfer {

// Same values as ProjectionType in projector.hpp
enum : int { FISHEYE = 0, ALLSPHERE = 1, EKISOLID = 2, ASPHERIC = 3 };

// With x = thn*1200, return r*1200
inline double allspherePoly(double x) {
	return (((((((((-1.553958085e-26*x + 1.430207232e-22)*x -4.958391394e-19)*x + 8.938737084e-16)*x -9.39081162e-13)*x + 5.979121144e-10)*x -2.293161246e-7)*x + 4.995598119e-5)*x -5.508786926e-3)*x + 1.665135788)*x + 6.526610628e-2;
}
inline double allspherePolyDeriv(double x) {
	return ((((((((-1.553958085e-26*10*x + 1.430207232e-22*9)*x -4.958391394e-19*8)*x + 8.938737084e-16*7)*x -9.39081162e-13*6)*x + 5.979121144e-10*5)*x -2.293161246e-7*4)*x + 4.995598119e-5*3)*x -5.508786926e-3*2)*x + 1.665135788;
}

inline float radius(int mode, float thn, float halfFov) {
	switch (mode) {
		case ALLSPHERE:
			return static_cast<float>(allspherePoly(thn * 1200.) / 1200.);
		case ASPHERIC:
			return static_cast<float>(std::tan(thn * halfFov * 0.5) / std::tan(halfFov * 0.5));
		default: // FISHEYE / EKISOLID
			return thn;
	}
}

// Return dr/dthn at thn = 0
inline float slope0(int mode, float halfFov) {
	switch (mode) {
		case ALLSPHERE:
			return 1.665135788f; // allspherePolyDeriv(0)
		case ASPHERIC:
			return static_cast<float>((halfFov * 0.5) / std::tan(halfFov * 0.5));
		default:
			return 1.f;
	}
}

// Return the normalized angle whose NDC radius is r
inline double angleNorm(int mode, double r, double halfFov) {
	switch (mode) {
		case ALLSPHERE: {
			double f = r;
			for (int i = 0; i < 10; ++i) {
				const double err = allspherePoly(f * 1200.) / 1200. - r;
				if (std::fabs(err) < 1e-10)
					break;
				f -= err / allspherePolyDeriv(f * 1200.);
			}
			return f;
		}
		case ASPHERIC:
			return 2. * std::atan(r * std::tan(halfFov * 0.5)) / halfFov;
		default:
			return r;
	}
}

// Return the normalized angle of the screen disc edge (r == 1)
inline float edgeAngleNorm(int mode, float halfFov) {
	return (mode == ALLSPHERE)
		? static_cast<float>(angleNorm(ALLSPHERE, 1., halfFov)) : 1.f;
}

} // namespace ProjectionTransfer

#endif /* end of include guard: PROJECTION_TRANSFER_HPP_ */
