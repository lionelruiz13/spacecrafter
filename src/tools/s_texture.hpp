/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#ifndef _S_TEXTURE_H_
#define _S_TEXTURE_H_

#include <string>
//
#include <map>
#include <memory>
#include "vulkanModule/Context.hpp"

//TODO supprimer cela et les remplacer par un enum class
#define PNG_ALPHA  0
#define PNG_SOLID  1
#define PNG_BLEND1 7
#define PNG_BLEND3 2

#define TEX_LOAD_TYPE_PNG_ALPHA 0
#define TEX_LOAD_TYPE_PNG_SOLID 1
#define TEX_LOAD_TYPE_PNG_BLEND3 2
#define TEX_LOAD_TYPE_PNG_SOLID_REPEAT 4
#define TEX_LOAD_TYPE_PNG_BLEND1 7

class Texture;
class StreamTexture;

class s_texture {
public:
	// création d'une texteure en détaillant ses paramètres
	s_texture(const std::string& _textureName, int _loadType, const bool mipmap = false, const bool keepOnCPU = false);
	// création d'une texture à partir d'une texture dynamique
	s_texture(const std::string& _textureName, StreamTexture *_imgTex);
	// création d'une texture basique sans mipmap
	s_texture(const std::string& _textureName, const bool keepOnCPU = false);
	// destructeur de texture
	~s_texture();
	// création d'une texture par copie d'une autre
	s_texture(const s_texture &t) = delete;
	s_texture(const s_texture *t);
	//interdiction d'opérateur =
	const s_texture &operator=(const s_texture &t) = delete;

	Texture *getTexture() const {return texture;}

	// Return the average texture luminance : 0 is black, 1 is white
	float getAverageLuminance() const;

	// Returne les dimensions de la texture
	void getDimensions(int &width, int &height) const;

	// Indique le chemin par défaut des textures par défaut.
	static void setTexDir(const std::string& _texDir) {
		s_texture::texDir = _texDir;
	}

	// Indique si l'on doit charger les textures en low resolution ou pas.
	static void setLoadInLowResolution(bool value, int _maxRes) {
		s_texture::loadInLowResolution = value;
		s_texture::lowResMax = _maxRes;
	}

	// crée une texture rouge en cas de textures non chargée
	void createEmptyTex(const bool keepOnCPU);

	// DEPRECATED, Renvoie la taille utilisée par les textures dans la carte graphique
	static unsigned long int getTotalGPUMem();

	static long int getNumberTotalTexture(){
		return texCache.size();
	}

	static void setContext(ThreadContext *_context) {context = _context;}
	bool use();
	void unuse();
private:
	void unload();
	bool load(const std::string& fullName, bool mipmap = false, bool keepOnCPU = false);

	struct texRecap {
		int width;
		int height;
		unsigned long int size;
		int nbLink;
		Texture *texture;
		std::unique_ptr<Texture> textureHandle; // To destroy owned textures
		bool mipmap;
	};

	void blend( const int, unsigned char* const, const unsigned int );

	std::string textureName;
	int loadType;
	int loadWrapping;
	int width;
	int height;
	Texture *texture;

	static ThreadContext *context;
	static std::string texDir;
	static std::map<std::string, texRecap*> texCache;
	static bool loadInLowResolution;
	static int lowResMax;
	std::map<std::string, texRecap*>::iterator it;
};


#endif // _S_TEXTURE_H_
