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

#include "tools/shader.hpp"
#include "tools/fader.hpp"
#include "yuv_wrapper.hpp"

#define VR360_FADER_DURATION 3000

class Projector;
class Navigator;
class OjmL;

class VR360 {
public:
	VR360();
	virtual ~VR360();
	VR360(VR360 const &) = delete;
	VR360& operator = (VR360 const &) = delete;

	void setTexture(YUV_WRAPPER _tex) {
		videoTex[0] = _tex.TexY;
		videoTex[1] = _tex.TexU;
		videoTex[2] = _tex.TexV;
	}

	void modeCube() {
		typeVR360=TYPE::V_CUBE;
	}

	void modeSphere() {
		typeVR360=TYPE::V_SPHERE;
	}

	void draw(const Projector* prj, const Navigator* nav);

	void display(bool alive);
	void displayStop();

	//! update les faders de la classe
	void update(int delta_time) {
		showFader.update(delta_time);
	}

private:
	enum class TYPE : char { V_CUBE, V_SPHERE, V_NONE };

	//! création des shaders
	void createShader();
	//! supression des shaders
	void deleteShader();

	OjmL* sphere = nullptr;
	OjmL* cube = nullptr;
	GLuint videoTex[3];
	bool isAlive = false;
	bool canDraw = false;

	shaderProgram* shaderVR360 = nullptr;
	DataGL VR360GL;
	TYPE typeVR360 = TYPE::V_NONE;

	LinearFader showFader;
};

#endif  //__VR360_HPP__
