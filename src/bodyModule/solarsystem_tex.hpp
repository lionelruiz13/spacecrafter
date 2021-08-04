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

#ifndef _SOLARSYSTEM_TEX_
#define _SOLARSYSTEM_TEX_

#include "solarsystem.hpp"

class SolarSystemTex {
public:
    SolarSystemTex(SolarSystem * _ssystem);
    ~SolarSystemTex();

    //switch tex map from Planet "name"
	void switchPlanetTexMap(const std::string &name, bool a);

    // return switch tex map value from Planet "name"
	bool getSwitchPlanetTexMap(const std::string &name);

	void createTexSkin(const std::string &name, const std::string &texName);

	void iniTextures();

    //initialise the body tesselation value
	void iniTess(int minTes, int maxTes, int planetTes, int moonTes, int earthTes) {
		bodyTesselation->setMinTes(minTes, true);
		bodyTesselation->setMaxTes(maxTes, true);
		bodyTesselation->setPlanetTes(planetTes,true);
		bodyTesselation->setMoonTes(moonTes,true);
		bodyTesselation->setEarthTes(earthTes,true);
	}    

	// send tesselation parms to body: name design the param to change to value
	void planetTesselation(std::string name, int value);

    void updateTesselation(int delta_time) {
        bodyTesselation->updateTesselation(delta_time);
    }

	void resetTesselationParams() {
	    bodyTesselation->resetTesselationParams();
    }

private:
    SolarSystem * ssystem;
    std::shared_ptr<BodyTesselation> bodyTesselation=nullptr;
};

#endif