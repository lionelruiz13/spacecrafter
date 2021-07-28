/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/

#include "bodyModule/orbit_creator_cor.hpp"
#include "bodyModule/solarsystem.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/log.hpp"
#include <iostream>
#include "navModule/navigator.hpp"
//#include "tools/fmath.hpp"
#include "tools/sc_const.hpp"

OrbitCreatorEliptic::OrbitCreatorEliptic(const OrbitCreator * _next, const SolarSystem * _ssystem) :
	OrbitCreator(_next)
{
	ssystem = _ssystem;
}

std::shared_ptr<Orbit> OrbitCreatorEliptic::handle(stringHash_t params) const
{

	if(params["coord_func"] != "ell_orbit") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("OrbitCreatorEliptic::handle unknown type");
			return nullptr;
		}
	}

	Body * parent = ssystem->searchByEnglishName(params["parent"]);

	double parent_rot_obliquity = 0.0;
	double parent_rot_asc_node = 0.0;
	double parent_rot_J2000_longitude = 0.0;

	if(!parent) {
		parent_rot_obliquity = Utility::strToDouble(params["parent_rot_obliquity"], 0.0);
		parent_rot_asc_node = Utility::strToDouble(params["parent_rot_asc_node"], 0.0);
		parent_rot_J2000_longitude = Utility::strToDouble(params["parent_rot_J2000_longitude"], 0.0);
	}
	else {

		parent_rot_obliquity = parent && parent->get_parent()
		                       ? parent->getRotObliquity() : 0.0;
		parent_rot_asc_node = parent && parent->get_parent()
		                      ? parent->getRotAscendingnode() : 0.0;

		if (parent && parent->get_parent()) {
			const double c_obl = cos(parent_rot_obliquity);
			const double s_obl = sin(parent_rot_obliquity);
			const double c_nod = cos(parent_rot_asc_node);
			const double s_nod = sin(parent_rot_asc_node);
			const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
			const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
			const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
			const Vec3d J2000Pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
			Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
			J2000NodeOrigin.normalize();
			parent_rot_J2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
		}
	}

	double period = Utility::strToDouble(params["orbit_period"]);
	double epoch = Utility::strToDouble(params["orbit_epoch"],J2000);
	double semi_major_axis = Utility::strToDouble(params["orbit_semimajoraxis"])/AU;
	double eccentricity = Utility::strToDouble(params["orbit_eccentricity"]);
	double inclination = Utility::strToDouble(params["orbit_inclination"])*M_PI/180.;
	double ascending_node = Utility::strToDouble(params["orbit_ascendingnode"])*M_PI/180.;
	double long_of_pericenter = Utility::strToDouble(params["orbit_longofpericenter"])*M_PI/180.;
	double mean_longitude = Utility::strToDouble(params["orbit_meanlongitude"])*M_PI/180.;

	double arg_of_pericenter = long_of_pericenter - ascending_node;
	double anomaly_at_epoch = mean_longitude - (arg_of_pericenter + ascending_node);
	double pericenter_distance = semi_major_axis * (1.0 - eccentricity);

	// Create an elliptical orbit
	return std::make_shared<EllipticalOrbit>(
	           pericenter_distance, eccentricity,
	           inclination, ascending_node,
	           arg_of_pericenter, anomaly_at_epoch,
	           period, epoch,
	           parent_rot_obliquity, parent_rot_asc_node,
	           parent_rot_J2000_longitude);

}

OrbitCreatorComet::OrbitCreatorComet(const OrbitCreator * _next, const SolarSystem * _ssystem) :
	OrbitCreator(_next)
{
	ssystem = _ssystem;
}

