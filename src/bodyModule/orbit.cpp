// orbit.cpp
//
// Modifications Copyright (C) 2011, Digitalis Education Solutions, Inc. <digitaliseducation.com>
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.

#include <functional>
#include <algorithm>
#include <math.h>

#include "bodyModule/solve.hpp"
#include "bodyModule/orbit.hpp"
#include "planetsephems/stellplanet.h"
#include "tools/vecmath.hpp"

// temp
#include <iostream>
#include <sstream>
#include "tools/fmath.hpp"
#include "bodyModule/body.hpp"
#include "tools/vecmath.hpp"


#define EPSILON 1e-10 //a placer dans my_const

// N m^2 / kg^2
#define GRAVITY 6.672e-11  //a placer dans my_const

#if defined(_MSC_VER)
// cuberoot is missing in VC++ !?
#define cbrt(x) pow((x),1./3.)
#endif

// Orbital velocity is computed by differentiation for orbits that don't
// override velocityAtTime().
//~ static const double ORBITAL_VELOCITY_DIFF_DELTA = 1.0 / 1440.0;

static void InitHyp(double q,double n,double e,double dt,double &a1,double &a2)
{
	const double a = q/(e-1.0);
	const double M = n * dt;
	double H = M;
	for (;;) { // Newton
		const double Hp = H;
		H = H-(e*sinh(H)-H-M)/(e*cosh(H)-1);
		if (fabs(H - Hp) < EPSILON) break;
	}
	const double h1 = q*sqrt((e+1.0)/(e-1.0));
	a1 = a*(e-cosh(H));
	a2 = h1*sinh(H);
}

static void InitPar(double q,double n,double dt,double &a1,double &a2)
{
	const double A = n*dt;
	const double h = sqrt(A*A+1.0);
	double c = cbrt(fabs(A)+h);
	c = c*c;
	const double tan_nu_h = 2*A/(1+c+1/c);
	a1 = q*(1-tan_nu_h*tan_nu_h);
	a2 = 2.0*q*tan_nu_h;
}

static void InitEll(double q,double n,double e,double dt,double &a1,double &a2)
{
	const double a = q/(1.0-e);
	double M = fmod(n*dt,2*M_PI);
	if (M < 0.0) M += 2.0*M_PI;
	double H = M;
	for (;;) { // Newton
		const double Hp = H;
		H = H-(M-H+e*sin(H))/(e*cos(H)-1);
		if (fabs(H-Hp) < EPSILON) break;
	}
	const double h1 = q*sqrt((1.0+e)/(1.0-e));
	a1 = a*(cos(H)-e);
	a2 = h1*sin(H);
}

static void Init3D(double i,double Omega,double o,double a1,double a2, double &x1,double &x2,double &x3)
{
	const double co = cos(o);
	const double so = sin(o);
	const double cOm = cos(Omega);
	const double sOm = sin(Omega);
	const double ci = cos(i);
	const double si = sin(i);
	const double d11=-so*sOm*ci+co*cOm;
	const double d12=-co*sOm*ci-so*cOm;
	const double d21= so*cOm*ci+co*sOm;
	const double d22= co*cOm*ci-so*sOm;
	const double d31= so*si;
	const double d32= co*si;
	x1 = d11*a1+d12*a2;
	x2 = d21*a1+d22*a2;
	x3 = d31*a1+d32*a2;
}

