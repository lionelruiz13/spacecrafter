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


std::unique_ptr<VertexArray> Halo::m_haloGL;
std::unique_ptr<shaderProgram> Halo::shaderHalo;
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

	m_haloGL->fillVertexBuffer(BufferType::POS2D, vecHaloPos);
	m_haloGL->fillVertexBuffer(BufferType::TEXTURE, vecHaloTex);

	m_haloGL->bind();
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	m_haloGL->unBind();
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

	insert_all(vecHaloPos, screenPosF[0]-rmag, screenPosF[1]-rmag, screenPosF[0]-rmag, screenPosF[1]+rmag);
	insert_all(vecHaloPos, screenPosF[0]+rmag, screenPosF[1]-rmag, screenPosF[0]+rmag, screenPosF[1]+rmag);
	insert_all(vecHaloTex, 0, 0, 0, 1, 1, 0, 1, 1);
}

void Halo::createSC_context()
{
	shaderHalo = std::make_unique<shaderProgram>();
	shaderHalo->init( "body_halo.vert", "body_halo.frag");
	shaderHalo->setUniformLocation({"Color", "cmag"});

	m_haloGL = std::make_unique<VertexArray>();
	m_haloGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_haloGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::DYNAMIC);
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