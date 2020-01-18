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

#include "spacecrafter.hpp"
#include "tools/app_settings.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"


// Stub implementations of functions that don't exist on windows and visa versa
#ifdef WIN32
time_t timegm(struct tm*)
{
	time_t t;
	memset( &t, 0, sizeof(t) );
	return t;
}
#else
int GetTimeZoneInformation( TIME_ZONE_INFORMATION* info )
{
	return 0;
}
#endif

AppSettings* AppSettings::m_instance = nullptr;

AppSettings::AppSettings( const std::string &configDir, const std::string &dataRoot, const std::string &lDir ) : m_configDir(configDir),
	m_dataRoot(dataRoot),
	m_LocaleDir(lDir)
{
}

void AppSettings::Init(const std::string &configDir, const std::string &dataRoot, const std::string &lDir )
{
	if( m_instance )
		delete m_instance;
	m_instance = new AppSettings( configDir, dataRoot, lDir );
}

AppSettings* AppSettings::Instance()
{
	if( !m_instance )
		return new AppSettings( "", "", "" );
	else
		return m_instance;
}

void AppSettings::close() {
	if (m_instance != nullptr) {
		delete m_instance;
	}
	m_instance = nullptr;
}

bool AppSettings::OSX() const
{
	#if defined(MACOSX)
	return true;
	#else
	return false;
	#endif
}

bool AppSettings::Windows() const
{
	#ifdef WIN32
	return true;
	#else
	return false;
	#endif
}

bool AppSettings::Unix() const
{
	#ifdef LINUX
	return true;
	#else
	return false;
	#endif
}

void AppSettings::loadAppSettings( InitParser* const conf ) const
{
	conf->load(m_configDir + "config.ini");
}

const std::string AppSettings::getConfigDir() const
{
	return m_configDir;
}

const std::string AppSettings::getConfigFile() const
{
	return getConfigDir() + "config.ini";
}

const std::string AppSettings::getLocaleDir() const
{
	return m_LocaleDir;
}

const std::string AppSettings::getDataRoot() const
{
	return m_dataRoot;
}

const std::string AppSettings::getDataDir() const
{
	#ifdef LINUX
	return m_dataRoot+"data/";
	#else
	return m_dataRoot+"data\\";
	#endif
}

const std::string AppSettings::getShaderDir() const
{
	#ifdef LINUX
	return m_dataRoot+"shaders/";
	#else
	return m_dataRoot+"shaders\\";
	#endif
}

const std::string AppSettings::getModel3DDir() const
{
	#ifdef LINUX
	return getUserDir()+REP_MODEL3D+"/";
	#else
	return getUserDir()+REP_MODEL3D+"\\";
	#endif
}

const std::string AppSettings::getPictureDir() const
{
	#ifdef LINUX
	return getUserDir()+"pictures/";
	#else
	return getUserDir()+"pictures\\";
	#endif
}

const std::string AppSettings::getTextureDir() const
{
	#ifdef LINUX
	return getUserDir()+"textures/";
	#else
	return getUserDir()+"textures\\";
	#endif
}

const std::string AppSettings::getUserFontDir() const
{
	#ifdef LINUX
	return getUserDir()  + REP_FONT+"/";
	#else
	return getUserDir()  + REP_FONT+"\\";
	#endif
}

const std::string AppSettings::getSkyCultureDir() const
{
	#ifdef LINUX
	return getUserDir()  + REP_SKY_CULTURE+"/";
	#else
	return getUserDir()  + REP_SKY_CULTURE+"\\";
	#endif
}

const std::string AppSettings::getScreenshotDirectory() const
{
	#ifdef LINUX
	return getUserDir()  +REP_SCREENSHOT+"/";
	#else
	return getUserDir()  +REP_SCREENSHOT+"\\";
	#endif
}

const std::string AppSettings::getVframeDirectory() const
{
	#ifdef LINUX
	return getUserDir()  + REP_VFRAME + "/";
	#else
	return getUserDir()  + REP_VFRAME + "\\";
	#endif
}

const std::string AppSettings::getScriptDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_SCRIPT + "/";
	#else
	return getUserDir()  + REP_SCRIPT + "\\";
	#endif

}

const std::string AppSettings::getAudioDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_AUDIO + "/";
	#else
	return getUserDir()  + REP_AUDIO + "\\";
	#endif
}

const std::string AppSettings::getVideoDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_VIDEO + "/";
	#else
	return getUserDir()  + REP_VIDEO + "\\";
	#endif
}

const std::string AppSettings::getMediaDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_MEDIA + "/";
	#else
	return getUserDir()  + REP_MEDIA + "\\";
	#endif
}

const std::string AppSettings::getVR360Dir() const
{
	#ifdef LINUX
	return getUserDir() + REP_VR360 + "/";
	#else
	return getUserDir()  + REP_VR360 + "\\";
	#endif
}

const std::string AppSettings::getFtpDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_FTP + "/";
	#else
	return getUserDir()  + REP_FTP + "\\";
	#endif
}

const std::string AppSettings::getLogDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_LOG + "/";
	#else
	return getUserDir()  + REP_LOG + "\\";
	#endif

}

const std::string AppSettings::getWebDir() const
{
	return getUserDir() + REP_WEB;
}

const std::string AppSettings::getLandscapeDir() const
{
	#ifdef LINUX
	return getUserDir() + REP_LANDSCAPE + "/";
	#else
	return getUserDir()  + REP_LANDSCAPE + "\\";
	#endif

}

const std::string AppSettings::getUserDir() const
{
	#ifdef LINUX
	std::string homeDir = getenv("HOME");
	std::string CDIR = homeDir + "/." + APP_LOWER_NAME + "/";
	return CDIR;
	#else
	return m_configDir;
	#endif
}

void AppSettings::display_all() const
{
	std::cout << "------------------------------------------------------" << std::endl;
	std::cout << "getConfigDir " << getConfigDir() << std::endl;
	std::cout << "getConfigFile " << getConfigFile() << std::endl;
	std::cout << "getLocaleDir " << getLocaleDir() << std::endl;
	std::cout << "getDataDir " << getDataDir() << std::endl;
	std::cout << "getDataRoot " << getDataRoot() << std::endl;
	std::cout << "getUserDir " << getUserDir() << std::endl;
	std::cout << "getLandscapeDir " << getLandscapeDir() << std::endl;
	std::cout << "getWebDir " << getWebDir() << std::endl;
	std::cout << "getLogDir " << getLogDir() << std::endl;
	std::cout << "getFtpDir " << getFtpDir() << std::endl;
	std::cout << "getVideoDir " << getVideoDir() << std::endl;
	std::cout << "getAudioDir " << getAudioDir() << std::endl;
	std::cout << "getScriptDir " << getScriptDir() << std::endl;
	std::cout << "getVframeDirectory " << getVframeDirectory() << std::endl;
	std::cout << "getScreenshotDirectory " << getScreenshotDirectory() << std::endl;
	std::cout << "getUserFontDir " << getUserFontDir() << std::endl;
	std::cout << "getTextureDir " << getTextureDir() << std::endl;
	std::cout << "getPictureDir " << getPictureDir() << std::endl;
	std::cout << "getShaderDir " << getShaderDir() << std::endl;
	std::cout << "getModel3DDir " << getModel3DDir() << std::endl;
	std::cout << "------------------------------------------------------" << std::endl;
}
