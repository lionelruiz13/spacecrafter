#ifndef ITERATIVE_ORBIT_HPP_
#define ITERATIVE_ORBIT_HPP_

constexpr double WARP_PRECISION = 1e-8;

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

	//! Update the position with a single iteration - Precision depends on time step
	Vec3d operator()(double dt, const Vec3d &d1, const Vec3d &d2)
	{
		H -= (e*sh-H-n*dt)/(e*ch-1);
		ch = cosh(H);
		sh = sinh(H);
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

	//! Update the position with a single iteration - Precision depends on time step
	Vec3d operator()(double dt, const Vec3d &d1, const Vec3d &d2) {
		double M = fmod(n*dt,2*M_PI);
		if (M < 0.0)
			M += 2.0*M_PI;
		H -= (M-H+e*s)/(e*c-1);
		c = cos(H);
		s = sin(H);
		return d1 * (a*(e-c)) + d2 * (h1*s);
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
