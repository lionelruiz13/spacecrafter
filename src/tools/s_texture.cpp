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
#include "tools/utility.hpp"

#include "tools/s_texture.hpp"
#include "tools/log.hpp"

std::string s_texture::texDir = "./";
std::map<std::string, s_texture::texRecap*> s_texture::texCache;

s_texture::s_texture(const std::string& _textureName) : textureName(_textureName), texID(0),
	loadType(PNG_BLEND1), loadWrapping(GL_CLAMP)
{
	//~ s_texture(_textureName, TEX_LOAD_TYPE_PNG_BLEND1, GL_CLAMP);
	bool succes;
	if (Utility::isAbsolute(textureName))
		succes = load(textureName);
	else
		succes = load(texDir + textureName);

	if (!succes)
		createEmptyTex();
}

s_texture::s_texture(const s_texture *t)
{
	textureName = t->textureName;
	loadType = t->loadType;
	loadWrapping = t->loadWrapping;
	texID = t->texID;

	it = texCache.find(textureName);

	if (it != texCache.end()) {
		texRecap * tmp = it->second;
		tmp->nbLink++;
	}
	else {
		//~ std::cout << "Erreur de duplication " << textureName << std::endl;
		cLog::get()->write("s_texture: erreur de duplication " + textureName , LOG_TYPE::L_ERROR);
	}
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
	bool succes;
	if (Utility::isAbsolute(textureName))
		succes = load(textureName, mipmap);
	else
		succes = load(texDir + textureName, mipmap);

	if (!succes)
		createEmptyTex();
}

s_texture::s_texture(const std::string& _textureName, GLuint _imgTex)
{
	//~ std::cout << "Création de s_texture " << _textureName <<" à partir d'un _imgTex" << std::endl;
	textureName = _textureName;
	texID = _imgTex;
	loadType = PNG_SOLID;
	loadWrapping = GL_CLAMP_TO_EDGE;
	texRecap * tmp = new texRecap;
	tmp->nbLink = 1;
	tmp->texID = texID;
	int w, h;
	int miplevel = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
	tmp->size = w*h*4;
	tmp->mipmap = false;
	texCache[textureName]= tmp;
}

s_texture::~s_texture()
{
	unload();
}

void s_texture::blend( const int type, unsigned char* const data, const unsigned int sz )
{
	unsigned char* a = nullptr;
	unsigned char* ptr = data;
	int r, g, b;

	switch( type ) {

		case PNG_BLEND1:
			for( unsigned int i = 0; i < sz; i+=4 ) {
				r = *ptr++;
				g = *ptr++;
				b = *ptr++;
				a = ptr++;
				if (r+g+b > 255) *a = 255;
				else *a = r+g+b;
			}
			break;

		case PNG_BLEND3:
			for( unsigned int i = 0; i < sz; i+=4 ) {
				r = *ptr++;
				g = *ptr++;
				b = *ptr++;
				a = ptr++;
				*a = (r+g+b)/3;
			}
			break;

		default:
			break;
	}
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
	//vérifions dans le cache si l'image n'est pas déjà utilisée ailleurs
	it = texCache.find(fullName);

	if (it != texCache.end()) { // texture existe deja
		//~ std::cout << "texture présente " << fullName << std::endl;
		texRecap * tmp = it->second;
		tmp->nbLink++;
		//~ std::cout << "on augmente son nbLink à " << tmp->nbLink << std::endl;
		texID = tmp->texID;
		cLog::get()->write("s_texture: already in cache " + fullName , LOG_TYPE::L_INFO);
		return true;
	} else { //texture n'existe pas, on l'intègre dans la map
		//code from Anthon Opengl4 tutorial
		try {
			int x, y, n;
			int force_channels = 4;
			unsigned char* image_data = nullptr;
			image_data = stbi_load (fullName.c_str(), &x, &y, &n, force_channels);
			if (!image_data) {
				cLog::get()->write("s_texture: could not load " + fullName , LOG_TYPE::L_ERROR);
				return false;
			}
			blend(loadType, image_data, x*y*4);

			// NPOT check
			if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
				cLog::get()->write("s_texture: not power-of-2 dimensions for " + fullName , LOG_TYPE::L_WARNING);
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

			texRecap * tmp = new texRecap;
			tmp->nbLink = 1;
			tmp->texID = texID;
			tmp->size = x*y*4;
			tmp->mipmap = mipmap;

			texCache[fullName]= tmp;

			// image_data != nullptr
			stbi_image_free(image_data);

		} catch( std::exception &e ) {
			//~ std::cerr << "WARNING : failed loading texture file! " << e.what() << std::endl;
			cLog::get()->write("s_texture: failed loading texture file " + fullName , LOG_TYPE::L_ERROR);
			return false;
		}
		cLog::get()->write("s_texture: loading " + fullName , LOG_TYPE::L_INFO);
		return true;
	}
	return false; //just for gcc
}

void s_texture::unload()
{
	//~ std::cout << "on unload la texture " << textureName << std::endl;
	it = texCache.find(textureName);

	if (it != texCache.end()) {
		texRecap * tmp = it->second;
		if (tmp->nbLink == 1) {
			//~ std::cout << "suppression réelle de " << textureName<< std::endl;
			glDeleteTextures(1, &texID);	// Delete The Texture
			texID = 0;
			delete tmp;
			texCache.erase(it);
		} else {
			//~ std::cout << "suppression virtuelle " << textureName<< std::endl;
			tmp->nbLink--;
			//~ std::cout << "Son link devient alors " << tmp->nbLink << std::endl;
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

// Return the average texture luminance : 0 is black, 1 is white
float s_texture::getAverageLuminance() const
{
	glBindTexture(GL_TEXTURE_2D, texID);
	GLint w, h , level=0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	//garde fou si texture trop petite
	if (w>64 && h>64) {
		level = 3;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &h);
	}
	GLfloat* p = (GLfloat*)calloc(w*h, sizeof(GLfloat));
	assert(p);

	glGetTexImage(GL_TEXTURE_2D, level, GL_LUMINANCE, GL_FLOAT, p);
	float sum = 0.f;
	for (int i=0; i<w*h; ++i) {
		sum += p[i];
	}
	free(p);

	return sum/(w*h);
}

unsigned long int s_texture::getTotalGPUMem()
{
	unsigned long int sizeGPUMem=0;
	texRecap* tmp=nullptr;
	std::map<std::string, texRecap*>::iterator itt;
	for(itt = texCache.begin(); itt != texCache.end(); itt++) {
		tmp= (itt)->second;
		(tmp->mipmap) ? sizeGPUMem += tmp->size*4/3 : sizeGPUMem = sizeGPUMem+tmp->size;
	}
	return sizeGPUMem;
}
