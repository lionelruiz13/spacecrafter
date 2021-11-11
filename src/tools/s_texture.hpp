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
#include <list>
#include <vector>
#include <memory>
#include "EntityCore/Tools/SafeQueue.hpp"
#include <vulkan/vulkan.h>

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

class s_texture {
public:
	// If resolution is set to 0, create a texture
	// Otherwise, look for a texture which resolution is (resolution*1024)x(resolution*512) by adding std::to_string(resolution) + "K" before the file extension
	// Note that a texture with lower power-of-two resolution may be querried depending to how many memory is available on the device
	// As general rule, the lowest resolution which can be querried is 4K
	s_texture(const std::string& _textureName, int _loadType = TEX_LOAD_TYPE_PNG_BLEND1, bool mipmap = false, int resolution = 0, int depth = 1, int nbChannels = 4, int channelSize = 1);
	// création d'une texture à partir d'une texture, transmet l'ownership de la texture à s_texture
	s_texture(const std::string& _textureName, Texture *_imgTex);
	// destructeur de texture
	~s_texture() = default;
	// création d'une texture par copie d'une autre
	s_texture(const s_texture &t) = delete;
	s_texture(const s_texture *t);
	//interdiction d'opérateur =
	const s_texture &operator=(const s_texture &t) = delete;

	// Return the big texture, or nullptr if not loaded at this frame
	Texture *getBigTexture();
	// Return the texture, request his loading if not loaded yet
	Texture &getTexture();

	// Return the average texture luminance : 0 is black, 1 is white
	float getAverageLuminance() const;

	// Returne les dimensions de la texture
	void getDimensions(int &width, int &height) const;
	void getDimensions(int &width, int &height, int &depth) const;

	// Indique le chemin par défaut des textures par défaut.
	static void setTexDir(const std::string& _texDir) {
		s_texture::texDir = _texDir;
	}

	// Indique si l'on doit charger les textures en low resolution ou pas.
	static void setLoadInLowResolution(bool value, int _maxRes) {
		s_texture::loadInLowResolution = value;
		s_texture::lowResMax = _maxRes*_maxRes*2;
	}

	// crée une texture rouge en cas de textures non chargée
	void createEmptyTex();

	static long int getNumberTotalTexture(){
		return texCache.size();
	}

	// Release every big textures which have not been querried with getBigTexture() for 2 frames
	static void update();
	// Unload every big textures
	static void forceUnload();
	// Release memory of every unused big textures, may have side effect
	static void releaseUnusedMemory();
	// Record transfer of every newly created textures
	static void recordTransfer(VkCommandBuffer cmd);
private:
	void unload();
	bool preload(const std::string& fullName, bool mipmap = false, int resolution = 0, int depth = 1, int nbChannels = 4, int channelSize = 1);
	bool load();
	struct bigTexRecap {
		unsigned int binding; // Binding id, increased by 1 when getting unused
		int resolution;
		std::unique_ptr<Texture> texture;
		std::string texName; // Name of the texture to load
		unsigned char lifetime = 0; // Number of frames before releasing this bigTexture
		bool ready = false; // Is this texture ready for use
		bool acquired = false; // Is this texture currently acquired
	};
	bigTexRecap *acquireBigTexture(int resolution);

	struct texRecap {
		unsigned long int size;
		int width;
		int height;
		int depth;
		std::unique_ptr<Texture> texture;
		int bigTextureResolution = 0;
		int bigTextureBinding = 0;
		bigTexRecap *bigTexture = nullptr;
		bool mipmap = false;
	};

	void blend( const int, unsigned char* const, const unsigned int );

	std::string textureName;
	std::shared_ptr<texRecap> texture;
	int loadType;
	int loadWrapping;
	int nbChannels;
	int channelSize;

	static std::string texDir;
	static bool loadInLowResolution;
	static int lowResMax;
	static std::map<std::string, std::weak_ptr<texRecap>> texCache;
	static std::list<bigTexRecap> bigTextures;
	static PushQueue<bigTexRecap *, 31> bigTextureQueue;
	static PushQueue<std::shared_ptr<texRecap>> textureQueue;
	static std::atomic<long> currentAllocation; // Allocations planned but not done yet
	static std::vector<std::shared_ptr<texRecap>> releaseMemory[3];
	static short releaseIdx;
};


#endif // _S_TEXTURE_H_
