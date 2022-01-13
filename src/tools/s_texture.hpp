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
#include <thread>

class BigSave;

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
class Set;
class ComputePipeline;
class PipelineLayout;

class s_texture {
public:
	// If resolution is set to 0, create a texture
	// Otherwise, look for a texture which resolution is (resolution*1024)x(resolution*512) by adding std::to_string(resolution) + "K" before the file extension
	// Note that a texture with lower power-of-two resolution may be querried depending to how many memory is available on the device
	// As general rule, the lowest resolution which can be querried is 4K
	s_texture(const std::string& _textureName, int _loadType = TEX_LOAD_TYPE_PNG_BLEND1, bool mipmap = false, bool resolution = false, int depth = 1, int nbChannels = 4, int channelSize = 1);
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

	// Return pointer to texture content, modification of this memory will result in undefined behavior
	// if nonPersistant is true and remain true after this call :
	// - texture content MAY be invalidated at the end of this frame
	// - you MUSTN'T call releaseContent for returned pointer
	void *acquireContent(bool &nonPersistant);
	// Invalidate previously acquired texture content
	void releaseContent(void *data);
	// Bind texture, assume set binding is 1 and only contain one texture at binding 0
	void bindTexture(VkCommandBuffer cmd, PipelineLayout *layout);

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
	// Display information about active big textures
	static void debugBigTexture();
	// Setup cache path for textures, also enable use of cache
	static void loadCache(const std::string &path);
private:
	void unload();
	bool preload(const std::string& fullName, bool mipmap = false, bool resolution = false, int depth = 1, int nbChannels = 4, int channelSize = 1);
	bool load();
	bool load(unsigned char *data, int realWidth, int realHeight);
	struct bigTexRecap {
		unsigned int binding; // Binding id, increased by 1 when getting unused
		unsigned short width;
		unsigned short height;
		std::unique_ptr<Texture> texture;
		std::string texName; // Name of the texture to load
		unsigned char lifetime = 0; // Number of frames before releasing this bigTexture
		unsigned char formatIdx;
		int quickLoader = -1; // Used to optimize loading
		bool ready = false; // Is this texture ready for use
		bool acquired = false; // Is this texture currently acquired
	};
	bigTexRecap *acquireBigTexture();
	bigTexRecap *acquireBigTexture(int width, int height);
	static void bigTextureLoader();

	struct texRecap {
		~texRecap();
		unsigned long int size;
		int width;
		int height;
		int depth;
		std::unique_ptr<Texture> texture;
		std::vector<VkImageView> imageViews;
		std::vector<std::unique_ptr<Set>> sets;
		std::unique_ptr<Set> ojmSet;
		unsigned int bigTextureBinding = 0;
		unsigned short bigWidth = 0;
		unsigned short bigHeight = 0;
		unsigned short bigDepth = 1; // Not implemented yet
		bool mipmap = false;
		bool quickloadable = false;
		bigTexRecap *bigTexture = nullptr;
	};

	//! Initialize resources required to build 3D mipmap
	static void init3DBuild(texRecap &tex);
	void blend( const int, unsigned char* const, const unsigned int );

	//! Optimized texture cache loader
	static void preQuickLoadCache(bigTexRecap *tex);
	static bool quickLoadCache(bigTexRecap *tex, void *data, size_t size);
	static void abortQuickLoadCache(bigTexRecap *tex);
	static void quickSaveCache(bigTexRecap *tex, void *data, size_t size);
	static std::string getCacheName(bigTexRecap *tex);

	std::string textureName;
	std::shared_ptr<texRecap> texture;
	int loadType;
	int loadWrapping;
	int nbChannels;
	int channelSize;

	// If an index have been used and is no longer used, don't reuse it for something else
	enum Section { // Sections of the cache
		BIG_TEXTURE = 0, // string map
	};

	struct BigTextureCache {
		int width;
		int height;
		bool cached;
	};

	static std::string texDir;
	static bool loadInLowResolution;
	static unsigned int lowResMax;
	static unsigned int minifyMax; // Maximal size of texture preview
	static bool releaseThisFrame; // Tell if releaseUnusedMemory have been called in this frame
	static std::map<std::string, std::weak_ptr<texRecap>> texCache;
	static std::list<bigTexRecap> bigTextures;
	static std::list<bigTexRecap> droppedBigTextures; // Big texture which have been freed
	static WorkQueue<bigTexRecap *, 31> bigTextureQueue;
	static PushQueue<std::shared_ptr<texRecap>> textureQueue;
	static PushQueue<std::unique_ptr<Texture>> droppedTextureQueue;
	static PushQueue<VkImage> bigTextureReady; // Read textures ready for use
	static std::atomic<long> currentAllocation; // Allocations/frees planned but not done yet
	static std::vector<std::shared_ptr<texRecap>> releaseMemory[3];
	static std::vector<std::unique_ptr<Texture>> releaseTexture[3];
	static short releaseIdx;
	static short releaseTexIdx;
	static PipelineLayout *layoutMipmap;
	static ComputePipeline *pipelineMipmap4;
	static ComputePipeline *pipelineMipmap1;
	static std::thread bigTextureThread;
	static VkFence uploadFence;
	static bool asyncUpload;
	static VkImageMemoryBarrier bigBarrier;
	static BigSave cache;
	static std::string cacheDir;
};

#endif // _S_TEXTURE_H_