std::shared_ptr<Orbit> OrbitCreatorComet::handle(stringHash_t params) const
{

	if(params["coord_func"] != "comet_orbit") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("OrbitCreatorComet::handle unknown type : " + params["coord_func"]);
			return nullptr;
		}
	}

	Body * parent = ssystem->searchByEnglishName(params["parent"]);

	double parent_rot_obliquity = 0.0;
	double parent_rot_asc_node = 0.0;
	double parent_rot_J2000_longitude = 0.0;

	if(!parent) {
		parent_rot_obliquity = Utility::strToDouble(params["parent_rot_obliquity"]);
		parent_rot_asc_node = Utility::strToDouble(params["parent_rot_asc_node"]);
		parent_rot_J2000_longitude = Utility::strToDouble(params["parent_rot_J2000_longitude"]);
	}
	else {
		parent_rot_obliquity = parent && parent->get_parent()
		                       ? parent->getRotObliquity() : 0.0;
		parent_rot_asc_node = parent && parent->get_parent()
		                      ? parent->getRotAscendingnode() : 0.0;

		if (parent && parent->get_parent()) {
			const double c_obl = cos(parent_rot_obliquity);
			const double s_obl = sin(parent_rot_obliquity);
			const double c_nod = cos(parent_rot_asc_node);
			const double s_nod = sin(parent_rot_asc_node);
			const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
			const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
			const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
			const Vec3d J2000Pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
			Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
			J2000NodeOrigin.normalize();
			parent_rot_J2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
		}
	}


	// Read the orbital elements
	const double eccentricity = Utility::strToDouble(params["orbit_eccentricity"],0.0);

	double pericenter_distance = Utility::strToDouble(params["orbit_pericenterdistance"],-1e100);

	double semi_major_axis;

	if (pericenter_distance <= 0.0) {
		semi_major_axis = Utility::strToDouble(params["orbit_semimajoraxis"],-1e100);

		if (semi_major_axis <= -1e100) {
			cLog::get()->write("OrbitCreatorComet::handle you must provide orbit_pericenterdistance or orbit_semimajoraxis");
			return nullptr;
		}
		else {
			if (eccentricity == 1.0) {
				cLog::get()->write("OrbitCreatorComet::handle parabolic orbits have no semi_major_axis");
				return nullptr;
			}
			pericenter_distance = semi_major_axis * (1.0-eccentricity);
		}
	}
	else {
		semi_major_axis = (eccentricity == 1.0)
		                  ? 0.0 // parabolic orbits have no semi_major_axis
		                  : pericenter_distance / (1.0-eccentricity);
	}
	double mean_motion = Utility::strToDouble(params["orbit_meanmotion"],-1e100);
	double period;
	if (mean_motion <= -1e100) {
		period = Utility::strToDouble(params["orbit_period"],-1e100);
		if (period <= -1e100) {
			if (parent->get_parent()) {
				cLog::get()->write("OrbitCreatorComet::handle When the parent body is not the Sun\nyou must provide orbit_MeanMotion or orbit_Period");
				return nullptr;
			}
			mean_motion = (eccentricity == 1.0)
			              ? 0.01720209895 * (1.5/pericenter_distance)
			              * sqrt(0.5/pericenter_distance)
			              : (semi_major_axis > 0.0)
			              ? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
			              : 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));
		}
		else {
			mean_motion = 2.0*M_PI/period;
		}
	}
	else {
		mean_motion *= (M_PI/180.0);
	}

	double time_at_pericenter = Utility::strToDouble(params["orbit_timeatpericenter"],-1e100);

	if (time_at_pericenter <= -1e100) {
		const double epoch = Utility::strToDouble(params["orbit_epoch"],-1e100);
		double mean_anomaly = Utility::strToDouble(params["orbit_meananomaly"],-1e100);
		if (epoch <= -1e100 || mean_anomaly <= -1e100) {
			cLog::get()->write("OrbitCreatorComet::handle when you do not provide orbit_TimeAtPericenter, you must provide both orbit_Epoch and orbit_MeanAnomaly");
			return nullptr;
		}
		else {
			mean_anomaly *= (M_PI/180.0);
			time_at_pericenter = epoch - mean_anomaly / mean_motion;
		}
	}

	const double inclination = Utility::strToDouble(params["orbit_inclination"])*(M_PI/180.0);
	const double ascending_node = Utility::strToDouble(params["orbit_ascendingnode"])*(M_PI/180.0);
	const double arg_of_pericenter = Utility::strToDouble(params["orbit_argofpericenter"])*(M_PI/180.0);

	return std::make_shared<CometOrbit>(
	           pericenter_distance, eccentricity,
	           inclination, ascending_node,
	           arg_of_pericenter, time_at_pericenter,
	           mean_motion, parent_rot_obliquity,
	           parent_rot_asc_node, parent_rot_J2000_longitude);

}

OrbitCreatorSpecial::OrbitCreatorSpecial(const OrbitCreator * next) :
	OrbitCreator(next) { }

std::shared_ptr<Orbit> OrbitCreatorSpecial::handle(stringHash_t params) const
{

	std::shared_ptr<SpecialOrbit> sorb = std::make_shared<SpecialOrbit>(params["coord_func"]);

	if(!sorb->isValid()) {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("OrbitCreatorComet::handle unknown type");
			return nullptr;
		}
	}

	return sorb;
}

OrbitCreatorBary::OrbitCreatorBary(const OrbitCreator * _next, SolarSystem * _ssystem):
	OrbitCreator(_next)
{
	ssystem = _ssystem;
}

std::shared_ptr<Orbit> OrbitCreatorBary::handle(stringHash_t params) const
{

	if(params["coord_func"] != "barycenter") {
		if(next != nullptr)
			return next->handle(params);
		else {
			cLog::get()->write("OrbitCreatorBary::handle unknown type");
			return nullptr;
		}
	}

	if(params["a"].empty() || params["b"].empty()) {
		cLog::get()->write("OrbitCreatorBary::handle missing barycenter coefficients");
		return nullptr;
	}

	if(params["body_A"].empty() || params["body_B"].empty()) {
		cLog::get()->write("OrbitCreatorBary::handle missing parents");
		return nullptr;
	}

	Body * bodyA = ssystem->searchByEnglishName(params["body_A"]);
	Body * bodyB = ssystem->searchByEnglishName(params["body_B"]);

	if(bodyA == nullptr || bodyB == nullptr) {
		cLog::get()->write("OrbitCreatorBary::couldn't find one of the bodies");
		return nullptr;
	}

	return std::make_shared<BarycenterOrbit>(bodyA, bodyB, stod(params["a"]), stod(params["b"]));
}
