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
//
//
#include "tools/fader.hpp"
#include "mediaModule/media_base.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

#define VP_FADER_DURATION 2000

class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class Navigator;

class ViewPort {
public:
	ViewPort();
	~ViewPort();

	//! trace une texture sur le viewport
	void draw(double heading);

	//! indique quelle id de texture (dans la CG) ViewPort utilisera pour affichage
	//! \param _tex, ref uint32_t textures YUV dans la CG
	void setTexture(VideoTexture _tex);

	//! build draw commands
	void build(int frameIdx);

	//! indique si la classe doit etre active ou pas
	void display(bool alive) {
		isAlive = true;
		fader=alive;
	}

	//! indique à la classe de se remettre en position de départ
	void displayStop();

	//! indique si le viewport affiche l'image sur tout le dôme ou jsute 2fois une moitiée
	void displayFullScreen(bool v) {
		fullScreen = v;
	}

	void disableFader() {
		//! fader.setDuration(400);
	}

	//! update le fader
	void update(int delta_time) {
		if (skipping and fader.isTransiting())
			fader.update(VP_FADER_DURATION);
		else
			fader.update(delta_time);
		if (not (fader.isTransiting() or fader)) {
			isAlive=false;
			skipping=false;
		}
	}

	//! indique si on active la transparence sur la KeyColor
	void setTransparency(bool v);

	//! KeyColor a utiliser pour la transparence
	void setKeyColor(const Vec3f&color, float intensity);

	void createSC_context();

private:
	//initialisation shader
	void initParam();
	//std::unique_ptr<shaderProgram> shaderViewPort;
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Set> set;
	struct s_frag {
		Vec4f noColor;
		float fader;
		VkBool32 transparency;
	};
	std::unique_ptr<SharedBuffer<s_frag>> uFrag;
	std::unique_ptr<VertexArray> vertexModel;
	std::unique_ptr<VertexBuffer> vertex; // First 4 = fullscreen, next 8 = dual
	std::shared_ptr<VideoSync> sync;
	VkCommandBuffer cmds[3];
	bool needUpdate[3]{};
	float lastHeading = 0;

	//uint32_t videoTex[3];	//!< indique quelles textures YUV sont utilisées pour affichage
	bool isAlive;		//!< active la classe
	bool fullScreen; 	//!< indique la façon d'afficher l'image
	bool skipping = false;		//!< initialise la variable définissant si on saute le fading ou non
	ParabolicFader fader;
};

#endif // __VP_HPP__
