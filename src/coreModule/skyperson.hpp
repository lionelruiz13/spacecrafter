/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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

#ifndef __SKYPERSON_H__
#define __SKYPERSON_H__

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
class SkyPerson {
public:
	enum PERSON_TYPE {
		AL,
		EQ
	};
	SkyPerson(PERSON_TYPE ptype);
	~SkyPerson();
	SkyPerson(SkyPerson const &) = delete;
	SkyPerson& operator = (SkyPerson const &) = delete;

	//!	void draw(const Projector* prj) const; 20060825 patch
	void draw(const Projector *prj,const Navigator *nav);
	void nautical_draw(const Projector *prj,const Navigator *nav, Vec3d equPos);
	void objCoord_draw(const Projector *prj,const Navigator *nav, Vec3d equPos);
	void angDist_draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);
	void loxodromy_draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);
	void orthodromy_draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);
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

	void loadData(std::string filename);

	void clear() ;

	void createShader();
	void deleteShader();

protected:
	Vec3f color;
	bool (Projector::*proj_func)(const Vec3d&, Vec3d&) const;
	void draw_text(const Projector *prj,const Navigator *nav);
	LinearFader fader;
	Vec3d pt0;
	Vec3d pt1;
	Vec3d pt2;
	Vec3d pt3;
	Vec3d pt4;
	Vec3d pt5;
	s_font * font;

private:
	std::vector<float> dataSky;
	double aperson;
	PERSON_TYPE ptype;
	DataGL sData;
	shaderProgram *shaderSkyPerson;
	const float deg2rad = 3.1415926/180.;   // Convert deg to radian
	const float rad2deg = 180./3.1415926;	// Converd radian to deg
	const float grad2rad = 3.1415926/18.;   // Convert grind pas to radian
	const float pi_div_2 = 1.5707963;		// pi/2
	
};

#endif // __SKYPERSON_H__



















//~ SettingsState state;
//~ if( line_type == EQUATOR ) {
//~ state.m_state.equator_line[0] = c[0];
//~ state.m_state.equator_line[1] = c[1];
//~ state.m_state.equator_line[2] = c[2];
//~ }
//~ if( line_type == GALACTIC_EQUATOR ) {
//~ state.m_state.galactic_line[0] = c[0];
//~ state.m_state.galactic_line[1] = c[1];
//~ state.m_state.galactic_line[2] = c[2];
//~ }
//~
//~ else if( line_type == ECLIPTIC ) {
//~ state.m_state.ecliptic_line[0] = c[0];
//~ state.m_state.ecliptic_line[1] = c[1];
//~ state.m_state.ecliptic_line[2] = c[2];
//~ }
//~ else if( line_type == MERIDIAN ) {
//~ state.m_state.meridian_line[0] = c[0];
//~ state.m_state.meridian_line[1] = c[1];
//~ state.m_state.meridian_line[2] = c[2];
//~ }
//~ else if( line_type == PRECESSION ) {
//~ state.m_state.precession_circle[0] = c[0];
//~ state.m_state.precession_circle[1] = c[1];
//~ state.m_state.precession_circle[2] = c[2];
//~ }
//~ else if( line_type == CIRCUMPOLAR ) {
//~ state.m_state.circumpolar_circle[0] = c[0];
//~ state.m_state.circumpolar_circle[1] = c[1];
//~ state.m_state.circumpolar_circle[2] = c[2];
//~ }
//~ else if( line_type == ZENITH ) {
//~ state.m_state.zenith_line[0] = c[0];
//~ state.m_state.zenith_line[1] = c[1];
//~ state.m_state.zenith_line[2] = c[2];
//~ }
//~ else if( line_type == POLE ) {
//~ state.m_state.polar_circle[0] = c[0];
//~ state.m_state.polar_circle[1] = c[1];
//~ state.m_state.polar_circle[2] = c[2];
//~ }
//~ else if( line_type == VERNAL_POINTS ) {
//~ state.m_state.vernal_points[0] = c[0];
//~ state.m_state.vernal_points[1] = c[1];
//~ state.m_state.vernal_points[2] = c[2];
//~ }
//~
//~ else if( line_type == ECLIPTIC_POLE) {
//~ state.m_state.ecliptic_center[0] = c[0];
//~ state.m_state.ecliptic_center[1] = c[1];
//~ state.m_state.ecliptic_center[2] = c[2];
//~ }
//~ else if( line_type == GALACTIC_POLE) {
//~ state.m_state.galactic_pole[0] = c[0];
//~ state.m_state.galactic_pole[1] = c[1];
//~ state.m_state.galactic_pole[2] = c[2];
//~ }
//~
//~ else if( line_type == GALACTIC_CENTER) {
//~ state.m_state.galactic_center[0] = c[0];
//~ state.m_state.galactic_center[1] = c[1];
//~ state.m_state.galactic_center[2] = c[2];
//~ }
//~
//~ else if( line_type == ANALEMMALINE ) {
//~ state.m_state.analemma_line[0] = c[0];
//~ state.m_state.analemma_line[1] = c[1];
//~ state.m_state.analemma_line[2] = c[2];
//~ }
//~ else if( line_type == ANALEMMA ) {
//~ state.m_state.analemma[0] = c[0];
//~ state.m_state.analemma[1] = c[1];
//~ state.m_state.analemma[2] = c[2];
//~ }
//~ SharedData::Instance()->Settings(state);

//~ ReferenceState state;
//~ if( line_type == MERIDIAN )
//~ state.meridian_line = b;
//~ else if( line_type == TROPIC )
//~ state.tropic_lines = b;
//~ else if( line_type == ECLIPTIC )
//~ state.ecliptic_line = b;
//~ else if( line_type == EQUATOR )
//~ state.equator_line = b;
//~ else if( line_type == GALACTIC_EQUATOR )
//~ state.galactic_line = b;
//~ else if( line_type == PRECESSION )
//~ state.precession_circle = b;
//~ else if( line_type == CIRCUMPOLAR )
//~ state.circumpolar_circle = b;
//~ else if( line_type == ZENITH )
//~ state.zenith_line = b;
//~ else if( line_type == POLE )
//~ state.polar_circle = b;
//~ else if( line_type == VERNAL_POINTS )
//~ state.vernal_points = b;
//~ else if( line_type == ECLIPTIC_POLE )
//~ state.ecliptic_center = b;
//~ else if( line_type == GALACTIC_POLE )
//~ state.galactic_pole = b;
//~ else if( line_type == GALACTIC_CENTER )
//~ state.galactic_center = b;
//~ else if( line_type == ANALEMMALINE )
//~ state.analemma_line = b;
//~ else if( line_type == ANALEMMA )
//~ state.analemma = b;
//~ SharedData::Instance()->References( state );

