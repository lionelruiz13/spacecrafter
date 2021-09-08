/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

//! @class Atmosphere
//!	@brief Class which compute and display the daylight sky color using openGL
//!	sky is computed with the Skylight class.

#ifndef _ATMOSTPHERE_H_
#define _ATMOSTPHERE_H_

#include <vector>
#include <memory>
#include "atmosphereModule/skybright.hpp"
#include "atmosphereModule/skylight.hpp"
#include "tools/fader.hpp"


#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"

#include "vulkanModule/Context.hpp"


class Skylight;
class Skybright;
class Projector;
class Navigator;
class ToneReproductor;
class VertexArray;
class Pipeline;

class Atmosphere: public NoCopy  {
public:
	Atmosphere(ThreadContext *context);
	virtual ~Atmosphere();

	void computeColor(double JD, Vec3d sunPos, Vec3d moonPos, float moon_phase, const ToneReproductor * eye, const Projector* prj, const std::string &planetName,
	                   float latitude = 45.f, float altitude = 200.f,
	                   float temperature = 15.f, float relative_humidity = 40.f);

	//! Draw the atmosphere using the precalc values stored in tab_sky
	void draw(const Projector* prj, const std::string &planetName);

	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! Set fade in/out duration in seconds
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}
	//! Get fade in/out duration in seconds
	float getFaderDuration() {
		return fader.getDuration()/1000.f;
	}

	//! Define whether to display atmosphere
	void setFlagShow(bool b) {
		fader = b;
	}

	//! Get whether atmosphere is displayed
	bool getFlagShow() const {
		return fader;
	}

	//! flip whether to display atmosphere
	void flipFlagShow() {
		fader = !fader;
	}

	//! tells you actual atm intensity due to eclipses + fader
	float getIntensity(void) {
		return atm_intensity;
	}

	//! let's you know how far faded in or out the atm is (0-1)
	float getFadeIntensity(void) {
		return fader.getInterstate();
	}

	float getWorldAdaptationLuminance(void) const { //unused
		return world_adaptation_luminance;
	}

	float getMilkywayAdaptationLuminance(void) const {
		return milkyway_adaptation_luminance;
	}

	//! for determining world adaptation luminance
	//! @warning DO NOT SET DIRECTLY, use core->setLightPollutionLimitingMagnitude instead
	void setLightPollutionLuminance(float luminance) {
		lightPollutionLuminance = luminance;
	}

	// void setFlagOptoma (bool b) {
	// 	(b==0) ? cor_optoma=0 : cor_optoma=1;
	// }

	// //! Get the Optoma flag
	// bool getFlagOptoma() const {
	// 	if (cor_optoma)
	// 		return true;
	// 	else return false;
	// }

	float getLightPollutionLuminance() {
		return lightPollutionLuminance;
	}

	//! construit la grille d'affichage des points pour les shaders
	void initGridPos();

	//! determine le viewport pour la construction des grilles
	void initGridViewport(const Projector *prj);

private:
	//! initialise les paramètres du shader
	void createSC_context(ThreadContext *context);
	//! remplir les couleurs du conteneur
	void fillOutDataColor();
	// void deleteShader();

	Skylight sky;
	Skybright skyb;
	Vec3f ** tab_sky=nullptr;	//!< For Atmosphere calculation

	float world_adaptation_luminance;
	float milkyway_adaptation_luminance;
	float atm_intensity;
	ParabolicFader fader;
	float lightPollutionLuminance; 	//! light pollution simulation, add to svn 20070220
//	int cor_optoma; //! flag for correction vp optoma

	std::vector<float> dataColor;
	std::vector<float> dataPos;
	std::unique_ptr<Pipeline> pipeline;
	int commandIndex;
	CommandMgr *commandMgr;
	std::unique_ptr<VertexArray> m_atmGL;

	//variables sur la position de la grille
	float stepX; //!< taille des pas sur l'axe des x
	float stepY; //!< taille des pas sur l'axe des y
	float viewport_left; //!<espacement à gauche de la grille
	float viewport_bottom; //!< espacement en bas de la grille
};

#endif // _ATMOSTPHERE_H_
