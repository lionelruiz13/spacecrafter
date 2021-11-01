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

#include "bodyModule/orbit_2d.hpp"
#include "bodyModule/body.hpp"
#include <iostream>
#include "bodyModule/body_color.hpp"

#include "tools/context.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

Orbit2D::Orbit2D(Body* _body, int segments) : OrbitPlot(_body, segments, ORBIT_ADDITIONNAL_POINTS)
{
}

bool Orbit2D::doDraw(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	return (orbit_fader.getInterstate()
		&& body->visibilityFader.getInterstate()
		&& body->re.sidereal_period); // TODO change name to visualization_period
}

void Orbit2D::drawOrbit(VkCommandBuffer &cmd, const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	if (!doDraw(nav, prj, mat))
		return;

	Context &context = *Context::instance;
	initDraw();

	vecOrbit2dVertex = static_cast<float *>(context.transfer->planCopy(orbit->get()));
	computeShader();

	// Normal transparency mode
	pipelineOrbit2d->bind(cmd);
	layoutOrbit2d->bindSet(cmd, *context.uboSet);
	orbit->bind(cmd);
	Vec4f color (body->myColor->getOrbit(), (orbit_fader.getInterstate()*body->visibilityFader.getInterstate()));
	Mat4f matF = mat.convert();
	layoutOrbit2d->pushConstant(cmd, 0, &color);
	layoutOrbit2d->pushConstant(cmd, 1, &matF);
	vkCmdDraw(cmd, ORBIT_POINTS + ORBIT_ADDITIONNAL_POINTS, 1, 0, 0);
}

void Orbit2D::computeShader()
{
	for ( int n=0; n<ORBIT_POINTS/2-1; n++) {
		*(vecOrbit2dVertex++) = orbitPoint[n][0];
		*(vecOrbit2dVertex++) = orbitPoint[n][1];
		*(vecOrbit2dVertex++) = orbitPoint[n][2];
	}

//-------------------------------------------------------------------------
//
// gestion du point passant par le centre de la planete
//
//-------------------------------------------------------------------------
	//ICI insertion du point passant par le centre de la planete
	body->orbit_position= body->get_ecliptic_pos();
	Vec3f center(body->orbit_position[0]-body->radius/10,
				 body->orbit_position[1]-body->radius/10,
			 	 body->orbit_position[2]-0*body->radius/10);
	float coef = 1.f;
	for (int n = 1; n<(ORBIT_ADDITIONNAL_POINTS / 2 + 1); n++) {
		coef /= 1.3f;
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS/2-1][0] * coef + center[0] * (1.f - coef);
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS/2-1][1] * coef + center[1] * (1.f - coef);
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS/2-1][2] * coef + center[2] * (1.f - coef);
	}
	*(vecOrbit2dVertex++) = center[0];
	*(vecOrbit2dVertex++) = center[1];
	*(vecOrbit2dVertex++) = center[2];
	for (int n = 1; n<(ORBIT_ADDITIONNAL_POINTS / 2 + 1); n++) {
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS/2+1][0] * coef + center[0] * (1.f - coef);
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS/2+1][1] * coef + center[1] * (1.f - coef);
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS/2+1][2] * coef + center[2] * (1.f - coef);
		coef *= 1.3f;
	}
//-------------------------------------------------------------------------
	for ( int n= ORBIT_POINTS/2+1; n< ORBIT_POINTS; n++) {
		*(vecOrbit2dVertex++) = orbitPoint[n][0];
		*(vecOrbit2dVertex++) = orbitPoint[n][1];
		*(vecOrbit2dVertex++) = orbitPoint[n][2];
	}
	//fermer la boucle
	if (body->close_orbit) {
		*(vecOrbit2dVertex++) = orbitPoint[0][0];
		*(vecOrbit2dVertex++) = orbitPoint[0][1];
		*(vecOrbit2dVertex++) = orbitPoint[0][2];
	} else {
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS-1][0];
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS-1][1];
		*(vecOrbit2dVertex++) = orbitPoint[ORBIT_POINTS-1][2];
	}
}
