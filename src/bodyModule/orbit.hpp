// orbit.h
//
// Copyright (C) 2001 Chris Laurel <claurel@shatters.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.

#ifndef _ORBIT_H_
#define _ORBIT_H_

#include "tools/vecmath.hpp"
#include <string>
#include <memory>

// The callback type for the external position computation function
typedef void (PositionFunctionType)(double jd,double xyz[3]);
typedef void (OsculatingFunctionType)(double jd0,double jd,double xyz[3]);

class Body;

class Orbit {
public:
	Orbit(void) {}
	virtual ~Orbit(void) {}

	// Compute position for a specified Julian date and return coordinates
	// given in "dynamical equinox and ecliptic J2000"
	// which is the reference frame for VSOP87
	virtual void positionAtTimevInVSOP87Coordinates(double, double, double*) const = 0;
	void positionAtTimevInVSOP87Coordinates(double JD, double* v) const {
		positionAtTimevInVSOP87Coordinates(JD, JD, v);
	}

	// If possible, do faster (and less accurate) calculation for orbits
	virtual void fastPositionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const {
		positionAtTimevInVSOP87Coordinates(JD, JD, v);
	}

	virtual OsculatingFunctionType * getOsculatingFunction() const {
		return nullptr;
	};

	virtual double getBoundingRadius() const {
		return 0;
	}

	// Is this orbit stable (exactly the same path over time)
	virtual bool isStable(double) const {
		return true;
	}

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double) const {
		return true;
	}

	virtual std::string saveOrbit() const = 0;

private:
	Orbit(const Orbit&);
	const Orbit &operator=(const Orbit&);
};


class EllipticalOrbit : public Orbit {
public:
	EllipticalOrbit(double pericenterDistance,
	                double eccentricity,
	                double inclination,
	                double ascendingNode,
	                double argOfPeriapsis,
	                double meanAnomalyAtEpoch,
	                double period,
	                double epoch, // = 2451545.0,
	                double _parent_rot_obliquity, // = 0.0,
	                double _parent_rot_ascendingnode, // = 0.0
	                double _parent_rot_J2000_longitude,
	                bool useParentPrecession=true);

	// Compute position for a specified Julian date and return coordinates
	// given in "dynamical equinox and ecliptic J2000"
	// which is the reference frame for VSOP87
	// In order to rotate to VSOP87
	// parent_rot_obliquity and parent_rot_ascendingnode must be supplied.
	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	// Original one
	Vec3d positionAtTime(double) const;
	double getPeriod() const;
	double getBoundingRadius() const;

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double) const {
		return m_UseParentPrecession;
	}

	virtual std::string saveOrbit() const;

private:
	double eccentricAnomaly(double) const;
	Vec3d positionAtE(double) const;

	double pericenterDistance;
	double eccentricity;
	double inclination;
	double ascendingNode;
	double argOfPeriapsis;
	double meanAnomalyAtEpoch;
	double period;
	double epoch;

	// Rotation to VSOP87 coordinate data
	// \todo replace with IAU formulas for planets
	double parent_rot_obliquity;
	double parent_rot_ascendingnode;
	double parent_rot_J2000_longitude;
	double rotate_to_vsop87[9];

	bool m_UseParentPrecession;
};


class CometOrbit : public Orbit {
public:
	CometOrbit(double pericenter_distance,
	           double eccentricity,
	           double inclination,
	           double ascendingNode,
	           double arg_of_perhelion,
	           double time_at_perihelion,
	           double mean_motion,
	           double parent_rot_obliquity,
	           double parent_rot_ascendingnode,
	           double parent_rot_J2000_longitude);

	// Compute the orbit for a specified Julian date and return an "application compliant" function
	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	Vec3d positionAtTime(double) const;
	double getPeriod() const;
	double getBoundingRadius() const;
	virtual std::string saveOrbit() const;

private:
	const double q;
	const double e;
	const double i;
	const double Om;
	const double o;
	const double t0;
	const double n;
	double rotate_to_vsop87[9];
};


//! A Special Orbit uses special ephemeris algorithms
class SpecialOrbit : public Orbit {
public:
	SpecialOrbit(std::string ephemerisName);
	virtual ~SpecialOrbit(void) {};

