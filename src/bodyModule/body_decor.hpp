/*
 * Copyright (C) 2018 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _BODYDECOR_HPP_
#define _BODYDECOR_HPP_

#include <memory>

class MilkyWay;
class Atmosphere;
struct  AtmosphereParams;


class BodyDecor {
public:
	BodyDecor(std::shared_ptr<MilkyWay> _milky, std::shared_ptr<Atmosphere> _atmosphere);
	~BodyDecor() {}

	//! indicates if we are in conditions that allow us to trace the landscape
	bool canDrawLandscape() {
		return drawLandscape;
	}
	//! indicates if you are in conditions that allow you to plot the meteors
	bool canDrawMeteor() {
		return drawMeteor;
	}
	//! indicates if the conditions are right to plot the 3D representation of the Body
	bool canDrawBody() {
		return drawBody;
	}

	// function that calculates the different flags when we are in space
	void anchorAssign(/*bool Spacecraft*/);

	// function calculating the different flags when we are on a body
	void bodyAssign(double altitude, const AtmosphereParams* atmParam/*, bool Spacecraft*/);

	bool getAtmosphereState() {
		return atmState;
	}

	void setAtmosphereState(bool value) {
		atmState = value;
	}

private:
	bool drawLandscape = false;
	bool drawMeteor = false;
	bool atmState = false;
	bool drawBody = false;

	std::shared_ptr<MilkyWay> milky;
	std::shared_ptr<Atmosphere> atmosphere;
};


#endif
