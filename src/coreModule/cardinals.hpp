/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#ifndef __CARDINALS_H__
#define __CARDINALS_H__

#include <string>
#include <fstream>
#include "tools/auto_fader.hpp"
#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "tools/ScModule.hpp"

class Projector;
class s_font;
class Translator;

//! Class which manages the cardinal points displaying
class Cardinals: public NoCopy , public ModuleColor, public ModuleFont , public AModuleFader<ALinearFader> {
public:
	Cardinals(float _radius = 1.);
	virtual ~Cardinals();

	void draw(const Projector* prj, double latitude, bool gravityON = false) const;

	void translateLabels(Translator& trans);  // for i18n

	void setInternalNav (bool a) {
		internalNav=a;
	}

	void setInternalAstronomical (bool a){
		internalAstronomical = a;
	}

private:
	float radius;
	std::string sNorth, sSouth, sEast, sWest;
	bool internalNav;
	bool internalAstronomical;
};


#endif // __CARDINALS_H__
