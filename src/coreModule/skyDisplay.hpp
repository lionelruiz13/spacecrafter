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
#include <memory>
#include <vector>

#include "tools/fader.hpp"
// #include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "tools/no_copy.hpp"
#include "tools/vecmath.hpp"


class Projector;
class Navigator;
class ToneReproductor;
class Translator;
class s_font;
class VertexArray;
class shaderProgram;

//! Class which manages a personal line to display around the sky
class SkyDisplay: public NoCopy  {
public:
	enum PROJECTION_TYPE {
		AL,
		EQ
	};
	SkyDisplay(PROJECTION_TYPE ptype);
	virtual ~SkyDisplay();

	virtual void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0));

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

	void flipFlagShow() {
		fader = !fader;
	}

	virtual void loadData(const std::string& filename){};
	virtual void loadString(const std::string& message){};

	void clear() ;

	static void setFont(s_font* _font){
		skydisplay_font = _font;
	}

	static void createShader();
	void createVao();

protected:
	Vec3f color;
	double aperson;
	bool (Projector::*proj_func)(const Vec3d&, Vec3d&) const;
	void draw_text(const Projector *prj,const Navigator *nav);
	LinearFader fader;
	Vec3d pt0, pt1, pt2, pt3, pt4, pt5;

	static s_font* skydisplay_font;

	std::vector<float> dataSky;
	PROJECTION_TYPE ptype;
	std::unique_ptr<VertexArray> m_dataGL;
	static std::unique_ptr<shaderProgram> shaderSkyDisplay;

private:
};

class SkyPerson : public SkyDisplay {
public:
	SkyPerson(PROJECTION_TYPE ptype);
	~SkyPerson() {};

	//void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0)) override;
	void loadData(const std::string& filename) override;
	void loadString(const std::string& message) override;

private:
};

class SkyNautic : public SkyDisplay {
public:
	SkyNautic(PROJECTION_TYPE ptype);
	~SkyNautic() {};

	void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0));

private:
};


class SkyCoords : public SkyDisplay {
public:
	SkyCoords();
	~SkyCoords() {};

	void draw(const Projector *prj,const Navigator *nav, Vec3d equPos= Vec3f(0,0,0), Vec3d oldEquPos= Vec3f(0,0,0));

private:
};

class SkyMouse : public SkyDisplay {
public:
	SkyMouse();
	~SkyMouse() {};

	void draw(const Projector *prj,const Navigator *nav, Vec3d _equPos= Vec3f(0,0,0), Vec3d _oldEquPos= Vec3f(0,0,0));

private:
};


class SkyAngDist : public SkyDisplay {
public:
	SkyAngDist();
	~SkyAngDist() {};

	void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);

private:
};

class SkyLoxodromy : public SkyDisplay {
public:
	SkyLoxodromy();
	~SkyLoxodromy() {};

	void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);

private:
};

class SkyOrthodromy : public SkyDisplay {
public:
	SkyOrthodromy();
	~SkyOrthodromy() {};

	void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);

private:
};

#endif // __SKYDISPLAY_HPP__