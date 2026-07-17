#ifndef PROJECTION_TRANSFER_HPP_
#define PROJECTION_TRANSFER_HPP_

#include <cmath>

// Radial projection transfer - the new path's single CPU authority for the
// per-mode screen mapping (INTENT 11.33). EXACT mirror of the GPU dispatch
// (shaders/src/custom_project.glsl, spec-const 8) and of the old-path CPU
// forms (projector.cpp per-mode projectCustom + invertAllspherePolynomial):
// CPU screenPos and GPU discs must land on the SAME transfer, or every
// screenPos consumer (halo, hint, pointer, label anchor - the 11.19 parity
// layer) drifts off its body under non-fisheye modes.
//
// All four modes are radially symmetric around the view axis: NDC radius r
// as a function of the normalized center angle thn = theta/halfFov (theta =
// angle from the -z view axis, radians; GPU clipping_fov.z == halfFov):
//   FISHEYE   r = thn                                  [custom_project.glsl:9]
//   ALLSPHERE r = poly(thn*1200)/1200                  [custom_project.glsl:25]
//   EKISOLID  = FISHEYE (mainline TODO, aliased there) [custom_project.glsl:48]
//   ASPHERIC  r = tan(theta/2)/tan(halfFov/2)          [custom_project.glsl:55]
// Mode source: Context::projectionType (config video/projection_type,
// parsed once at app init - LAUNCH-CONSTANT, the same precondition the old
// path's 52 per-pipeline spec-constant sites rely on; a runtime change
// would need the registry's rebuild+publish-swap path, out of scope).
namespace ProjectionTransfer {

// projector.hpp ProjectionType values (== Context::projectionType).
enum : int { FISHEYE = 0, ALLSPHERE = 1, EKISOLID = 2, ASPHERIC = 3 };

// Allsphere distortion polynomial + derivative, coefficients VERBATIM from
// custom_project.glsl / projector.cpp:275/390 (one authority per language;
// x = thn*1200, output = r*1200).
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

//! dr/dthn at thn=0: the small-angle limit slope for the center-singularity
//! guard (ModularBody::update screenPos). Note ALLSPHERE also carries a
//! constant term c0/1200 = 5.44e-5 NDC (~0.06 px at 2048) which the guard
//! branch drops - the GPU keeps it; sub-0.1px inside the guard radius only.
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

//! Inverse transfer: normalized angle whose radius is r (atmosphere grid
//! directions). ALLSPHERE inverts by Newton-Raphson - projector.cpp:373
//! invertAllspherePolynomial mirrored (10 iterations, 1e-10 tolerance).
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

//! Normalized angle of the screen-disc edge (r == 1): the visibility-cone
//! bound. Exactly 1 for FISHEYE/EKISOLID (identity transfer) and ASPHERIC
//! (its transfer reaches 1 at halfFov by construction); 0.96735 for
//! ALLSPHERE (the polynomial overshoots the edge: r(1) = 1.0225 - INTENT
//! 11.33 derivation). Never BELOW the visible edge for any mode, which is
//! what keeps the angular cone tests safe to tighten with this factor.
inline float edgeAngleNorm(int mode, float halfFov) {
	return (mode == ALLSPHERE)
		? static_cast<float>(angleNorm(ALLSPHERE, 1., halfFov)) : 1.f;
}

} // namespace ProjectionTransfer

#endif /* end of include guard: PROJECTION_TRANSFER_HPP_ */