CometOrbit::CometOrbit(double pericenter_distance,
                       double eccentricity,
                       double inclination,
                       double ascendingNode,
                       double arg_of_perhelion,
                       double time_at_perihelion,
                       double mean_motion,
                       double parent_rot_obliquity,
                       double parent_rot_ascendingnode,
                       double parent_rot_J2000_longitude)
	:q(pericenter_distance),e(eccentricity),i(inclination),
	 Om(ascendingNode),o(arg_of_perhelion),t0(time_at_perihelion),
	 n(mean_motion)
{

	const double c_obl = cos(parent_rot_obliquity);
	const double s_obl = sin(parent_rot_obliquity);
	const double c_nod = cos(parent_rot_ascendingnode);
	const double s_nod = sin(parent_rot_ascendingnode);
	const double cj = cos(parent_rot_J2000_longitude);
	const double sj = sin(parent_rot_J2000_longitude);
	rotate_to_vsop87[0] =  c_nod*cj-s_nod*c_obl*sj;
	rotate_to_vsop87[1] = -c_nod*sj-s_nod*c_obl*cj;
	rotate_to_vsop87[2] =           s_nod*s_obl;
	rotate_to_vsop87[3] =  s_nod*cj+c_nod*c_obl*sj;
	rotate_to_vsop87[4] = -s_nod*sj+c_nod*c_obl*cj;
	rotate_to_vsop87[5] =          -c_nod*s_obl;
	rotate_to_vsop87[6] =                 s_obl*sj;
	rotate_to_vsop87[7] =                 s_obl*cj;
	rotate_to_vsop87[8] =                 c_obl;

}


void CometOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	Vec3d pos = positionAtTime(JD);

	v[0] = rotate_to_vsop87[0]*pos[0] + rotate_to_vsop87[1]*pos[1] + rotate_to_vsop87[2]*pos[2];
	v[1] = rotate_to_vsop87[3]*pos[0] + rotate_to_vsop87[4]*pos[1] + rotate_to_vsop87[5]*pos[2];
	v[2] = rotate_to_vsop87[6]*pos[0] + rotate_to_vsop87[7]*pos[1] + rotate_to_vsop87[8]*pos[2];
}

Vec3d CometOrbit::positionAtTime(double JD) const
{

	JD -= t0;
	double a1,a2;

	if (e < 1.0) InitEll(q,n,e,JD,a1,a2);
	else if (e > 1.0) InitHyp(q,n,e,JD,a1,a2);
	else InitPar(q,n,JD,a1,a2);
	Vec3d p;
	Init3D(i,Om,o,a1,a2,p[0],p[1],p[2]);

	return p;

}


double CometOrbit::getPeriod() const
{
	return 0;  // Undefined
}

double CometOrbit::getBoundingRadius() const
{
	return -1;  // Undefined
}

EllipticalOrbit::EllipticalOrbit(double pericenterDistance,
                                 double eccentricity,
                                 double inclination,
                                 double ascendingNode,
                                 double argOfPeriapsis,
                                 double meanAnomalyAtEpoch,
                                 double period,
                                 double epoch,
                                 double _parent_rot_obliquity,
                                 double _parent_rot_ascendingnode,
                                 double _parent_rot_J2000_longitude,
                                 bool useParentPrecession) :
	pericenterDistance(pericenterDistance),
	eccentricity(eccentricity),
	inclination(inclination),
	ascendingNode(ascendingNode),
	argOfPeriapsis(argOfPeriapsis),
	meanAnomalyAtEpoch(meanAnomalyAtEpoch),
	period(period),
	epoch(epoch),
	parent_rot_obliquity(_parent_rot_obliquity),
	parent_rot_ascendingnode(_parent_rot_ascendingnode),
	parent_rot_J2000_longitude(_parent_rot_J2000_longitude),
	m_UseParentPrecession(useParentPrecession)
{

	const double c_obl = cos(parent_rot_obliquity);
	const double s_obl = sin(parent_rot_obliquity);
	const double c_nod = cos(parent_rot_ascendingnode);
	const double s_nod = sin(parent_rot_ascendingnode);
	const double cj = cos(parent_rot_J2000_longitude);
	const double sj = sin(parent_rot_J2000_longitude);

	rotate_to_vsop87[0] =  c_nod*cj-s_nod*c_obl*sj;
	rotate_to_vsop87[1] = -c_nod*sj-s_nod*c_obl*cj;
	rotate_to_vsop87[2] =           s_nod*s_obl;
	rotate_to_vsop87[3] =  s_nod*cj+c_nod*c_obl*sj;
	rotate_to_vsop87[4] = -s_nod*sj+c_nod*c_obl*cj;
	rotate_to_vsop87[5] =          -c_nod*s_obl;
	rotate_to_vsop87[6] =                 s_obl*sj;
	rotate_to_vsop87[7] =                 s_obl*cj;
	rotate_to_vsop87[8] =                 c_obl;

}


void EllipticalOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	Vec3d pos = positionAtTime(JD);

	v[0] = rotate_to_vsop87[0]*pos[0] + rotate_to_vsop87[1]*pos[1] + rotate_to_vsop87[2]*pos[2];
	v[1] = rotate_to_vsop87[3]*pos[0] + rotate_to_vsop87[4]*pos[1] + rotate_to_vsop87[5]*pos[2];
	v[2] = rotate_to_vsop87[6]*pos[0] + rotate_to_vsop87[7]*pos[1] + rotate_to_vsop87[8]*pos[2];
}



Vec3d EllipticalOrbit::positionAtE(double E) const
{
	double x, y;

	if (eccentricity < 1.0) {
		double a = pericenterDistance / (1.0 - eccentricity);
		x = a * (cos(E) - eccentricity);
		y = a * sqrt(1 - eccentricity * eccentricity) * sin(E);
	}
	else if (eccentricity > 1.0) {
		double a = pericenterDistance / (1.0 - eccentricity);
		x = -a * (eccentricity - cosh(E));
		y = -a * sqrt(eccentricity * eccentricity - 1) * sinh(E);
	}
	else {
		// TODO: Handle parabolic orbits
		x = 0.0;
		y = 0.0;
	}

	Mat4d R = (Mat4d::zrotation(ascendingNode) *
	           Mat4d::xrotation(inclination) *
	           Mat4d::zrotation(argOfPeriapsis));

	return R * Vec3d(x, y, 0);

}

// Standard iteration for solving Kepler's Equation
struct SolveKeplerFunc1 : public std::unary_function<double, double> {
	double ecc;
	double M;

	SolveKeplerFunc1(double _ecc, double _M) : ecc(_ecc), M(_M) {};

	double operator()(double x) const
	{
		return M + ecc * sin(x);
	}
};


// Faster converging iteration for Kepler's Equation; more efficient
// than above for orbits with eccentricities greater than 0.3.  This
// is from Jean Meeus's _Astronomical Algorithms_ (2nd ed), p. 199
struct SolveKeplerFunc2 : public std::unary_function<double, double> {
	double ecc;
	double M;

	SolveKeplerFunc2(double _ecc, double _M) : ecc(_ecc), M(_M) {};

	double operator()(double x) const
	{
		return x + (M + ecc * sin(x) - x) / (1 - ecc * cos(x));
	}
};

static double sign(double x)
{
	if (x < 0.)
		return -1.;
	else if (x > 0.)
		return 1.;
	else
		return 0.;
}

struct SolveKeplerLaguerreConway : public std::unary_function<double, double> {
	double ecc;
	double M;

	SolveKeplerLaguerreConway(double _ecc, double _M) : ecc(_ecc), M(_M) {};

	double operator()(double x) const
	{
		double s = ecc * sin(x);
		double c = ecc * cos(x);
		double f = x - s - M;
		double f1 = 1 - c;
		double f2 = s;
		x += -5 * f / (f1 + sign(f1) * sqrt(abs(16 * f1 * f1 - 20 * f * f2)));

		return x;
	}
};

struct SolveKeplerLaguerreConwayHyp : public std::unary_function<double, double> {
	double ecc;
	double M;

	SolveKeplerLaguerreConwayHyp(double _ecc, double _M) : ecc(_ecc), M(_M) {};

	double operator()(double x) const
	{
		double s = ecc * sinh(x);
		double c = ecc * cosh(x);
		double f = s - x - M;
		double f1 = c - 1;
		double f2 = s;
		x += -5 * f / (f1 + sign(f1) * sqrt(abs(16 * f1 * f1 - 20 * f * f2)));

		return x;
	}
};

