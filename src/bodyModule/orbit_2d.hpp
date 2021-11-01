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
//! \file orbit_2d.hpp
//! \brief Draws a body's orbit
//! \author Julien LAFILLE
//! \date april 2018

#ifndef ORBIT2D_HPP
#define ORBIT2D_HPP

#define ORBIT_ADDITIONNAL_POINTS 80

#include "bodyModule/orbit_plot.hpp"
#include "tools/fader.hpp"

#include <vector>
#include <vulkan/vulkan.h>

class Body;
class Projector;
class Navigator;

class Orbit2D : public OrbitPlot {
public:

	Orbit2D()=delete;
	Orbit2D(const Orbit2D&) = delete;
	Orbit2D(Body* body, int segments = 320);

	virtual void drawOrbit(VkCommandBuffer &cmd, const Navigator * nav, const Projector* prj, const Mat4d &mat) override;
	virtual bool doDraw(const Navigator * nav, const Projector* prj, const Mat4d &mat) override;
	void computeShader();

private:
	float *vecOrbit2dVertex;
};

#endif
