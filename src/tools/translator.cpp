/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Fabien Chereau
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

#include <cassert>
#include <dirent.h>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "tools/translator.hpp"
#include "tools/app_settings.hpp"
#include "spacecrafter.hpp"


Translator* Translator::lastUsed = nullptr;

std::string Translator::systemLangName = "C";

// Use system locale language by default
Translator Translator::globalTranslator = Translator(PACKAGE, AppSettings::Instance()->getLocaleDir(), "system");

#ifdef WIN32
# include <Windows.h>
# include <Winnls.h>
#endif

Translator::Translator(const std::string& _domain, const std::string& _moDirectory, const std::string& _langName) :
	domain(_domain), moDirectory(_moDirectory), langName(_langName)
{
	Translator::lastUsed = nullptr;
}

//! Try to determine system language from system configuration
void Translator::initSystemLanguage()
{
	char* lang = getenv("LANGUAGE");
	if (lang) Translator::systemLangName = lang;
	else {
		lang = getenv("LANG");
		if (lang) Translator::systemLangName = lang;
		else {
			#ifdef WIN32
			char cc[3];
			if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, cc, 3)) {
				cc[2] = '\0';
				Translator::systemLangName = cc;
			} else {
				Translator::systemLangName = "C";
			}
			#else
			Translator::systemLangName = "C";
			#endif
		}
	}
}

void Translator::reload()
{
	if (Translator::lastUsed == this) return;
	// This needs to be static as it is used a each gettext call... It tooks me quite a while before I got that :(
	static char envstr[25];
	#ifndef MACOSX
	if (langName=="system" || langName=="system_default") {
		snprintf(envstr, 25, "LANGUAGE=%s", Translator::systemLangName.c_str());
		// cout << "ENV=" << envstr << " " << Translator::systemLangName << endl;
	} else {
		snprintf(envstr, 25, "LANGUAGE=%s", langName.c_str());
	}
	#else
	if (langName=="system" || langName=="system_default") {
		snprintf(envstr, 25, "LANG=%s", Translator::systemLangName.c_str());
	} else {
		snprintf(envstr, 25, "LANG=%s", langName.c_str());
	}
	#endif
	//printf("Setting locale: %s\n", envstr);
	putenv(envstr);
	#ifdef LINUX
	setlocale(LC_MESSAGES, "");
	#endif
	std::string result = bind_textdomain_codeset(domain.c_str(), "UTF-8");
	assert(result=="UTF-8");
	bindtextdomain (domain.c_str(), moDirectory.c_str());
	textdomain (domain.c_str());
	Translator::lastUsed = this;
}

//! Get available language codes from directory tree
std::string Translator::getAvailableLanguagesCodes(const std::string& localeDir)
{
	struct dirent *entryp;
	DIR *dp;
	std::vector<std::string> result;

	//cout << "Reading translations in directory: " << localeDir << endl;
	if ((dp = opendir(localeDir.c_str())) == NULL) {
		std::cerr << "Unable to find locale directory containing translations:" << localeDir << std::endl;
		return "";
	}

	while ((entryp = readdir(dp)) != NULL) {
		std::string tmp = entryp->d_name;
		std::string tmpdir = localeDir+"/"+tmp+"/LC_MESSAGES/" + APP_LOWER_NAME + ".mo";
		FILE* fic = fopen(tmpdir.c_str(), "r");
		if (fic) {
			result.push_back(tmp);
			fclose(fic);
		}
	}
	closedir(dp);

	// Sort the language names by alphabetic order
	std::sort(result.begin(), result.end());

	std::string output;
	std::vector<std::string>::iterator iter;
	for (iter=result.begin(); iter!=result.end(); ++iter) {
		if (iter!=result.begin()) output+="\n";
		output+=*iter;
	}
	return output;
}

