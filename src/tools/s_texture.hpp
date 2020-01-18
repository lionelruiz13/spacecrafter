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
#include <GL/glew.h>
#include <map>

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


class s_texture {
public:
	// création d'une texture basique sans mipmap
	s_texture(const std::string& _textureName);

	// création d'une texteure en détaillant ses paramètres
	s_texture(const std::string& _textureName, int _loadType, const bool mipmap = false);
	// création d'une texture à partir d'un GLuint
	s_texture(const std::string& _textureName, GLuint _imgTex);
	// destructeur de texture
	~s_texture();
	// création d'une texture par copie d'une autre
	s_texture(const s_texture &t) = delete;
	s_texture(const s_texture *t);
	//interdiction d'opérateur = 
	const s_texture &operator=(const s_texture &t) = delete;

	// Renvoie la référence de la texture en openGL
	unsigned int getID() const {
		return texID;
	}

	// Return the average texture luminance : 0 is black, 1 is white
	float getAverageLuminance() const;

	// Returne les dimensions de la texture
	void getDimensions(int &width, int &height) const;

	// Indique le chemin par défaut des textures par défaut.
	static void setTexDir(const std::string& _texDir) {
		s_texture::texDir = _texDir;
	}

	// crée une texture rouge en cas de textures non chargée
	void createEmptyTex();

	// Renvoie la taille utilisée par les textures dans la carte graphique
	static unsigned long int getTotalGPUMem();

	static long int getNumberTotalTexture(){
		return texCache.size();
	}

private:
	void unload();
	bool load(const std::string& fullName);
	bool load(const std::string& fullName, bool mipmap);

	struct texRecap {
		unsigned long int size;
		int nbLink;
		GLuint texID;
		bool mipmap;
	};

	void blend( const int, unsigned char* const, const unsigned int );

	std::string textureName;
	GLuint texID;
	int loadType;
	GLint loadWrapping;

	static std::string texDir;
	static std::map<std::string, texRecap*> texCache;
	std::map<std::string, texRecap*>::iterator it;
};


#endif // _S_TEXTURE_H_
