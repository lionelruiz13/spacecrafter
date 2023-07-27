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
class s_font;

enum class CLASSEFONT : char {
	CLASS_HIPSTARS,
	CLASS_NEBULAE,
	CLASS_SSYSTEM,
	CLASS_SKYGRID,
	CLASS_SKYLINE,
	CLASS_SKYDISPLAY,
	CLASS_CARDINALS,
	CLASS_ASTERIMS,
	CLASS_UI,
	CLASS_MENU
};


struct FontContener{
	CLASSEFONT classeFont;
	std::unique_ptr<s_font> fontPtr;
	float sizeFont;
	std::string nameFont;
	FontContener(CLASSEFONT _classeFont, float _sizeFont,	std::string _nameFont) {
		classeFont = _classeFont;
		sizeFont = _sizeFont;
		nameFont = _nameFont;
	};
};

class InitParser;
class Media;

class FontFactory {

public:
    FontFactory();
    ~FontFactory();
	//! initialize fonts: files and sizes
    void init(int _resolution, const InitParser& conf);
	//! builds all fonts
	void buildAllFont();

	//! Asks FontFactory to deliver a pointer to the font of _cf
	s_font* registerFont(CLASSEFONT _cf);
	// special case of Media managing its own fonts
	void initMediaFont(Media * _media);

	//! updates Core fonts
	void updateFont(const std::string& targetName, const std::string& fontName, const std::string& sizeValue);
	void updateAllFont(const std::string& fontName);

	//! resets all FontFactory fonts
	void reloadAllFont();

private:
	void setStrToTarget();
	Media* media;
	// All in one
	std::list<FontContener> listFont;

	// // Media
	std::string FontFileNameText;
	double FontSizeText;
	float fontFactor;
	std::map< std::string, CLASSEFONT> m_strToTarget;
};


#endif //_FONTFACTORY_HPP_
