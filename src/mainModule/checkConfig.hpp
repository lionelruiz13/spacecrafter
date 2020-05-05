/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

// Class which parses an init file
// C++ warper for the iniparser free library from N.Devillard

#ifndef _CHECKKEYS_HPP_
#define _CHECKKEYS_HPP_

#include <string>
#include <map>
#include "tools/init_parser.hpp"

class CheckConfig {
public:
    CheckConfig();
    ~CheckConfig();
    void checkConfigIni(const std::string &fullpathfile, const std::string &_VERSION);

private:
	void insertKeyFromTmpSettings(std::string nameSection);

	void checkMainSettings();
	void checkIoSettings();
	void checkVideoSettings();
	void checkRenderingSettings();
	void checkLocalizationSettings();
	void checkStarSettings();
	void checkGuiSettings();
	void checkFontSettings();
	void checkTuiSettings();
	void checkLandscapeSettings();
	void checkColorSettings();
	void checkViewingSettings();
	void checkNavigationSettings();
	void checkAstroSettings();
	void checkLocationSettings();

    InitParser user_conf;
    std::list<std::string> sectionSettings;
	std::list<std::string> sectionKeySettings;
	std::map<std::string,std::string> tmpSettings;
};



#endif // _CHECKKEYS_HPP_
