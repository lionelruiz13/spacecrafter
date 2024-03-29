/*
 * Copyright (C) 2020-2021 of Association Sirius & Association Andromède
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

#include "tools/vecmath.hpp"
#include "tools/fader.hpp"
#include "tools/s_font.hpp"
#include <cassert>
#include <memory>


class ModuleColor {
public:
	//! Set color for Module
	void setColor(const Vec3f& c) {
		color = c;
	}
	//! Get color for Module
	const Vec3f& getColor() const {
		return color;
	}

protected:
    Vec3f color;
};


class ModuleFont {
public:
	ModuleFont(){};
	virtual ~ModuleFont(){};
	// get font attribued to this class
	virtual void registerFont(s_font* _font){
    	font=_font;
	}
protected:
    s_font* font=nullptr;
};

template <class faderType>
class AModuleFader {
public:
	// Fix fader duration (in s) beteween states
	void setFaderDuration(float duration) {
		fader.setDuration(duration);
	}
	//! Set display flag for Module
	void setFlagShow(bool b) {
		fader = b;
	}
	//! Get display flag for Module
	bool getFlagShow() const {
		return fader.finalState();
	}
	//! Inverse fader
	void flipFlagShow() {
		fader = !fader.finalState();
	}

protected:
    faderType fader;
};


template <class faderType>
class ModuleFader {
public:
	// Fix fader duration (in s) beteween states
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}
	//! Set display flag for Module
	void setFlagShow(bool b) {
		fader = b;
	}
	//! Get display flag for Module
	bool getFlagShow() const {
		return fader;
	}
	//! Inverse fader
	void flipFlagShow() {
		fader = !fader;
	}

protected:
    faderType fader;
};

#endif
