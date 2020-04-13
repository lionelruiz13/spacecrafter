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
#include "coreModule/core.hpp"
#include "tools/init_parser.hpp"
#include "tools/app_settings.hpp"

CoreFont::CoreFont(Core* _core, int _resolution)
{
    core= _core;
    resolution = _resolution;
}

CoreFont::~CoreFont()
{}

void CoreFont::setFont()
{
    core->hip_stars->setFont(FontSizeGeneral, FontFileNameGeneral);
	core->nebulas->setFont(FontSizeGeneral, FontFileNameGeneral);
	core->ssystem->setFont(FontSizePlanet, FontFileNamePlanet);
	core->skyGridMgr->setFont(FontSizeGeneral, FontFileNameGeneral);
	core->skyLineMgr->setFont(FontSizeGeneral, FontFileNameGeneral);
	core->skyDisplayMgr->setFont(FontSizePlanet, FontFileNamePlanet);
	core->cardinals_points->setFont(FontSizeCardinalPoints, FontFileNameGeneral);
	core->asterisms->setFont(FontSizeConstellation, FontFileNameConstellation);
	core->text_usr->setFont(FontSizeText, FontFileNameText);
}

void CoreFont::init(const InitParser& conf)
{
	FontFileNameGeneral = AppSettings::Instance()->getUserFontDir()+conf.getStr("font", "font_general_name");
	FontFileNamePlanet = AppSettings::Instance()->getUserFontDir()+conf.getStr("font", "font_planet_name");
	FontFileNameConstellation = AppSettings::Instance()->getUserFontDir()+conf.getStr("font", "font_constellation_name");
	FontFileNameText =  AppSettings::Instance()->getUserFontDir()+conf.getStr("font", "font_text_name");

	double FontResolution = conf.getDouble("font", "font_resolution_size");
    if (FontResolution <1.0)
        FontResolution = 1024.0;
    FontSizeText =  conf.getDouble("font", "font_text_size");
	FontSizeGeneral = conf.getDouble ("font","font_general_size");
	FontSizeConstellation = conf.getDouble("font","font_constellation_size");
	FontSizePlanet = conf.getDouble("font","font_planet_size");
	FontSizeCardinalPoints = conf.getDouble("font","font_cardinalpoints_size");

    // mise à l'échelle des FontSize
    FontSizeText = round(FontSizeText * resolution / FontResolution) ;
    FontSizeGeneral = round(FontSizeGeneral * resolution / FontResolution) ;
    FontSizeConstellation = round(FontSizeConstellation * resolution / FontResolution) ;
    FontSizePlanet = round(FontSizePlanet * resolution / FontResolution) ;
    FontSizeCardinalPoints = round(FontSizeCardinalPoints * resolution / FontResolution) ;
}
