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
#include "tools/fader.hpp"
#include "mediaModule/media_base.hpp"
#include "tools/no_copy.hpp"
#include "vulkanModule/Context.hpp"

// #define VR360_FADER_DURATION 3000

class Projector;
class Navigator;
class OjmL;
class Pipeline;
class PipelineLayout;
class Set;
class Uniform;
//class shaderProgram;

class VR360: public NoCopy {
public:
	VR360();
	virtual ~VR360();

	void init(ThreadContext *context);

	void setTexture(VideoTexture _tex);

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

	//! update les faders de la classe
	void update(int delta_time) {
		showFader.update(delta_time);
	}

	//! création des shaders
	void createSC_context(ThreadContext *_context);
private:
	enum class TYPE : char { V_CUBE, V_SPHERE, V_NONE };

	OjmL* sphere = nullptr;
	OjmL* cube = nullptr;
	uint32_t videoTex[3];
	bool isAlive = false;
	bool canDraw = false;

	//std::unique_ptr<shaderProgram> shaderVR360;
	ThreadContext *context;
	int commandIndex;
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Set> set;
	std::unique_ptr<Uniform> uModelViewMatrix;
	Mat4f *pModelViewMatrix;

	TYPE typeVR360 = TYPE::V_NONE;

	LinearFader showFader;
};

#endif  //__VR360_HPP__
