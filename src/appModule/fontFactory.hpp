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

#ifndef _FONTFACTORY_HPP_
#define _FONTFACTORY_HPP_

#include <string>
#include <map>
#include <list>
#include <utility>
#include <memory>

#include "tools/s_font.hpp"

/**
 * \file fontFactory.hpp
 * \brief All Font Management : creation / update
 * \author Association Sirius
 * \version 1
 *
 * \class FontFactory
 *
 * \brief Transforms alphanumeric entry to Font 
 *
 * The purpose of this class is to analyze character strings (string name and string size) 
 * and get to all Core Font Module aprouved s_font parameters
 *
 * Each planetarium has its own resolution. To harmonize the size of the fonts regardless of the resolution,
 * we use a reference font size : FontResolution. And we apply proportionality to determine the size of the 
 * fonts to display. 
 *
*/


//! used to translate string to TARGETFONT
enum class TARGETFONT : char {
	CF_TEXTS, 
	CF_PLANETS,
	CF_CONSTELLATIONS,
	CF_CARDINALS,
	CF_HIPSTARS,
	CF_UIMENU,
	CF_GENERAL,
	CF_NONE
};

enum class CLASSEFONT : char {
	CLASS_UI,
	CLASS_TEXTMGR
};


typedef std::pair< CLASSEFONT, std::unique_ptr<s_font>> pairNameFontPtr;

class InitParser;
class Media;

class FontFactory {

public:
	//friend class Core;

    FontFactory(int _resolution);
    ~FontFactory();
	//! initialise les fontes: fichiers et tailles 
    void init(const InitParser& conf);
	//! construit toutes les fontes
	void buildAllFont();

	s_font* getUiFont() {
		//std::cout << "get ui fonts" << std::endl;
		return tuiFont.get();
	}

	s_font* registerFont(CLASSEFONT _cf);

	// cas particulier de Media qui gère ses propres fontes
	void initMediaFont(Media * _media);

	//! met à jour des fontes de Core
	void updateFont(const std::string& targetName, const std::string& fontName, const std::string& sizeValue);

private:
	void setStrToTarget();
	// Core

	// Media
	Media* media;
	// Ui
	std::unique_ptr<s_font> tuiFont = nullptr;

	// All in one
	std::list< pairNameFontPtr > listFont;

	// Core
	std::string FontFileNameGeneral;			//! The font file used by default during initialization
	std::string FontFileNamePlanet;				//! The font for the planet system
	std::string FontFileNameConstellation;		//! The font for all asterims
	std::string FontFileNameMenu;
	double FontSizeGeneral;
	double FontSizePlanet;
	double FontSizeConstellation;
	double FontSizeCardinalPoints;
	// Media
	std::string FontFileNameText;
	double FontSizeText;
	// Ui
	std::string FontNameTuiMenu;
	float FontSizeTuiMenu;
	
    int m_resolution;
	double m_fontResolution;
	std::map< std::string, TARGETFONT> m_strToTarget;
};


#endif //_FONTFACTORY_HPP_