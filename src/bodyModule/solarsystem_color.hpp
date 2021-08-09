/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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

#ifndef _SSYSTEMCOLOR_H_
#define _SSYSTEMCOLOR_H_

#include "solarsystem.hpp"

class SolarSystemColor {
public:
    SolarSystemColor(ProtoSystem * _ssystem);
    ~SolarSystemColor();

	void changeSystem(ProtoSystem * _ssystem) {
		ssystem = _ssystem;
	}

    void setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c);

    //void setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c);
	const Vec3f getBodyColor(const std::string &englishName, const std::string& colorName) const;
    
    void setDefaultBodyColor(const std::string& colorName, const Vec3f& c);
	const Vec3f getDefaultBodyColor(const std::string& colorName) const;

	void setDefaultBodyColor(const std::string& halo, const std::string& label, const std::string& orbit, const std::string& trail) {
		BodyColor::setDefault(halo, label, orbit, trail);
	}

    //return the default halo, label, orbit and trail color for default body
	void iniColor(const std::string& _halo, const std::string& _label, const std::string& _orbit, const std::string& _trail) {
		BodyColor::setDefault(_halo, _label, _orbit, _trail);
	}

private:
    ProtoSystem * ssystem;
};

#endif