/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014-2017 of the LSS Team & Association Sirius
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
#include "tools/s_font_common.hpp"
#include "mediaModule/text_mgr.hpp"
#include "mediaModule/text.hpp"



TextMgr::TextMgr()
{
	for(int i=0; i<7; i++)
		textFont[i]=nullptr;
}

TextMgr::~TextMgr()
{
	this->clear();

	for(int i=0; i<7; i++) {
		if (textFont[i]) delete textFont[i];
		textFont[i]=nullptr;
	}
}

void TextMgr::update(int delta_time)
{
	for (auto iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		(*iter).second->update(delta_time);
	}
}


void TextMgr::add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &size, const Vec3f &color)
{
	this->del(name);
	FONT_SIZE textSize = convertToFontSize(size);
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
	std::unique_ptr token  = std::make_unique<Text>(name, text, altitude, azimuth, tmp, color);
	textUsr[name]=std::move(token);
}

void TextMgr::add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &size)
{
	return add(name, text, altitude, azimuth, size, defaultTextColor);
}


void TextMgr::setColor(const Vec3f& c)
{
	defaultTextColor = c;
}

void TextMgr::clear()
{
	textUsr.clear();

	for(int i=0; i<7; i++) {
		textFont[i]->clearCache();
	}
}

void TextMgr::del(const std::string &name)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		textUsr.erase(it);
}

void TextMgr::textUpdate(const std::string &name, const std::string &text)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		(*it).second->textUpdate(text);
}

void TextMgr::textDisplay(const std::string &name , bool displ)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		(*it).second->setDisplay(displ);
}


void TextMgr::setFont(float font_size, const std::string& font_name)
{
	if (font_size<SIZE_MIN_TO_DISPLAY) {
		font_size=SIZE_MIN_TO_DISPLAY;
		cLog::get()->write("text size to small fixed to minimal", LOG_TYPE::L_WARNING);
	}

	for(int i=0; i<7; i++) {
		if (textFont[i]) delete textFont[i];
		textFont[i]=nullptr;
	}
	for(int i=0; i<7; i++) {
		textFont[i] = new s_font(font_size+2*(i-3), font_name);
		if (textFont[i]==nullptr) {
			cLog::get()->write("TEXT: can't create text usr font", LOG_TYPE::L_ERROR);
			isUsable = false;
		}
	}
	isUsable= true;

	if (!isUsable)
		cLog::get()->write("TEXT: module disable", LOG_TYPE::L_WARNING);
}


void TextMgr::draw(const Projector* prj)
{
	if (!isUsable)
		return;

	for (auto iter = textUsr.begin(); iter != textUsr.end(); ++iter) {
		(*iter).second->draw(prj);
	}
}



FONT_SIZE TextMgr::convertToFontSize(const std::string &size)
{
	if (size=="XX_SMALL")		{return FONT_SIZE::T_XX_SMALL;}
	else if (size=="X_SMALL")	{return FONT_SIZE::T_X_SMALL;}
	else if (size=="SMALL")		{return FONT_SIZE::T_SMALL; }
	else if (size=="MEDIUM")	{return FONT_SIZE::T_MEDIUM;}
	else if (size=="LARGE")		{return FONT_SIZE::T_LARGE; }
	else if (size=="X_LARGE")	{return FONT_SIZE::T_X_LARGE; }
	else if (size=="XX_LARGE")	{return FONT_SIZE::T_XX_LARGE; }
	else {return FONT_SIZE::T_MEDIUM;}
}
