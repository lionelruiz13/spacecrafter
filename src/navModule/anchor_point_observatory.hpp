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

/**
 * 
 * Similar to anchor point body except you are attached to a fix point in space
 * 
 **/



#ifndef ANCHOR_POINT_OBSERVATORY_HPP
#define ANCHOR_POINT_OBSERVATORY_HPP

#include <string>
#include "navModule/anchor_point.hpp"
#include "tools/vecmath.hpp"


class AnchorPointObservatory : public AnchorPoint{

public:

    AnchorPointObservatory() noexcept;

    AnchorPointObservatory(double x, double y, double z);

    AnchorPointObservatory(const Vec3d& v);

    AnchorPointObservatory(const AnchorPointObservatory&) = delete;

    virtual ~AnchorPointObservatory();

	Mat4d getRotLocalToEquatorial(double jd, double lat, double lon, double altitude) const noexcept;

	std::string saveAnchor()const noexcept;

};

#endif
