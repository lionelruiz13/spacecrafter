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

	//! draws a texture on the viewport
	void draw(double heading);

	//! indicates which texture id (in the CG) ViewPort will use for display
	//! \param _tex, ref uint32_t YUV textures in the CG
	void setTexture(VideoTexture _tex);

	//! build draw commands
	void build(int frameIdx);

	//! indicates if the class should be active or not
	void display(bool alive) {
		isAlive = true;
		fader=alive;
	}

	//! tells the class to go back to the start position
	void displayStop();

	//! indicates if the viewport displays the image on the whole dome or only 2 times a half
	void displayFullScreen(bool v) {
		fullScreen = v;
	}

	void disableFader() {
		disable_fader = true;
	}

	//! update the fader
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

	//! indicates whether to enable transparency on the KeyColor
	void setTransparency(bool v);

	//! KeyColor to use for transparency
	void setKeyColor(const Vec3f&color, float intensity);

	void createSC_context();

private:
	//initialization shader
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

	//uint32_t videoTex[3];	//!< indicates which YUV textures are used for display
	bool isAlive;		//!< activate the class
	bool fullScreen; 	//!< indicates the way to display the image
	bool skipping = false;		//!< initializes the variable defining if we skip fading or not
	ParabolicFader fader;
	bool disable_fader = false;
};

#endif // __VP_HPP__
