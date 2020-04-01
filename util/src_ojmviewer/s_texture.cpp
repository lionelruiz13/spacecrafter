/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2018 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <iostream>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <exception>
#include "s_texture.hpp"


s_texture::s_texture(const std::string& _textureName) : textureName(_textureName), texID(0),
	loadType(PNG_BLEND1), loadWrapping(GL_CLAMP)
{
	bool succes = load(textureName);
	if (!succes)
		createEmptyTex();
}

s_texture::s_texture(const std::string& _textureName, int _loadType, const bool mipmap) : textureName(_textureName),
	texID(0), loadType(PNG_BLEND1), loadWrapping(GL_CLAMP_TO_EDGE)
{
	switch (_loadType) {
		case TEX_LOAD_TYPE_PNG_ALPHA :
			loadType=PNG_ALPHA;
			break;
		case TEX_LOAD_TYPE_PNG_SOLID :
			loadType=PNG_SOLID;
			break;
		case TEX_LOAD_TYPE_PNG_BLEND3:
			loadType=PNG_BLEND3;
			break;
		case TEX_LOAD_TYPE_PNG_BLEND1:
			loadType=PNG_BLEND1;
			break;
		case TEX_LOAD_TYPE_PNG_SOLID_REPEAT:
			loadType=PNG_SOLID;
			loadWrapping=GL_REPEAT;
			break;
		default :
			loadType=PNG_BLEND3;
	}
	texID=0;
	bool succes = load(textureName, mipmap);
	if (!succes)
		createEmptyTex();
}

s_texture::~s_texture()
{
	unload();
}

bool s_texture::load(const std::string& fullName)
{
	// assume NO mipmap - DIGITALIS - put in svn
	return load(fullName, false);
}

void s_texture::createEmptyTex()
{
	glGenTextures (1, &texID);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, texID);

	unsigned char image_data[4] = {255,0,0,255};
	glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, loadWrapping);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, loadWrapping);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//~ std::cout << "texture createEmptyTex" << textureName << std::endl;
}

bool s_texture::load(const std::string& fullName, bool mipmap)
{
	//~ std::cout << "lecture de la texture |"<< fullName << "| " << std::endl;

		//code from Anthon Opengl4 tutorial
		try {
			int x, y, n;
			int force_channels = 4;
			unsigned char* image_data = nullptr;
			image_data = stbi_load (fullName.c_str(), &x, &y, &n, force_channels);
			if (!image_data) {
				std::cout << "s_texture: could not load " + fullName << std::endl;
				return false;
			}

			// NPOT check
			if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
				std::cout << "s_texture: not power-of-2 dimensions for " << fullName << std::endl;
				//~ fprintf (stderr, "WARNING: texture %s is not power-of-2 dimensions\n", fullName.c_str());
			}
			int width_in_bytes = x * 4;
			unsigned char *top = nullptr;
			unsigned char *bottom = nullptr;
			unsigned char temp = 0;
			int half_height = y / 2;

			for (int row = 0; row < half_height; row++) {
				top = image_data + row * width_in_bytes;
				bottom = image_data + (y - row - 1) * width_in_bytes;
				for (int col = 0; col < width_in_bytes; col++) {
					temp = *top;
					*top = *bottom;
					*bottom = temp;
					top++;
					bottom++;
				}
			}

			glGenTextures (1, &texID);
			glActiveTexture (GL_TEXTURE0);
			glBindTexture (GL_TEXTURE_2D, texID);
			glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,x,y,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);

			if( mipmap ) {
				glGenerateMipmap (GL_TEXTURE_2D);
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			} else {
				glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}

			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, loadWrapping);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, loadWrapping);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			GLfloat max_aniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
			// set the maximum!
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

			// image_data != nullptr
			stbi_image_free(image_data);

		} catch( std::exception &e ) {
			//~ std::cerr << "WARNING : failed loading texture file! " << e.what() << std::endl;
			std::cout << "s_texture: failed loading texture file " << fullName << std::endl;
			return false;
		}
		std::cout << "s_texture: loading " << fullName << std::endl;
		return true;
}

void s_texture::unload()
{
	//~ std::cout << "on unload la texture " << textureName << std::endl;
	glDeleteTextures(1, &texID);	// Delete The Texture
}

void s_texture::getDimensions(int &width, int &height) const
{
	glBindTexture(GL_TEXTURE_2D, texID);

	GLint w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	width = w;
	height = h;
}

// Return the average texture luminance : 0 is black, 1 is white
float s_texture::getAverageLuminance() const
{
	glBindTexture(GL_TEXTURE_2D, texID);
	GLint w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	GLfloat* p = (GLfloat*)calloc(w*h, sizeof(GLfloat));
	assert(p);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_FLOAT, p);
	float sum = 0.f;
	for (int i=0; i<w*h; ++i) {
		sum += p[i];
	}
	free(p);

	return sum/(w*h);
}