typedef std::pair<double, double> Solution;


double EllipticalOrbit::eccentricAnomaly(double M) const
{
	if (eccentricity == 0.0) {
		// Circular orbit
		return M;
	}
	else if (eccentricity < 0.2) {
		// Low eccentricity, so use the standard iteration technique
		Solution sol = solveIterationFixed(SolveKeplerFunc1(eccentricity, M), M, 5);
		return sol.first;
	}
	else if (eccentricity < 0.9) {
		// Higher eccentricity elliptical orbit; use a more complex but
		// much faster converging iteration.
		Solution sol = solveIterationFixed(SolveKeplerFunc2(eccentricity, M), M, 6);
		// Debugging
		// printf("ecc: %f, error: %f mas\n",
		//        eccentricity, radToDeg(sol.second) * 3600000);
		return sol.first;
	}
	else if (eccentricity < 1.0) {
		// Extremely stable Laguerre-Conway method for solving Kepler's
		// equation.  Only use this for high-eccentricity orbits, as it
		// requires more calcuation.
		double E = M + 0.85 * eccentricity * sign(sin(M));
		Solution sol = solveIterationFixed(SolveKeplerLaguerreConway(eccentricity, M), E, 8);
		return sol.first;
	}
	else if (eccentricity == 1.0) {
		// Nearly parabolic orbit; very common for comets
		// TODO: handle this
		return M;
	}
	else {
		// Laguerre-Conway method for hyperbolic (ecc > 1) orbits.
		double E = log(2 * M / eccentricity + 1.85);
		Solution sol = solveIterationFixed(SolveKeplerLaguerreConwayHyp(eccentricity, M), E, 30);
		return sol.first;
	}
}


// Return the offset from the center
Vec3d EllipticalOrbit::positionAtTime(double t) const
{
	t = t - epoch;
	double meanMotion = 2.0 * M_PI / period;
	double meanAnomaly = meanAnomalyAtEpoch + t * meanMotion;
	double E = eccentricAnomaly(meanAnomaly);

	return positionAtE(E);
}

double EllipticalOrbit::getPeriod() const
{
	return period;
}


double EllipticalOrbit::getBoundingRadius() const
{
	// TODO: watch out for unbounded parabolic and hyperbolic orbits
	return pericenterDistance * ((1.0 + eccentricity) / (1.0 - eccentricity));
}

std::string EllipticalOrbit::saveOrbit() const
{

	std::ostringstream os;

	os << "coord_func = ell_orbit" << std::endl;
	os << "orbit_period = " << period << std::endl;
	os << "orbit_epoch = " << epoch << std::endl;
	os << "orbit_eccentricity = " << eccentricity << std::endl;
	os << "orbit_inclination = " << inclination / M_PI*180. << std::endl;
	os << "orbit_ascendingnode = " << ascendingNode / M_PI*180. << std::endl;

	double semi_major_axis = pericenterDistance / (1.0 - eccentricity) * AU;
	double mean_longitude = meanAnomalyAtEpoch + argOfPeriapsis + ascendingNode;
	double long_of_pericenter = argOfPeriapsis + ascendingNode;

	os << "orbit_longofpericenter = " << long_of_pericenter / M_PI*180. << std::endl;
	os << "orbit_meanlongitude = " << mean_longitude/ M_PI * 180 << std::endl;
	os << "orbit_semimajoraxis = " << semi_major_axis * AU << std::endl;

	os << "parent_rot_obliquity = " << parent_rot_obliquity << std::endl;
	os << "parent_rot_asc_node = " << parent_rot_ascendingnode << std::endl;
	os << "parent_rot_J2000_longitude = " << parent_rot_J2000_longitude << std::endl;

	return os.str();
}

