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

FontFactory::FontFactory(int _resolution)
{
	setStrToTarget();
	m_resolution = _resolution;
}

void FontFactory::setStrToTarget()
{
    m_strToTarget[TF_TEXT] = TARGETFONT::CF_TEXTS;
	m_strToTarget[TF_PLANETS] = TARGETFONT::CF_PLANETS;
	m_strToTarget[TF_CONSTELLATIONS] = TARGETFONT::CF_CONSTELLATIONS;
	m_strToTarget[TF_CARDINAL] = TARGETFONT::CF_CARDINALS;
	m_strToTarget[TF_STARS] = TARGETFONT::CF_HIPSTARS;
	m_strToTarget[TF_MENU] = TARGETFONT::CF_UIMENU;
	m_strToTarget[TF_GENERAL] = TARGETFONT::CF_GENERAL;
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
	std::cout << "debut construction des fonts" << std::endl;

	// cas des objets de Core
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_ASTERIMS, std::make_unique<s_font>(FontSizeConstellation, FontFileNameConstellation)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_SSYSTEM, std::make_unique<s_font>(FontSizePlanet, FontFileNamePlanet)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_SKYDISPLAY, std::make_unique<s_font>(FontSizeDisplay, FontFileNameDisplay)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_CARDINALS, std::make_unique<s_font>(FontSizeCardinalPoints, FontFileNameCardinalPoints)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_HIPSTARS, std::make_unique<s_font>(FontSizeHipStars, FontFileNameHipStars)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_NEBULAS, std::make_unique<s_font>(FontSizeNebulas, FontFileNameNebulas)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_SKYGRID, std::make_unique<s_font>(FontSizeGrid, FontFileNameGrid)) );
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_SKYLINE, std::make_unique<s_font>(FontSizeLines, FontFileNameLines)) );

	// cas de Ui
	listFont.push_back( std::make_pair( CLASSEFONT::CLASS_UI, std::make_unique<s_font>(FontSizeTuiMenu, FontNameTuiMenu)) );

	//cas spécial de Media
	media->setTextFont(FontSizeText, FontFileNameText);
	std::cout << "fin construction des fonts" << std::endl;
}

void FontFactory::init(const InitParser& conf)
{
	FontFileNameGeneral = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GENERAL_NAME);
	FontFileNamePlanet = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_PLANET_NAME);
	FontFileNameConstellation = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CONSTELLATION_NAME);
	FontFileNameDisplay = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_DISPLAY_NAME);
	FontFileNameCardinalPoints = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CARDINALPOINTS_NAME);
	FontFileNameHipStars = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_HIPSTARS_NAME);
	FontFileNameNebulas = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_NEBULAS_NAME);
	FontFileNameGrid = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GRID_NAME);
	FontFileNameLines = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_LINES_NAME);
	FontFileNameText =  AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_TEXT_NAME);

	FontNameTuiMenu = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_MENU_NAME);

	FontSizeGeneral = conf.getDouble (SCS_FONT,SCK_FONT_GENERAL_SIZE);
	FontSizeConstellation = conf.getDouble(SCS_FONT,SCK_FONT_CONSTELLATION_SIZE);
	FontSizePlanet = conf.getDouble(SCS_FONT,SCK_FONT_PLANET_SIZE);
	FontSizeCardinalPoints = conf.getDouble(SCS_FONT,SCK_FONT_CARDINALPOINTS_SIZE);
	FontSizeGrid = conf.getDouble (SCS_FONT,SCK_FONT_GRID_SIZE);
	FontSizeLines = conf.getDouble (SCS_FONT,SCK_FONT_LINE_SIZE);
	FontSizeDisplay = conf.getDouble (SCS_FONT,SCK_FONT_DISPLAY_SIZE);
	FontSizeHipStars = conf.getDouble (SCS_FONT,SCK_FONT_HIPSTARS_SIZE);
	FontSizeNebulas = conf.getDouble (SCS_FONT,SCK_FONT_NEBULAS_SIZE);

    FontSizeText =  conf.getDouble(SCS_FONT, SCK_FONT_TEXT_SIZE);

	FontSizeTuiMenu   = conf.getDouble (SCS_FONT, SCK_FONT_MENUTUI_SIZE);

	m_fontResolution = conf.getDouble(SCS_FONT, SCK_FONT_RESOLUTION_SIZE);
    if (m_fontResolution <1.0)
        m_fontResolution = 1024.0;

    // mise à l'échelle des FontSize
    FontSizeText = round(FontSizeText * m_resolution / m_fontResolution) ;
    FontSizeGeneral = round(FontSizeGeneral * m_resolution / m_fontResolution) ;
    FontSizeConstellation = round(FontSizeConstellation * m_resolution / m_fontResolution) ;
    FontSizePlanet = round(FontSizePlanet * m_resolution / m_fontResolution) ;
    FontSizeCardinalPoints = round(FontSizeCardinalPoints * m_resolution / m_fontResolution) ;
	FontSizeTuiMenu = round(FontSizeTuiMenu * m_resolution / m_fontResolution) ;
}


void FontFactory::updateFont(const std::string& targetName, const std::string& fontName, const std::string& sizeValue)
{
	// gestion de la taille
	double size = Utility::strToDouble(sizeValue) * m_resolution / m_fontResolution;

	//gestion du module
	auto const it = m_strToTarget.find(targetName);
	if (it == m_strToTarget.end()) {
		cLog::get()->write("Unknown FontFactory target "+targetName, LOG_TYPE::L_WARNING);
		return;
	}

	switch(it->second) {
		// Media
		case TARGETFONT::CF_TEXTS :
			media->setTextFont(size==0 ? FontSizeText : size, fontName );
			break;
		// Core
		case TARGETFONT::CF_PLANETS :
			// ssystem->setFont(size==0 ? FontSizePlanet : size, fontName );
			break;
		case TARGETFONT::CF_CONSTELLATIONS :
			// asterisms->setFont(size==0 ? FontSizeConstellation : size, fontName );
			break;
		case TARGETFONT::CF_CARDINALS :
			// cardinals_points->setFont(size==0 ? FontSizeCardinalPoints : size, fontName );
			break;
		case TARGETFONT::CF_HIPSTARS :
			// hip_stars->setFont(size==0 ? FontSizeGeneral : size, fontName );
			break;
		case TARGETFONT::CF_UIMENU: {
			auto it = std::find_if( listFont.begin(), listFont.end(),
    			[&](const pairNameFontPtr &element){ return element.first == CLASSEFONT::CLASS_UI;} );
			(*it).second->rebuild(size==0 ? FontSizeText : size, fontName);
			}
			break;
		case TARGETFONT::CF_GENERAL :
			break;
		case TARGETFONT::CF_NONE:
			break;
	}
}


s_font*  FontFactory::registerFont(CLASSEFONT _cf)
{
	auto it = std::find_if( listFont.begin(), listFont.end(), [&](const pairNameFontPtr &element){ return element.first == _cf;} );
	return (*it).second.get();
}


void FontFactory::reloadAllFont()
{
	
}