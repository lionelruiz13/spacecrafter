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

#include <algorithm>

#include "appModule/fontFactory.hpp"
#include "tools/init_parser.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include "mainModule/define_key.hpp"
#include "mediaModule/media.hpp"
#include "interfaceModule/base_command_interface.hpp"

FontFactory::FontFactory()
{
	setStrToTarget();
}

void FontFactory::setStrToTarget()
{
    m_strToTarget[TF_TEXT] = CLASSEFONT::CLASS_MENU;
	m_strToTarget[TF_PLANETS] = CLASSEFONT::CLASS_SSYSTEM;
	m_strToTarget[TF_CONSTELLATIONS] = CLASSEFONT::CLASS_ASTERIMS;
	m_strToTarget[TF_CARDINAL] = CLASSEFONT::CLASS_CARDINALS;
	m_strToTarget[TF_STARS] = CLASSEFONT::CLASS_HIPSTARS;
	m_strToTarget[TF_MENU] = CLASSEFONT::CLASS_UI;
	m_strToTarget[TF_NEBULAE] = CLASSEFONT::CLASS_NEBULAE;
	m_strToTarget[TF_GRIDS] = CLASSEFONT::CLASS_SKYGRID;
	m_strToTarget[TF_LINES] = CLASSEFONT::CLASS_SKYLINE;
	m_strToTarget[TF_DISPLAYS] = CLASSEFONT::CLASS_SKYDISPLAY;
}

FontFactory::~FontFactory()
{
	listFont.clear();
}

void FontFactory::initMediaFont(Media * _media)
{
	media=_media;
}

void FontFactory::buildAllFont()
{
	std::for_each(listFont.begin(), listFont.end(), 
		[](FontContener &n){ n.fontPtr = std::make_unique<s_font>(n.sizeFont, n.nameFont); }
	);

	media->setTextFont(FontSizeText, FontFileNameText);
	//cas spécial de Media
	media->buildTextFont();
}

