#ifndef ITERATIVE_ORBIT_HPP_
#define ITERATIVE_ORBIT_HPP_

constexpr double WARP_PRECISION = 1e-8;

//! How many Newton steps each ITERATIVE position solver advances per call.
//!
//! [vixy 2026-09-07, S11.223(b), verbatim: "keeping the same iteration count
//! but doubling each iteraton per cycle on the newton path (or the one Eris
//! involves)"], resolving row S5.145.
//!
//! Why the count exists at all: these solvers advance from the PREVIOUS call's
//! seed instead of converging inside the call, so a body gets exactly as many
//! steps as it gets calls.  A body the update walk stopped evaluating gets
//! only the calls the D8 use-site barrier buys it -- 1 + RESUME_EXTRA_ITERATIONS
//! refreshes, ModularBody.cpp:512-514, which this constant does NOT touch.  At
//! one step per call that is five steps, measured SHORT for the corpus's
//! slowest converger: Eris answered 1.198725 deg off the old path's own
//! position, where nine steps reach 1.1e-05 deg (S11.220(j1), one date, one
//! binary, the evaluation count the only variable).  At two it is ten.
//!
//! Why HERE and not at the call site (I2): this header is the one home both
//! solver families include -- EllipticalOrbit::eccentricAnomaly reaches it
//! through orbit.hpp, IterativeEll/IterativeHyp are declared in it -- and the
//! choice of count is the owner's, not any caller's.
//!
//! Cost: one extra sin/cos pair and one division per iterating body per
//! evaluation.  Both render paths share this solver, so the OLD path converges
//! twice as fast per frame after a date jump as well -- strictly more exact on
//! the comparison baseline, an as-if change under the ruling above.
constexpr int ITERATIVE_STEPS_PER_CALL = 2;

class IterativeHyp
{
public:
	IterativeHyp(double q,double n,double e) :
		n(n), e(e), a(q/(e-1.0)), h1(q*sqrt((e+1.0)/(e-1.0)))
	{
	}

	//! Warp at a given time, with low precision
	void quickwarp(double dt)
	{
		H = n*dt;
		ch = cosh(H);
		sh = sinh(H);
	}

	//! Warp at a given time, with high precision
	void warp(double dt)
	{
		const double M = n*dt;
		H = M;
		{
			double tmp;
			do {
				tmp = (e*sinh(H)-H-M)/(e*cosh(H)-1);
				H -= tmp;
			} while (fabs(tmp) >= WARP_PRECISION);
		}
		ch = cosh(H);
		sh = sinh(H);
	}

	//! Update the position with ITERATIVE_STEPS_PER_CALL iterations - Precision
	//! depends on time step and on how stale the seed (H, ch, sh) is
	Vec3d operator()(double dt, const Vec3d &d1, const Vec3d &d2)
	{
		for (int i = 0; i < ITERATIVE_STEPS_PER_CALL; ++i) {
			H -= (e*sh-H-n*dt)/(e*ch-1);
			ch = cosh(H);
			sh = sinh(H);
		}
		return d1 * (a*(e-ch)) + d2 * (h1*sh);
	}
private:
	const double n;
	const double e;
	const double a;
	const double h1;
	double H = 0;
	double ch = 1;
	double sh = 0;
};

class IterativeEll
{
public:
	IterativeEll(double q,double n,double e) :
		n(n), e(e), a(q/(1.0-e)), h1(q*sqrt((1.0+e)/(1.0-e)))
	{
	}

	//! Warp at a given time, with low precision
	void quickwarp(double dt)
	{
		double H = fmod(n*dt,2*M_PI);
		if (H < 0.0)
			H += 2.0*M_PI;
		c = cos(H);
		s = sin(H);
	}

	//! Warp at a given time, with high precision
	void warp(double dt)
	{
		double M = fmod(n*dt,2*M_PI);
		if (M < 0.0)
			M += 2.0*M_PI;
		H = M;
		{
			double tmp;
			do {
				tmp = (M-H+e*sin(H))/(e*cos(H)-1);
				H -= tmp;
			} while (fabs(tmp) >= WARP_PRECISION);
		}
		c = cos(H);
		s = sin(H);
	}

	//! Update the position with ITERATIVE_STEPS_PER_CALL iterations - Precision
	//! depends on time step and on how stale the seed (H, c, s) is
	Vec3d operator()(double dt, const Vec3d &d1, const Vec3d &d2) {
		double M = fmod(n*dt,2*M_PI);
		if (M < 0.0)
			M += 2.0*M_PI;
		for (int i = 0; i < ITERATIVE_STEPS_PER_CALL; ++i) {
			H -= (M-H+e*s)/(e*c-1);
			c = cos(H);
			s = sin(H);
		}
		return d1 * (a*(c-e)) + d2 * (h1*s);
	}
private:
	const double n;
	const double e;
	const double a;
	const double h1;
	double H = 0;
	double c = 1;
	double s = 0;
};

#endif /* end of include guard: ITERATIVE_ORBIT_HPP_ */