std::string CometOrbit::saveOrbit() const
{
	/*
	ostringstream os;
	os << "coord_func = comet_orbit" << endl;
	os << "orbit_eccentricity = " << eccentricity << endl;
	os << "orbit_pericenterdistance = " << pericenter_distance <<endl;
	os << "time_at_pericenter = " << time_at_perihelion << endl;

	if (pericenter_distance <= 0.0) {
		if(eccentricity == 1){
			os << "semi_major_axis = 0" <<endl
		}
		else{
			double semi_major_axis = pericenter_distance / (1.0 - eccentricity);
			os << "semi_major_axis = " << semi_major_axis << endl;
		}
	}

	os << "mean_motion = " << mean_motion << endl;
	os << "orbit_timeatpericenter = " << orbit_timeatpericenter << endl;
	os << "



	os << "parent_rot_obliquity = " << parent_rot_obliquity << endl;
	os << "parent_rot_asc_node = " << parent_rot_ascendingnode << endl;
	os << "parent_rot_J2000_longitude = " << parent_rot_J2000_longitude <<endl;
	*/
	return "Pas encore complet";
}

std::string BinaryOrbit::saveOrbit() const
{

	return "Pas encore complet";
}

std::string MixedOrbit::saveOrbit() const
{

	return "Pas encore complet";
}

std::string SpecialOrbit::saveOrbit() const
{

	return "Pas encore complet";
}

std::string linearOrbit::saveOrbit() const
{

	return "Pas encore complet";
}

std::string stillOrbit::saveOrbit() const
{

	return "Pas encore complet";
}


// Based on code from Celestia 1.6
// Except here we use the velocity direction of the body
// but set the magnitude to match the defined orbit period

static EllipticalOrbit* StateVectorToOrbit(const Vec3d& position,  // km
        const Vec3d& v,         // km/hour
        double period,          // days
        double mass,
        double t,
        double parent_rot_obliquity,
        double parent_rot_ascendingnode,
        double parent_rot_J2000_longitude,
        bool useParentPrecession)
{

	Vec3d R = position;
	double magR = R.length();

	double G = GRAVITY * 1e-9; // convert from meters to kilometers
	double GM = G * mass;

	// Compute the semimajor axis given period
	double T = period;
	double a = pow(GM * pow(86400*T/2.0/M_PI, 2), 1.0/3.0);

	Vec3d V = v;

	double magV = sqrt(GM * (2/magR - 1/a));
	V.normalize();
	V *= magV;

	Vec3d L = R ^ V;
	double magL = L.length();
	L.normalize();
	Vec3d W = L ^ (R / magR);

	// Compute the eccentricity
	double p = (magL*magL) / GM;
	double q = R * V;

	double ex = 1.0 - magR / a;
	double ey = q / sqrt(a * GM);

	double e = sqrt(ex * ex + ey * ey);

	// Compute the mean anomaly
	double E = atan2(ey, ex);
	double M = E - e * sin(E);

	// Compute the inclination
	double cosi = L * Vec3d(0, 1.0, 0);
	double i = 0.0;
	if (cosi < 1.0)
		i = acos(cosi);

	// Compute the longitude of ascending node
	double Om = atan2(L[0], L[2]);

	// Compute the argument of pericenter
	Vec3d U = R / magR;
	double s_nu = (V * U) * sqrt(p / GM);
	double c_nu = (V * W) * sqrt(p / GM) - 1;
	s_nu /= e;
	c_nu /= e;
	Vec3d P = U * c_nu - W * s_nu;
	Vec3d Q = U * s_nu + W * c_nu;
	double om = atan2(P[1], Q[1]);

	return new EllipticalOrbit((a * (1 - e))/AU, e, i, Om, om, M, T, t,
	                           parent_rot_obliquity,
	                           parent_rot_ascendingnode,
	                           parent_rot_J2000_longitude,
	                           useParentPrecession);
}


//! A Special Orbit uses special ephemeris algorithms

