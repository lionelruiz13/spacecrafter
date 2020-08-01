/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2004 Robert Spearman, 2005 Fabien Chereau
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

#ifndef _SKY_LOCALIZER_H_
#define _SKY_LOCALIZER_H_

#include <cstdio>
#include <string>
#include "tools/utility.hpp"

class SkyLocalizer {

public:
	SkyLocalizer(const std::string& cultureDir);
	virtual ~SkyLocalizer();

	//! returns newline delimited list of human readable culture names in english
	std::string getSkyCultureListEnglish();

	//! returns newline delimited list of human readable culture names translated to current language
	std::string getSkyCultureListI18();

	//! returns newline delimited hash of translated culture names and directories
	std::string getSkyCultureHash();

	//! Get the culture name in english associated to the passed directory
	std::string directoryToSkyCultureEnglish(const std::string& directory);

	//! Get the culture name translated to current language associated to the passed directory
	std::string directoryToSkyCultureI18(const std::string& directory);

	//! Get the directory associated to the passed translated culture name
	std::string skyCultureToDirectory(const std::string& cultureName);

private:
	stringHash_t dirToNameEnglish;
};


#endif // _SKY_LOCALIZER_H_
