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
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"


std::unique_ptr<shaderProgram> Trail::shaderTrail;
std::unique_ptr<VertexArray> Trail::m_dataGL;

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
	vecTrailIntensity.clear();
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
		// vecTrailPos.push_back( body->getEarthEquPos(nav)[0] );
		// vecTrailPos.push_back( body->getEarthEquPos(nav)[1] );
		// vecTrailPos.push_back( body->getEarthEquPos(nav)[2] );
		insert_vec3(vecTrailPos, body->getEarthEquPos(nav));
		vecTrailIntensity.push_back(1.0);
	}

	for (iter=begin; iter != trail.end(); iter++) {
		segment++;
		// vecTrailPos.push_back( (*iter).point[0] );
		// vecTrailPos.push_back( (*iter).point[1] );
		// vecTrailPos.push_back( (*iter).point[2] );
		insert_vec3(vecTrailPos, (*iter).point);
		vecTrailIntensity.push_back( segment);
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

		// glBindVertexArray(m_dataGL.vao);

		// glBindBuffer(GL_ARRAY_BUFFER,m_dataGL.pos);
		// glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vecTrailPos.size(), vecTrailPos.data(), GL_DYNAMIC_DRAW);
		// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

		// glBindBuffer(GL_ARRAY_BUFFER,m_dataGL.color);
		// glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vecTrailIntensity.size(), vecTrailIntensity.data(), GL_DYNAMIC_DRAW);
		// glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);
		m_dataGL->fillVertexBuffer(BufferType::POS3D, vecTrailPos);
		m_dataGL->fillVertexBuffer(BufferType::MAG, vecTrailIntensity);

		m_dataGL->bind();
		glDrawArrays(GL_LINE_STRIP, 0, nbPos);
		m_dataGL->unBind();
		//glBindVertexArray(0);
		
		shaderTrail->unuse();
		StateGL::enable(GL_BLEND);
	}

	vecTrailPos.clear();
	vecTrailIntensity.clear();
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

void Trail::updateFader(int delta_time)
{
	trail_fader.update(delta_time);
}


void Trail::createGL_context()
{
	shaderTrail = std::make_unique<shaderProgram>();
	shaderTrail->init( "body_trail.vert","body_trail.geom","body_trail.frag");
	shaderTrail->setUniformLocation({"Mat", "Color", "fader", "nbPoints"});

	m_dataGL = std::make_unique<VertexArray>();
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
	m_dataGL->registerVertexBuffer(BufferType::MAG, BufferAccess::DYNAMIC);

	// glGenVertexArrays(1,&m_dataGL.vao);
	// glBindVertexArray(m_dataGL.vao);

	// glGenBuffers(1,&m_dataGL.pos);
	// glGenBuffers(1,&m_dataGL.color);
	// glEnableVertexAttribArray(0);
	// glEnableVertexAttribArray(1);
}

// void Trail::deleteShader()
// {
// 	if(shaderTrail) shaderTrail=nullptr;

// 	glDeleteBuffers(1,&m_dataGL.color);
// 	glDeleteBuffers(1,&m_dataGL.pos);
// 	glDeleteVertexArrays(1,&m_dataGL.vao);
// }
