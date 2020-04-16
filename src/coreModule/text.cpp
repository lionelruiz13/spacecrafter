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
#include "coreModule/text.hpp"



void Text::convertToFontSize(const std::string &size)
{
	if (size=="XX_SMALL")		{textSize=FontSize::T_XX_SMALL; return;}
	else if (size=="X_SMALL")	{textSize=FontSize::T_X_SMALL; return;}
	else if (size=="SMALL")		{textSize=FontSize::T_SMALL;  return;}
	else if (size=="MEDIUM")	{textSize=FontSize::T_MEDIUM; return;}
	else if (size=="LARGE")		{textSize=FontSize::T_LARGE;  return;}
	else if (size=="X_LARGE")	{textSize=FontSize::T_X_LARGE;  return;}
	else if (size=="XX_LARGE")	{textSize=FontSize::T_XX_LARGE;  return;}
	else {textSize=FontSize::T_MEDIUM; return;}
}


Text::Text(const std::string &_name, const std::string &_text, int _altitude, int _azimuth , const std::string &size, const Vec3f &color, int _timeout)
{
	name= _name;
	text= _text;
	altitude= _altitude;
	azimuth= _azimuth;
	textColor = color;
	convertToFontSize(size);
	isTextDead = false;
	timeout = _timeout;
	printf("Duration fixée à %i\n", timeout);
}


Text::~Text()
{
	printf("texte %s est détruit\n", name.c_str());
}


void Text::update(int delta_time){
	fader.update(delta_time);
	if (timeout>0 && fader.getInterstate()>0) {
		timeout -= delta_time;
		//~ printf("nd: %i\n", duration);
		if (timeout< fader.getDuration() && !isDying) {
			//~ printf("Le text est entrain de mourir\n");
			fader= false;
			isDying = true;
		}
		if (timeout<1) {
			isTextDead = true;}
	}
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
		case FontSize::T_XX_SMALL: tmp = textFont[0]; break;
		case FontSize::T_X_SMALL: tmp = textFont[1]; break;
		case FontSize::T_SMALL: tmp = textFont[2]; break;
		case FontSize::T_MEDIUM: tmp = textFont[3]; break;
		case FontSize::T_LARGE: tmp = textFont[4]; break;
		case FontSize::T_X_LARGE: tmp = textFont[5]; break;
		case FontSize::T_XX_LARGE: tmp = textFont[6]; break;
		default: tmp = textFont[3]; break; // cas medium
	}
	tmp->printHorizontal(prj, altitude, azimuth, text,textColor, true); //, 1, true/*, true*/);
}

void Text::textUpdate(const std::string &_text, s_font *textFont[]){
	s_font *tmp;
	switch (textSize) {
		case FontSize::T_XX_SMALL: tmp = textFont[0]; break;
		case FontSize::T_X_SMALL: tmp = textFont[1]; break;
		case FontSize::T_SMALL: tmp = textFont[2]; break;
		case FontSize::T_MEDIUM: tmp = textFont[3]; break;
		case FontSize::T_LARGE: tmp = textFont[4]; break;
		case FontSize::T_X_LARGE: tmp = textFont[5]; break;
		case FontSize::T_XX_LARGE: tmp = textFont[6]; break;
		default: tmp = textFont[3]; break; // cas medium
	}
	tmp->clearCache(text);
	text=_text;
}