SpecialOrbit::SpecialOrbit(std::string ephemerisName) :
	stable(true), m_UseParentPrecession(true)
{

	positionFunction = nullptr;
	osculatingFunction = nullptr;


	if (ephemerisName=="sun_special")
		positionFunction = &get_sun_helio_coordsv;

	if (ephemerisName=="mercury_special") {
		positionFunction = &get_mercury_helio_coordsv;
		osculatingFunction = &get_mercury_helio_osculating_coords;
	}

	if (ephemerisName=="venus_special") {
		positionFunction = &get_venus_helio_coordsv;
		osculatingFunction = &get_venus_helio_osculating_coords;
	}

	if (ephemerisName=="earth_special") {
		positionFunction = &get_earth_helio_coordsv;
		osculatingFunction = &get_earth_helio_osculating_coords;
		stable = false;
	}

	// Earth-Moon Barycenter
	if (ephemerisName=="emb_special") {
		positionFunction = &get_emb_helio_coordsv;
		osculatingFunction = &get_emb_helio_osculating_coords;
	}

	if (ephemerisName=="lunar_special") {
		positionFunction = &get_lunar_parent_coordsv;
		m_UseParentPrecession = false;
	}

	if (ephemerisName=="mars_special") {
		positionFunction = &get_mars_helio_coordsv;
		osculatingFunction = &get_mars_helio_osculating_coords;
	}

	if (ephemerisName=="phobos_special")
		positionFunction = &get_phobos_parent_coordsv;

	if (ephemerisName=="deimos_special")
		positionFunction = &get_deimos_parent_coordsv;

	if (ephemerisName=="jupiter_special") {
		positionFunction = &get_jupiter_helio_coordsv;
		osculatingFunction = &get_jupiter_helio_osculating_coords;
	}

	if (ephemerisName=="europa_special")
		positionFunction = &get_europa_parent_coordsv;

	if (ephemerisName=="calisto_special")
		positionFunction = &get_callisto_parent_coordsv;

	if (ephemerisName=="io_special")
		positionFunction = &get_io_parent_coordsv;

	if (ephemerisName=="ganymede_special")
		positionFunction = &get_ganymede_parent_coordsv;

	if (ephemerisName=="saturn_special") {
		positionFunction = &get_saturn_helio_coordsv;
		osculatingFunction = &get_saturn_helio_osculating_coords;
		stable = false;
	}

	if (ephemerisName=="mimas_special")
		positionFunction = &get_mimas_parent_coordsv;

	if (ephemerisName=="enceladus_special")
		positionFunction = &get_enceladus_parent_coordsv;

	if (ephemerisName=="tethys_special")
		positionFunction = &get_tethys_parent_coordsv;

	if (ephemerisName=="dione_special")
		positionFunction = &get_dione_parent_coordsv;

	if (ephemerisName=="rhea_special")
		positionFunction = &get_rhea_parent_coordsv;

	if (ephemerisName=="titan_special")
		positionFunction = &get_titan_parent_coordsv;

	if (ephemerisName=="iapetus_special")
		positionFunction = &get_iapetus_parent_coordsv;

	if (ephemerisName=="hyperion_special")
		positionFunction = &get_hyperion_parent_coordsv;

	if (ephemerisName=="uranus_special") {
		positionFunction = &get_uranus_helio_coordsv;
		osculatingFunction = &get_uranus_helio_osculating_coords;
		stable = false;
	}

	if (ephemerisName=="miranda_special")
		positionFunction = &get_miranda_parent_coordsv;

	if (ephemerisName=="ariel_special")
		positionFunction = &get_ariel_parent_coordsv;

	if (ephemerisName=="umbriel_special")
		positionFunction = &get_umbriel_parent_coordsv;

	if (ephemerisName=="titania_special")
		positionFunction = &get_titania_parent_coordsv;

	if (ephemerisName=="oberon_special")
		positionFunction = &get_oberon_parent_coordsv;

	if (ephemerisName=="neptune_special") {
		positionFunction = &get_neptune_helio_coordsv;
		osculatingFunction = &get_neptune_helio_osculating_coords;
		stable = false;
	}

	if (ephemerisName=="pluto_special")
		positionFunction = &get_pluto_helio_coordsv;

	// \todo better error checking

}


