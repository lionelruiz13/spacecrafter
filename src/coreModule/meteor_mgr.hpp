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

#ifndef _METEOR_MGR_H_
#define _METEOR__MGR_H_

#include <vector>
#include <list>
#include <functional>
#include <memory>

#include "coreModule/time_mgr.hpp"
#include "meteor.hpp"
#include "renderGL/stateGL.hpp"
#include "tools/no_copy.hpp"

class Projector;
class Navigator;
class VertexArray;
class shaderProgram;

class MeteorMgr: public NoCopy {

public:
	// base_zhr is zenith hourly rate sans meteor shower
	MeteorMgr(int zhr, int maxv );  
	virtual ~MeteorMgr();

	// set zenith hourly rate
	void setZHR(int zhr){   
		ZHR = zhr;
	}
	// get zenith hourly rate
	int getZHR() const {
		return ZHR;
	}
  	// set maximum meteoroid velocity km/s
	void setMaxVelocity(int maxv){
		max_velocity = maxv;
	};

	//! update positions
	void update(Projector *proj, Navigator* nav, TimeMgr* timeMgr, ToneReproductor* eye, int delta_time);          
	// Draw the meteors
	void draw(Projector *proj, Navigator* nav);		

private:
	void createSC_context();
	std::list<std::unique_ptr<Meteor>> m_activeMeteor;		// list containing all active meteors

	int ZHR;
	int max_velocity;
	double zhr_to_wsr;  // factor to convert from zhr to whole earth per second rate

	// un meteor = 3 positions, donc 6 floats points
	std::vector<float> vecPos;
	std::vector<float> vecColor;

	std::unique_ptr<VertexArray> m_meteorGL;
	std::unique_ptr<shaderProgram> m_shaderMeteor;
};
#endif // _METEOR_MGR_H
