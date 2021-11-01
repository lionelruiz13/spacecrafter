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

Orbit3D::Orbit3D(Body* _body, int segments) : OrbitPlot(_body, segments, 1)
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

	vkCmdDraw(cmd, ORBIT_POINTS + body->close_orbit, 1, 0, 0);
}

void Orbit3D::computeShader()
{
	// only draw moon orbits as zoom in
	Vec3d onscreen;

	int i = 0;

	while(i < ORBIT_POINTS) {

		*(orbitSegments++) = orbitPoint[i][0];
		*(orbitSegments++) = orbitPoint[i][1];
		*(orbitSegments++) = orbitPoint[i][2];
		i++;
	}

	if (body->close_orbit) {
		*(orbitSegments++) = orbitPoint[0][0];
		*(orbitSegments++) = orbitPoint[0][1];
		*(orbitSegments++) = orbitPoint[0][2];
	}
}
