/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Association Sirius
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

/* This class handles parsing of a simple command syntax for scripting,
   UI components, network commands, etc.
*/

#ifndef _APP_COMMAND_COLOR_HPP_
#define _APP_COMMAND_COLOR_HPP_

#include <string>
#include "tools/vecmath.hpp"

/**
 * \class AppCommandColor
 * \brief Analyse multiple des choix de couleur
 * \author Olivier NIVOIX
 * \date 23 juin 2018
 * 
 * Cette classe a pour but d'analyser une saisie de couleur à partir de différentes chaines
 * 
 * @section DESCRIPTION
 * 
 * Pour analyser la couleur, plusieures possibilités sont envisagées
 * 
 * On utilise les paramètres r g b classiques avec 0=<r ou g ou b =<1
 
 * On utilise une chaine de charactère genre rXXgYYbZZ ou 0=<XX ou YY ou ZZ =<255
 * 
 * On utilise une chaine hexadécimale genre xXXYYZZ avec XX YY et ZZ les octets représentants la couleur
 * 
 * @section FONCTIONNEMENT 
 * 
 * le constructeur choisit la bonne fonction membre pour analyser les infos
 * 
 * les fonctions membres privées s'occupent chaqu'une d'un cas particulier
 * 
 * isOkay indique si la couleur a été bien analysée
 * 
 * debug_message reçoit en cas d'erreur le type d'erreur rencontré 
 * 
 * color reçoit les couleurs analysées
 * 
 */


class AppCommandColor {

public:
	AppCommandColor(Vec3f &color, std::string &debug_message,
	                const std::string &_value,
	                const std::string &_r, const std::string &_g, const std::string &_b);
	~AppCommandColor(){};
	AppCommandColor(AppCommandColor const &) = delete;
	AppCommandColor& operator = (AppCommandColor const &) = delete;

	explicit operator bool() const {
		return isOkay;
	}
private:
	bool isOkay = false;
	void setClassicColor(Vec3f &color, std::string debug_message, const std::string &_r,const std::string &_g,const std::string &_b);
	void setHexColor(Vec3f &color, const std::string &_value);
	void decodeRGBColor(Vec3f &color, const std::string &_value, std::string &debug_message);
};


#endif // _APP_COMMAND_COLOR_HPP_
