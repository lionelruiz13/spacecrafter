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


#ifndef __MEDIA_BASE_HPP__
#define __MEDIA_BASE_HPP__


#include "vulkanModule/Texture.hpp"

struct VideoTexture {
	StreamTexture *tex[3];
	StreamTexture *&y = tex[0];
	StreamTexture *&u = tex[1];
	StreamTexture *&v = tex[2];
};

struct Resolution {
	int array[2];
	int &w = array[0];
	int &h = array[1];
};

enum class IMG_POSITION : char {
	POS_VIEWPORT,
	POS_HORIZONTAL,
	POS_EQUATORIAL,
	POS_J2000,
	POS_DOME
};

enum class IMG_PROJECT : char {
	ONCE,
	TWICE,
	THRICE
};

enum class VID_TYPE : char {
	V_NONE,
	V_VRCUBE,
	V_VR360,
	V_FULLVIEWPORT,
	V_DUALVIEWPORT,
	V_IMAGE
};

#endif  // __MEDIA_BASE_HPP__
