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
#include "vulkanModule/VertexArray.hpp"

#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/CommandMgr.hpp"

Orbit3D::Orbit3D(Body* _body, int segments) : OrbitPlot(_body, segments, 1)
{
	set = std::make_unique<Set>(context->surface, context->setMgr, layoutOrbit3d);
	uModelViewMatrix = std::make_unique<Uniform>(context->surface, sizeof(Mat4f));
	pModelViewMatrix = static_cast<typeof(pModelViewMatrix)>(uModelViewMatrix->data);
	set->bindUniform(uModelViewMatrix.get(), 0);
	uColor = std::make_unique<Uniform>(context->surface, sizeof(Vec3f));
	pColor = static_cast<typeof(pColor)>(uColor->data);
	set->bindUniform(uColor.get(), 1);
	uclipping_fov = std::make_unique<Uniform>(context->surface, sizeof(Vec3f));
	pclipping_fov = static_cast<typeof(pclipping_fov)>(uclipping_fov->data);
	set->bindUniform(uclipping_fov.get(), 2);

	commandIndex = cmdMgr->getCommandIndex();
	cmdMgr->init(commandIndex, pipelineOrbit3d);
	cmdMgr->bindSet(layoutOrbit3d, set.get());
	cmdMgr->bindVertex(orbit.get());
	cmdMgr->draw(segments + 1);
	cmdMgr->compile();
}

void Orbit3D::drawOrbit(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	if (!orbit_fader.getInterstate())
		return;
	if (!body->visibilityFader.getInterstate())
		return;

	if (!body->re.sidereal_period)
		return; // TODO change name to visualization_period

	// StateGL::enable(GL_DEPTH_TEST);
	//
	// // Normal transparency mode
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// StateGL::enable(GL_BLEND);
	// StateGL::disable(GL_CULL_FACE);
	*pColor = Vec4f(body->myColor->getOrbit(), (orbit_fader.getInterstate()*body->visibilityFader.getInterstate()));

	// shaderOrbit3d->use();
	// shaderOrbit3d->setUniform("Color", Color);

	//paramétrage des matrices pour opengl4
	// Mat4f proj = prj->getMatProjection().convert();
	*pModelViewMatrix = mat.convert();
	// shaderOrbit3d->setUniform("ModelViewProjectionMatrix",proj*matrix);
	// shaderOrbit3d->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	// shaderOrbit3d->setUniform("ModelViewMatrix",matrix);
	// shaderOrbit3d->setUniform("clipping_fov",prj->getClippingFov());
	*pclipping_fov = prj->getClippingFov();

	setPrjMat(prj,mat);
	computeShader();

	// glBindVertexArray(m_Orbit3dGL.vao);
	// glBindBuffer(GL_ARRAY_BUFFER,m_Orbit3dGL.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*orbitSegments.size(),orbitSegments.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	orbit->fillVertexBuffer(BufferType::POS3D, orbitSegments);
	orbit->update();


	// m_Orbit3dGL->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,0,orbitSegments.size()/3);
	// m_Orbit3dGL->unBind();
	//Renderer::drawArrays(shaderOrbit3d.get(), orbit.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,0,orbitSegments.size()/3);
	cmdMgr->setSubmission(commandIndex, false);

	// glBindVertexArray(0);
	orbitSegments.clear();

	//StateGL::disable(GL_DEPTH_TEST);
}

void Orbit3D::setPrjMat(const Projector* _prj, const Mat4d &_mat)
{
	prj = _prj;
	mat = _mat;
}

void Orbit3D::computeShader()
{
	// only draw moon orbits as zoom in
	Vec3d onscreen;

	int i = 0;

	while(i < ORBIT_POINTS) {

		orbitSegments.push_back( (float) orbitPoint[i][0] );
		orbitSegments.push_back( (float) orbitPoint[i][1] );
		orbitSegments.push_back( (float) orbitPoint[i][2] );
		i++;
	}

	if (body->close_orbit) {
		orbitSegments.push_back( (float) orbitPoint[0][0] );
		orbitSegments.push_back( (float) orbitPoint[0][1] );
		orbitSegments.push_back( (float) orbitPoint[0][2] );
	} else {
		orbitSegments.push_back( (float) orbitPoint[ORBIT_POINTS-1][0] );
		orbitSegments.push_back( (float) orbitPoint[ORBIT_POINTS-1][1] );
		orbitSegments.push_back( (float) orbitPoint[ORBIT_POINTS-1][2] );
	}
}
