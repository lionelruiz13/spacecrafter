/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 of Association Sirius
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

#include "tools/file_path.hpp"
//#include "tools/utility.hpp"
#include "tools/call_system.hpp"
#include "tools/app_settings.hpp"

std::string FilePath::scriptPath;

FilePath::FilePath(const std::string& fileName)
{
	FilePath(fileName, TFP::NONE);
}

FilePath::FilePath(const std::string& fileName, const std::string& localisation)
{
	if (fileName.empty())
		return;

	//~ printf("FilePath solicited on internationalisation\n");
	//~ printf("FilePath langue %s\n", localisation.c_str());

	if (scriptPath.empty()) {
		//~ printf("FilePath scriptPath not initialized\n");
		return;
	}

	std::string fileNameAdapted = fileName;
	fileNameAdapted[fileNameAdapted.size()-6]=localisation[0];
	fileNameAdapted[fileNameAdapted.size()-5]=localisation[1];
	//~ printf("FilePath internationalisation work on %s\n", fileNameAdapted.c_str());

	if ( !CallSystem::isAbsolute(fileName)) {
		fullFileName = scriptPath+fileNameAdapted;
		// localisation in scriptPath
		isFileExist = CallSystem::fileExist(fullFileName);
		if (isFileExist)
			return;

		fullFileName = AppSettings::Instance()->getMediaDir() + fileNameAdapted;
		// localisation in media
		isFileExist = CallSystem::fileExist(fullFileName);
		if (isFileExist)
			return;

		//localisation non existante
		fullFileName = scriptPath+fileName;
		// localisation in scriptPath
		isFileExist = CallSystem::fileExist(fullFileName);
		if (isFileExist)
			return;

		fullFileName = AppSettings::Instance()->getMediaDir() + fileName;
		// localisation in media, last test
		isFileExist = CallSystem::fileExist(fullFileName);
	}
	else {
		//test version of localisation
		fullFileName = fileNameAdapted;
		isFileExist = CallSystem::fileExist(fullFileName);
		if (isFileExist)
			return;

		//test basic file
		fullFileName = fileName;
		isFileExist = CallSystem::fileExist(fullFileName);
	}
}

std::string FilePath::getPath()
{
	if (isFileExist) {
		std::size_t found = fullFileName.find_last_of("/");
		return fullFileName.substr(0,found+1);
	} else
		return "";
}


FilePath::FilePath(const std::string& fileName, TFP type)
{
	if (fileName.empty())
		return;

	// we test if the file has an absolute name, we simply test its existence
	if (CallSystem::isAbsolute(fileName)) {
		fullFileName = fileName;
		isFileExist = CallSystem::fileExist(fullFileName);
		return;
	}

	// First we search in the scripts folder if the folder exists
	if ( ! scriptPath.empty() ) {
		fullFileName = scriptPath+fileName;
		isFileExist = CallSystem::fileExist(fullFileName);
		// if the form exists, nothing more to do.
		if (isFileExist)
			return;
	}

	fullFileName = fileName;
	//~ std::cout << "search (off script) file existence  " << fullFileName << std::endl;
	// If the file does not exist, then we look in the specified folder
	isFileExist = CallSystem::fileExist(fullFileName);
	if (!isFileExist) {
		switch(type) {
			case TFP::AUDIO : fullFileName = AppSettings::Instance()->getAudioDir() + fileName; break;
			case TFP::VIDEO : fullFileName = AppSettings::Instance()->getVideoDir() + fileName; break;
			case TFP::MEDIA : fullFileName = AppSettings::Instance()->getMediaDir() + fileName; break;
			case TFP::VR360 : fullFileName = AppSettings::Instance()->getVR360Dir() + fileName; break;
			case TFP::SCRIPT: fullFileName = AppSettings::Instance()->getScriptDir() + fileName; break;
			case TFP::IMAGE : fullFileName = AppSettings::Instance()->getPictureDir() + fileName; break;
			case TFP::TEXTURE : fullFileName = AppSettings::Instance()->getTextureDir() + fileName; break;
			case TFP::DATA  : fullFileName = AppSettings::Instance()->getUserDir() + fileName; break;
			case TFP::FONTS  : fullFileName = AppSettings::Instance()->getUserFontDir() + fileName; break;
			case TFP::MODEL3D : fullFileName = AppSettings::Instance()->getModel3DDir() + fileName; break;
			default: fullFileName = fileName; break;
		};
	isFileExist = CallSystem::fileExist(fullFileName);
	}
}
