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


#include "bodyModule/halo.hpp"
#include "bodyModule/body.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "bodyModule/body_color.hpp"
#include "tools/tone_reproductor.hpp"
#include "tools/s_texture.hpp"
#include <iostream>
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"


DataGL Halo::m_haloGL;
shaderProgram* Halo::shaderHalo = nullptr;
s_texture * Halo::tex_halo = nullptr;

Halo::Halo(Body * _body)
{
	body = _body;
}

void Halo::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	computeHalo(nav, prj, eye);
	if (rmag<1.21 && cmag < 0.05)
		return;

	StateGL::BlendFunc(GL_ONE, GL_ONE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_halo->getID());

	StateGL::enable(GL_BLEND);

	shaderHalo->use();
	shaderHalo->setUniform("Color", body->myColor->getHalo());
	shaderHalo->setUniform("cmag", cmag);

	glBindVertexArray(m_haloGL.vao);

	glBindBuffer(GL_ARRAY_BUFFER,m_haloGL.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHaloPos.size(),vecHaloPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,m_haloGL.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHaloTex.size(),vecHaloTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glBindVertexArray(0);

	shaderHalo->unuse();

	vecHaloPos.clear();
	vecHaloTex.clear();
}

void Halo::computeHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
	float fov_q = prj->getFov();
	if (fov_q > 60) fov_q = 60;
	fov_q = 1.f/(fov_q*fov_q);

	rmag = sqrtf(eye->adaptLuminance((expf(-0.92103f*(body->computeMagnitude(nav->getObserverHelioPos()) + 12.12331f)) * 108064.73f) * fov_q)) * 30.f * Body::object_scale;

	if (body->is_satellite)	{
		if (prj->getFov()>60) rmag=rmag/25; // usefull when going there
		else rmag=rmag/5; // usefull when zooming onto planet
	}
	cmag = 1.f;

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rmag<1.2f) {
		if (body->computeMagnitude(nav->getObserverHelioPos())>0.) cmag=rmag*rmag/1.44f;
		else cmag=rmag/1.2f;
		rmag=1.2f;
	}
	else {

		float limit = Body::object_size_limit/1.8;
		if (rmag>limit) {
			rmag = limit + sqrt(rmag-limit)/(limit + 1);

			if (rmag > Body::object_size_limit) {
				rmag = Body::object_size_limit;
			}
		}
	}

	float screen_r = body->getOnScreenSize(prj, nav);
	cmag *= 0.5*rmag/screen_r;
	if (cmag>1.f) cmag = 1.f;

	if (rmag<screen_r) {
		cmag*=rmag/screen_r;
		rmag = screen_r;
	}

	if (body->is_satellite) {
		Vec3d _planet = body->get_parent()->get_heliocentric_ecliptic_pos();
		Vec3d _satellite = body->get_heliocentric_ecliptic_pos();
		double c = _planet.dot(_satellite);
		double OP = _planet.length();
		double OS = _satellite.length();
		if (c>0 && OP < OS)
			if (fabs(acos(c/(OP*OS)))<atan(body->get_parent()->getRadius()/OP)) {
				cmag = 0.0;
			}
	}

	if (rmag<1.21 && cmag < 0.05)
		return;

	Vec2f screenPosF ((float) body->screenPos[0], (float)body->screenPos[1]);

	vecHaloPos.push_back( screenPosF[0]-rmag );
	vecHaloPos.push_back( screenPosF[1]-rmag );
	vecHaloPos.push_back( screenPosF[0]-rmag );
	vecHaloPos.push_back( screenPosF[1]+rmag );
	vecHaloPos.push_back( screenPosF[0]+rmag );
	vecHaloPos.push_back( screenPosF[1]-rmag );
	vecHaloPos.push_back( screenPosF[0]+rmag );
	vecHaloPos.push_back( screenPosF[1]+rmag );

	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 1 );
	vecHaloTex.push_back( 1 );
	vecHaloTex.push_back( 0 );
	vecHaloTex.push_back( 1 );
	vecHaloTex.push_back( 1 );
}

void Halo::createShader()
{

	shaderHalo = new shaderProgram();
	shaderHalo->init( "body_halo.vert", "body_halo.frag");
	shaderHalo->setUniformLocation("Color");
	shaderHalo->setUniformLocation("cmag");

	glGenVertexArrays(1,&m_haloGL.vao);
	glBindVertexArray(m_haloGL.vao);
	glGenBuffers(1,&m_haloGL.tex);
	glGenBuffers(1,&m_haloGL.pos);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

}

bool Halo::setTexHaloMap(const std::string &texMap)
{
	tex_halo = new s_texture(texMap, TEX_LOAD_TYPE_PNG_SOLID_REPEAT,1);
	if (tex_halo != nullptr)
		return true;
	else
		return false;
}

void Halo::deleteDefaultTexMap()
{
	if(tex_halo != nullptr) {
		delete tex_halo;
		tex_halo = nullptr;
	}
}

void Halo::deleteShader()
{
	glDeleteBuffers(1, &m_haloGL.tex);
	glDeleteBuffers(1, &m_haloGL.pos);
	glDeleteVertexArrays(1, &m_haloGL.vao);

}
