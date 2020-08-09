/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Author: Trystan Larey-Williams
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

/**
 * @file app_setting.h
 * 
 * @class AppSettings
 * 
 * @brief Provides a single point of access to common application settings such as data
 * and configuration directories. The singleton is initialized in main.cpp and
 * is henceforth available from anywhere via the Instance method.
 *
 */

#include <string>
#include "tools/init_parser.hpp"
#include <time.h>
#include "spacecrafter.hpp"
#include "tools/no_copy.hpp"

#ifndef _APPSETTINGS_HPP_
#define _APPSETTINGS_HPP_

class AppSettings: public NoCopy {

public:
	static AppSettings* Instance();
	static void Init(const std::string &, const std::string &, const std::string &);
	static void close();

	//! Obtains config.ini settings. Caller must allocate InitParser.
	void loadAppSettings( InitParser* const ) const;

	//! renvie le fichier de configuration du logiciel
	const std::string getConfigFile() const;

	//! renvoie le répertoire de configuration
	const std::string getConfigDir() const;

	//! renvoie le nom du répertoire des données
	const std::string getDataRoot() const;

	//! Get the name of the directory containing the data
	const std::string getDataDir() const;

	//! renvoie le répertoire contenant les locales du logiciel
	const std::string getLocaleDir() const;

	//! Get the fullname of the directory containing the fonts user
	const std::string getUserFontDir() const;

	//! Get the name of the local script directory
	const std::string getScriptDir() const;

	//! Get the fullname of the directory containing the data user
	const std::string getUserDir() const;

	//! Get the fullname of the directory containing all SkyCulture
	const std::string getSkyCultureDir() const;

	//! Get the fullname of the directory containing the log
	const std::string getLogDir() const;

	//! Get the fullname of the directory containing the web server
	const std::string getWebDir() const;

	//! Get the fullname of the directory containing the landscape user
	const std::string getLandscapeDir() const;

	//! Get the fullname of the directory containing the audio user
	const std::string getAudioDir() const;

	//! Get the fullname of the directory containing the videos user
	const std::string getVideoDir() const;

	//! Get the fullname of the directory containing the media VR user
	const std::string getMediaDir() const;

	//! Get the fullname of the directory containing the ftp user
	const std::string getFtpDir() const;

	//! Get the fullname of the directory containing the Shader program
	const std::string getShaderDir() const;

	//! Get the fullname of the directory containing the Model3D user
	const std::string getModel3DDir() const;

	//! Get the fullname of the directory containing the VR360 user
	const std::string getVR360Dir() const;

	//! Get the fullname of the directory containing the picture user
	const std::string getPictureDir() const;

	//! Get the fullname of the directory containing the Texture user
	const std::string getTextureDir() const;

	//! Get the fullname of the directory containing all translation text
	const std::string getLanguageDir() const;

	//! Determine where screenshot files should go on different platforms
	const std::string getScreenshotDirectory() const;

	//! Determine where vframes files should go on different platforms
	const std::string getVframeDirectory() const;

	//! Indique de quel est la version de Spacecrafter
	const std::string getVersion() const {
		return VERSION;
	}

	//! Console output of all methods of the class
	void display_all() const;

private:
	AppSettings();
	~AppSettings(){};
	AppSettings( const std::string &, const std::string &, const std::string &);


	static AppSettings* m_instance;
	const std::string m_configDir;
	const std::string m_dataRoot;
	const std::string m_LocaleDir;
};

#endif // APP_SETTINGS__