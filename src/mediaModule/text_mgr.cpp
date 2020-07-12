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

	strToFontSize["XX_SMALL"] = FONT_SIZE::T_XX_SMALL;
	strToFontSize["X_SMALL"] = FONT_SIZE::T_X_SMALL;
	strToFontSize["SMALL"] = FONT_SIZE::T_SMALL; 
	strToFontSize["MEDIUM"] = FONT_SIZE::T_MEDIUM;
	strToFontSize["LARGE"] = FONT_SIZE::T_LARGE; 
	strToFontSize["X_LARGE"] = FONT_SIZE::T_X_LARGE; 
	strToFontSize["XX_LARGE"] = FONT_SIZE::T_XX_LARGE;

	strToTextAlign["LEFT"] = TEXT_ALIGN::LEFT;
	strToTextAlign["RIGHT"] = TEXT_ALIGN::RIGHT;
	strToTextAlign["CENTER"] = TEXT_ALIGN::CENTER;
}

TextMgr::~TextMgr()
{
	this->clear();
	strToFontSize.clear();
	strToTextAlign.clear();

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


void TextMgr::add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &fontSize, const std::string &_textAlign, const Vec3f &color)
{
	this->del(name);

	//set font size
	FONT_SIZE textSize = FONT_SIZE::T_MEDIUM;
	if (!fontSize.empty()) {
		auto it = strToFontSize.find(fontSize);
		if (it!=strToFontSize.end())
			textSize=strToFontSize[fontSize];
		else
			cLog::get()->write("text size was not recognized", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
	}

	//set font
	s_font *tmp;
	switch (textSize) {
		case FONT_SIZE::T_XX_SMALL: tmp = textFont[0]; break;
		case FONT_SIZE::T_X_SMALL: tmp = textFont[1]; break;
		case FONT_SIZE::T_SMALL: tmp = textFont[2]; break;
		case FONT_SIZE::T_MEDIUM: tmp = textFont[3]; break;
		case FONT_SIZE::T_LARGE: tmp = textFont[4]; break;
		case FONT_SIZE::T_X_LARGE: tmp = textFont[5]; break;
		case FONT_SIZE::T_XX_LARGE: tmp = textFont[6]; break;
	}

	//set align
	TEXT_ALIGN textAlign = TEXT_ALIGN::LEFT;
	if (!_textAlign.empty()) {
		auto it = strToTextAlign.find(_textAlign);
		if (it!=strToTextAlign.end())
			textAlign=strToTextAlign[_textAlign];
		else
			cLog::get()->write("text alignement was not recognized", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
	}
	std::unique_ptr token  = std::make_unique<Text>(name, text, altitude, azimuth, tmp, textAlign, color);
	textUsr[name]=std::move(token);
}

void TextMgr::add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &fontSize, const std::string &textAlign)
{
	this->add(name, text, altitude, azimuth, fontSize, textAlign, defaultTextColor);
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
	else
		cLog::get()->write("Not found to delete text named "+name, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
}

void TextMgr::textUpdate(const std::string &name, const std::string &text)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		(*it).second->textUpdate(text);
	else
		cLog::get()->write("Not found to update text named "+name, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
}

void TextMgr::textDisplay(const std::string &name , bool displ)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		(*it).second->setDisplay(displ);
	else
		cLog::get()->write("Not found to display text named "+name, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);

}


void TextMgr::setFont(float font_size, const std::string& font_name)
{
	if (font_size<SIZE_MIN_TO_DISPLAY) {
		font_size=SIZE_MIN_TO_DISPLAY;
		cLog::get()->write("text size to small fixed to minimal", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
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
	for (const auto& [key, value] : textUsr) {
    	value->draw(prj);
	}
}
