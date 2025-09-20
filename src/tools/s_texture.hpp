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
#include <map>
#include <list>
#include <vector>
#include <memory>
#include "EntityCore/Tools/SafeQueue.hpp"
#include "EntityCore/Executor/AsyncBuilder.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include <vulkan/vulkan.h>
#include <thread>
#include <fstream>
#ifdef __linux__
#include <unistd.h>
#endif

class BigSave;

//TODO delete this and replace it with an enum class
#define PNG_ALPHA  0
#define PNG_SOLID  1
#define PNG_BLEND1 7
#define PNG_BLEND3 2

#define TEX_LOAD_TYPE_PNG_ALPHA 0
#define TEX_LOAD_TYPE_PNG_SOLID 1
#define TEX_LOAD_TYPE_PNG_BLEND3 2
#define TEX_LOAD_TYPE_NORMAL_MAP 3
#define TEX_LOAD_TYPE_PNG_SOLID_REPEAT 4
#define TEX_LOAD_TYPE_PNG_BLEND1 7

class Set;
class ComputePipeline;
class PipelineLayout;

class s_texture;

struct TextureCache {
	uint8_t timestamp[sizeof(uint64_t)];
	float averageLuminance; // Average luminance, -1 if not computed
	int width;
	int height;
};

class TextureLoader : public AsyncBuilder {
public:
	TextureLoader(s_texture *tex, TextureCache &cache, const std::string &fullName, bool resolution, int depth, bool force3D, int depthColumn);
	~TextureLoader();
	virtual void asyncLoad() override;
    virtual void postLoad() override;
	s_texture *tex;
	TextureCache &cache;
	unsigned char *data = nullptr;
	const std::string fullName;
	int depth;
	int depthColumn;
	bool resolution;
	bool force3D;
};

class s_texture {
	friend class TextureLoader; // Because TextureLoader handle internal s_texture loading, it need internal access
public:
	// If resolution is true, use texture with -preview suffix, or downscale original texture
	// In this case, the full resolution texture can be dynamically loaded and querried with getBigTexture()
	s_texture(const std::string& _textureName, int _loadType = TEX_LOAD_TYPE_PNG_BLEND1, bool mipmap = false, bool resolution = false, int depth = 1, int nbChannels = 4, int channelSize = 1, bool useBlendMipmap = false, bool force3D = false, int depthColumn = 0);
	// create a texture from a texture, pass the ownership of the texture to s_texture
	s_texture(const std::string& _textureName, Texture *_imgTex);
	// texture destructor
	~s_texture() = default;
	// création d'une texture par copie d'une autre
	s_texture(const s_texture &t) = delete;
	s_texture(const s_texture *t);
	//ban operator =
	const s_texture &operator=(const s_texture &t) = delete;
	// Allow comparing a texture by his source filename
	inline bool operator==(const std::string &filename) const {
		return filename == textureName;
	}
	// Allow comparing a texture by his source filename
	inline bool operator!=(const std::string &filename) const {
		return filename != textureName;
	}

	// Return the big texture, or nullptr if not loaded at this frame
	Texture *getBigTexture();
	// Return the texture, request his loading if not loaded yet
	Texture &getTexture();
	// Return true if the texture is currently loading
	inline bool isLoading() {
		return (texture->loader && texture->loader->priority != LoadPriority::DONE);
	}
	// Modify the level of priority
	inline void prioritize(LoadPriority level) {
		if (texture->loader && texture->loader->priority > LoadPriority::LOADING)
			texture->loader->priority = level;
	}

	// Return the average texture luminance : 0 is black, 1 is white
	float getAverageLuminance() const;

	// returns the dimensions of the texture
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

	// Indicates the default path of the textures.
	static void setTexDir(const std::string& _texDir) {
		s_texture::texDir = _texDir;
	}
	static const std::string& getTexDir() {
		return texDir;
	}

	// Indicates if we must load the textures in low resolution or not.
	static void setLoadInLowResolution(bool value, int _maxRes) {
		s_texture::loadInLowResolution = value;
		s_texture::lowResMax = _maxRes*_maxRes*2;
	}

	// creates a red texture in case of not loaded textures
	void createEmptyTex();

	static int64_t getNumberTotalTexture(){
		return texCache.size();
	}

