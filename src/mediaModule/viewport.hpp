/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/


#ifndef __VP_HPP__
#define __VP_HPP__

#include <memory>
#include "tools/vecmath.hpp"
//#include "tools/shader.hpp"
#include "tools/stateGL.hpp"
#include "tools/fader.hpp"
#include "yuv_wrapper.hpp"

#define VP_FADER_DURATION 3000

class VertexArray;
class shaderProgram;

class ViewPort {
public:
	ViewPort();
	~ViewPort();
	//! créer le shader
	void createShader();
	//! détruit le shader
	// void deleteShader();
	//! trace une texture sur le viewport
	void draw();
	//! indique quelle id de texture (dans la CG) ViewPort utilisera pour affichage
	//! \param _tex, YUV_WRAPPER YUV dans la CG
	void setTexture(YUV_WRAPPER _tex) {
		videoTex[0] = _tex.TexY;
		videoTex[1] = _tex.TexU;
		videoTex[2] = _tex.TexV;
	}

	//! indique si la classe doit etre active ou pas
	void display(bool alive) {
		isAlive = true;
		fader=alive;
	}

	//! indique à la classe de se remettre en position de départ
	void displayStop() {
		isAlive = false;
		fader=false;
		fader.update(VP_FADER_DURATION);
		transparency = false;
		noColor = Vec4f::null();
	}

	//! indique si le viewport affiche l'image sur tout le dôme ou jsute 2fois une moitiée
	void displayFullScreen(bool v) {
		fullScreen = v;
	}

	void disableFader() {
		fader.setDuration(400);
	}

	//! update le fader
	void update(int delta_time) {
		if (skipping and fader.isTransiting())
			fader.update(VP_FADER_DURATION);
		else
			fader.update(delta_time);
		if (not (fader.isTransiting() or fader.getState())) {
			isAlive=false;
			skipping=false;
		}
	}

	//! indique si on active la transparence sur la KeyColor
	void setTransparency(bool v) {
		transparency = v;
	}

	//! KeyColor a utiliser pour la transparence
	void setKeyColor(const Vec3f&color, float intensity) {
		noColor = Vec4f(color[0], color[1], color[2],intensity);
	}

	void createGL_context();

private:
	//initialisation shader
	void initParam();
	std::unique_ptr<shaderProgram> shaderViewPort;
	std::unique_ptr<VertexArray> m_dualGL, m_fullGL;

	GLuint videoTex[3];	//!< indique quelles textures YUV sont utilisées pour affichage
	bool isAlive;		//!< active la classe
	bool fullScreen; 	//!< indique la façon d'afficher l'image
	bool skipping = false;		//!< initialise la variable définissant si on saute le fading ou non
	bool transparency = false;	//!< active la transparence
	// indique quelle couleur est à effacer de l'image
	Vec4f noColor=Vec4f::null();
	ParabolicFader fader;
};

#endif // __VP_HPP__
