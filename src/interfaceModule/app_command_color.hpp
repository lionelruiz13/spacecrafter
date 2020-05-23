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


#ifndef _APP_COMMAND_COLOR_HPP_
#define _APP_COMMAND_COLOR_HPP_

#include <string>
#include "tools/vecmath.hpp"

/**
* \file app_command_color.hpp
* \brief Parse std::string to Vec3f color
* \author Olivier NIVOIX
* \version 1
*
* \class AppCommandColor
*
* \brief Transforms an alphanumeric entry into its color representation
*
* The purpose of this class is to analyze a character string (std :: string) and to return a color (Vec3f)
* 
* @section Description
*
* To analyze the color, several possibilities are considered
*
* We use the classic r g b parameters with 0=<r,g,b=<1
*
* We use a character string in the format rXXgYYbZZ with 0=<XX, YY, ZZ =<255
*
* We use a hexadecimal chain like xXXYYZZ with XX,YY,ZZ representing the hexadecimal component of the color
* 
* @section Working
* 
* The constructor chooses the right member function to analyze the information
* 
*/

class AppCommandColor {

public:
	/** Convert string to color
	* @param color modified variable which takes the extracted color
	* @param debug_message receives in case of error the type of error encountered
	* @param value other input form to translate 
	* @param r,g,b the r,b,g color composants
	*/
	AppCommandColor(Vec3f &color, std::string &debug_message,
	                const std::string &_value,
	                const std::string &_r, const std::string &_g, const std::string &_b);
	~AppCommandColor(){};
	AppCommandColor(AppCommandColor const &) = delete;
	AppCommandColor& operator = (AppCommandColor const &) = delete;

	/// indicates the success of the conversion
	/// @return True if a color has been deduced from the constructor otherwise false
	explicit operator bool() const {
		return isOkay;
	}
private:
	bool isOkay = false;
	// transform r g b string to color
	void setClassicColor(Vec3f &color, std::string debug_message, const std::string &_r,const std::string &_g,const std::string &_b);
	// transform hexadécimal string to color
	void setHexColor(Vec3f &color, const std::string &_value);
	// transform rXXgYYbZZ string to color
	void decodeRGBColor(Vec3f &color, const std::string &_value, std::string &debug_message);
};


#endif // _APP_COMMAND_COLOR_HPP_
