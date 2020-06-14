/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2004 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/meteor_mgr.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"

#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"


MeteorMgr::MeteorMgr(int zhr, int maxv )
{
	ZHR = zhr;
	max_velocity = maxv;

	// calculate factor for meteor creation rate per second since visible area ZHR is for
	// estimated visible radius of 458km
	// (calculated for average meteor magnitude of +2.5 and limiting magnitude of 5)

	zhr_to_wsr = 1.6667f/3600.f;
	// this is a correction factor to adjust for the model as programmed to match observed rates

	createShader();
}

MeteorMgr::~MeteorMgr() 
{
	// deleteShader();
}

void MeteorMgr::setZHR(int zhr)
{
	ZHR = zhr;
}

int MeteorMgr::getZHR()
{
	return ZHR;
}

void MeteorMgr::setMaxVelocity(int maxv)
{
	max_velocity = maxv;
}

void MeteorMgr::update(Projector *proj, Navigator* nav, TimeMgr* timeMgr, ToneReproductor* eye, int delta_time)
{

	// step through and update all active meteors
	int n =0;
	//if (active.empty())
	//	return;

	for (std::vector<Meteor*>::iterator iter = active.begin(); iter != active.end(); ++iter) {
		n++;
		//printf("Meteor %d update\n", ++n);
		if ( !( (*iter)->update(delta_time) ) ) {
			// remove dead meteor
			//      printf("Meteor \tdied\n");
			delete *iter;
			active.erase(iter);
			iter--;  // important!
		}
	}

	// only makes sense given lifetimes of meteors to draw when time_speed is realtime
	// otherwise high overhead of large numbers of meteors
	double tspeed = timeMgr->getTimeSpeed() *86400;  // sky seconds per actual second
	if (tspeed <= 0 || fabs(tspeed) > 1 ) {
		// don't start any more meteors
		return;
	}

	/*
	// debug - one at a time
	if(active.begin() == active.end() ) {
	  Meteor *m = new Meteor(projection, navigation, max_velocity);
	  active.push_back(m);
	    }
	*/

	// if application has been suspended, don't create huge number of meteors to
	// make up for lost time!
	if ( delta_time > 500 ) {
		delta_time = 500;
	}

	// determine average meteors per frame needing to be created
	int mpf = (int)((double)ZHR*zhr_to_wsr*(double)delta_time/1000.0f + 0.5);
	if ( mpf < 1 ) mpf = 1;

	int mlaunch = 0;
	for (int i=0; i<mpf; i++) {

		// start new meteor based on ZHR time probability
		double prob = (double)rand()/((double)RAND_MAX+1);
		if ( ZHR > 0 && prob < ((double)ZHR*zhr_to_wsr*(double)delta_time/1000.0f/(double)mpf) ) {
			Meteor *m = new Meteor(proj, nav, eye, max_velocity);
			active.push_back(m);
			mlaunch++;
		}
	}
	//  printf("mpf: %d\tm launched: %d\t(mps: %f)\t%d\n", mpf, mlaunch, ZHR*zhr_to_wsr, delta_time);
}

void MeteorMgr::createShader()
{
	shaderMeteor = std::make_unique<shaderProgram>();
	shaderMeteor->init("meteor.vert","meteor.frag");

	// glGenVertexArrays(1,&meteor.vao);
	// glBindVertexArray(meteor.vao);

	// glGenBuffers(1,&meteor.color);
	// glGenBuffers(1,&meteor.pos);

	// glEnableVertexAttribArray(0);
	// glEnableVertexAttribArray(1);

 	meteor = std::make_unique<VertexArray>();
	meteor->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	meteor->registerVertexBuffer(BufferType::COLOR4, BufferAccess::DYNAMIC);
}

// void MeteorMgr::deleteShader()
// {
// 	if(shaderMeteor) shaderMeteor=nullptr;

// 	glDeleteBuffers(1,&meteor.pos);
// 	glDeleteBuffers(1,&meteor.color);
// 	glDeleteVertexArrays(1,&meteor.vao);
// }

void MeteorMgr::draw(Projector *proj, Navigator* nav)
{
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	StateGL::enable(GL_BLEND);

	for (std::vector<Meteor*>::iterator iter = active.begin(); iter != active.end(); ++iter)
		(*iter)->draw(proj, nav, vecPos, vecColor);

	if (vecPos.size()==0)
		return;

	// glBindVertexArray(meteor.vao);

	// glBindBuffer(GL_ARRAY_BUFFER,meteor.color);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecColor.size(),vecColor.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,meteor.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecPos.size(),vecPos.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	meteor->fillVertexBuffer(BufferType::POS2D, vecPos);
	meteor->fillVertexBuffer(BufferType::COLOR4, vecColor);

	shaderMeteor->use();
	meteor->bind();
	for(unsigned int i=0; i < (vecPos.size()/3) ; i++)
		glDrawArrays(GL_LINE_STRIP, 3*i, 3);
	
	meteor->unBind();
	// glBindVertexArray(0);
	shaderMeteor->unuse();

	vecPos.clear();
	vecColor.clear();
}