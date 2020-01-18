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
 * This anchor point is attached to a body.
 * It follows the body's trajectory
 * The rotation is followed when close enough to the body
 * It is also possible to ignore the rotation with overrideRotationCondition. 
 * This parameter is set by the script : camera action follow_rotation [true/false]
 */



#ifndef ANCHOR_POINT_BODY_HPP
#define ANCHOR_POINT_BODY_HPP

#include "navModule/anchor_point.hpp"

class Body;

class AnchorPointBody : public AnchorPoint {

public:

	static AnchorPointBody * getAnchorFromParams(stringHash_t params);

	AnchorPointBody() = delete;

	AnchorPointBody(const Body * body)noexcept;

	~AnchorPointBody() { }

	Mat4d getRotLocalToEquatorial(double jd, double lat, double lon, double alt) const noexcept override;

	Mat4d getRotEquatorialToVsop87() const noexcept override;

	void update() noexcept override;

	bool isOnBody(const Body * body)const noexcept override;

	bool isOnBody() const noexcept;

	const Body* getBody() const noexcept override {
		return body;
	}

	std::string saveAnchor()const noexcept;

private:
	const Body * body;
};

#endif