	// Compute position for a specified Julian date and return coordinates
	// given in "dynamical equinox and ecliptic J2000"
	// which is the reference frame for VSOP87
	// In order to rotate to VSOP87
	// parent_rot_obliquity and parent_rot_ascendingnode must be supplied.
	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	virtual OsculatingFunctionType * getOsculatingFunction() const {
		return osculatingFunction;
	}

	// An ephemeris orbit is not exactly the same through time
	virtual bool isStable(double) const {
		return stable;
	}

	virtual bool isValid() {
		return (positionFunction != nullptr);
	}

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double) const {
		return m_UseParentPrecession;
	}
	virtual std::string saveOrbit() const;


private:
	PositionFunctionType *positionFunction;
	OsculatingFunctionType *osculatingFunction;
	bool stable;  // does not osculate noticeably for performance caching orbit visualization
	bool m_UseParentPrecession;
};


/*! A mixed orbit is a composite orbit, typically used when you have a
 *  custom orbit calculation that is only valid over limited span of time.
 *  When a mixed orbit is constructed, it computes elliptical orbits
 *  to approximate the behavior of the primary orbit before and after the
 *  span over which it is valid.
 */
class MixedOrbit : public Orbit {
public:
	MixedOrbit(std::unique_ptr<Orbit> orbit, double period, double t0, double t1, double mass,
	           double _parent_rot_obliquity, // = 0.0,
	           double _parent_rot_ascendingnode, // = 0.0
	           double _parent_rot_J2000_longitude,
	           bool useParentPrecession=true);

	virtual ~MixedOrbit();

	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	virtual bool isStable(double jd) const;

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double jd) const;

	virtual std::string saveOrbit() const;

private:
	std::unique_ptr<Orbit> primary;
	Orbit* afterApprox;
	Orbit* beforeApprox;
	double begin;
	double end;
	double boundingRadius;
};



/*! A binary orbit is a two body gravitational system where each orbits
 *  a common barycenter, which itself orbits a parent.  One body is
 *  designated the primary body, the other the secondary.
 *  Primary body position is calculated from secondary body position,
 *  the barycenter position, and the relative masses.
 *  Example: the Earth/Moon system.
 *  Because primary body is loaded before the secondary at startup
 *  the secondary has to be added after construction.
 *  Note that if we define r =  mass(secondary)/mass(primary)
 *  ratio = r/(1+r)
 */
class BinaryOrbit : public Orbit {
public:
	BinaryOrbit(std::unique_ptr<Orbit> barycenter, double ratio);

	virtual ~BinaryOrbit();

	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	// If possible, do faster (and less accurate) calculation for orbits
	virtual void fastPositionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	virtual bool isStable(double jd) const;

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double jd) const {
		return barycenter->useParentPrecession(jd);
	}

	virtual void setSecondaryOrbit(Orbit *second) {
		secondary = second;
	}

	virtual std::string saveOrbit() const;

private:
	std::unique_ptr<Orbit> barycenter;
	Orbit* secondary;
	double ratio;

};

/*! Une orbit still représente une orbite figée par rapport à son repère
 *  Exemple: satellite géostationnaire figé
 *  Elle est utilisée à des fin d'observation principalement
 *
 */
class stillOrbit : public Orbit {
public:
	//! création d'une orbite still au point de coordonnée cartésienne (x,y,z)
	stillOrbit(double _x, double _y, double _z);

	virtual ~stillOrbit();

	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double jd) const {
		return false;
	}

	std::string saveOrbit() const;

private:
	double x,y,z;
};

class linearOrbit : public Orbit {
public:
	linearOrbit(double _t_start, double _t_end, double *_posInitial, double *_posFinal );
	virtual ~linearOrbit();

	virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const;

	// Do the body coordinates precess with the parent?
	virtual bool useParentPrecession(double jd) const {
		return false;
	}

	std::string saveOrbit() const;


private:
	double t_start;
	double t_end;
	float t_duration;
	Vec3d posInitial;
	Vec3d posFinal;
};

class BarycenterOrbit : public Orbit {
public:
	BarycenterOrbit() = delete;
	BarycenterOrbit(Body * bodyA, Body * bodyB, double a, double b);
	virtual ~BarycenterOrbit() { }

	void positionAtTimevInVSOP87Coordinates(double, double, double*) const;

	bool useParentPrecession(double) const {
		return false;
	}

	std::string saveOrbit() const;

private:
	Body * bodyA, *bodyB;
	double a,b;
};

#endif // _ORBIT_H_
