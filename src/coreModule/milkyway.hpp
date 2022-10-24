/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2016 of the LSS Team & Association Sirius
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

#ifndef __MILKYWAY_HPP__
#define __MILKYWAY_HPP__

#include <string>
#include <fstream>
#include <memory>

#include "tools/fader.hpp"
#include "tools/vecmath.hpp"
#include "tools/scalable.hpp"
#include "tools/no_copy.hpp"

/*!
 * \file milkyway.hpp
 * \brief container of the MilkyWay class
 * \date 2016-03-10
 */

/*! \class MilkyWay
 * \brief class representing the management of the milky way
 *
 *  The class allows to display on a celestial sphere a texture of the Milky Way
 *
 *  It also represents the zodiacal light
 *
 */

class OjmL;
class Projector;
class Navigator;
class ToneReproductor;
class s_texture;
class Pipeline;
class PipelineLayout;
class Set;

class MilkyWay: public NoCopy {

public:
	MilkyWay();
	virtual ~MilkyWay();

	//! draws the sphere and the texture associated to the Milkyway.
	void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav, double julianDay);

	//! update the faders of the class
	void update(int delta_time) {
		showFader.update(delta_time);
		switchTexFader.update(delta_time);
		intensityMilky.update(delta_time);
		pollum.update(delta_time);
		zodiacalFader.update(delta_time);
	}

	//! changes the light intensity of the texture representing the Milkyway
	void setIntensity(float _intensity){
		intensityMilky = _intensity;
	}

	//! returns the intensity of the texture representing the Milkyway
	float getIntensity() const {
		return intensityMilky;
	};

	//! sets the state of the fader of the Milkyway display
	void setFlagShow(bool b) {
		showFader = b;
	}

	//! sets the state of the zodiacal light
	//! \param tex_file determines the name of the texture
	//! \param _intensity sets the base intensity associated with the texture
	void defineZodiacalState(const std::string& tex_file, float _intensity);

	//! sets the initial state of the Milkyway
	//! \param tex_file determines the name of the texture representing it
	//! \param _intensity determines the basic intensity associated with this texture
	void defineInitialMilkywayState(const std::string& path_file,const std::string& tex_file, const std::string& iris_tex_file, float _intensity);

	//! prepares the software for a change of Milkyway
	//! \param tex_file détermine le nom de la nouvelle texture
	//! \param _intensity determines the basic intensity associated with the new texture
	void changeMilkywayState(const std::string& full_tex_file, float _intensity);

	//! prepares the software to a change of Milkyway without touching its intensity
	//! \param tex_file determines the name of the new texture
	void changeMilkywayStateWithoutIntensity(const std::string& full_tex_file);


	//! retrieves the state of the fader of the Milkyway display
	bool getFlagShow(void) const {
		return showFader;
	}

	//! sets the state of the fader of the Milkyway display
	void setFlagZodiacal(bool b) {
		zodiacalFader = b;
	}

	//! retrieves the fader state from the Milkyway display
	bool getFlagZodiacal(void) const {
		return zodiacalFader;
	}

	//! sets the transition time between two display fader states
	void setFaderDuration(float f) {
		f *=1000;
		showFader.setDuration(f);
		switchTexFader.setDuration(f);
		zodiacalFader.setDuration(f);
		intensityMilky.setDuration(f);
		pollum.setDuration(f);
	}

	//! resets the intensity of the Milkyway to the initial intensity.
	void restoreIntensity() {
		intensityMilky = currentMilky.intensity;
	}

	//! swaps the currentTex and nextTex textures after a transition
	void endTexTransition();

	//! restores the default milkyway
	void restoreDefaultMilky();

	void useIrisTexture(bool v) {
		if (useIrisMilky) {
			displayIrisMilky = v;
		}
	}

	void needToUseIris(bool value) {
		useIrisMilky = value;
	}

	void setZodiacalIntensity(float _intensity) {
		zodiacal.intensity = _intensity;
	}

	void enableZodiacal(bool _allowZodiacal) {
		allowZodiacal = _allowZodiacal;
	}

	void setPollum(float value) {
		pollum = value;
	}

private:
	struct MilkyData{
		std::string name; // the exact name of the texture
		std::unique_ptr<s_texture> tex;
		float intensity;
	};

	int cmds[3] = {-1, -1, -1};
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<PipelineLayout> layoutTwoTex;
	Pipeline *pipelineMilky;
	std::unique_ptr<Pipeline> pipelineZodiacal;
	std::unique_ptr<Set> setMilky;
	std::unique_ptr<Set> setIrisMilky;
	std::unique_ptr<Set> setZodiacal;
	LinearFader showFader;
	ParabolicFader switchTexFader;
	LinearFader zodiacalFader;
	bool isDraw = false;

	OjmL* sphere = nullptr;

	bool onTextureTransition = false;		//!< indicates a transition on the texture
	bool displayIrisMilky = false;			//!< indicates that we need to use the iris texture
	bool useIrisMilky = false;				//!< do we need to use the iris texture?
	bool allowZodiacal = false;

	Scalable<float> intensityMilky;
	Scalable<float> pollum;

	Mat4d modelMilkyway;
	Mat4d modelZodiacal;

	MilkyData zodiacal;
	MilkyData defaultMilky;
	MilkyData currentMilky;
	MilkyData nextMilky;
	MilkyData irisMilky;


	void createSC_context();
	// void deleteShader();
	void initModelMatrix();			//! creation of the Model matrices for MilkyWay and Zodiacal
	void deleteMapTex();
	void buildMilkyway();
	void buildZodiacal();
};

#endif // __MILKYWAY_HPP__
