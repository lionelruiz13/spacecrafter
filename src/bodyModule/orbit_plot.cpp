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

#include <iostream>


#include "bodyModule/orbit_plot.hpp"
#include "bodyModule/body.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"


shaderProgram* OrbitPlot::shaderOrbit2d = nullptr;
DataGL OrbitPlot::m_Orbit2dGL;

shaderProgram* OrbitPlot::shaderOrbit3d = nullptr;
DataGL OrbitPlot::m_Orbit3dGL;


OrbitPlot::OrbitPlot(Body* _body, int segments)
{
	body = _body;
	ORBIT_POINTS = segments;
	orbitPoint = new Vec3d[ORBIT_POINTS];
	orbit_cached = 0;
}

OrbitPlot::~OrbitPlot()
{
	delete[] orbitPoint;
}

void OrbitPlot::init()
{
	delta_orbitJD = body->re.sidereal_period/ORBIT_POINTS;
}

void OrbitPlot::createGL_context()
{

	shaderOrbit2d = new shaderProgram();
	shaderOrbit2d->init( "body_orbit2d.vert", "body_orbit2d.geom","body_orbit2d.frag");
	shaderOrbit2d->setUniformLocation("Mat");
	shaderOrbit2d->setUniformLocation("Color");

	glGenVertexArrays(1,&m_Orbit2dGL.vao);
	glBindVertexArray(m_Orbit2dGL.vao);
	glGenBuffers(1,&m_Orbit2dGL.pos);
	glEnableVertexAttribArray(0);

	shaderOrbit3d = new shaderProgram();
	shaderOrbit3d->init( "body_orbit3d.vert", "body_orbit3d.geom","body_orbit3d.frag");
	shaderOrbit3d->setUniformLocation("Color");
	shaderOrbit3d->setUniformLocation("ModelViewProjectionMatrix");
	shaderOrbit3d->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderOrbit3d->setUniformLocation("ModelViewMatrix");
	shaderOrbit3d->setUniformLocation("clipping_fov");

	glGenVertexArrays(1,&m_Orbit3dGL.vao);
	glBindVertexArray(m_Orbit3dGL.vao);

	glGenBuffers(1,&m_Orbit3dGL.pos);
	glEnableVertexAttribArray(0);
}

void OrbitPlot::updateShader(double delta_time)
{
	orbit_fader.update(delta_time);
}

// void OrbitPlot::deleteShader()
// {
// 	if(shaderOrbit2d) shaderOrbit2d=nullptr;
// 	if(shaderOrbit3d) shaderOrbit3d=nullptr;

// 	glDeleteBuffers(1,&m_Orbit3dGL.pos);
// 	glDeleteBuffers(1,&m_Orbit3dGL.vao);
// 	glDeleteBuffers(1,&m_Orbit2dGL.pos);
// 	glDeleteVertexArrays(1,&m_Orbit2dGL.vao);
// }

void OrbitPlot::computeOrbit(double date)
{
	// Large performance advantage from avoiding object overhead
	OsculatingFunctionType *oscFunc = body->orbit->getOsculatingFunction();


	// for performance only update orbit points if visible
	if (body->visibilityFader.getInterstate()>0.000001 && delta_orbitJD > 0 && (fabs(last_orbitJD-date)>delta_orbitJD || !orbit_cached)) {
		// calculate orbit first (for line drawing)
		double date_increment = body->re.sidereal_period/ORBIT_POINTS;
		double calc_date;
		//	  int delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		int delta_points;

		if ( date > last_orbitJD ) {
			delta_points = (int)(0.5 + (date - last_orbitJD)/date_increment);
		}
		else {
			delta_points = (int)(-0.5 + (date - last_orbitJD)/date_increment);
		}
		double new_date = last_orbitJD + delta_points*date_increment;

		//printf( "Updating orbit coordinates for %s (delta %f) (%d points)\n", name.c_str(), delta_orbitJD, delta_points);
		//cout << englishName << ": " << delta_points << "  " << orbit_cached << endl;

		if ( delta_points > 0 && delta_points < ORBIT_POINTS && orbit_cached) {
			for ( int d=0; d<ORBIT_POINTS; d++ ) {
				if (d + delta_points >= ORBIT_POINTS ) {
					// calculate new points
					calc_date = new_date + (d-ORBIT_POINTS/2)*date_increment;
					// date increments between points will not be completely constant though

					if(oscFunc)
						(*oscFunc)(date,calc_date,orbitPoint[d]);
					else
						body->orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
				}
				else {
					orbitPoint[d] = orbitPoint[d+delta_points];
				}
			}

			last_orbitJD = new_date;

		}
		else {
			if ( delta_points < 0 && abs(delta_points) < ORBIT_POINTS && orbit_cached) {
				for ( int d=ORBIT_POINTS-1; d>=0; d-- ) {
					if (d + delta_points < 0 ) {
						// calculate new points
						calc_date = new_date + (d-ORBIT_POINTS/2)*date_increment;

						if(oscFunc)
							(*oscFunc)(date,calc_date,orbitPoint[d]);
						else
							body->orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
					}
					else {
						orbitPoint[d] = orbitPoint[d+delta_points];
					}
				}

				last_orbitJD = new_date;

			}
			else {
				if ( delta_points || !orbit_cached ) {
					// update all points (less efficient)
					for ( int d=0; d<ORBIT_POINTS; d++ ) {
						calc_date = date + (d-ORBIT_POINTS/2)*date_increment;

						if(oscFunc)
							(*oscFunc)(date,calc_date,orbitPoint[d]);
						else
							body->orbit->fastPositionAtTimevInVSOP87Coordinates(date,calc_date,orbitPoint[d]);
					}

					last_orbitJD = date;

					// \todo remove this for efficiency?  Can cause rendering issues near body though
					// If orbit is largely constant through time cache it
					if (body->orbit->isStable(date))
						orbit_cached = 1;
				}
			}
		}
	}
}
