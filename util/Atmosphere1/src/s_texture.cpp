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

std::string s_texture::texDir = "";
std::map<std::string, s_texture::texRecap*> s_texture::texCache;


s_texture::s_texture(const std::string& _textureName) : textureName(_textureName), texID(0),
	loadType(PNG_BLEND1), loadWrapping(GL_CLAMP)
{
	bool succes;
	if (textureName[0]=='/')
		succes = load(textureName);
	else
		succes = load(texDir + textureName);
}

s_texture::~s_texture()
{
	unload();
}

bool s_texture::load(std::string fullName)
{
	// assume NO mipmap - DIGITALIS - put in svn
	return load(fullName, false);
}

bool s_texture::load(std::string fullName, bool mipmap)
{
	std::cout << "reading the texture |"<< fullName << "| " << std::endl;
	// let's check in the cache if the image is not already used elsewhere
	it = texCache.find(fullName);

	if (it != texCache.end()) { // texture already exists
		//~ std::cout << "present texture " << fullName << std::endl;
		texRecap * tmp = it->second;
		tmp->nbLink++;
		//~ std::cout << "we increase its nbLink to " << tmp->nbLink << std::endl;
		texID = tmp->texID;
		//Log.write("s_texture: already in cache " + fullName , cLog::LOG_TYPE::L_INFO);
		return true;
	} else { // texture does not exist, we integrate it in the map
		//code from Anthon Opengl4 tutorial
		try {
			int x, y, n;
			int force_channels = 4;
			unsigned char* image_data = stbi_load (fullName.c_str(), &x, &y, &n, force_channels);
			if (!image_data) {
				//Log.write("s_texture: could not load " + fullName , cLog::LOG_TYPE::L_ERROR);
				return false;
			}
			
			int width_in_bytes = x * 4;
			unsigned char *top = NULL;
			unsigned char *bottom = NULL;
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

			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			texRecap * tmp = new texRecap;
			tmp->nbLink = 1;
			tmp->texID = texID;
			tmp->size = x*y*4;
			tmp->mipmap = mipmap;

			texCache[fullName]= tmp;

		} catch( std::exception &e ) {
			std::cerr << "WARNING : failed loading texture file! " << e.what() << std::endl;
			//Log.write("s_texture: failed loading texture file " + fullName , cLog::LOG_TYPE::L_ERROR);
			return false;
		}
		//Log.write("s_texture: loading " + fullName , cLog::LOG_TYPE::L_INFO);
		return true;
	}
	return false; //just for gcc
}

void s_texture::unload()
{
	//~ std::cout << "we unload the texture " << textureName << std::endl;
	it = texCache.find(textureName);

	if (it != texCache.end()) {
		texRecap * tmp = it->second;
		if (tmp->nbLink == 1) {
			//~ std::cout << "actual deletion of " << textureName<< std::endl;
			glDeleteTextures(1, &texID);	// Delete The Texture
			texID = 0;
			delete tmp;
			texCache.erase(it);
		} else {
			//~ std::cout << "virtual deletion " << textureName<< std::endl;
			tmp->nbLink--;
			//~ std::cout << "His link then becomes " << tmp->nbLink << std::endl;
		}
	}
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

unsigned long int s_texture::getTotalGPUMem()
{
	unsigned long int sizeGPUMem=0;
	texRecap* tmp=NULL;
	std::map<std::string, texRecap*>::iterator itt;
	for(itt = texCache.begin(); itt != texCache.end(); itt++) {
		tmp= (itt)->second;
		(tmp->mipmap) ? sizeGPUMem += tmp->size*4/3 : sizeGPUMem = sizeGPUMem+tmp->size;
	}
	return sizeGPUMem;
}