// Compute position for a specified Julian date and return coordinates
// given in "dynamical equinox and ecliptic J2000"
// which is the reference frame for VSOP87
// In order to rotate to VSOP87
// parent_rot_obliquity and parent_rot_ascendingnode must be supplied.
void SpecialOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double* v) const
{
	if(osculatingFunction) (*osculatingFunction)(JD0, JD, v);
	else positionFunction(JD, v);
}


MixedOrbit::MixedOrbit(Orbit* orbit, double period, double t0, double t1, double mass,
                       double _parent_rot_obliquity,
                       double _parent_rot_ascendingnode,
                       double _parent_rot_J2000_longitude,
                       bool useParentPrecession) :
	primary(orbit),
	afterApprox(nullptr),
	beforeApprox(nullptr),
	begin(t0),
	end(t1),
	boundingRadius(0.0)
{
	// \todo Remove assert!
	assert(t1 > t0);
	assert(orbit != nullptr);


	double dt = 1.0 / 1440.0; // 1 minute
	Vec3d p0, p1, q0, q1, v0, v1;
	primary->positionAtTimevInVSOP87Coordinates(t0, p0);
	primary->positionAtTimevInVSOP87Coordinates(t0 + dt, q0);
	primary->positionAtTimevInVSOP87Coordinates(t1, p1);
	primary->positionAtTimevInVSOP87Coordinates(t1 + dt, q1);

	// convert to Celestia coordinate system to work with this method
	// NS(x, y, z) -> Celestia(x, z, -y)
	double tmp;
	tmp = p0[2];
	p0[2] = -p0[1];
	p0[1] = tmp;

	tmp = p1[2];
	p1[2] = -p1[1];
	p1[1] = tmp;

	tmp = q0[2];
	q0[2] = -q0[1];
	q0[1] = tmp;

	tmp = q1[2];
	q1[2] = -q1[1];
	q1[1] = tmp;

	// and convert to km
	p0 *= AU;
	p1 *= AU;
	q0 *= AU;
	q1 *= AU;

	// velocity in km/s
	v0 = (q0 - p0) / (86400 * dt);
	v1 = (q1 - p1) / (86400 * dt);

//	cout << "IN KM:\n";
//	cout << "p0: " << p0 << "\tq0: " << q0 << "\tv0: " << v0 << endl;
//	cout << "p1: " << p1 << "\tq1: " << q1 << "\tv1: " << v1 << endl;

	beforeApprox = StateVectorToOrbit(p0, v0, period, mass, t0,
	                                  _parent_rot_obliquity,
	                                  _parent_rot_ascendingnode,
	                                  _parent_rot_J2000_longitude,
	                                  useParentPrecession);

	afterApprox = StateVectorToOrbit(p1, v1, period, mass, t1,
	                                 _parent_rot_obliquity,
	                                 _parent_rot_ascendingnode,
	                                 _parent_rot_J2000_longitude,
	                                 useParentPrecession);
}

MixedOrbit::~MixedOrbit()
{
	if (primary != nullptr)
		delete primary;
	if (beforeApprox != nullptr)
		delete beforeApprox;
	if (afterApprox != nullptr)
		delete afterApprox;
}


void MixedOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	if (JD < begin)
		beforeApprox->positionAtTimevInVSOP87Coordinates(JD0, JD, v);
	else if (JD < end)
		primary->positionAtTimevInVSOP87Coordinates(JD0, JD, v);
	else
		afterApprox->positionAtTimevInVSOP87Coordinates(JD0, JD, v);
}

bool MixedOrbit::isStable(double jd) const
{
	if (jd < begin)
		return beforeApprox->isStable(jd);
	else if (jd < end)
		return primary->isStable(jd);
	else
		return afterApprox->isStable(jd);
}

