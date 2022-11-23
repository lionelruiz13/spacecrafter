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

#include "bodyModule/orbit_3d.hpp"
#include "bodyModule/body.hpp"
#include <iostream>
#include "coreModule/projector.hpp"
#include "bodyModule/body_color.hpp"

#include "tools/context.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

Orbit3D::Orbit3D(Body* _body, int segments) : OrbitPlot(_body, segments, ORBIT_ADDITIONNAL_POINTS)
{
}

bool Orbit3D::doDraw(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	return (orbit_fader.getInterstate()
		&& body->visibilityFader.getInterstate()
		&& body->re.sidereal_period); // TODO change name to visualization_period
}

void Orbit3D::drawOrbit(VkCommandBuffer &cmd, const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	if (!doDraw(nav, prj, mat))
		return;

	Context &context = *Context::instance;
	initDraw();
	Vec4f color (body->myColor->getOrbit(), (orbit_fader.getInterstate()*body->visibilityFader.getInterstate()));
	struct {
		Mat4f mat;
		Vec3f clipping_fov;
	} pushGeom {mat.convert(), prj->getClippingFov()};

	pipelineOrbit3d->bind(cmd);
	orbit->bind(cmd);
	layoutOrbit3d->pushConstant(cmd, 0, &color);
	layoutOrbit3d->pushConstant(cmd, 1, &pushGeom);

	orbitSegments = static_cast<float *>(context.transfer->planCopy(orbit->get()));
	computeShader();

	vkCmdDraw(cmd, ORBIT_POINTS + ORBIT_ADDITIONNAL_POINTS, 1, 0, 0);
}

void Orbit3D::computeShader()
{
	for ( int n=0; n<ORBIT_POINTS/2-1; n++) {
		*(orbitSegments++) = orbitPoint[n][0];
		*(orbitSegments++) = orbitPoint[n][1];
		*(orbitSegments++) = orbitPoint[n][2];
	}

	//-------------------------------------------------------------------------
	//
	// management of the point passing through the center of the planet
	//
	//-------------------------------------------------------------------------
	//HERE insertion of the point passing through the center of the planet
	body->orbit_position= body->get_ecliptic_pos();
	Vec3f center(body->orbit_position[0]-body->radius/10,
				 body->orbit_position[1]-body->radius/10,
			 	 body->orbit_position[2]-0*body->radius/10);
	float coef = 1.f;
	for (int n = 1; n<(ORBIT_ADDITIONNAL_POINTS / 2 + 1); n++) {
		coef /= 1.3f;
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS/2-1][0] * coef + center[0] * (1.f - coef);
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS/2-1][1] * coef + center[1] * (1.f - coef);
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS/2-1][2] * coef + center[2] * (1.f - coef);
	}
	*(orbitSegments++) = center[0];
	*(orbitSegments++) = center[1];
	*(orbitSegments++) = center[2];
	for (int n = 1; n<(ORBIT_ADDITIONNAL_POINTS / 2 + 1); n++) {
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS/2+1][0] * coef + center[0] * (1.f - coef);
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS/2+1][1] * coef + center[1] * (1.f - coef);
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS/2+1][2] * coef + center[2] * (1.f - coef);
		coef *= 1.3f;
	}
//-------------------------------------------------------------------------
	for ( int n= ORBIT_POINTS/2+1; n< ORBIT_POINTS; n++) {
		*(orbitSegments++) = orbitPoint[n][0];
		*(orbitSegments++) = orbitPoint[n][1];
		*(orbitSegments++) = orbitPoint[n][2];
	}
	//fermer la boucle
	if (body->close_orbit) {
		*(orbitSegments++) = orbitPoint[0][0];
		*(orbitSegments++) = orbitPoint[0][1];
		*(orbitSegments++) = orbitPoint[0][2];
	} else {
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS-1][0];
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS-1][1];
		*(orbitSegments++) = orbitPoint[ORBIT_POINTS-1][2];
	}}
