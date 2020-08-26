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
#include "renderGL/OpenGL.hpp"
#include "renderGL/Renderer.hpp"

Orbit2D::Orbit2D(Body* _body, int segments) : OrbitPlot(_body, segments) { }

void Orbit2D::drawOrbit(const Navigator * nav, const Projector* prj, const Mat4d &mat)
{
	if (!orbit_fader.getInterstate())
		return;
	if (!body->visibilityFader.getInterstate())
		return;

	Vec3d onscreen;
	if (!body->re.sidereal_period)
		return;

	computeShader();

	// Normal transparency mode
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	StateGL::enable(GL_BLEND);
	Vec4f Color ( body->myColor->getOrbit(), (orbit_fader.getInterstate()*body->visibilityFader.getInterstate()) );

	shaderOrbit2d->use();
	shaderOrbit2d->setUniform("Mat", mat.convert());
	shaderOrbit2d->setUniform("Color", Color);

	// glBindVertexArray(m_Orbit2dGL.vao);
	// glBindBuffer(GL_ARRAY_BUFFER,m_Orbit2dGL.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecOrbit2dVertex.size(),vecOrbit2dVertex.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	m_Orbit2dGL->fillVertexBuffer(BufferType::POS3D,vecOrbit2dVertex );

	// m_Orbit2dGL->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, vecOrbit2dVertex.size()/3);
	// m_Orbit2dGL->unBind();
	// // glBindVertexArray(0);
	// shaderOrbit2d->unuse();
	Renderer::drawArrays(shaderOrbit2d.get(), m_Orbit2dGL.get(), VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, 0, vecOrbit2dVertex.size()/3);

	vecOrbit2dVertex.clear();

}

void Orbit2D::computeShader()
{

	for ( int n=0; n<ORBIT_POINTS/2-1; n++) {
		vecOrbit2dVertex.push_back( (float)(orbitPoint[n][0]) );
		vecOrbit2dVertex.push_back( (float)(orbitPoint[n][1]) );
		vecOrbit2dVertex.push_back( (float)(orbitPoint[n][2]) );
	}

//-------------------------------------------------------------------------
//
// gestion du point passant par le centre de la planete
//
//-------------------------------------------------------------------------
	//ICI insertion du point passant par le centre de la planete
	body->orbit_position= body->get_ecliptic_pos();
	vecOrbit2dVertex.push_back( (float) (body->orbit_position[0]-body->radius/10)  );
	vecOrbit2dVertex.push_back( (float) (body->orbit_position[1]-body->radius/10) );
	vecOrbit2dVertex.push_back( (float) (body->orbit_position[2]-0*body->radius/10)  );
//-------------------------------------------------------------------------
	for ( int n= ORBIT_POINTS/2+1; n< ORBIT_POINTS-1; n++) {
		vecOrbit2dVertex.push_back( (float)orbitPoint[n][0] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[n][1] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[n][2] );
	}
	//fermer la boucle
	if (body->close_orbit) {
		vecOrbit2dVertex.push_back( (float)orbitPoint[0][0] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[0][1] );
		vecOrbit2dVertex.push_back( (float)orbitPoint[0][2] );
	}
}
