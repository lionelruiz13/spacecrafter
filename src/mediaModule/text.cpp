/*
 * Spacecrafter astronomy simulation and visualization
 *
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

// Class which handles  Text for User script
#include <string>
#include <fstream>
#include <iostream>

#include "tools/log.hpp"
#include "mediaModule/text.hpp"


Text::Text(const std::string &_name, const std::string &_text, float _altitude, float _azimuth, s_font* _myFont, const TEXT_ALIGN &_textAlign,  const Vec3f &color, bool _textFader)
{
	name= _name;
	text= _text;
	altitude= _altitude;
	azimuth= _azimuth;
	textColor = color;
	textFont =_myFont;
	textAlign = _textAlign;
	smooth = _textFader;
	flag_location = 0;

}


Text::~Text()
{}

void Text::update(int delta_time)
{
	if (flag_location) {
		my_timer += delta_time; // update local timer
		if (my_timer < end_time) {
			altitude = start_altitude + my_timer*x_move; // linear function
		} else {
			altitude = end_altitude;
			flag_location = 0;
		}
		if (my_timer < end_time) {
			azimuth = start_azimuth + my_timer*y_move; // linear function
		} else {
			azimuth = end_azimuth;
			flag_location = 0;
		}
	}
}

void Text::draw(const Projector* prj)
{
	if (fader.isZero())
		return;

	textColor[3] = fader;
	textFont->printHorizontal(prj, altitude, azimuth, text,textColor, textAlign, true);
}

void Text::setLocation(float _altitude, bool deltax, float _azimuth, bool deltay, float duration)
{
	if (duration<=0) {
		if (deltax) altitude = _altitude;
		if (deltay) azimuth = _azimuth;
		return;
	}
	start_altitude = altitude;
	start_azimuth = azimuth;

	my_timer = 0;// count time elapsed from the beginning of the command

	// only move if changing value
	if (deltax) end_altitude = _altitude;
	else end_altitude = altitude;

	if (deltay) end_azimuth = _azimuth;
	else end_azimuth = azimuth;

	// the new script begin here
	x_move = end_altitude - start_altitude;
	y_move = end_azimuth - start_azimuth;
	if (y_move > 180)
		y_move = y_move - 360;
	else if (y_move < -180)
		y_move = y_move + 360;
	end_time = int(duration * 1000.f); // movement duration in milliseconds
	x_move = x_move / (1000.f*duration);
	y_move = y_move / (1000.f*duration);
	flag_location = 1;
}

void Text::textUpdate(const std::string &_text)
{
	textFont->clearCache(text);
	text = _text;
}
