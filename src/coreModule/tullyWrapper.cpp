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
* (c) 2017 - 2020 all rights reserved
*
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <thread>

#include "coreModule/TullyWrapper.hpp"

std::string TullyWrapper::getInfoString(const Navigator *nav) const
{
	const Vec3d j2000_pos = getObsJ2000Pos(nav);
	double dec_j2000, ra_j2000;
	Utility::rectToSphe(&ra_j2000,&dec_j2000,j2000_pos);
	const Vec3d equatorial_pos = nav->j2000ToEarthEqu(j2000_pos);
	double dec_equ, ra_equ;
	Utility::rectToSphe(&ra_equ,&dec_equ,equatorial_pos);
	std::stringstream oss;
	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(2);

	oss << "Type: ";
	switch (typeGalaxy) {
		case 9:
			oss << "Globular Cluster";
			break;
		default:
			oss << "Galaxy";
	}
	oss << "J2000" << " " << "RA/DE: " << Utility::printAngleHMS(ra_j2000,true) << " / " << Utility::printAngleDMS(dec_j2000,true) << std::endl;
	oss << "Equ of date" << " " << "RA/DE: " << Utility::printAngleHMS(ra_equ) << " / " << Utility::printAngleDMS(dec_equ) << std::endl;

	// calculate alt az
	double az,alt;
	Utility::rectToSphe(&az,&alt,nav->earthEquToLocal(equatorial_pos));
	az = 3*M_PI - az;  // N is zero, E is 90 degrees
	if(az > M_PI*2) az -= M_PI*2;
	oss << "Alt/Az: " << Utility::printAngleDMS(alt) << " / " << Utility::printAngleDMS(az) << std::endl;

	return oss.str();
}

std::string TullyWrapper::getShortInfoString(const Navigator *nav) const
{
	std::stringstream oss;
	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(2);
	oss << "Type: ";
	switch (typeGalaxy) {
		case 9:
			oss << "Globular Cluster";
			break;
		default:
			oss << "Galaxy";
	}
	oss << " - Name: " << name;
	return oss.str();
}

float TullyWrapper::getMag(const Navigator *nav) const
{
	return 0;
}
