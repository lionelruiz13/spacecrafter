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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/milkyway.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/fmath.hpp"
#include "ojmModule/ojml.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "tools/s_font.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/tone_reproductor.hpp"




MilkyWay::MilkyWay()
{
	sphere = new OjmL(AppSettings::Instance()->getModel3DDir()+"MilkyWay.ojm");
	if (!sphere->getOk()) {
		cLog::get()->write("MilkyWay : error loading sphere, no draw", LOG_TYPE::L_ERROR);
		return;
	} else {
		cLog::get()->write("MilkyWay : successful loading sphere", LOG_TYPE::L_INFO);
		isDraw = true;
	}
	switchTexFader = false;
	intensityMilky.set(0.f);
	createShader();
	initModelMatrix();
}

void MilkyWay::createShader()
{
	shaderMilkyway= new shaderProgram();
	shaderMilkyway->init("milkyway.vert","milkyway.frag");
	
	shaderMilkyway->setUniformLocation("cmag");
	shaderMilkyway->setUniformLocation("texTransit");
	shaderMilkyway->setUniformLocation("ModelViewProjectionMatrix");
	shaderMilkyway->setUniformLocation("ModelViewMatrix");
	shaderMilkyway->setUniformLocation("inverseModelViewProjectionMatrix");

	shaderMilkyway->setSubroutineLocation(GL_FRAGMENT_SHADER, "useOneTex");
	shaderMilkyway->setSubroutineLocation(GL_FRAGMENT_SHADER, "useTwoTex");
}

void MilkyWay::initModelMatrix()
{
	modelMilkyway = Mat4d::scaling(1.1) *
	                Mat4d::xrotation(M_PI)*
	                Mat4d::yrotation(M_PI)*
	                Mat4d::zrotation(M_PI/180*270);

	modelZodiacal = Mat4d::scaling(1.0) *
		            Mat4d::xrotation(M_PI*23.5/180.0)*
		            Mat4d::yrotation(M_PI);
}


MilkyWay::~MilkyWay()
{
	if (sphere) delete sphere;
	deleteMapTex();
	deleteShader();
}


void MilkyWay::deleteShader()
{
	if (shaderMilkyway) delete shaderMilkyway;
}

void MilkyWay::deleteMapTex()
{
	if (defaultMilky.tex != nullptr) delete defaultMilky.tex;
	if (currentMilky.tex != nullptr) delete currentMilky.tex;
	if (nextMilky.tex != nullptr) delete nextMilky.tex;
}

