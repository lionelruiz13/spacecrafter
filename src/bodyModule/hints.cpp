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
#include "bodyModule/hints.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body.hpp"
#include "coreModule/projector.hpp"
#include "bodyModule/body_color.hpp"


shaderProgram* Hints::shaderHints=nullptr;
DataGL Hints::HintsData;
const int Hints::nbrFacets = 24;
const int Hints::hintCircleRadius = 8;

Hints::Hints(Body * _body)
{
	body = _body;
}

void Hints::createShader()
{
	shaderHints = new shaderProgram();
	shaderHints->init( "bodyHints.vert", "bodyHints.frag");
	shaderHints->setUniformLocation("Color");
	shaderHints->setUniformLocation("fader");

	glGenVertexArrays(1,&HintsData.vao);
	glBindVertexArray(HintsData.vao);
	glGenBuffers(1,&HintsData.pos);
	glEnableVertexAttribArray(0);
}

void Hints::deleteShader()
{
	glDeleteBuffers(1,&HintsData.pos);
	glDeleteVertexArrays(1,&HintsData.vao);
}

void Hints::drawHints(const Navigator* nav, const Projector* prj)
{
	if (!hint_fader.getInterstate())
		return;

	// Draw nameI18 + scaling if it's not == 1.
	float tmp = 10.f + body->getOnScreenSize(prj, nav)/2.f; // Shift for nameI18 printing

	Vec4f Color( body->myColor->getLabel(),hint_fader.getInterstate());
	prj->printGravity180(body->planet_name_font, body->screenPos[0], body->screenPos[1], body->getSkyLabel(nav), Color,/*1,*/ tmp, tmp);

	drawHintCircle(nav, prj);
}

void Hints::drawHintCircle(const Navigator* nav, const Projector* prj)
{
	computeHints();

	shaderHints->use();
	shaderHints->setUniform("Color", body->myColor->getLabel());
	shaderHints->setUniform("fader", hint_fader.getInterstate() );

	glBindVertexArray(HintsData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,HintsData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecHintsPos.size(),vecHintsPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_LINE_LOOP,0,nbrFacets);
	glBindVertexArray(0);

	shaderHints->unuse();

	vecHintsPos.clear();
}

void Hints::computeHints()
{

	Vec3d pos = body->screenPos;
	float angle;

	for (int i = 0; i < nbrFacets; i++) {
		angle = 2.0f*M_PI*i/nbrFacets;
		vecHintsPos.push_back( pos[0] + hintCircleRadius * sin(angle) );
		vecHintsPos.push_back( pos[1] + hintCircleRadius * cos(angle) );
	}
}

void Hints::updateShader(double delta_time)
{
	hint_fader.update(delta_time);
}
