/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2020 Association Sirius
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
#include <cstdio>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <filesystem>
#include "tools/translator.hpp"
#include "tools/app_settings.hpp"


Translator* Translator::lastUsed = nullptr;
//std::string Translator::systemLangName = "C";
std::map<std::string, std::string> Translator::m_translator;

// Use system locale language by default
Translator Translator::globalTranslator = Translator(AppSettings::Instance()->getLocaleDir(), "system");

const std::string _(const std::string& t){
	return Translator::globalTranslator.translateUTF8(t);
}

Translator::Translator(const std::string& _moDirectory, const std::string& _langName) :
	moDirectory(_moDirectory), langName(_langName)
{
	Translator::lastUsed = nullptr;
	reload();
}

//! Try to determine system language from system configuration
// void Translator::initSystemLanguage()
// {
// 	char* lang = getenv("LANGUAGE");
// 	if (lang) Translator::systemLangName = lang;
// 	else {
// 		lang = getenv("LANG");
// 		if (lang) Translator::systemLangName = lang;
// 		else {
// 			#ifdef WIN32
// 			char cc[3];
// 			if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, cc, 3)) {
// 				cc[2] = '\0';
// 				Translator::systemLangName = cc;
// 			} else {
// 				Translator::systemLangName = "C";
// 			}
// 			#else
// 			Translator::systemLangName = "C";
// 			#endif
// 		}
// 	}
// }

void Translator::reload()
{
	if (Translator::lastUsed == this) return;

	//load all string in m_translator, so clear existing
	m_translator.clear();

	//read translation file
	std::ifstream infile;
	infile.open(moDirectory+langName+".txt", std::ifstream::in);
	//std::cout << "translation file : " << moDirectory << langName <<".txt"<<std::endl;
	if (infile.is_open()) {
		std::string line;
		std::string key;
		std::string value;
		std::size_t found;
		while (std::getline(infile, line))
		{
			if (line[line.length() - 1] == '\r')
				line.pop_back();
        	if (line[0] != '#' && line[0] != '\r' && line[0] != '\n' ) {
				found = line.find("\";\"");
				if (found != std::string::npos ) {
					key = line.substr(1, found-1);
					value = line.substr(found+3, line.length()-(found+4) );
                    //std::cout << key << "<->" << value << std::endl;
					if (!value.empty())
                    	m_translator[key] = value;
				}
        	}
		}
		infile.close();
	}

	// DEBUG
 	// for (auto& x: m_translator) {
    // 	std::cout << x.first << ": " << x.second << '\n';
  	// }
	Translator::lastUsed = this;
}

//! Get available language codes from directory tree
std::string Translator::getAvailableLanguagesCodes(const std::string& localeDir)
{
	std::error_code ec;
	std::set<std::string> result; // So results are sorted

	for (const auto &entry : std::filesystem::directory_iterator(localeDir, ec)) {
		if (entry.is_regular_file() && entry.path().extension() == ".txt") {
			result.insert(entry.path().stem().string());
		}
	}

	if (ec) {
		std::cerr << "Unable to find locale directory containing translations:" << localeDir << std::endl;
		return {};
	}

	std::string output;
	for (auto &str : result) {
		output += str;
		output.push_back('\n');
	}
	output.pop_back(); // Remove the '\n' at the end of the list
	return output;
}

const std::string Translator::translateUTF8(const std::string& s)
{
	auto it = m_translator.find(s);
 	if (it != m_translator.end())
    	return m_translator[s];
	else
		return s;
}