void MilkyWay::defineZodiacalState(const std::string& tex_file, float _intensity)
{
	if (zodiacal.tex==nullptr) { //fist time to read this texture
		zodiacal.tex = new s_texture(tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
		zodiacal.intensity = std::clamp(_intensity, 0.f, 1.f);
		zodiacal.name = tex_file;
	} else {
		cLog::get()->write("Milkyway: zodicalState already exist, function aborded" , LOG_TYPE::L_WARNING);
	}
}


void MilkyWay::defineInitialMilkywayState(const std::string& path_file,const std::string& tex_file, const std::string& iris_tex_file, float _intensity)
{
	if (defaultMilky.tex==nullptr) {
		defaultMilky.tex = new s_texture(path_file + tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
		defaultMilky.intensity = std::clamp(_intensity, 0.f, 1.f);
		defaultMilky.name = path_file +tex_file;
		currentMilky.tex = new s_texture(path_file + tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
		currentMilky.intensity =  std::clamp(_intensity, 0.f, 1.f);
		currentMilky.name = path_file +tex_file;
		intensityMilky.set(currentMilky.intensity);

		if (useIrisMilky && !iris_tex_file.empty()) {
			irisMilky.tex = new s_texture(path_file + iris_tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
			irisMilky.intensity =  std::clamp(_intensity, 0.f, 1.f);
			irisMilky.name = path_file + iris_tex_file;
			cLog::get()->write("Milkyway: define irisMilky, name "+ iris_tex_file, LOG_TYPE::L_DEBUG);
		} else
			cLog::get()->write("Milkyway: no irisMilky define" , LOG_TYPE::L_DEBUG);
	} else {
		cLog::get()->write("Initial textures already define, function aborded", LOG_TYPE::L_WARNING);
	}
}

void MilkyWay::changeMilkywayStateWithoutIntensity(const std::string& full_tex_file)
{
	this->changeMilkywayState(full_tex_file,intensityMilky.final());
}


void MilkyWay::changeMilkywayState(const std::string& tex_file, float _intensity)
{
	if (nextMilky.tex!=nullptr) {
		delete nextMilky.tex;
	}
	nextMilky.tex = new s_texture(tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);	
	nextMilky.intensity = _intensity;
	nextMilky.name = tex_file;
	onTextureTransition = true;
	switchTexFader = true;
	setIntensity(nextMilky.intensity);
}


void MilkyWay::restoreDefaultMilky() 
{
	nextMilky.tex = new s_texture(defaultMilky.name, TEX_LOAD_TYPE_PNG_BLEND1, true);	
	// nextMilky.intensity = defaultMilky.intensity;
	nextMilky.name = defaultMilky.name;
	onTextureTransition = true;
	switchTexFader = true;
	setIntensity(defaultMilky.intensity);
	// cout << "Valeur de Intensity " << intensityMilky << endl;
}


void MilkyWay::endTexTransition()
{
	if (currentMilky.tex != nullptr)
		delete currentMilky.tex;

	currentMilky.tex = nextMilky.tex;
	currentMilky.name = nextMilky.name;
	nextMilky.tex = nullptr;
	onTextureTransition = false;
	switchTexFader = false;
}



void MilkyWay::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav, double julianDay)
{
	if (!isDraw) return;
	if (!showFader.getInterstate() ) return;

	// .045 chosen so that ad_lum = 1 at standard NELM of 6.5
	float ad_lum=eye->adaptLuminance(.045);

	// NB The tone reproducer code is simply incorrect, so this function fades out the Milky Way by the time eye limiting mag gets to ~5
	if(ad_lum < .9987 )
		ad_lum = -ad_lum*3.6168 +ad_lum*ad_lum*9.6253 -ad_lum*ad_lum*ad_lum*5.0121;
	if(ad_lum < 0) ad_lum = 0;

	float cmag = ad_lum  * showFader.getInterstate();

	StateGL::enable(GL_CULL_FACE);

	cmag *= intensityMilky;

	if (onTextureTransition && (switchTexFader.getInterstate()>0.99))
		endTexTransition();

	shaderMilkyway->use();

	//first MilkyWay
	glActiveTexture(GL_TEXTURE0);

	if (displayIrisMilky && currentMilky.name == defaultMilky.name)
		glBindTexture(GL_TEXTURE_2D, irisMilky.tex->getID());
	else
		glBindTexture(GL_TEXTURE_2D, currentMilky.tex->getID());

	shaderMilkyway->setUniform("cmag", cmag);

	if (onTextureTransition) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nextMilky.tex->getID());
		shaderMilkyway->setSubroutine(GL_FRAGMENT_SHADER, "useTwoTex");
		//~ std::cout << "cmag : " << cmag << std::endl;
		//~ std::cout << "Valeur transition : " << switchTexFader.getInterstate() << std::endl;
		shaderMilkyway->setUniform("texTransit", switchTexFader.getInterstate());
		glActiveTexture(GL_TEXTURE0);
	} else
		shaderMilkyway->setSubroutine(GL_FRAGMENT_SHADER, "useOneTex");

	Mat4f proj=prj->getMatProjection().convert();
	Mat4f matrix = (nav->getJ2000ToEyeMat() * modelMilkyway ).convert();
					//~ Mat4d::scaling(1.1) *
	                //~ Mat4d::xrotation(M_PI)*
	                //~ Mat4d::yrotation(M_PI)*
	                //~ Mat4d::zrotation(M_PI/180*270)).convert();

	shaderMilkyway->setUniform("inverseModelViewProjectionMatrix", (proj*matrix).inverse());
	shaderMilkyway->setUniform("ModelViewProjectionMatrix", proj*matrix);
	shaderMilkyway->setUniform("ModelViewMatrix",matrix);


	StateGL::disable(GL_BLEND);
	sphere->draw();

	//nextZodiacalLight
	if (zodiacal.tex != nullptr && zodiacalFader.getInterstate()) {
		
		cmag = ad_lum * zodiacal.intensity * zodiacalFader.getInterstate();
		//glActiveTexture(GL_TEXTURE0); ???
		glBindTexture(GL_TEXTURE_2D, zodiacal.tex->getID());
		shaderMilkyway->setUniform("cmag", cmag);
	
		//~ proj=prj->getMatProjection().convert();
		//	23.5 c'est l'obliquité de l'écliptique
		//	365.2422 c'est la période de révolution terrestre
		//	27.5 c'est le shift de la texture ça n'a aucun sens
		matrix = (nav->getJ2000ToEyeMat() * modelZodiacal *
						//~ Mat4d::scaling(1.0) *
		                //~ Mat4d::xrotation(M_PI*23.5/180.0)*
		                //~ Mat4d::yrotation(M_PI)*
		                Mat4d::zrotation(2*M_PI*(-julianDay+27.5)/365.2422)).convert();
	
		shaderMilkyway->setSubroutine(GL_FRAGMENT_SHADER, "useOneTex");
		shaderMilkyway->setUniform("inverseModelViewProjectionMatrix", (proj*matrix).inverse());
		shaderMilkyway->setUniform("ModelViewProjectionMatrix", proj*matrix);
		shaderMilkyway->setUniform("ModelViewMatrix",matrix);
	
	
		StateGL::enable(GL_BLEND);
		sphere->draw();
	}
	
	//end
	StateGL::disable(GL_CULL_FACE);
	shaderMilkyway->unuse();
}
