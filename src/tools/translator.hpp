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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <string>
#include <iostream>
#include <cerrno>

// These macro are used as global function replacing standard gettext operation
#include "tools/gettext.hpp"
#define _(String) Translator::globalTranslator.translateUTF8( gettextNoop(String) ).c_str()
#define N_(String) gettextNoop(String)



//! Class used to translate strings to any language.
//! Implements a nice interface to gettext which is UTF-8 compliant and is somewhat multiplateform
//! All its operations do not modify the global locale.
//! The purpose of this class is to remove all non-OO C locale functions
//! It could be extended for all locale management using e.g the portable IBM ICU library.
//! @author Fabien Chereau
class Translator {
public:

	//! @brief Create a translator from a language name.
	//! If the passed locale name cannot be handled by the system, default value will be used.
	//! The passed language name is a language code string like "fr" or "fr_FR".
	//! This class wrap gettext to simulate an object oriented multiplateform gettext UTF8 translator
	//! @param _domain The name of the domain to use for translation
	//! @param _moDirectory The directory where to look for the domain.mo translation files.
	//! @param _langName The C locale name or language name like "fr" or "fr_FR". If string is "" or "system" it will use the system locale.
	Translator(const std::string& _domain, const std::string& _moDirectory, const std::string& _langName);

	//! @brief Translate input message.
	//! @param s input string in english.
	//! @return The translated string in UTF-8 characters.
	std::string translateUTF8(const std::string& s) {
		reload();
		return gettext(s.c_str());
	}

	//! @brief Get translator locale name. This name can be used to create a translator.
	//! Could be artificial "system" value for config file
	//! @return Locale name e.g "fr_FR"
	std::string getLocaleName() {
		return langName;
	}

	//! Used as a global translator by the whole app
	static Translator globalTranslator;

	//! Get available language codes from passed locales directory
	static std::string getAvailableLanguagesCodes(const std::string& localeDir);

	//! Try to determine system language from system configuration
	static void initSystemLanguage();

private:
	//! Reload the current locale info so that gettext use them
	void reload();

	//! The domain name
	std::string domain;

	//! The directory where the locale file tree stands
	std::string moDirectory;

	//! The two letter string defining the current language name
	std::string langName;

	//! Keep in memory which one was the last used transator to prevent reloading it at each tranlate() call
	static Translator* lastUsed;

	//! Store the system default language name as taken from LANGUAGE environement variable
	static std::string systemLangName;
};

#endif