void FontFactory::init(int resolution, const InitParser& conf)
{
	// test if the software can use fonts
	s_font::initBaseFont(AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GENERAL_NAME));

	//scale resolution
	int m_fontResolution = conf.getInt(SCS_FONT, SCK_FONT_RESOLUTION_SIZE);
    if (m_fontResolution<1) {
        m_fontResolution = 1024;
	}
	fontFactor = (float) resolution / (float)m_fontResolution;

	std::string FontFileNamePlanet = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_PLANET_NAME);
	float FontSizePlanet = conf.getDouble(SCS_FONT,SCK_FONT_PLANET_SIZE);
    FontSizePlanet = round(FontSizePlanet * fontFactor);
	listFont.push_back(FontContener(CLASSEFONT::CLASS_SSYSTEM, FontSizePlanet, FontFileNamePlanet));

	std::string FontFileNameConstellation = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CONSTELLATION_NAME);
	float FontSizeConstellation = conf.getDouble(SCS_FONT,SCK_FONT_CONSTELLATION_SIZE);
    FontSizeConstellation = round(FontSizeConstellation * fontFactor);
	listFont.push_back(FontContener(CLASSEFONT::CLASS_ASTERIMS, FontSizeConstellation, FontFileNameConstellation) );

	std::string FontFileNameCardinalPoints = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CARDINALPOINTS_NAME);
	float FontSizeCardinalPoints = conf.getDouble(SCS_FONT,SCK_FONT_CARDINALPOINTS_SIZE);
    FontSizeCardinalPoints = round(FontSizeCardinalPoints * fontFactor) ;
	listFont.push_back(FontContener(CLASSEFONT::CLASS_CARDINALS, FontSizeCardinalPoints, FontFileNameCardinalPoints) );

	std::string FontFileNameHipStars = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_HIPSTARS_NAME);
	float FontSizeHipStars = conf.getDouble (SCS_FONT,SCK_FONT_HIPSTARS_SIZE);
	FontSizeHipStars = round(FontSizeHipStars * fontFactor);
	listFont.push_back(FontContener(CLASSEFONT::CLASS_HIPSTARS,FontSizeHipStars, FontFileNameHipStars) );

	std::string FontNameTuiMenu = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_MENU_NAME);
	float FontSizeTuiMenu   = conf.getDouble (SCS_FONT, SCK_FONT_MENUTUI_SIZE);
	FontSizeTuiMenu = round(FontSizeTuiMenu * fontFactor) ;
	listFont.push_back(FontContener(CLASSEFONT::CLASS_UI, FontSizeTuiMenu, FontNameTuiMenu) );

	std::string FontFileNameNebulas = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_NEBULAS_NAME);
	float FontSizeNebulas = conf.getDouble (SCS_FONT,SCK_FONT_NEBULAS_SIZE);
	FontSizeNebulas = round(FontSizeNebulas * fontFactor) ;
	listFont.push_back(FontContener(CLASSEFONT::CLASS_NEBULAE, FontSizeNebulas, FontFileNameNebulas) );

	std::string FontFileNameGrid = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GRID_NAME);
	float FontSizeGrid = conf.getDouble (SCS_FONT,SCK_FONT_GRID_SIZE);
	FontSizeGrid = round(FontSizeGrid * fontFactor) ;
	listFont.push_back(FontContener(CLASSEFONT::CLASS_SKYGRID, FontSizeGrid, FontFileNameGrid) );

	std::string FontFileNameLines = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_LINES_NAME);
	float FontSizeLines = conf.getDouble (SCS_FONT,SCK_FONT_LINE_SIZE);
	FontSizeLines = round(FontSizeLines * fontFactor) ;
	listFont.push_back(FontContener(CLASSEFONT::CLASS_SKYLINE, FontSizeLines, FontFileNameLines) );

	std::string FontFileNameDisplay = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_DISPLAY_NAME);
	float FontSizeDisplay = conf.getDouble (SCS_FONT,SCK_FONT_DISPLAY_SIZE);
	FontSizeDisplay = round(FontSizeDisplay * fontFactor) ;
	listFont.push_back(FontContener(CLASSEFONT::CLASS_SKYDISPLAY, FontSizeDisplay, FontFileNameDisplay) );

	FontFileNameText =  AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_TEXT_NAME);
    FontSizeText =  conf.getDouble(SCS_FONT, SCK_FONT_TEXT_SIZE);
    FontSizeText = round(FontSizeText * fontFactor) ;
	// TODO pourquoi cette instruction plante ?
	//	media->setTextFont(FontSizeText, FontFileNameText);
}


void FontFactory::updateFont(const std::string& targetName, const std::string& fontName, const std::string& sizeValue)
{
	// gestion de la taille
	double size = Utility::strToDouble(sizeValue) * fontFactor;
	if (size ==0.f)
		size=12.f;

	//gestion du module
	auto const it = m_strToTarget.find(targetName);
	if (it == m_strToTarget.end()) {
		cLog::get()->write("Unknown FontFactory target "+targetName, LOG_TYPE::L_WARNING);
		return;
	}

	// cas des texts
	if (it->second == CLASSEFONT::CLASS_MENU) {
		media->updateTextFont(size, fontName);
		return;
	}

	auto it2 = std::find_if( listFont.begin(), listFont.end(), [&](const FontContener &element){ return element.classeFont == it->second;} );
	if (it2 != std::end(listFont))
		it2->fontPtr->rebuild(size, fontName);
	else {
		//std::cout << "erreur updateFont " << targetName << " not found" << std::endl;
		cLog::get()->write("Error updating font : target "+targetName+ " not found", LOG_TYPE::L_WARNING);
		assert(0);
	}
}


s_font* FontFactory::registerFont(CLASSEFONT _cf)
{
	auto it = std::find_if( listFont.begin(), listFont.end(), [&](const FontContener &element){ return element.classeFont == _cf;} );
	if (it != std::end(listFont)) {
		return (*it).fontPtr.get();
	}	else {
		//std::cout << "erreur registerFont" << std::endl;
		cLog::get()->write("Error registerFont", LOG_TYPE::L_ERROR);
		assert(0);
		return nullptr;
	}
}


void FontFactory::reloadAllFont()
{
	std::for_each(listFont.begin(), listFont.end(), [](FontContener &n){ n.fontPtr->rebuild(n.sizeFont, n.nameFont); });
	media->resetTextFont();
}