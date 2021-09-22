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
	//! Type de répertoires utilisés par FilePath
	enum class TFP : char {NONE, ///< pas de répertoire particulier 
							AUDIO, ///< le répertoire audio
							VIDEO, ///< le répertoire vidéo
							MEDIA, ///< le répertoire média
							VR360, ///< le répertoire des VR360
							IMAGE,///< le répertoire image
							TEXTURE,///< le répertoire des textures
							DATA, ///< le répertoire data
							FONTS, ///< le répertoire fonts
							MODEL3D,///< le répertoire model3D
							SCRIPT ///< le répertoire de script
							};

	/** Constructeur par défaut: le répertoire utilisé est NONE
	* @param fileName le nom du fichier dont on cherche l'existance
	*/
	FilePath(const std::string& fileName);

	/** Constructeur spécial lié à l'internationalisation
	* Cette fonction sert à trouver la bande son adéquate à un fichier vidéo en fonction de localisation
	* @param fileName le nom du fichier dont on cherche l'existance
	* @param localisation la langue du fichier son à sélectionner
	*/
	FilePath(const std::string& fileName, const std::string& localisation);

	//! Constructeur général pour la recherche du fichier en fonction du type de données qu'il représente
	FilePath(const std::string& fileName, TFP type);

	// renvoi le chemin d'accès du fichier
	std::string getPath();

	/** renvoie le nom complet d'un fichier analysé
	* @return le nom complet du fichier 
	*/
	const std::string& toString() const {return fullFileName; }

	/** renvoie l'existance du fichier dans le système
	* @return true si le fichier existe sur le système, false sinon
	*/
	bool exist() const {return isFileExist;}

	/** fixe le chemin d'accès à la classe
	 * @param nom du répertoire utilisé par le script 
	 */
	static void fixScriptPath (const std::string& _scriptPath) {
		FilePath::scriptPath= _scriptPath;
	}

	//! facilité d'écriture pour récupérer l'existance d'un fichier sur le système
	explicit operator bool() const { return exist(); }

	//! facilité d'écriture pour récupérer le nom complet du fichier
	operator std::string() const { return toString(); }

private:
	bool isFileExist = false; 		//!< indique l'existance du fichier sur le système
	std::string fullFileName;		//!< nom complet du fichier à analyser

	static std::string scriptPath;	//!< nom du répertoire du script
};

#endif // _FILEPATH_
