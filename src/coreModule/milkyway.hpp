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
 * \brief conteneur de la classe MilkyWay
 * \date 2016-03-10
 */

/*! \class MilkyWay
 * \brief classe representant le gestion de la voie lactée
 *
 *  La classe permet d'afficher sur une sphère céleste une texture de la voie lactée
 *
 *  Elle représente aussi la lumière zodiacale
 *
 */

class OjmL;
class s_font;
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

	//! dessine la sphère et la texture associée à la Milkyway.
	void draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav, double julianDay);

	//! update les faders de la classe
	void update(int delta_time) {
		showFader.update(delta_time);
		switchTexFader.update(delta_time);
		intensityMilky.update(delta_time);
		zodiacalFader.update(delta_time);
	}

	//! modifie l'intensité lumineuse de la texture représentant la Milkyway
	void setIntensity(float _intensity){
		intensityMilky = _intensity;
	}

	//! renvoie l'intensité de la texture représentant la Milkyway
	float getIntensity() const {
		return intensityMilky;
	};

	//! fixe l'état du fader de l'affichage de la Milkyway
	void setFlagShow(bool b) {
		showFader = b;
	}

	//! définie l'état de la lumière zodiacale
	//! \param tex_file détermine le nom de la texture
	//! \param _intensity détermine l'intensité de base associée à la texture
	void defineZodiacalState(const std::string& tex_file, float _intensity);

	//! définie l'état initial de la Milkyway
	//! \param tex_file détermine le nom de la texture la représenant
	//! \param _intensity détermine l'intensité de base associée à cette texture
	void defineInitialMilkywayState(const std::string& path_file,const std::string& tex_file, const std::string& iris_tex_file, float _intensity);

	//! prépare le logiciel à un changement de Milkyway
	//! \param tex_file détermine le nom de la nouvelle texture
	//! \param _intensity détermine l'intensité de base associée à la nouvelle texture
	void changeMilkywayState(const std::string& full_tex_file, float _intensity);

	//! prépare le logiciel à un changement de Milkyway sans toucher à son intensité
	//! \param tex_file détermine le nom de la nouvelle texture
	void changeMilkywayStateWithoutIntensity(const std::string& full_tex_file);


	//! récupère l'état du fader de l'affichage de la Milkyway
	bool getFlagShow(void) const {
		return showFader;
	}

	//! fixe l'état du fader de l'affichage de la Milkyway
	void setFlagZodiacal(bool b) {
		zodiacalFader = b;
	}

	//! récupère l'état du fader de l'affichage de la Milkyway
	bool getFlagZodiacal(void) const {
		return zodiacalFader;
	}

	//! fixe la durée de transition entre deux états du fader de l'affichage
	void setFaderDuration(float f) {
		f *=1000;
		showFader.setDuration(f);
		switchTexFader.setDuration(f);
		zodiacalFader.setDuration(f);
		intensityMilky.setDuration(f);
	}

	//! remet l'intensité de la Milkyway à l'intensité initiale.
	void restoreIntensity() {
		intensityMilky = currentMilky.intensity;
	}

	//! permutte les textures currentTex et nextTex après une transition
	void endTexTransition();

	//! restaure la milkyway par défaut
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

private:
	struct MilkyData{
		std::string name; // le nom exact de la texture
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

	bool onTextureTransition = false;		//!< indique uen transition sur la texture
	bool displayIrisMilky = false;			//!< indique que l'on doit utiliser la texture iris
	bool useIrisMilky = false;				//!< avons nous besoin d'utiliser la texture iris ?

	Scalable<float> intensityMilky;

	Mat4d modelMilkyway;
	Mat4d modelZodiacal;

	MilkyData zodiacal;
	MilkyData defaultMilky;
	MilkyData currentMilky;
	MilkyData nextMilky;
	MilkyData irisMilky;


	void createSC_context();
	// void deleteShader();
	void initModelMatrix();			//! création des matrices Model pour MilkyWay et Zodiacal
	void deleteMapTex();
	void buildMilkyway();
	void buildZodiacal();
};

#endif // __MILKYWAY_HPP__
