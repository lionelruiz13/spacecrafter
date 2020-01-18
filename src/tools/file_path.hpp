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
#include "tools/app_settings.hpp"


/*! \class FilePath
* \brief classe recherchant un fichier sur le systeme
*
* Lors de l'exécution d'un script par exemple, un fichier de donnée utilisé peut 
* se retrouver dans différents emplacements sur le systeme. FilePath recherche 
* un nom de fichier sur plusieurs emplacements en fonction des indications de 
* l'utilisateur
* 
* Si le nom du fichier est donné de manière absolu, la classe vérifie son existance
* sinon la classe teste diverses possibilités
* 
* FilePath va chercher en priorité le fichier dans le répertoire ou à été lu le 
* fichier de script sinon, il procèdera à la recherche dans un répertoire TFP 
* 
* Il fournira alors un résultat d'existance du fichier et retournera le nom exact
* du fichier cherché sur le systeme
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
	std::string toString() const {return fullFileName; }

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
	void testFileExistance();		//!< fonction qui teste l'existance présumée du fichier
	std::string fullFileName;		//!< nom complet du fichier à analyser

	static std::string scriptPath;	//!< nom du répertoire du script
};

#endif // _FILEPATH_
