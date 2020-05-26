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

#include <sstream>

#include "bodyModule/orbit.hpp"
#include "bodyModule/solarsystem.hpp"
#include "coreModule/time_mgr.hpp"
#include "navModule/anchor_point_orbit.hpp"
#include "tools/vecmath.hpp"
#include "tools/fmath.hpp"
#include "tools/log.hpp"


AnchorPointOrbit::AnchorPointOrbit(Orbit * _orbit, const TimeMgr* _timeMgr, const Body * _parent, Vec3d _orbitCenter) noexcept
{
	orbit = _orbit;
	timeMgr = _timeMgr;
	parent = _parent;
	orbitCenter = _orbitCenter;
}

AnchorPointOrbit::~AnchorPointOrbit()
{
	delete orbit;
}

void AnchorPointOrbit::update() noexcept
{

	double * v = new double[3];
	orbit->positionAtTimevInVSOP87Coordinates(timeMgr->getJDay(), v);
	Vec3d pos(v[0], v[1], v[2]);
	delete[] v;

	Mat4d rotVsop87toJ2000 = (
	                             Mat4d::xrotation(-23.4392803055555555556*(C_PI/180)) *
	                             Mat4d::zrotation(0.0000275*(C_PI/180))
	                         ).transpose();

	pos = rotVsop87toJ2000 * pos;

	if(parent != nullptr) {
		pos = pos + parent->get_heliocentric_ecliptic_pos();
	}
	else {
		pos = pos + orbitCenter;
	}

	setHeliocentricEclipticPos(pos);

}

std::string AnchorPointOrbit::saveAnchor()const noexcept
{

	std::ostringstream os;
	os << "type orbit" << std::endl;
	os << orbit->saveOrbit();

	if(parent != nullptr)
		os << "parent = " << parent->getEnglishName() << std::endl;
	else {
		os << "orbit_center_x = " << orbitCenter[0] << std::endl;
		os << "orbit_center_y = " << orbitCenter[1] << std::endl;
		os << "orbit_center_z = " << orbitCenter[2] << std::endl;
	}

	return os.str();
}
