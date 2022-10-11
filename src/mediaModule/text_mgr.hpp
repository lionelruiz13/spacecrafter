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

#ifndef _TEXT_MGR_H
#define _TEXT_MGR_H


#include <map>
#include <memory>

#include "tools/s_font.hpp"
#include "tools/fader.hpp"
#include "text.hpp"
#include "coreModule/projector.hpp"

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"

/**
 * \file text_mgr.hpp
 * \brief This class processes all text entities used by the user.
 * \author Olivier NIVOIX
 * \version 2
 *
 * \class TextMgr
 *
 * \brief This class processes all text entities used by the user.
 *
 * The class is operational when setFont succeeds in loading a font in 7 size ranges.
 * A sentry who puts the class on standby when it cannot initialize properly.
 *
 * It provides classic access to the management of horizontal texts.
 *
 * The container contains all the texts which are independent of each other.
 * The name of the text serves as a key. (uniqueness)
 *
 */

struct TEXT_MGR_PARAM {
	std::string string;
	float altitude;
	float azimuth;
	std::string fontSize;
	std::string textAlign;
	Vec3f color;
	bool useColor;
	bool fader;
};

class TextMgr: public NoCopy {
public:
	TextMgr();
	~TextMgr();
	//! transmits the time variations to the different texts
	void update(int delta_time);

	//! orders the layout of the different texts
	void draw(const Projector* prj);

	//! adds a text to the textUsr container
	void add(const std::string& name, const TEXT_MGR_PARAM& textParam);

	//! removes a text from the textUsr container
	void del(const std::string &name);

	//! removes all the texts from the container
	void clear();

	//! allows to change the text of a text in the container
	void textUpdate(const std::string &name, const std::string &text);

	//! allows to hide a text in the container
	void textDisplay(const std::string &name, bool displ);

	//! allows to change the fading of a text in the container
	void setFadingDuration(float t) {
		fadingDuration = t;
	}

	//! initialize all the fonts used by the class
	void setFont(float font_size, const std::string& font_name);

	//! updates the fonts with the new parameters fontName and sizeValue
	void updateFont(double size, const std::string& fontName);
	//! builds the fonts determined by setFont 
	void buildFont();

	//! update the original fonts
	void resetFont(){
		updateFont(mFontSize, mFontName);
	};

	//! change the default color of the future new text
	void setColor(const Vec3f& c);
private:
	void clearCache();			// empties the fonts caches
	std::map<std::string, std::unique_ptr<Text>> textUsr; // the container for all texts
	std::map<std::string, FONT_SIZE> strToFontSize; // convert txt to FONT_SIZE
	std::map<std::string, TEXT_ALIGN> strToTextAlign; // convert txt to TEXT_POSITION
	std::vector<std::unique_ptr<s_font>> textFont;		// the set of fonts used
	Vec3f defaultTextColor;		// default color vector
	bool isUsable = false;		// indicator if the class is operational
	float fadingDuration;		// duration of a text fading (if it exists) in seconds
	std::string mFontName;		// remembers what fontName is used
	float mFontSize; 			// remembers what fontSize is used
};

#endif
