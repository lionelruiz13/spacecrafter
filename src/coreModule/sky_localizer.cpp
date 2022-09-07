/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2004-2006 Robert Spearman
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

#include <iostream>
#include <fstream>
#include "coreModule/sky_localizer.hpp"
#include "tools/translator.hpp"
#include "tools/init_parser.hpp"
#include <cassert>
#include <filesystem>

SkyLocalizer::SkyLocalizer(const std::string& cultureDir)
{
	// struct dirent *entryp;
	// DIR *dp;
	//
	// if ((dp = opendir(cultureDir.c_str())) == NULL) {
	// 	std::cerr << "Unable to find culture directory:" << cultureDir << std::endl;
	// 	assert(0);
	// }
	//
	// while ((entryp = readdir(dp)) != NULL) {
	// 	std::string tmp = entryp->d_name;
	// 	std::string tmpfic = cultureDir+"/"+tmp+"/info.ini";
	// 	FILE* fic = fopen(tmpfic.c_str(), "r");
	// 	if (fic) {
	// 		InitParser conf;
	// 		conf.load(tmpfic);
	// 		dirToNameEnglish[tmp] = conf.getStr("info:name");
	// 		fclose(fic);
	// 	}
	// }
	// closedir(dp);

	for (const auto &entry : std::filesystem::directory_iterator{cultureDir}) {
		if (!entry.is_directory())
			continue;
		const std::filesystem::path configName = entry.path()/"info.ini";
		if (!std::filesystem::exists(configName))
			continue;
		InitParser conf;
		conf.load(entry.path().string());
		dirToNameEnglish[entry.path().filename().string()] = conf.getStr("info:name");
	}
}

SkyLocalizer::~SkyLocalizer()
{
}

//! returns newline delimited list of human readable culture names in english
std::string SkyLocalizer::getSkyCultureListEnglish()
{
	std::string cultures;
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {
		cultures += iter->second + "\n";
	}
	return cultures;
}

//! returns newline delimited list of human readable culture names translated to current locale
std::string SkyLocalizer::getSkyCultureListI18()
{
	std::string cultures;
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {
		if (iter != dirToNameEnglish.begin()) cultures += "\n";
		cultures += _(iter->second);
	}
	//wcout << cultures << endl;
	return cultures;
}

//! returns newline delimited hash of human readable culture names translated to current locale
//! and the directory names
std::string SkyLocalizer::getSkyCultureHash()
{
	std::string cultures;
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {

		// weed out invalid hash entries from invalid culture lookups in hash
		// TODO how to keep hash clean in the first place
		if (iter->second == "") continue;

		cultures += _(iter->second);
		cultures += std::string("\n") + iter->first + "\n";
	}
	// wcout << cultures << endl;
	return cultures;
}


std::string SkyLocalizer::directoryToSkyCultureEnglish(const std::string& directory)
{
	return dirToNameEnglish[directory];
}

std::string SkyLocalizer::directoryToSkyCultureI18(const std::string& directory)
{
	return _(dirToNameEnglish[directory]);
}

std::string SkyLocalizer::skyCultureToDirectory(const std::string& cultureName)
{
	for ( stringHashIter_t iter = dirToNameEnglish.begin(); iter != dirToNameEnglish.end(); ++iter ) {
		if (_(iter->second) == cultureName) return iter->first;
	}
	return "";
}
