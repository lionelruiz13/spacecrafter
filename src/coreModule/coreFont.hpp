/*
 * Copyright (C) 2020 of the LSS Team & Association Sirius
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

#ifndef _COREFONT_HPP_
#define _COREFONT_HPP_

#include <string>

//class Core;
class InitParser;

class HipStarMgr;
class NebulaMgr;
class SolarSystem;
class SkyGridMgr;
class SkyLineMgr;
class SkyDisplayMgr;
class Cardinals;
class ConstellationMgr;
class TextMgr;

class CoreFont {

public:
	friend class Core;

    CoreFont(/*Core* _core,*/ int _resolution);
    ~CoreFont();
    void init(const InitParser& conf);
    void setFont();

private:
    //Core* core;
	HipStarMgr * hip_stars;		// Manage the hipparcos stars
	NebulaMgr * nebulas;				// Manage the nebulas
	SolarSystem* ssystem;				// Manage the solar system
	SkyGridMgr * skyGridMgr;			//! gestionnaire des grilles
	SkyLineMgr* skyLineMgr;				//! gestionnaire de lignes
	SkyDisplayMgr* skyDisplayMgr; 		//! gestionnaire de skyDisplay
	Cardinals * cardinals_points;		// Cardinals points
	ConstellationMgr * asterisms;		// Manage constellations (boundaries, names etc..)
	TextMgr * text_usr;				// manage all user text in dome


	std::string FontFileNameGeneral;			//! The font file used by default during initialization
	std::string FontFileNamePlanet;				//! The font for the planet system
	std::string FontFileNameConstellation;		//! The font for all asterims
	std::string FontFileNameMenu;
	std::string FontFileNameText;
	double FontSizeText;
	double FontSizeGeneral;
	double FontSizePlanet;
	double FontSizeConstellation;
	double FontSizeCardinalPoints;

    int resolution;
};


#endif //_COREFONT_HPP_