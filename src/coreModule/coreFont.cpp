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

#include "coreModule/coreFont.hpp"
//#include "coreModule/core.hpp"
#include "tools/init_parser.hpp"
#include "tools/app_settings.hpp"
#include "tools/file_path.hpp"
#include "tools/log.hpp"

#include "coreModule/constellation_mgr.hpp"
#include "bodyModule/solarsystem.hpp"
#include "coreModule/backup_mgr.hpp"
#include "coreModule/cardinals.hpp"
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skygrid.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skyline.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/skyDisplay.hpp"
#include "mediaModule/text_mgr.hpp"
#include "mainModule/define_key.hpp"
#include "starModule/hip_star_mgr.hpp"


CoreFont::CoreFont(/*Core* core,*/ int _resolution)
{
    //core= _core;
    resolution = _resolution;
}

CoreFont::~CoreFont()
{}

void CoreFont::setFont()
{
    hip_stars->setFont(FontSizeGeneral, FontFileNameGeneral);
	nebulas->setFont(FontSizeGeneral, FontFileNameGeneral);
	ssystem->setFont(FontSizePlanet, FontFileNamePlanet);
	skyGridMgr->setFont(FontSizeGeneral, FontFileNameGeneral);
	skyLineMgr->setFont(FontSizeGeneral, FontFileNameGeneral);
	skyDisplayMgr->setFont(FontSizePlanet, FontFileNamePlanet);
	cardinals_points->setFont(FontSizeCardinalPoints, FontFileNameGeneral);
	asterisms->setFont(FontSizeConstellation, FontFileNameConstellation);
	text_usr->setFont(FontSizeText, FontFileNameText);
}

void CoreFont::init(const InitParser& conf)
{
	FontFileNameGeneral = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_GENERAL_NAME);
	FontFileNamePlanet = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_PLANET_NAME);
	FontFileNameConstellation = AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_CONSTELLATION_NAME);
	FontFileNameText =  AppSettings::Instance()->getUserFontDir()+conf.getStr(SCS_FONT, SCK_FONT_TEXT_NAME);

	double FontResolution = conf.getDouble(SCS_FONT, SCK_FONT_RESOLUTION_SIZE);
    if (FontResolution <1.0)
        FontResolution = 1024.0;
    FontSizeText =  conf.getDouble(SCS_FONT, SCK_FONT_TEXT_SIZE);
	FontSizeGeneral = conf.getDouble (SCS_FONT,SCK_FONT_GENERAL_SIZE);
	FontSizeConstellation = conf.getDouble(SCS_FONT,SCK_FONT_CONSTELLATION_SIZE);
	FontSizePlanet = conf.getDouble(SCS_FONT,SCK_FONT_PLANET_SIZE);
	FontSizeCardinalPoints = conf.getDouble(SCS_FONT,SCK_FONT_CARDINALPOINTS_SIZE);

    // mise à l'échelle des FontSize
    FontSizeText = round(FontSizeText * resolution / FontResolution) ;
    FontSizeGeneral = round(FontSizeGeneral * resolution / FontResolution) ;
    FontSizeConstellation = round(FontSizeConstellation * resolution / FontResolution) ;
    FontSizePlanet = round(FontSizePlanet * resolution / FontResolution) ;
    FontSizeCardinalPoints = round(FontSizeCardinalPoints * resolution / FontResolution) ;
}

void CoreFont::updateFont(const std::string& moduleName, const std::string& fontName, const std::string& sizeValue);
{
	FilePath myFile  = FilePath(fontName, FilePath::TFP::FONTS);
	if (!myFile.exist()) {
		cLog::get()->write("Unable to find "+ fontName, LOG_TYPE::L_WARNING);
		return;
	}

}

/*
if (args[W_ALL] != "") setFont(W_ALL, &Core::loadFont);
		FilePath myFile  = FilePath(args[W_FILENAME], FilePath::TFP::FONTS);
	else if (args[ACP_CN_TEXT] != "") setFont(ACP_CN_TEXT, &Core::setFontText);
			if (myFile) {
	else if (args[W_PLANET] != "") setFont(W_PLANET, &Core::setFontPlanets);
				int size = 10;
	else if (args[W_CONSTELLATION] != "") setFont(W_CONSTELLATION, &Core::setFontConstellations);
				if (!args[W_SIZE].empty())
	else if (args[ACP_FN_CARDINAL_POINTS] != "") setFont(ACP_FN_CARDINAL_POINTS, &Core::setFontCardinalPoints);
					size = evalInt(args[W_SIZE]);
	else if (args[ACP_FN_STARS] != "") setFont(ACP_FN_STARS, &Core::setFontStars);

	*/