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

#ifndef _SOLARSYSTEM_DISPLAY_
#define _SOLARSYSTEM_DISPLAY_

#include <vector>

class ProtoSystem;
class Projector;
class Navigator;
class Observer;
class ToneReproductor;
class Body;

/**
 * \file solarsystem_display.hpp
 * \brief Handle solar system drawing functions
 * \author Jérémy Calvo
 * \version 1
 *
 * \class SolarSystemDisplay
 *
 * \brief Contains solar system drawing and updating functions
 *
 * Allow to set display flags
 *
*/

class SolarSystemDisplay {
public:
    SolarSystemDisplay(ProtoSystem * _ssystem);
    ~SolarSystemDisplay() {
        instance = nullptr;
    }

    void changeSystem(ProtoSystem * _ssystem) {
		ssystem = _ssystem;
	}

	void computePreDraw(const Projector * prj, const Navigator * nav);

    //! Draw the shadows so they can be used when drawing the main body
    void drawShadow(Projector * prj, const Navigator * nav);

	//! Draw all the elements of the solar system
	void draw(Projector * prj, const Navigator * nav, const Observer* observatory,
	          const ToneReproductor* eye,
	          bool drawHomePlanet );

	//! get flag for Activate/Deactivate planets display
	bool getFlagShow(void) const {
		return flagShow;
	}

	//! set flag for Activate/Deactivate planets display
	void setFlagPlanets(bool b) {
		flagShow = b;
	}

	bool getFlagLightTravelTime(void) const {
		return flag_light_travel_time;
	}

	void setFlagLightTravelTime(bool b) {
		flag_light_travel_time = b;
	}

	//! Compute the position for every elements of the solar system.
	//! home_planet is needed for light travel time computation
	void computePositions(double date,const Observer *obs);

	//! Compute the transformation matrix for every elements of the solar system.
	//! home_planet is needed for light travel time computation
	void computeTransMatrices(double date,const Observer * obs);

    static SolarSystemDisplay *instance;
private:
    ProtoSystem * ssystem;
    Body *mainBody;

	bool flagShow= true;
	bool flag_light_travel_time = false;

    struct depthBucket {
		double znear;
		double zfar;
	};

    struct ShadowingBody {
        Body *body;
        float distToSun;
        float distToMainBody;
    };

	std::vector<depthBucket> listBuckets;
    std::vector<ShadowingBody> shadowingBody; // Bodies who project a shadow on the mainBody
    int cmds[3] {-1};
};

#endif