bool MixedOrbit::useParentPrecession(double jd) const
{
	if (jd < begin)
		return beforeApprox->useParentPrecession(jd);
	else if (jd < end)
		return primary->useParentPrecession(jd);
	else
		return afterApprox->useParentPrecession(jd);
}


BinaryOrbit::BinaryOrbit(Orbit* barycenter, double ratio) :
	barycenter(barycenter),
	secondary(nullptr),
	ratio(ratio)
{

}

BinaryOrbit::~BinaryOrbit()
{
	if (barycenter != nullptr)
		delete barycenter;
	// secondary is deleted by it's own planet
}

void BinaryOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	Vec3d posSecondary(0.0, 0.0, 0.0);

	barycenter->positionAtTimevInVSOP87Coordinates(JD0, JD, v);

	if(secondary) secondary->positionAtTimevInVSOP87Coordinates(JD0, JD, posSecondary);

	v[0] -= ratio * posSecondary[0];
	v[1] -= ratio * posSecondary[1];
	v[2] -= ratio * posSecondary[2];
}

void BinaryOrbit::fastPositionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	barycenter->positionAtTimevInVSOP87Coordinates(JD0, JD, v);
}


bool BinaryOrbit::isStable(double jd) const
{
	return barycenter->isStable(jd);
}



stillOrbit::stillOrbit(double _x, double _y, double _z)
{
	x= _x;
	y= _y;
	z= _z;
}

stillOrbit::~stillOrbit()
{
}

void stillOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}



linearOrbit::linearOrbit(double _t_start, double _t_end, double *_posInitial, double *_posFinal )
{
	t_start = _t_start;
	t_end = _t_end;
	posInitial[0]= _posInitial[0];
	posInitial[1]= _posInitial[1];
	posInitial[2]= _posInitial[2];
	posFinal[0]=_posFinal[0];
	posFinal[1]=_posFinal[1];
	posFinal[2]=_posFinal[2];
	t_duration = t_end - t_start;
}

linearOrbit::~linearOrbit()
{}

void linearOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const
{
	if (JD0< t_start) {
		v[0]=posInitial[0];
		v[1]=posInitial[1];
		v[2]=posInitial[2];
		return;
	}

	if (JD0>t_end) {
		v[0]=posFinal[0];
		v[1]=posFinal[1];
		v[2]=posFinal[2];
		return;
	}

	v[0]=(JD0-t_start)/t_duration *posInitial[0] + (1-(JD0-t_start)/t_duration)* posFinal[0];
	v[1]=(JD0-t_start)/t_duration *posInitial[1] + (1-(JD0-t_start)/t_duration)* posFinal[1];
	v[2]=(JD0-t_start)/t_duration *posInitial[2] + (1-(JD0-t_start)/t_duration)* posFinal[2];
}

BarycenterOrbit::BarycenterOrbit(Body * _bodyA, Body * _bodyB, double _a, double _b)
{
	bodyA = _bodyA;
	bodyB = _bodyB;
	a = _a;
	b = _b;
}

void BarycenterOrbit::positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v)const
{

	Vec3d posA = bodyA->get_heliocentric_ecliptic_pos();
	Vec3d posB = bodyB->get_heliocentric_ecliptic_pos();

	Vec3d posBary = (posB - posA) * b/(a+b);

	Mat4d J2000toVsop87(
	    Mat4d::xrotation(-23.4392803055555555556*(M_PI/180)) *
	    Mat4d::zrotation(0.0000275*(M_PI/180)));

	posBary = J2000toVsop87 * posBary;

	v[0] = posBary[0];
	v[1] = posBary[1];
	v[2] = posBary[2];

}

std::string BarycenterOrbit::saveOrbit() const
{

	std::ostringstream os;

	os << "coord_func = barycenter" << std::endl;
	os << "body_A = " << bodyA->getEnglishName() << std::endl;
	os << "body_B = " << bodyB->getEnglishName() << std::endl;
	os << "a = " << a << std::endl;
	os << "b = " << b << std::endl;

	return os.str();
}
