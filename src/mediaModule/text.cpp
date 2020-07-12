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


Text::Text(const std::string &_name, const std::string &_text, int _altitude, int _azimuth , const FONT_SIZE &_size, const Vec3f &color)
{
	name= _name;
	text= _text;
	altitude= _altitude;
	azimuth= _azimuth;
	textColor = color;
	textSize = _size;
	fader = true;
}


Text::~Text()
{
	printf("texte %s est détruit\n", name.c_str());
}


void Text::update(int delta_time){
	fader.update(delta_time);
}

void Text::draw(const Projector* prj, s_font *textFont[])
{
	//~ glEnable(GL_TEXTURE_2D);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	StateGL::enable(GL_BLEND);
	s_font *tmp;

	if ( !fader.getInterstate() ) return;
	//~ glColor4f(r,g,b,fader.getInterstate());
	switch (textSize) {
		case FONT_SIZE::T_XX_SMALL: tmp = textFont[0]; break;
		case FONT_SIZE::T_X_SMALL: tmp = textFont[1]; break;
		case FONT_SIZE::T_SMALL: tmp = textFont[2]; break;
		case FONT_SIZE::T_MEDIUM: tmp = textFont[3]; break;
		case FONT_SIZE::T_LARGE: tmp = textFont[4]; break;
		case FONT_SIZE::T_X_LARGE: tmp = textFont[5]; break;
		case FONT_SIZE::T_XX_LARGE: tmp = textFont[6]; break;
		default: tmp = textFont[3]; break; // cas medium
	}
	tmp->printHorizontal(prj, altitude, azimuth, text,textColor, TEXT_POSITION::LEFT, true); //, 1, true/*, true*/);
}

void Text::textUpdate(const std::string &_text, s_font *textFont[]){
	s_font *tmp;
	switch (textSize) {
		case FONT_SIZE::T_XX_SMALL: tmp = textFont[0]; break;
		case FONT_SIZE::T_X_SMALL: tmp = textFont[1]; break;
		case FONT_SIZE::T_SMALL: tmp = textFont[2]; break;
		case FONT_SIZE::T_MEDIUM: tmp = textFont[3]; break;
		case FONT_SIZE::T_LARGE: tmp = textFont[4]; break;
		case FONT_SIZE::T_X_LARGE: tmp = textFont[5]; break;
		case FONT_SIZE::T_XX_LARGE: tmp = textFont[6]; break;
		default: tmp = textFont[3]; break; // cas medium
	}
	tmp->clearCache(text);
	text=_text;
}