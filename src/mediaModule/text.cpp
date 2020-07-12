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


Text::Text(const std::string &_name, const std::string &_text, int _altitude, int _azimuth , s_font* _myFont, const TEXT_ALIGN &_textAlign,  const Vec3f &color)
{
	name= _name;
	text= _text;
	altitude= _altitude;
	azimuth= _azimuth;
	textColor = color;
	textFont =_myFont;
	textAlign = _textAlign;
	fader = true;
}


Text::~Text()
{}


void Text::update(int delta_time){
	fader.update(delta_time);
}

void Text::draw(const Projector* prj)
{
	if ( !fader.getInterstate() ) return;

	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	StateGL::enable(GL_BLEND);
	textFont->printHorizontal(prj, altitude, azimuth, text,textColor, textAlign, true);
}

void Text::textUpdate(const std::string &_text)
{
	textFont->clearCache(text);
	text=_text;
}