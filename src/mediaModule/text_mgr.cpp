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

#include <string>
#include <fstream>
#include <iostream>

#include "tools/log.hpp"
#include "tools/s_font_common.hpp"
#include "mediaModule/text_mgr.hpp"
#include "mediaModule/text.hpp"

#define NB_MAX_SIZE 7


TextMgr::TextMgr()
{
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

	this->clearCache();
	textFont.clear();
}


void TextMgr::add(const std::string& name, const TEXT_MGR_PARAM& textParam)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		textUsr.erase(it);

	//set font size
	FONT_SIZE textSize = FONT_SIZE::T_MEDIUM;
	if (!textParam.fontSize.empty()) {
		auto it = strToFontSize.find(textParam.fontSize);
		if (it!=strToFontSize.end())
			textSize=strToFontSize[textParam.fontSize];
		else
			cLog::get()->write("text size was not recognized", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
	}

	//set font
	s_font *tmp;
	switch (textSize) {
		case FONT_SIZE::T_XX_SMALL:
			tmp = textFont[0].get();
			break;
		case FONT_SIZE::T_X_SMALL:
			tmp = textFont[1].get();
			break;
		case FONT_SIZE::T_SMALL:
			tmp = textFont[2].get();
			break;
		case FONT_SIZE::T_MEDIUM:
			tmp = textFont[3].get();
			break;
		case FONT_SIZE::T_LARGE:
			tmp = textFont[4].get();
			break;
		case FONT_SIZE::T_X_LARGE:
			tmp = textFont[5].get();
			break;
		case FONT_SIZE::T_XX_LARGE:
			tmp = textFont[6].get();
			break;
	}

	//set align
	TEXT_ALIGN textAlign = TEXT_ALIGN::LEFT;
	if (!textParam.textAlign.empty()) {
		auto it = strToTextAlign.find(textParam.textAlign);
		if (it!=strToTextAlign.end())
			textAlign=strToTextAlign[textParam.textAlign];
		else
			cLog::get()->write("text alignement was not recognized", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
	}

	//set color
	Vec3f color;
	if (textParam.useColor==true)
		color = textParam.color;
	else
		color = defaultTextColor;

	std::unique_ptr token  = std::make_unique<Text>(name, textParam.string, textParam.altitude, textParam.azimuth, tmp, textAlign, color, textParam.fader);
	textUsr[name]=std::move(token);
}


void TextMgr::setColor(const Vec3f& c)
{
	defaultTextColor = c;
}


void TextMgr::clear()
{
	textUsr.clear();
}


void TextMgr::clearCache()
{
	for (auto &it : textFont)
		it->clearCache();
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


void TextMgr::textDisplay(const std::string &name, bool displ)
{
	auto it = textUsr.find(name);
	if (it!=textUsr.end())
		(*it).second->setDisplay(displ);
	else
		cLog::get()->write("Not found to display text named "+name, LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
}


void TextMgr::setFont(float font_size, const std::string& font_name)
{
	mFontSize = font_size;
	mFontName = font_name;
	if (mFontSize<SIZE_MIN_TO_DISPLAY) {
		mFontSize=SIZE_MIN_TO_DISPLAY;
		cLog::get()->write("text size to small fixed to minimal", LOG_TYPE::L_WARNING, LOG_FILE::SCRIPT);
	}
}

void TextMgr::buildFont()
{
	this->clearCache();
	this->clear();
	isUsable= true;
	textFont.reserve(NB_MAX_SIZE);
	for(int i=0; i<NB_MAX_SIZE; i++) {
		textFont.push_back(std::make_unique<s_font>(mFontSize+2*(i-3), mFontName));
		if (textFont.back()==nullptr) {
			cLog::get()->write("TEXT: can't create text usr font", LOG_TYPE::L_ERROR);
			isUsable = false;
			break;
		}
	}
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

void TextMgr::updateFont(double size, const std::string& fontName)
{
	this->clearCache();
	this->clear();
	for(int i=0; i<NB_MAX_SIZE; i++)
		textFont[i]->rebuild(size+2*(i-3), fontName);
}
