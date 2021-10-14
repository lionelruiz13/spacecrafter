/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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

#include "bodyModule/ssystem_factory.hpp"

#ifndef _EXECUTOR_MODULE_
#define _EXECUTOR_MODULE_

enum MODULE {SOLAR_SYSTEM, IN_GALAXY, IN_UNIVERSE};

class ExecutorModule {
public:

    ~ExecutorModule() {};
    
	virtual void onEnter() = 0;
	virtual void onExit() = 0;
	virtual void update(int delta_time)=0;
	virtual void draw(int delta_time)=0;
	virtual bool testValidAltitude(double altitude)=0;

	void defineDownMode(ExecutorModule *_downMode) {
		downMode = _downMode;
	}

	void defineUpMode(ExecutorModule *_upMode) {
		upMode = _upMode;
	}

	ExecutorModule *getNextMode() {
		return nextMode;
	}

	MODULE getExecutorModule() {
		return module;
	}
	
protected:

	double minAltToGoDown = 0.0;	// altitude min avant changement de mode vers upMode
	double maxAltToGoUp = 0.0;		// altitude max avant changement de mode vers downMode

	MODULE module;
	ExecutorModule *downMode = nullptr;
	ExecutorModule *upMode = nullptr;
	ExecutorModule *nextMode = nullptr;
};

#endif