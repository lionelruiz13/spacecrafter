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

#ifndef _TEXT_HPP
#define _TEXT_HPP

#include "tools/s_font.hpp"
#include "tools/s_font_common.hpp"
#include "tools/fader.hpp"
#include "coreModule/projector.hpp"
#include <vector>

#include "tools/vecmath.hpp"


/**
 * \file text.hpp
 * \brief This class manages a single text to display.
 * \author Olivier NIVOIX
 * \version 2
 *
 * @class Text
 * @brief This class manages a single text to display.
 *
 * A text is represented by its color, its message and its position.
 * His message can be modified.
 */
class Text {
public:
	Text(const std::string &_name, const std::string &_text, float _altitude, float _azimuth, s_font *_myFont, const TEXT_ALIGN &_textAlign, const Vec3f &color);
	~Text();

	//! hide or not the text on the display
	void setDisplay(bool b) {
		fader=b;
	}

	//! indicates the state of the text with respect to the display
	bool getDisplay() const {
		return fader;
	}

	//! returns the name of the text
	std::string getName() const {
		return name;
	}

	//! modify the text's message
	void textUpdate(const std::string &_text);

	//! displays the text on the screen
	void draw(const Projector* prj);

	//! updates the state of the display fader
	void update(int delta_time);

private:
	std::string name;
	std::string text;
	Vec3f textColor;
	float altitude;
	float azimuth;
	BooleanFader fader;
	s_font* textFont;
	TEXT_ALIGN textAlign;
};

#endif // TEXT_HPP