	// Inform the kernel that we will load this texture in a few seconds
	static void willRead(const std::string &_textureName);
	// Release every big textures which have not been querried with getBigTexture() for 2 frames
	static void update();
	// Unload every big textures
	static void forceUnload();
	// Release memory of every unused big textures, might have side effect
	static void releaseUnusedMemory();
	// Release as many memory as possible, including actively used big textures. Can have side effect.
	static void releaseAllMemory();
	// Record transfer of every newly created textures
	static void recordTransfer(VkCommandBuffer cmd);
	// Display information about active big textures
	static void debugBigTexture();
	// Setup cache path for textures, also enable use of cache
	static void loadCache(const std::string &path, bool _cacheTexture);
	// Set the new value for bigTextureLifetime, and return his previous value
	static int setBigTextureLifetime(int lifetime) {
		int ret = bigTextureLifetime;
		bigTextureLifetime = lifetime;
		return ret;
	}
	static void setLoadingStrategy(const std::string &strategy);
private:
	void unload();
	bool preload(const std::string& fullName, bool mipmap = false, bool resolution = false, int depth = 1, int nbChannels = 4, int channelSize = 1, bool useBlendMipmap = false, bool force3D = false, int depthColumn = 0);
	bool loadInternal(const std::string &fullName, bool resolution, int depth, bool force3D, int depthColumn);
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
		uint64_t size;
		float averageLuminance;
		int width;
		int height;
		int depth;
		std::unique_ptr<TextureLoader> loader;
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
		bool blendMipmap = false;
		bool blendPacked = false;
		bigTexRecap *bigTexture = nullptr;
	};

	//! Initialize resources required to build 3D mipmap
	static void init3DBuild(texRecap &tex);
	void blend( const int, unsigned char* const, const unsigned int );

	// If an index have been used and is no longer used, don't reuse it for something else
	enum Section { // Sections of the cache
		BIG_TEXTURE = 0, // string map
		CACHE_VERSION = 1, // Single int
	};

	enum class Loading : unsigned char {
		LEGACY,
		DISPATCHED, // Blocking multi-thread loading
	};

	struct BigTextureCache {
		int64_t datetime;
		int width;
		int height;
		unsigned int jpegSize; // Size of the jpeg portion of the image
		unsigned int rawSize; // Size of the raw portion of the image
		unsigned char jpegLayers; // Number of jpeg layers
		unsigned char rawLayers; // Number of raw layers
		bool cached;
	};

	struct CacheSaveData {
		std::ofstream file;
		bigTexRecap *tex;
		BigTextureCache *cache;
	};

	//! Optimized texture cache loader
	static void preQuickLoadCache(bigTexRecap *tex);
	static bool quickLoadCache(bigTexRecap *tex, const BigTextureCache &cache, void *stor, void *data, int width);
	static void abortQuickLoadCache(bigTexRecap *tex);
	static void quickSaveCache(CacheSaveData &info, void *firstLayer, void *mipmaps);
	static void subQuickSaveCache(CacheSaveData *info, char *data, int size);
	static std::string getCacheName(bigTexRecap *tex);
	static std::string getCacheName(const std::string &name);
	static std::string getCacheEntryName(const std::string &name);

	std::string textureName;
	std::shared_ptr<texRecap> texture;
	int loadType;
	int loadWrapping;
	int nbChannels;
	int channelSize;
	VkImageUsageFlags usage;
	VkImageType imgType;
	VkFormat formatOverride = VK_FORMAT_UNDEFINED;

	static std::string texDir;
	static bool loadInLowResolution;
	static unsigned int lowResMax;
	static unsigned int minifyMax; // Maximal size of texture preview
	static bool releaseThisFrame; // Tell if releaseUnusedMemory have been called in this frame
	static int wantReleaseAllMemory; // Tell the Big texture loader to release all memory
	static std::map<std::string, std::weak_ptr<texRecap>> texCache;
	static std::list<bigTexRecap> bigTextures;
	static std::list<bigTexRecap> droppedBigTextures; // Big texture which have been freed
	static WorkQueue<bigTexRecap *, 31> bigTextureQueue;
	static PushQueue<std::shared_ptr<texRecap>, 2047> textureQueue;
	static PushQueue<std::unique_ptr<Texture>, 2047> droppedTextureQueue;
	static PushQueue<VkImage, 2047> bigTextureReady; // Read textures ready for use
	static std::atomic<int64_t> currentAllocation; // Allocations/frees planned but not done yet
	static std::vector<std::shared_ptr<texRecap>> releaseMemory[3];
	static std::vector<std::unique_ptr<Texture>> releaseTexture[3];
	static short releaseIdx;
	static short releaseTexIdx;
	static PipelineLayout *layoutMipmap;
	static ComputePipeline *pipelineMipmap4;
	static ComputePipeline *pipelineMipmap1;
	static ComputePipeline *pipelinePackedMipmap4;
	static ComputePipeline *pipelinePackedMipmap1;
	static std::thread bigTextureThread;
	static VkFence uploadFence;
	static bool asyncUpload;
	static VkImageMemoryBarrier bigBarrier;
	static BigSave cache;
	static std::string cacheDir;
	static bool cacheTexture;
	static Loading strategy;
	static int bigTextureLifetime;
	static std::atomic<int16_t> textureQueueSize; // Expected number of elements in textureQueue
	static std::mutex dispatchedLoadMutex;
};

#endif // _S_TEXTURE_H_
