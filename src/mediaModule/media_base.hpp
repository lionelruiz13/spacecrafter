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
    GLuint &rgb = tex[0];
};

struct Resolution {
    int array[2];
    int &w = array[0];
    int &h = array[1];
};

#endif  // __MEDIA_BASE_HPP__
