/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2020 of the LSS Team & Association Sirius
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

#ifndef __SKYDISPLAY_HPP__
#define __SKYDISPLAY_HPP__

#include <string>
#include <fstream>
#include "tools/fader.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include <vector>

class Projector;
class Navigator;
class ToneReproductor;
class Translator;

//! Class which manages a personal line to display around the sky
class SkyDisplay {
public:
	enum PROJECTION_TYPE {
		AL,
		EQ
	};
	SkyDisplay(PROJECTION_TYPE ptype);
	~SkyDisplay();
	SkyDisplay(SkyDisplay const &) = delete;
	SkyDisplay& operator = (SkyDisplay const &) = delete;

	//!	void draw(const Projector* prj) const; 20060825 patch
	virtual void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0)) = 0;

	void setColor(const Vec3f& c) {
		color = c;
	}

	const Vec3f& getColor() {
		return color;
	}

	void update(int delta_time) {
		fader.update(delta_time);
	}

	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	void setFlagShow(bool b) {
		fader = b;
	}

	bool getFlagShow(void) const {
		return fader;
	}

	virtual void loadData(std::string filename);

	void clear() ;

	void createShader();
	void deleteShader();

protected:
	Vec3f color;
	bool (Projector::*proj_func)(const Vec3d&, Vec3d&) const;
	void draw_text(const Projector *prj,const Navigator *nav);
	LinearFader fader;
	Vec3d pt0, pt1, pt2, pt3, pt4, pt5;

	s_font * font;
	std::vector<float> dataSky;
	PROJECTION_TYPE ptype;
	DataGL sData;
	shaderProgram *shaderSkyDisplay;
	double aperson;
private:
	// const float deg2rad = 3.1415926/180.;   // Convert deg to radian
	// const float rad2deg = 180./3.1415926;	// Converd radian to deg
	// const float grad2rad = 3.1415926/18.;   // Convert grind pas to radian
	// const float pi_div_2 = 1.5707963;		// pi/2
	
};

class SkyPersonR : public SkyDisplay {
public:
    SkyPersonR(PROJECTION_TYPE ptype);
    ~SkyPersonR();

    void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0)) override;
	void loadData(std::string filename) override;

private:
};

class SkyNautic : public SkyDisplay {
public:
    SkyNautic(PROJECTION_TYPE ptype);
    ~SkyNautic();

    void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0));

private:
};


class SkyCoords : public SkyDisplay {
public:
    SkyCoords();
    ~SkyCoords();

    void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0));

private:
};

class SkyAngDist : public SkyDisplay {
public:
    SkyAngDist();
    ~SkyAngDist();

    void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);

private:
};

class SkyLoxodromy : public SkyDisplay {
public:
    SkyLoxodromy();
    ~SkyLoxodromy();

    void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);

private:
};

class SkyOrthodromy : public SkyDisplay {
public:
    SkyOrthodromy();
    ~SkyOrthodromy();

    void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);

private:
};

#endif // __SKYDISPLAY_HPP__