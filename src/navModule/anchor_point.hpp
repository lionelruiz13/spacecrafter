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
//! \file anchor_point.hpp
//! \brief Anchor point for the camera
//! \author Julien LAFILLE
//! \date may 2018

/*
 * This class is representing a point in space to which the Camera will be attached
 * The camera has an altitude/longitude/latitude relative to this point.
 * 
 * Any anchor point needs to be able to give three informations to the camera :
 * 	- its position in the J2000 coordinate system
 * 	- its rotation relative to the J2000 and Vsop87 coordinate system
 * 	- if it is attached to a body (and if so which body)
 * 
 * The position is a 3d vector (xyz). The position is given in astronomical units
 * The rotation is a 4x4 rotation matrix. It is mostly used to follow a planet's rotation
 * 
 * It is up to you to feed anything you want into the position and rotation 
 * 
 * These two parameters are updated by the update method.
 * This method is called by the anchor_manager on the current anchor point
 * 
 */



#ifndef ANCHOR_POINT_HPP
#define ANCHOR_POINT_HPP

#include <memory>

#include "tools/vecmath.hpp"
#include "tools/utility.hpp"

class Body;
class AnchorPoint {

public:

	AnchorPoint() noexcept;

	AnchorPoint(double x, double y, double z) noexcept;

	AnchorPoint(const Vec3d& pos)noexcept;

	AnchorPoint(const AnchorPoint &) = delete;

	virtual ~AnchorPoint() { }

	const Vec3d& getHeliocentricEclipticPos() const noexcept {
		return heliocentric_ecliptic_pos;
	}

	void setHeliocentricEclipticPos(const Vec3d& pos) noexcept {
		heliocentric_ecliptic_pos = pos;
	}

	virtual Mat4d getRotLocalToEquatorial(double jd, double lat, double lon, double altitude) const noexcept;

	void setRotLocalToEquatorial(Mat4d mat) {
		rotLocalToEquatorial = mat;
	}

	virtual Mat4d getRotEquatorialToVsop87() const noexcept;

	void setRotEquatorialToVsop87(Mat4d mat) {
		rotEquatorialToVsop87 = mat;
	}

	virtual void update() noexcept;

	virtual bool isOnBody(Body *body) const noexcept;

	virtual bool isOnBody() const noexcept;

	virtual std::shared_ptr<Body> getBody() const noexcept;

	virtual std::string saveAnchor()const noexcept;

private:

	Vec3d heliocentric_ecliptic_pos;
	Mat4d rotLocalToEquatorial = Mat4d::identity();
	Mat4d rotEquatorialToVsop87 = Mat4d::identity();
};

#endif
