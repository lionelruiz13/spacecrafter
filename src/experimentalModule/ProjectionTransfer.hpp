#ifndef PROJECTION_TRANSFER_HPP_
#define PROJECTION_TRANSFER_HPP_

#include <cmath>

// Radial projection transfer of each mode, on the CPU; must equal shaders/src/custom_project.glsl (spec-const 8)
// r = NDC radius, thn = theta/halfFov, theta = angle from the -z view axis in radians
//   FISHEYE r = thn | ALLSPHERE r = poly(thn*1200)/1200 | EKISOLID = FISHEYE | ASPHERIC r = tan(theta/2)/tan(halfFov/2)
// The mode (Context::projectionType) is constant after launch
namespace ProjectionTransfer {

// projector.hpp ProjectionType values (== Context::projectionType).
enum : int { FISHEYE = 0, ALLSPHERE = 1, EKISOLID = 2, ASPHERIC = 3 };

//! Allsphere distortion polynomial and its derivative: x = thn*1200, output = r*1200. Same as custom_project.glsl
inline double allspherePoly(double x) {
	return (((((((((-1.553958085e-26*x + 1.430207232e-22)*x -4.958391394e-19)*x + 8.938737084e-16)*x -9.39081162e-13)*x + 5.979121144e-10)*x -2.293161246e-7)*x + 4.995598119e-5)*x -5.508786926e-3)*x + 1.665135788)*x + 6.526610628e-2;
}
inline double allspherePolyDeriv(double x) {
	return ((((((((-1.553958085e-26*10*x + 1.430207232e-22*9)*x -4.958391394e-19*8)*x + 8.938737084e-16*7)*x -9.39081162e-13*6)*x + 5.979121144e-10*5)*x -2.293161246e-7*4)*x + 4.995598119e-5*3)*x -5.508786926e-3*2)*x + 1.665135788;
}

//! NDC radius of a point at normalized center angle thn (forward transfer).
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

//! dr/dthn at thn = 0: the small-angle slope, for use around the view center
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

//! Inverse transfer: normalized angle whose NDC radius is r
inline double angleNorm(int mode, double r, double halfFov) {
	switch (mode) {
		case ALLSPHERE: {
			double f = r; // initial guess (near-linear transfer)
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

//! Normalized angle of the screen-disc edge (r == 1): the visibility-cone bound, never below the visible edge
inline float edgeAngleNorm(int mode, float halfFov) {
	return (mode == ALLSPHERE)
		? static_cast<float>(angleNorm(ALLSPHERE, 1., halfFov)) : 1.f;
}

} // namespace ProjectionTransfer

#endif /* end of include guard: PROJECTION_TRANSFER_HPP_ */
