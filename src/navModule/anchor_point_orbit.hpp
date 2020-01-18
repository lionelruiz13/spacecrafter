
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
//! \file anchor_point_orbit.hpp
//! \brief Anchor point for the camera
//! \author Julien LAFILLE
//! \date may 2018

/*
 * An anchor point that follows an orbit
 * Has no particular rotation 
 */



#ifndef ANCHOR_POINT_ORBIT_HPP
#define ANCHOR_POINT_ORBIT_HPP

#include <string>
#include "navModule/anchor_point.hpp"
#include "tools/utility.hpp"

class Orbit;
class TimeMgr;
class SolarSystem;

class AnchorPointOrbit : public AnchorPoint {

public :

	AnchorPointOrbit() = delete;

	AnchorPointOrbit(Orbit * orbit, const TimeMgr * timeMgr, const Body * parent, Vec3d orbitCenter = Vec3d(0,0,0)) noexcept;

	AnchorPointOrbit(const AnchorPointOrbit &) = delete;

	virtual ~AnchorPointOrbit();

	void update() noexcept;

	std::string saveAnchor()const noexcept;

private :

	Orbit * orbit = nullptr;
	const TimeMgr * timeMgr = nullptr;
	const Body * parent = nullptr;
	Vec3d orbitCenter;
};

#endif
