/*
 * Copyright (C) 2020 of Association Sirius & Association Andromède
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _SC_MODULE_HPP_
#define _SC_MODULE_HPP_

class Projector;
class Navigator;
//class s_font;

#include "tools/vecmath.hpp"
#include "tools/fader.hpp"
#include "tools/s_font.hpp"
#include <cassert>


class ModuleColor {
public:
	void setColor(const Vec3f& c) {
		color = c;
	}
	Vec3f getColor() {
		return color;
	}

protected:
    Vec3f color;
};


class ModuleFont {
public:
	void setFont(float font_size, const std::string& font_name) {
	if (font) {
		delete font;
		font=nullptr;
	}
	font = new s_font(font_size, font_name);
	assert(font);
	}

protected:
    s_font* font=nullptr;
};

template <class faderType>
class ModuleFader {
public:
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}
	void setFlagShow(bool b) {
		fader = b;
	}
	bool getFlagShow(void) const {
		return fader;
	}

	void flipFlagShow() {
		fader = !fader;
	}

protected:
    faderType fader;
};

/*
class CoreModule {
public:

	void setColor(const Vec3f& c) {
		color = c;
	}
	Vec3f getColor() {
		return color;
	}

	virtual void draw(const Projector* prj, const Navigator* nav){};

	virtual void update(int delta_time) {
		fader.update(delta_time);
	}

    virtual void preDraw(const Projector* prj, const Navigator* nav){};

    void drawFBO(){};

	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}
	void setFlagShow(bool b) {
		fader = b;
	}
	bool getFlagShow(void) const {
		return fader;
	}

	void flipFlagShow() {
		fader = !fader;
	}

protected:
    Vec3f color;
    s_font* font=nullptr;
    LinearFader fader;
    // le FBO ?
};
*/
#endif
