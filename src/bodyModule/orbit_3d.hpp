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
//! \file orbit_3d.hpp
//! \brief Draws a body's orbit
//! \author Julien LAFILLE
//! \date april 2018

#ifndef ORBIT3D_HPP
#define ORBIT3D_HPP

#include "bodyModule/orbit_plot.hpp"
#include "tools/fader.hpp"


#include <vector>

class Body;
class Projector;
class Navigator;

class Uniform;

class Orbit3D : public OrbitPlot {
public:

	Orbit3D()=delete;
	Orbit3D(const Orbit3D&) = delete;
	Orbit3D(Body* body, int segments = 180);

	void drawOrbit(const Navigator * nav, const Projector* prj, const Mat4d &mat);

	void computeShader();

private:

	void setPrjMat(const Projector* prj, const Mat4d &mat);

	std::vector<float> orbitSegments;
	const Projector * prj;
	Mat4d mat;

	int commandIndex;
	std::unique_ptr<Uniform> uModelViewMatrix, uColor, uclipping_fov;
	std::unique_ptr<Set> set;
	Mat4f *pModelViewMatrix;
	Vec4f *pColor;
	Vec3f *pclipping_fov;
};

#endif
