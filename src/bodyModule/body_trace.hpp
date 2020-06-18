/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015-2020 of the LSS Team & Association Sirius
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

#ifndef __BODYTRACE_H__
#define __BODYTRACE_H__

#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "tools/fader.hpp"
#include "tools/stateGL.hpp"
#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"

#define NB_MAX_LIST 7
#define MAX_POINTS 16384

class Navigator;
class Projector;
class VertexArray;
class shaderProgram;


//! Class which manages a line to display an object position around the sky
class BodyTrace: public NoCopy {
public:
	struct BodyList {
		Vec3f color;
		int size;
		Vec3f punts[MAX_POINTS];
		Vec2f old_punt;
		bool hide;
	};

	BodyTrace();
	virtual ~BodyTrace();

	void draw(const Projector *prj,const Navigator *nav);

	void setColor(const Vec3f& c, int numberlist) {
		bodyData[numberlist].color = c;
	}

	const Vec3f& getColor( int numberlist) {
		return bodyData[numberlist].color;
	}

	void hide(int numberList);

	void update(int delta_time) {
		fader.update(delta_time);
	}

	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	void setFlagShow(bool b) {
		fader = b;
		if (b==false)
			clear();
	}

	void flipFlagShow() {
		fader = ! fader;
		if (fader==false)
			clear();
	}

	bool getFlagShow(void) const {
		return fader;
	}

	void addData(const Navigator *nav, double alt, double ra);

	void upPen() {
		is_tracing = false;
		currentUsedList+=1;
		if (currentUsedList==NB_MAX_LIST)
			currentUsedList=NB_MAX_LIST-1;
	}

	void downPen() {
		is_tracing = true;
	}

	void togglePen() {
		if (is_tracing==true)
			upPen();
		else
			downPen();
	}

	void clear() {
		currentUsedList=0;
		for(int i= 0; i<NB_MAX_LIST; i++)
			bodyData[i].size=0;
	}


private:
	BodyList bodyData[NB_MAX_LIST];
	bool is_tracing;
	int currentUsedList;
	LinearFader fader;
	Vec3d pt1;
	Vec3d pt2;

	void createGL_context();
	std::vector<float> vecVertex;
	std::unique_ptr<shaderProgram> shaderTrace;
	std::unique_ptr<VertexArray> m_dataGL;
};


#endif // __BODYTRACE_H__


