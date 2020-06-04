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

#include "bodyModule/trail.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/time_mgr.hpp"
#include "bodyModule/body.hpp"
#include "bodyModule/body_color.hpp"



shaderProgram* Trail::shaderTrail=nullptr;
DataGL Trail::TrailData;

Trail::Trail(Body * _body,
             int _MaxTrail,
             double _DeltaTrail,
             double _last_trailJD,
             bool _trail_on,
             bool _first_point) :
	MaxTrail(_MaxTrail),
	DeltaTrail(_DeltaTrail),
	last_trailJD(_last_trailJD),
	trail_on(_trail_on),
	first_point(_first_point)
{

	body = _body;

}


Trail::~Trail()
{
	vecTrailPos.clear();
	vecTrailColor.clear();
	trail.clear();
}

void Trail::drawTrail(const Navigator * nav, const Projector* prj)
{
	float fade = trail_fader.getInterstate();
	if (!fade)
		return;
	if (trail.empty())
		return;

	std::list<TrailPoint>::iterator iter;
	std::list<TrailPoint>::iterator begin = trail.begin();

	float segment = 0;

	// draw final segment to finish at current Body position
	if ( !first_point) {
		vecTrailPos.push_back( body->getEarthEquPos(nav)[0] );
		vecTrailPos.push_back( body->getEarthEquPos(nav)[1] );
		vecTrailPos.push_back( body->getEarthEquPos(nav)[2] );

		vecTrailColor.push_back(1.0);
	}

	for (iter=begin; iter != trail.end(); iter++) {
		segment++;
		vecTrailPos.push_back( (*iter).point[0] );
		vecTrailPos.push_back( (*iter).point[1] );
		vecTrailPos.push_back( (*iter).point[2] );

		vecTrailColor.push_back( segment);
	}

	int nbPos = vecTrailPos.size()/3 ;
	if (nbPos >= 2) {

		StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		StateGL::enable(GL_BLEND);

		shaderTrail->use();
		shaderTrail->setUniform("Mat", prj->getMatEarthEquToEye());
		shaderTrail->setUniform("Color", body->myColor->getTrail());
		shaderTrail->setUniform("fader", fade);
		shaderTrail->setUniform("nbPoints", nbPos);

		glBindVertexArray(TrailData.vao);

		glBindBuffer(GL_ARRAY_BUFFER,TrailData.pos);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vecTrailPos.size(), vecTrailPos.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

		glBindBuffer(GL_ARRAY_BUFFER,TrailData.color);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vecTrailColor.size(), vecTrailColor.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);

		glDrawArrays(GL_LINE_STRIP, 0, nbPos);
		glBindVertexArray(0);
		
		shaderTrail->unuse();
		StateGL::enable(GL_BLEND);
	}

	vecTrailPos.clear();
	vecTrailColor.clear();
}

// update trail points as needed
void Trail::updateTrail(const Navigator* nav, const TimeMgr* timeMgr)
{
	if (trail_fader.getInterstate()< 0.001)
		return;

	double date = timeMgr->getJulian();

	int dt=0;
	if (first_point || (dt=abs(int((date-last_trailJD)/DeltaTrail))) > MaxTrail) {
		dt=1;
		trail.clear();
		first_point = 0;
	}

	// Note that when jump by a week or day at a time, loose detail on trails
	// particularly for moon (if decide to show moon trail)
	// add only one point at a time, using current position only
	if (dt) {
		last_trailJD = date;
		TrailPoint tp;
		Vec3d v = body->get_heliocentric_ecliptic_pos();
		tp.point = nav->helioToEarthPosEqu(v);
		tp.date = date;
		trail.push_front( tp );

		if ( trail.size() > (unsigned int)MaxTrail ) {
			trail.pop_back();
		}
	}

	// because sampling depends on speed and frame rate, need to clear out
	// points if trail gets longer than desired
	std::list<TrailPoint>::iterator iter;
	std::list<TrailPoint>::iterator end = trail.end();

	for ( iter=trail.begin(); iter != end; iter++) {
		if ( fabs((*iter).date - date)/DeltaTrail > MaxTrail ) {
			trail.erase(iter, end);
			break;
		}
	}
}

void Trail::startTrail(bool b)
{
	if (b) {
		trail_on = true; // No trail for Sun or moons
		first_point = true;
	}
	else {
		trail_on = false;
	}
}

void Trail::updateShader(int delta_time)
{
	trail_fader.update(delta_time);
}


void Trail::createShader()
{

	shaderTrail = new shaderProgram();
	shaderTrail->init( "body_trail.vert","body_trail.geom","body_trail.frag");
	shaderTrail->setUniformLocation("Mat");
	shaderTrail->setUniformLocation("Color");
	shaderTrail->setUniformLocation("fader");
	shaderTrail->setUniformLocation("nbPoints");

	glGenVertexArrays(1,&TrailData.vao);
	glBindVertexArray(TrailData.vao);

	glGenBuffers(1,&TrailData.pos);
	glGenBuffers(1,&TrailData.color);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

}

void Trail::deleteShader()
{
	if(shaderTrail) shaderTrail=nullptr;

	glDeleteBuffers(1,&TrailData.color);
	glDeleteBuffers(1,&TrailData.pos);
	glDeleteVertexArrays(1,&TrailData.vao);
}
