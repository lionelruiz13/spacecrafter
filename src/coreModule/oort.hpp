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

#ifndef ___OORT_HPP___
#define ___OORT_HPP___

#include <string>
#include <fstream>

#include "tools/fader.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include <vector>


//! Class which manages the Oort Cloud

class Projector;
class Navigator;

class Oort {
public:
	Oort();
	~Oort();

	//! affiche le nuage de points
	void draw(double distance, const Projector *prj,const Navigator *nav) noexcept;

	//! fixe la couleur du nuage
	void setColor(const Vec3f& c) {
		color = c;
	}

	//! renvoie la couleur du nuage
	const Vec3f& getColor() {
		return color;
	}

	//! mise à jour du fader
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! modifie la durée du fader
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	//! modifie le fader
	void setFlagShow(bool b) {
		fader = b;
	}

	//! renvoie la valeur du fader
	bool getFlagShow(void) const {
		return fader;
	}

	//! construit le nuage
	//! \param nbr le nombre de points dans le nuage
	void populate(unsigned int nbr) noexcept;

private:
	//vide le tampon
	void clear();
	// initialise le shader et les vao-vbo
	void createShader();
	// supprime le shader et les vao-vbo
	void deleteShader();

	// couleur uniforme du nuage
	Vec3f color;
	// fader pour affichage
	LinearFader fader;
	// coefficient sur l'intensité lumineuse
	float intensity;
	//tableau de float pour tampon openGL
	std::vector<float> dataOort;
	// données openGL
	DataGL sData;
	// shader responsable de l'affichage du nuage
	shaderProgram *shaderOort;
};

#endif // ___OORT_HPP___



















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

