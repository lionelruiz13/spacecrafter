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

#include <GL/glew.h>

struct VideoTexture {
    GLuint tex[3];
    GLuint &y = tex[0];
    GLuint &u = tex[1];
    GLuint &v = tex[2];
};

struct Resolution {
    int array[2];
    int &w = array[0];
    int &h = array[1];
};

enum class IMAGE_POSITIONING : char {
	POS_VIEWPORT,
	POS_HORIZONTAL,
	POS_EQUATORIAL,
	POS_J2000,
	POS_DOME
};

enum class IMG_COPIES : char{
    ONCE,
    TWICE,
    THRICE
};

#endif  // __MEDIA_BASE_HPP__
