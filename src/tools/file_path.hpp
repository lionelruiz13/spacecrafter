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


#ifndef _FILEPATH_
#define _FILEPATH_

#include <string>
//#include "tools/app_settings.hpp"

/**
* \file file_path.hpp
* \brief searches for a file on the system
* \author Olivier NIVOIX
* \version 1
*
*! \class FilePath
*
* \brief searches for a file on the system
*
* When executing a script for example, a data file used can
* be found in different locations on the system. FilePath is looking for
* a file name in several locations according to the indications of the user.
* 
* If the name of the file is given in an absolute way, the class checks its existence
* otherwise the class tests various possibilities regarding its location on the hard disk
* 
* FilePath will first search for the file in the directory where the
* script file otherwise, it will search in a TFP directory
*
* It will then provide an existence result of the file and return the exact name
* of the file sought on the system
*
*/
class FilePath
{
public:
	//! Type of directories used by FilePath
	enum class TFP : char {NONE, ///< no particular directory 
							AUDIO, ///< the audio directory
							VIDEO, ///< the video directory
							MEDIA, ///< the media directory
							VR360, ///< VR360 directory
							IMAGE,///< the image directory
							TEXTURE,///< the textures directory
							DATA, ///< data directory
							FONTS, ///< the fonts directory
							MODEL3D,///< the model3D directory
							SCRIPT ///< the script directory
							};

	/** Default constructor: the directory used is NONE
	* @param fileName the name of the file whose existence is being searched
	*/
	FilePath(const std::string& fileName);

	/** Special constructor related to internationalization
	* This function is used to find the appropriate soundtrack for a video file depending on the location
	* @param fileName the name of the file whose existence is sought
	* @param localisation the language of the sound file to select
	*/
	FilePath(const std::string& fileName, const std::string& localisation);

	//! General constructor for finding the file according to the type of data it represents
	FilePath(const std::string& fileName, TFP type);

	// returns the path of the file
	std::string getPath();

	/** returns the full name of an analyzed file
	* @return the full name of the file 
	*/
	const std::string& toString() const {return fullFileName; }

	/** returns the existence of the file on the system
	* @return true if the file exists on the system, false otherwise
	*/
	bool exist() const {return isFileExist;}

	/** sets the path to the class
	 * @param name of the directory used by the script 
	 */
	static void fixScriptPath (const std::string& _scriptPath) {
		FilePath::scriptPath= _scriptPath;
	}

	//! write facility to retrieve the existence of a file on the system
	explicit operator bool() const { return exist(); }

	//! write facility to retrieve the full name of the file
	operator std::string() const { return toString(); }

private:
	bool isFileExist = false; 		//!< indicates the existence of the file on the system
	std::string fullFileName;		//!< full name of the file to analyze

	static std::string scriptPath;	//!< name of the script directory
};

#endif // _FILEPATH_
