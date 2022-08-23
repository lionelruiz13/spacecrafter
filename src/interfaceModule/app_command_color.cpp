/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 of the LSS Team & Association Sirius
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

#include <iostream>
#include <sstream>

#include "interfaceModule/app_command_color.hpp"
#include "tools/utility.hpp"


static bool checkHexColor(std::string s)
{
	if(s[0]!='x')
		return false;
	if (s.length() !=7)
		return false;
	char c;
	for(int i=1; i<7; i++) {
		c=s[i];
		//on vérifie que le char est bien dans les bons intervales
		if (!(((c>=48)&&(c<=57)) || ((c>=65)&&(c<=70)) || ((c>=97)&&(c<=102))))
			return false;
	}
	return true;
}

static int evalTruncColor(char c)
{
	if (c<=57)
		return (c-'0');
	else if (c<=70)
		return (c-'A'+10);
	else
		return (c-'a'+10);
}

void AppCommandColor::setClassicColor(Vec3f &color, std::string debug_message,
                                  const std::string &_r,const std::string &_g,const std::string &_b)
{
	float r,g,b;
	r = Utility::strToDouble(_r);
	g = Utility::strToDouble(_g);
	b = Utility::strToDouble(_b);
	// test des valeurs obtenues
	if ( r<0.f ||g<0.f || b<0.f) {
		debug_message = "r g b : negative value";
		return;
	}
	if ( r>1.f ||g>1.f || b>1.f) {
		debug_message = "r g b : value > 1.";
		return;
	}
	color[0]=r;
	color[1]=g;
	color[2]=b;
	isOkay = true;
}

void AppCommandColor::setHexColor(Vec3f &color, const std::string &_value)
{
	float r,g,b;
	r=(evalTruncColor(_value[1])*16+evalTruncColor(_value[2]))/255.f;
	g=(evalTruncColor(_value[3])*16+evalTruncColor(_value[4]))/255.f;
	b=(evalTruncColor(_value[5])*16+evalTruncColor(_value[6]))/255.f;
	color[0]=r;
	color[1]=g;
	color[2]=b;
	isOkay = true;
}

AppCommandColor::AppCommandColor(Vec3f &color, std::string &debug_message,
                                 const std::string &_value,
                                 const std::string &_argR, const std::string &_argG, const std::string &_argB)
{
	if (!_value.empty()) {
		//cas HEXADECIMAL
		if (_value[0]=='x') {
			if (checkHexColor(_value)) {
				this->setHexColor(color, _value);
				return;
			} else {
				debug_message = "'color' wrong hexColor string";
				return;
			}
		} else { //cas RBG_COLOR
			decodeRGBColor(color, _value, debug_message);
			return;
		}
	}
	//cas R G B
	if( _argR.empty() && _argG.empty() && _argB.empty()) {
		debug_message = "'color': missing expected argument 'r' or 'g' or 'b'";
	} else {
		this->setClassicColor(color, debug_message, _argR, _argG, _argB);
		return;
	}
	// aucun des cas
	debug_message = "'color': no argument color";
}


void AppCommandColor::decodeRGBColor(Vec3f &color, const std::string &_value, std::string &debug_message)
{
	float r,g,b;
	size_t i,j,k;
	i = _value.find("r");
	j = _value.find("g");
	k = _value.find("b");

	//~ std::cout << "Start " << _value << " i " << i << " j " << j << " k " << k << std::endl;

	std::string RColor, GColor, BColor;
	RColor = _value.substr(i+1,j-(i+1));
	GColor = _value.substr(j+1,k-(j+1));
	BColor = _value.substr(k+1,std::string::npos);

	//~ std::cout << "R " << RColor << " G " << GColor << " B " << BColor << std::endl;

	r=Utility::strToDouble(RColor)/255.f;
	g=Utility::strToDouble(GColor)/255.f;
	b=Utility::strToDouble(BColor)/255.f;

	if ( r<0.f ||g<0.f || b<0.f) {
		debug_message = "r g b : negative value";
		return;
	}
	if ( r>1.f ||g>1.f || b>1.f) {
		debug_message = "r g b : value > 1.";
		return;
	}

	//~ std::cout << "setRGBCOlor oki" << std::endl;
	color[0]=r;
	color[1]=g;
	color[2]=b;
	isOkay = true;
	return;
}
