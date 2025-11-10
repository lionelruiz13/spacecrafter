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


#ifndef __VR360_HPP__
#define __VR360_HPP__

#include <memory>
//
#include "tools/auto_fader.hpp"
#include "mediaModule/media_base.hpp"
#include "tools/no_copy.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

// #define VR360_FADER_DURATION 3000

class Projector;
class Navigator;
class OjmL;
class Pipeline;
class PipelineLayout;
class Set;
//class shaderProgram;

class VR360: public NoCopy {
public:
	VR360();
	virtual ~VR360();

	void init();

	void setTexture(VideoTexture _tex);
	void setHasAlphaChannel(bool hasAlpha) { hasAlphaChannel = hasAlpha; }

	void modeCube() {
		typeVR360=TYPE::V_CUBE;
	}

	void modeSphere() {
		typeVR360=TYPE::V_SPHERE;
	}

	void draw(const Projector* prj, const Navigator* nav);

	void display(bool alive);
	void displayStop();
	//! build draw command
	void build();

	//! ccreation of shaders
	void createSC_context();
private:
	enum class TYPE : char { V_CUBE, V_SPHERE, V_NONE };

	OjmL* sphere = nullptr;
	OjmL* cube = nullptr;
	uint32_t videoTex[4]; // Support pour YUVA (4 textures)
	bool isAlive = false;
	bool canDraw = false;
	bool hasAlphaChannel = false; // Flag pour indiquer si la vidéo a un canal alpha

	struct UniformData {
		Mat4f mat;
		float fading;
		VkBool32 hasAlphaChannel;
	};

	//std::unique_ptr<shaderProgram> shaderVR360;
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Pipeline> pipelineAlpha;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Set> set;
	std::shared_ptr<VideoSync> sync;
	SharedBuffer<UniformData> uniform;
	VkCommandBuffer cmds[3];

	TYPE typeVR360 = TYPE::V_NONE;

	ALinearFader showFader;
};

#endif  //__VR360_HPP__
