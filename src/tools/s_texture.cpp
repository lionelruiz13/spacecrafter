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
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
// We need bilinear sampling, and it seemed to be the closest of it
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_TRIANGLE
#include "stb_image_resize.h"
#include "tiny_jpeg.h"
#include <exception>
#include "tools/call_system.hpp"
#include <cassert>
#include <vulkan/vulkan.h>

#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include "tools/context.hpp"
#include "EntityCore/Tools/BigSave.hpp"
#include "EntityCore/Core/MemoryManager.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/ComputePipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Tools/SafeQueue.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include <filesystem>

#define MAX_LOW_RES 1024*512*4

std::string s_texture::texDir = "./";
std::map<std::string, std::weak_ptr<s_texture::texRecap>> s_texture::texCache;
std::list<s_texture::bigTexRecap> s_texture::bigTextures;
std::list<s_texture::bigTexRecap> s_texture::droppedBigTextures;
WorkQueue<s_texture::bigTexRecap *, 31> s_texture::bigTextureQueue;
PushQueue<std::shared_ptr<s_texture::texRecap>, 2047> s_texture::textureQueue;
PushQueue<std::unique_ptr<Texture>, 2047> s_texture::droppedTextureQueue;
PushQueue<VkImage, 2047> s_texture::bigTextureReady;
std::atomic<int64_t> s_texture::currentAllocation(0); // Allocations planned but not done yet
bool s_texture::loadInLowResolution = false;
unsigned int s_texture::lowResMax = MAX_LOW_RES;
unsigned int s_texture::minifyMax = 4*1024*1024; // Maximal size of texture preview
bool s_texture::releaseThisFrame = true;
int s_texture::wantReleaseAllMemory = 0;
std::vector<std::shared_ptr<s_texture::texRecap>> s_texture::releaseMemory[3];
std::vector<std::unique_ptr<Texture>> s_texture::releaseTexture[3];
short s_texture::releaseIdx = 0;
short s_texture::releaseTexIdx = 0;
PipelineLayout *s_texture::layoutMipmap = nullptr;
ComputePipeline *s_texture::pipelineMipmap4 = nullptr;
ComputePipeline *s_texture::pipelineMipmap1 = nullptr;
std::thread s_texture::bigTextureThread;
VkFence s_texture::uploadFence = VK_NULL_HANDLE;
bool s_texture::asyncUpload = true;
VkImageMemoryBarrier s_texture::bigBarrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, VK_NULL_HANDLE, {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1}};
BigSave s_texture::cache;
std::string s_texture::cacheDir;
bool s_texture::cacheTexture = false;
// Number of tic from the last use after which a big texture is released
// Set to 1 to release every big textures every tic
int s_texture::bigTextureLifetime = 90;

// Conversion table
const VkFormat formatTable[] = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM};

s_texture::texRecap::~texRecap()
{
   if (texture)
	   releaseTexture[releaseTexIdx].push_back(std::move(texture));
}

s_texture::s_texture(const s_texture *t)
{
	textureName = t->textureName;
	loadType = t->loadType;
	loadWrapping = t->loadWrapping;
	texture = t->texture;
}

s_texture::s_texture(const std::string& _textureName, int _loadType, bool mipmap, bool resolution, int depth, int nbChannels, int channelSize, bool useBlendMipmap, bool force3D) : textureName(_textureName),
	loadType(PNG_BLEND1), loadWrapping(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
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
			loadWrapping=VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		default :
			loadType=PNG_BLEND3;
	}
	bool succes;

	if (CallSystem::isAbsolute(textureName) || CallSystem::fileExist(textureName))
		succes = preload(textureName, mipmap, resolution, depth, nbChannels, channelSize, useBlendMipmap, force3D);
	else
		succes = preload(texDir + textureName, mipmap, resolution, depth, nbChannels, channelSize, useBlendMipmap, force3D);

	if (!succes)
		createEmptyTex();
}

s_texture::s_texture(const std::string& _textureName, Texture *_imgTex)
{
	texture = std::make_shared<texRecap>();
	texture->texture = std::unique_ptr<Texture>(_imgTex);
	textureName = _textureName;
	loadType = PNG_SOLID;
	loadWrapping = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	_imgTex->getDimensions(texture->width, texture->height, texture->depth);
	texture->size = texture->width * texture->height * texture->depth;
	texCache[textureName] = texture;
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

void s_texture::createEmptyTex()
{
	unsigned char image_data[8] = {255,0,0,255, 255,0,0,255}; // Must be a multiple of 8

	auto &tex = texCache["\0"];
	if (tex.expired()) {
		texture = std::make_shared<texRecap>();
		texture->size = 8;
		texture->width = 2;
		texture->height = 1;
		texture->depth = 1;
		texture->texture = std::make_unique<Texture>(*VulkanMgr::instance, *Context::instance->stagingMgr, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "Fallback texture");
		texture->texture->init(2, 1, image_data);
        textureQueue.emplace(texture);
		tex = texture;
	} else {
		texture = tex.lock();
	}
}

bool s_texture::preload(const std::string& fullName, bool mipmap, bool resolution, int depth, int _nbChannels, int _channelSize, bool useBlendMipmap, bool force3D)
{
	auto &tex = texCache[fullName];
	nbChannels = _nbChannels;
	channelSize = _channelSize;
	if (tex.expired()) {
        if (fullName.substr(fullName.size() - 5) == "empty") {
            unsigned char image_data[8] = {0,0,0,0, 0,0,0,0}; // Must be a multiple of 8
            texture = std::make_shared<texRecap>();
    		texture->size = 8;
    		texture->width = 2;
    		texture->height = 1;
    		texture->depth = 1;
    		texture->texture = std::make_unique<Texture>(*VulkanMgr::instance, *Context::instance->stagingMgr, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "Empty texture");
    		texture->texture->init(2, 1, image_data);
            textureQueue.emplace(texture);
    		tex = texture;
            return true;
        }

		int channels;
		texture = std::make_shared<texRecap>();
		stbi_uc *data = nullptr;
        int realWidth, realHeight;
        if (resolution) {
            const int pos = fullName.find_last_of('.');
            std::string previewName = fullName.substr(0, pos) + "-preview" + fullName.substr(pos);
            if (channelSize == 1)
                data = stbi_load(previewName.c_str(), &realWidth, &realHeight, &channels, nbChannels);
            else
                data = reinterpret_cast<stbi_uc*>(stbi_load_16(previewName.c_str(), &realWidth, &realHeight, &channels, nbChannels));
        }

        std::string cacheEntryName = getCacheEntryName(fullName);
        const bool minified = (data != nullptr);
        if (minified) { // We need to determine the size of the big texture
            auto &bigData = cache[Section::BIG_TEXTURE][cacheEntryName].get<BigTextureCache>();
            if (bigData.width == 0) {
                ++cache.get<int>(); // Inform update, required because reducedCheck is true
                stbi_image_free(stbi_load(fullName.c_str(), &bigData.width, &bigData.height, &channels, nbChannels));
            }
            texture->bigWidth = bigData.width;
            texture->bigHeight = bigData.height;
            texture->quickloadable = bigData.cached;
        } else {
            if (channelSize == 1)
                data = stbi_load(fullName.c_str(), &realWidth, &realHeight, &channels, nbChannels);
            else
                data = reinterpret_cast<stbi_uc*>(stbi_load_16(fullName.c_str(), &realWidth, &realHeight, &channels, nbChannels));
            if (!data) {
    			cLog::get()->write("s_texture: could not load " + fullName , LOG_TYPE::L_ERROR);
    			return false;
    		}
            if (resolution) {
                auto &bigData = cache[Section::BIG_TEXTURE][cacheEntryName].get<BigTextureCache>();
                if (bigData.width == 0) {
                    ++cache.get<int>(); // Inform update, required because reducedCheck is true
                    bigData.width = realWidth;
                    bigData.height = realHeight;
                }
                texture->quickloadable = bigData.cached;
            }
        }

		tex = texture;
        texture->width = realWidth;
        texture->height = realHeight;
		texture->depth = depth;
		texture->mipmap = mipmap;
        texture->blendMipmap = useBlendMipmap;
        if (resolution && !minified && (unsigned int) (realWidth * realHeight *4+2)/3 * nbChannels * channelSize > minifyMax) {
            texture->bigWidth = realWidth;
            texture->bigHeight = realHeight;
            while ((unsigned int) (texture->width * texture->height *4+2)/3 * nbChannels * channelSize > minifyMax) {
                texture->width /= 2;
                texture->height /= 2;
            }
        } else {
            const int depthPower = std::log2(depth);
    		texture->width /= (1 << (depthPower / 2));
    		texture->height /= (1 << ((depthPower + 1) / 2));
        }
        texture->size = texture->width * texture->height * nbChannels * channelSize;
		if (loadInLowResolution && depth == 1 && texture->size > lowResMax) {
			// Scale image, don't modify the x/y ratio
			float scale = std::sqrt(lowResMax / (float) texture->size);
			texture->width *= scale;
			texture->height *= scale;
			texture->size = texture->width * texture->height * nbChannels * channelSize;
		}
		if (channelSize == 1)
			blend(loadType, data, texture->size);
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (mipmap)
			usage |= (useBlendMipmap) ? VK_IMAGE_USAGE_STORAGE_BIT : VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		const VkImageType imgType = (depth > 1 || force3D) ? VK_IMAGE_TYPE_3D : ((texture->height > 1) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
		--_nbChannels;
		const VkFormat format = formatTable[(channelSize == 1) ? _nbChannels : (_nbChannels | 4)];
		texture->texture = std::make_unique<Texture>(*VulkanMgr::instance, *Context::instance->texStagingMgr, usage, fullName.substr(fullName.find(".spacecrafter/")+14), format, imgType);
        load(data, realWidth, realHeight);
		stbi_image_free(data);
    	textureQueue.emplace(texture);
	} else {
		texture = tex.lock();
		cLog::get()->write("s_texture: already in cache " + fullName , LOG_TYPE::L_INFO);
	}
	return true;
}

void s_texture::unload()
{
	texture.reset();
}

// Should be updated to load preview instead of full-resolution
bool s_texture::load()
{
	const std::string fullName = (CallSystem::isAbsolute(textureName) || CallSystem::fileExist(textureName)) ? textureName : texDir + textureName;
	stbi_uc *data;
	int realWidth, realHeight, unused;
	if (channelSize == 1) {
		data = stbi_load(fullName.c_str(), &realWidth, &realHeight, &unused, nbChannels);
		blend(loadType, data, texture->size);
	} else
		data = reinterpret_cast<stbi_uc*>(stbi_load_16(fullName.c_str(), &realWidth, &realHeight, &unused, nbChannels));
    auto ret = load(data, realWidth, realHeight);
    stbi_image_free(data);
    return ret;
}

bool s_texture::load(stbi_uc *data, int realWidth, int realHeight)
{
	if (realWidth != texture->width && texture->depth == 1) {
		// This texture have been rescaled
		stbi_uc *dataIn = data;
		data = new stbi_uc[texture->size];
		stbir_resize_uint8(dataIn, realWidth, realHeight, 0, data, texture->width, texture->height, 0, nbChannels);
	}
	// Use negated height to flip this axis
    const auto texHeight = (texture->depth == 1) ? -texture->height : texture->height;
	bool ret = texture->texture->init(texture->width, texHeight, data, texture->mipmap, nbChannels, channelSize, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, texture->depth);
	if (!ret) {
		releaseUnusedMemory();
		ret = texture->texture->init(texture->width, texHeight, data, texture->mipmap, nbChannels, channelSize, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, texture->depth);
	}
	if (realWidth != texture->width && texture->depth == 1)
		delete[] data;
	if (!ret) {
		cLog::get()->write("s_texture: not enough memory for " + textureName, LOG_TYPE::L_ERROR);
		return false;
	}
	return true;
}

void s_texture::getDimensions(int &width, int &height) const
{
	width = texture->width;
	height = texture->height;
}

void s_texture::getDimensions(int &width, int &height, int &depth) const
{
	width = texture->width;
	height = texture->height;
	depth = texture->depth;
}

// Return the average texture luminance : 0 is black, 1 is white
float s_texture::getAverageLuminance() const
{
	double sum = 0;
	uint8_t *p;
	const int64_t size = static_cast<int64_t>(texture->width)*static_cast<int64_t>(texture->height);
	if (texture->texture->isOnCPU()) {
		p = (uint8_t *) texture->texture->acquireStagingMemoryPtr();
		for (int64_t i = 0; i < size; ++i)
			sum += p[i * 4];
		// texture->texture->releaseStagingMemoryPtr();
	} else {
		int width, height, channels;
		p = stbi_load(textureName.c_str(), &width, &height, &channels, 4);
		if (!p) {
			cLog::get()->write("Failed to compute average luminance for " + textureName, LOG_TYPE::L_ERROR);
			return (0);
		}
		for (int64_t i = 0; i < size; ++i)
			sum += p[i * 4];
		stbi_image_free(p);
	}
	return (sum/size);
}

void s_texture::forceUnload()
{
	releaseMemory[0].clear();
	releaseMemory[1].clear();
	releaseMemory[2].clear();
	releaseTexture[0].clear();
	releaseTexture[1].clear();
	releaseTexture[2].clear();
	for (auto &value : texCache) {
		auto tex = value.second.lock();
		if (tex) {
			cLog::get()->write("Force unloading of " + value.first + " used " + std::to_string(tex.use_count() - 1) + " times", LOG_TYPE::L_WARNING);
			tex->texture = nullptr;
		}
	}
    bigTextureQueue.close();
    if (bigTextureThread.joinable())
        bigTextureThread.join();
    bigTextures.clear();
    if (layoutMipmap) {
        delete layoutMipmap;
        delete pipelineMipmap4;
        delete pipelineMipmap1;
        layoutMipmap = nullptr;
    }
    cache.store();
}

void s_texture::update()
{
	releaseTexIdx = (releaseTexIdx + 1) % 3;
	releaseTexture[releaseTexIdx].clear();
	for (auto &bt : bigTextures) {
		if (bt.ready) {
			if (--bt.lifetime == 0) {
				++bt.binding;
				bt.ready = false;
				bt.acquired = false;
			}
		}
	}
    releaseThisFrame = false;
}

Texture &s_texture::getTexture()
{
	if (!texture->texture->isOnGPU()) {
		texture->texture->use();
	}
	return *texture->texture;
}

Texture *s_texture::getBigTexture()
{
    if (texture->bigWidth == 0) {
        return &getTexture();
    }
	if (texture->bigTexture) {
		if (texture->bigTexture->binding == texture->bigTextureBinding) {
			texture->bigTexture->lifetime = bigTextureLifetime;
			return (texture->bigTexture->ready ? texture->bigTexture->texture.get() : nullptr);
		}
	}
	texture->bigTexture = acquireBigTexture();
	if (texture->bigTexture && texture->bigTexture->ready)
		return texture->bigTexture->texture.get();
    texture->quickloadable = true;
	return nullptr;
}

s_texture::bigTexRecap *s_texture::acquireBigTexture()
{
    unsigned int width = texture->bigWidth;
    unsigned int height = texture->bigHeight;
    auto bt = acquireBigTexture(width, height);
    if (bt)
        return bt;
    if (!bigTextureThread.joinable() && asyncUpload) {
        bigTextureThread = std::thread(&s_texture::bigTextureLoader);
    }
	// There is no bigTexRecap available, create another one
    for (MemoryQuerry &querry : VulkanMgr::instance->getMemoryManager()->querryMemory()) {
        if (!(querry.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
            continue;
        if (width * height * nbChannels * channelSize * 2 > querry.free - currentAllocation)
            releaseUnusedMemory();
        break;
    }
	for (MemoryQuerry &querry : VulkanMgr::instance->getMemoryManager()->querryMemory()) {
		if (!(querry.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
			continue;
		while (width * height * nbChannels * channelSize * 2 > querry.free - currentAllocation) {
            bt = acquireBigTexture(width, height);
            if (bt)
                return bt;
            width /= 2;
            height /= 2;
        }
		if ((width * height *4+2)/3 * nbChannels * channelSize <= minifyMax)
			return nullptr; // Don't create a big texture with worse resolution than the preview one
		bigTextures.push_back({1, (unsigned short) width, (unsigned short) height, nullptr, textureName, 3, (unsigned char) (nbChannels + 4 * channelSize - 5), -1, false, true});
        bt = &bigTextures.back();
		texture->bigTextureBinding = 1;
        if (texture->quickloadable)
            preQuickLoadCache(bt);
		bigTextureQueue.emplace(bt);
		currentAllocation += (width * height *4+2)/3 * nbChannels * channelSize;
		return bt;
	}
	return nullptr;
}

s_texture::bigTexRecap *s_texture::acquireBigTexture(int width, int height)
{
    for (auto &bt : bigTextures) {
        if (bt.width == width && bt.height == height && bt.formatIdx == (nbChannels + 4 * channelSize - 5)) {
            if (bt.acquired) {
                if (textureName == bt.texName) {
                    bt.lifetime = bigTextureLifetime;
                    texture->bigTextureBinding = bt.binding;
                    return &bt;
                }
                continue;
            }
            if (textureName == bt.texName && bt.texture) {
                bt.lifetime = bigTextureLifetime;
                bt.ready = true;
                bt.acquired = true;
                texture->bigTextureBinding = bt.binding;
                return &bt;
            }
            bt.texName = textureName;
            texture->bigTextureBinding = bt.binding;
            bt.lifetime = bigTextureLifetime;
            bt.ready = false;
            bt.acquired = true;
            if (texture->quickloadable)
                preQuickLoadCache(&bt);
            bigTextureQueue.emplace(&bt);
            return &bt;
        }
    }
    return nullptr;
}

void s_texture::releaseUnusedMemory()
{
    if (releaseThisFrame)
        return;
    releaseThisFrame = true;
	cLog::get()->write("Attempt to release memory from unused big texture", LOG_TYPE::L_DEBUG);
	for (auto it = bigTextures.begin(); it != bigTextures.end();) {
		if (!it->acquired) {
            auto tmp = it++;
            currentAllocation -= tmp->texture->getTextureSize();
            // Move texture to drop queue, thus tmp->texture became nullptr
            droppedTextureQueue.push(tmp->texture);
            droppedBigTextures.splice(droppedBigTextures.end(), bigTextures, tmp);
        } else
            ++it;
	}
}

void s_texture::releaseAllMemory()
{
    ++wantReleaseAllMemory;
    cLog::get()->write("Attempt to release as many memory as possible", LOG_TYPE::L_DEBUG);
    for (auto it = bigTextures.begin(); it != bigTextures.end();) {
        auto tmp = it++;
        if (it->ready || !it->acquired) {
            if (it->lifetime != bigTextureLifetime) {
                currentAllocation -= tmp->texture->getTextureSize();
                // Move texture to drop queue, thus tmp->texture became nullptr
                droppedTextureQueue.push(tmp->texture);
                droppedBigTextures.splice(droppedBigTextures.end(), bigTextures, tmp);
            } else {
                cLog::get()->write("Can't release memory from texture '" + it->texName + "' : already acquired this frame", LOG_TYPE::L_WARNING);
            }
        } else {
            // The texture will be released, but not immediately
            ++it->binding;
            it->ready = false;
            it->acquired = false;
            droppedBigTextures.splice(droppedBigTextures.end(), bigTextures, tmp);
            // cLog::get()->write("Can't release memory from texture '" + it->texName + "' : currently loading", LOG_TYPE::L_WARNING);
        }
    }
    bigTextureQueue.emplace(nullptr);
}

void s_texture::recordTransfer(VkCommandBuffer cmd)
{
	for (auto &t : releaseMemory[releaseIdx]) {
		t->texture->detach();
        for (auto &v : t->imageViews)
            vkDestroyImageView(VulkanMgr::instance->refDevice, v, nullptr);
    }
	releaseMemory[releaseIdx].clear();
	std::shared_ptr<texRecap> tex;
	while (textureQueue.pop(tex)) {
        if (tex->blendMipmap) {
            {
                VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->texture->getImage(), {tex->texture->getAspect(), 0, 1, 0, 1}};
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }
            tex->texture->use(cmd, false);
            init3DBuild(*tex);
            {
                VkImageMemoryBarrier barrier[2] {
                    {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->texture->getImage(), {tex->texture->getAspect(), 0, 1, 0, 1}},
                    {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->texture->getImage(), {tex->texture->getAspect(), 1, VK_REMAINING_MIP_LEVELS, 0, 1}}
                };
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 2, barrier);
            }
            VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->texture->getImage(), {tex->texture->getAspect(), 1, 1, 0, 1}};
            pipelineMipmap4->bind(cmd);
            int setIdx = 0;
            for (int depth = tex->depth / 8; depth > 0; depth /= 2) {
                layoutMipmap->bindSet(cmd, *tex->sets[setIdx++], 0, VK_PIPELINE_BIND_POINT_COMPUTE);
                vkCmdDispatch(cmd, depth, depth, depth);
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                ++barrier.subresourceRange.baseMipLevel;
            }
            pipelineMipmap1->bind(cmd);
            // Build 2x2x2 mipmap
            layoutMipmap->bindSet(cmd, *tex->sets[setIdx++], 0, VK_PIPELINE_BIND_POINT_COMPUTE);
            vkCmdDispatch(cmd, 2, 2, 2);
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            // Build 1x1x1 mipmap
            layoutMipmap->bindSet(cmd, *tex->sets[setIdx++], 0, VK_PIPELINE_BIND_POINT_COMPUTE);
            vkCmdDispatch(cmd, 1, 1, 1);
            // Perform final transition
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        } else
            tex->texture->use(cmd, true);
		releaseMemory[releaseIdx].push_back(tex);
	}
    std::vector<VkImageMemoryBarrier> barriers;
    while (bigTextureReady.pop(bigBarrier.image))
        barriers.push_back(bigBarrier);
    if (!barriers.empty()) {
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
    }
	releaseIdx = (releaseIdx + 1) % 3;
}

void s_texture::init3DBuild(texRecap &tex)
{
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;
    if (!layoutMipmap) {
        layoutMipmap = new PipelineLayout(vkmgr);
        layoutMipmap->setImageLocation(0);
        layoutMipmap->setImageLocation(1);
        layoutMipmap->buildLayout();
        layoutMipmap->build();
        pipelineMipmap4 = new ComputePipeline(vkmgr, layoutMipmap);
        pipelineMipmap4->bindShader("smartDepthMipmap.comp.spv");
        pipelineMipmap4->build();
        pipelineMipmap1 = new ComputePipeline(vkmgr, layoutMipmap);
        pipelineMipmap1->bindShader("smartDepthMipmapMini.comp.spv");
        pipelineMipmap1->build();
    }
    const int mipmaps = tex.texture->getMipmapCount();
    tex.imageViews.reserve(mipmaps);
    for (int i = 0; i < mipmaps; ++i)
        tex.imageViews.push_back(tex.texture->createView(i));
    tex.sets.reserve(mipmaps - 1);
    for (int i = 1; i < mipmaps; ++i) {
        tex.sets.push_back(std::make_unique<Set>(vkmgr, *context.setMgr, layoutMipmap, -1, true, true));
        tex.sets.back()->bindStorageImage(tex.imageViews[i - 1], 0);
        tex.sets.back()->bindStorageImage(tex.imageViews[i], 1);
    }
}

void *s_texture::acquireContent(bool &nonPersistant)
{
    if (nonPersistant && texture->texture->isOnCPU())
        return texture->texture->acquireStagingMemoryPtr();
    nonPersistant = false;
    int realWidth, realHeight, unused;
    const std::string fullName = (CallSystem::isAbsolute(textureName) || CallSystem::fileExist(textureName)) ? textureName : texDir + textureName;
    return stbi_load(fullName.c_str(), &realWidth, &realHeight, &unused, nbChannels);
}

void s_texture::releaseContent(void *data)
{
    stbi_image_free(data);
}

void s_texture::bindTexture(VkCommandBuffer cmd, PipelineLayout *layout)
{
    if (!texture->ojmSet) {
        texture->ojmSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, layout, 1, true, true);
        texture->ojmSet->bindTexture(*texture->texture, 0);
    }
    layout->bindSet(cmd, *texture->ojmSet, 1);
}

void s_texture::bigTextureLoader()
{
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;
    bigTexRecap *tex;
    std::unique_ptr<Texture> droppedTex;
    VkQueue queue;
    VkFence fence;
    VkCommandBuffer cmd;
    VkCommandPool pool;
    stbi_uc *stor = new stbi_uc[BIG_TEXTURE_MIPMAP_SIZE];
    VkSubmitInfo submit {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cmd, 0, nullptr};
    auto queueFamily = vkmgr.acquireQueue(queue, VulkanMgr::QueueType::TRANSFER, "Async big texture loader");
    {
        if (!queueFamily) {
            vkmgr.putLog("Async dynamic texture loading unavailable for this GPU", LogType::WARNING);
            asyncUpload = false;
            return;
        }
        bigBarrier.srcQueueFamilyIndex = queueFamily->id;
        bigBarrier.dstQueueFamilyIndex = context.graphicFamily->id;
        VkFenceCreateInfo fenceInfo {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
        vkCreateFence(vkmgr.refDevice, &fenceInfo, nullptr, &fence);
        VkCommandPoolCreateInfo poolInfo {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, queueFamily->id};
        vkCreateCommandPool(vkmgr.refDevice, &poolInfo, nullptr, &pool);
        VkCommandBufferAllocateInfo cmdInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
        vkAllocateCommandBuffers(vkmgr.refDevice, &cmdInfo, &cmd);
    }
    bigTextureQueue.acquire();
    while (bigTextureQueue.pop(tex)) {
        if (wantReleaseAllMemory) {
            if (tex) {
                if (tex->texture) {
                    tex->texture.reset();
                } else {
                    currentAllocation -= (tex->width * tex->height * 4+2)/3 * (tex->formatIdx + 1);
                }
            } else {
                --wantReleaseAllMemory;
            }
            continue;
        }
        while (droppedTextureQueue.pop(droppedTex)) {
            currentAllocation += droppedTex->getTextureSize();
            droppedTex.reset();
        }
        std::string shortName = tex->texName.substr(tex->texName.find(".spacecrafter/")+14);
        std::string texName = "big \"" + shortName + "\"";
        cLog::get()->write("Loading big " + shortName + "...", LOG_TYPE::L_DEBUG);
        unsigned int width = tex->width;
        unsigned int height = tex->height;
        // Not fully implemented 16-bpp support from here
        const int _nbChannels = tex->formatIdx + 1;
        if (tex->texture) {
            tex->texture->rename(texName);
        } else {
            const VkFormat format = formatTable[tex->formatIdx];
            tex->texture = std::make_unique<Texture>(vkmgr, width, height, VK_SAMPLE_COUNT_1_BIT, texName, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, format, VK_IMAGE_ASPECT_COLOR_BIT, true);
            tex->texture->use();
            currentAllocation -= (width * height * 4+2)/3 * _nbChannels;
        }
        auto buffer = context.asyncTexStagingMgr->fastAcquireBuffer((width * height *4+2)/3 * _nbChannels);
        stbi_uc *finalDst = (stbi_uc *) context.asyncTexStagingMgr->getPtr(buffer);

        auto &bigData = cache[Section::BIG_TEXTURE][getCacheEntryName(tex->texName)].get<BigTextureCache>();
        int64_t datetime;
        try {
            datetime = std::chrono::duration_cast<std::chrono::seconds>(std::filesystem::last_write_time(tex->texName).time_since_epoch()).count();
        } catch (...) {
            cLog::get()->write("Can't check modification time for this file, assume unchanged.", LOG_TYPE::L_DEBUG);
            datetime = bigData.datetime;
        }
        if (bigData.datetime != datetime) {
            ++cache.get<int>(); // Inform update, required because reducedCheck is true
            bigData.datetime = datetime;
            bigData.cached = false;
            abortQuickLoadCache(tex);
        }

        stbi_uc *data = nullptr;
        int realWidth, realHeight, unused;
        if (bigData.cached) {
            // Load the cached image which include mipmaps
            bigData.cached = quickLoadCache(tex, bigData, stor, finalDst, width);
        }
        stbi_uc *src;
        stbi_uc *dst;
        if (!bigData.cached) {
            // Load the original image, which is one single layer
            data = stbi_load(tex->texName.c_str(), &realWidth, &realHeight, &unused, _nbChannels);
            bigData.width = realWidth;
            bigData.height = realHeight;
            {
                // Invert Y axis by flipping pixels
                int64_t *src = (int64_t *) data;
                int64_t *dst;
                int64_t tmp;
                // Only work for textures with pair height and width
                const int64_t lineSize = realWidth * _nbChannels / sizeof(int64_t);
                int j = realHeight / 2;
                while (j--) {
                    dst = src + (j * 2 + 1) * lineSize;
                    int i = lineSize;
                    while (i--) {
                        tmp = *src;
                        *(src++) = *dst;
                        *(dst++) = tmp;
                    }
                }
            }
            // Push the first layer, resize it if the texture is smaller than the first layer
            src = data;
            dst = stor;
            if ((unsigned int) bigData.width + bigData.height != width + height) {
                stbir_resize_uint8(src, realWidth, realHeight, 0, dst, width, height, 0, _nbChannels);
                src = dst;
                dst += width * height * _nbChannels;
            } else {
                memcpy(finalDst, src, width * height * _nbChannels);
                finalDst += width * height * _nbChannels;
            }
        }

        // Compute
        VkBufferImageCopy region {buffer.offset, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {}, {width, height, 1}};
        std::vector<VkBufferImageCopy> regions;
        int subsize = width * height * _nbChannels;
        while (width + height > 2) {
            regions.push_back(region);
            ++region.imageSubresource.mipLevel;
            region.bufferOffset += subsize;
            width = (width == 1) ? 1 : width/2;
            height = (height == 1) ? 1 : height/2;
            if (!bigData.cached) {
                stbir_resize_uint8(src, region.imageExtent.width, region.imageExtent.height, 0, dst, width, height, 0, _nbChannels);
                subsize = width * height * _nbChannels;
                src = dst;
                dst += subsize;
            } else
                subsize = width * height * _nbChannels;
            region.imageExtent.width = width;
            region.imageExtent.height = height;
        }
        regions.push_back(region);
        // Upload cached datas
        if (asyncUpload) {
            if (!bigData.cached)
                memcpy(finalDst, stor, dst - stor);
            VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
            vkBeginCommandBuffer(cmd, &beginInfo);
            VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, tex->texture->getImage(), {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1}};
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            vkCmdCopyBufferToImage(cmd, buffer.buffer, tex->texture->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
            barrier.srcAccessMask = barrier.dstAccessMask;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = barrier.newLayout;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = queueFamily->id;
            barrier.dstQueueFamilyIndex = context.graphicFamily->id;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            vkEndCommandBuffer(cmd);
            vkQueueSubmit(queue, 1, &submit, fence);
            auto res = vkWaitForFences(vkmgr.refDevice, 1, &fence, VK_TRUE, 60L*1000*1000*1000); // 60 seconds
            if (res != VK_SUCCESS) {
                vkmgr.putLog("Async texture upload has failed", LogType::ERROR);
                continue;
            } else
                vkResetFences(vkmgr.refDevice, 1, &fence);
            vkResetCommandPool(vkmgr.refDevice, pool, 0);
            context.asyncTexStagingMgr->reset();
            bigTextureReady.push(barrier.image);
        }
        if (wantReleaseAllMemory) { // In case we want release all memory
            tex->texture.release();
            continue;
        }
        tex->ready = true;
        cLog::get()->write(texName + " is ready for use", LOG_TYPE::L_DEBUG);
        if (!bigData.cached && cacheTexture && (bigData.width == tex->width || bigData.width == tex->width / 2)) {
            CacheSaveData info;
            info.tex = tex;
            info.cache = &bigData;
            quickSaveCache(info, data, stor);
            ++cache.get<int>();
        }
        if (data)
            stbi_image_free(data);
        if (wantReleaseAllMemory) { // Just in case we want to release all memory now, but not just before
            tex->ready = false;
            tex->texture.release();
        }
    }
    bigTextureQueue.release();
    vkDestroyFence(vkmgr.refDevice, fence, nullptr);
    vkDestroyCommandPool(vkmgr.refDevice, pool, nullptr);
    delete[] stor;
}

void s_texture::debugBigTexture()
{
    cLog::get()->write("Big texture resume :", LOG_TYPE::L_DEBUG);
    for (auto &bt : bigTextures) {
        std::ostringstream desc;
        desc << "- " << bt.width << 'x' << bt.height;
        if (bt.acquired) {
            desc << ((bt.ready) ? " <active> " : " <uploading> ") << bt.texName.substr(bt.texName.find(".spacecrafter/")+14);
        } else
            desc << " <unused>";
        cLog::get()->write(desc, LOG_TYPE::L_DEBUG);
    }
}

void s_texture::loadCache(const std::string &path, bool _cacheTexture)
{
    cacheDir = path;
    cacheTexture = _cacheTexture;
    CallSystem::ensurePathExist(path);
    cache.open(path + "texture-cache", false, true, true);
    if (cache[Section::CACHE_VERSION].get<int>() != 1) {
        // Invalidate cache content
        cLog::get()->write("Cache version incompatible with current version, reset cache.", LOG_TYPE::L_WARNING);
        cache.reset();
        cache[Section::CACHE_VERSION].get<int>() = 1;
    }
}

std::string s_texture::getCacheName(const std::string &name)
{
    const auto p1 = name.find(".spacecrafter/")+14;
    const auto p2 = name.find_first_of('/', p1) - 1;
    std::string filename = name.substr();
    filename = name.substr(p2, name.size() - p2);
    filename.front() = name[p1];
    for (char &c : filename) {
        if (c == '/')
            c = '-';
    }
    memcpy(filename.data() + filename.size() - 3, "dat", 3);
    return filename;
}

std::string s_texture::getCacheEntryName(const std::string &name)
{
    const auto p1 = name.find(".spacecrafter/")+14;
    const auto p2 = name.find_first_of('/', p1) - 1;
    std::string filename = name.substr();
    filename = name.substr(p2, name.size() - p2 - 4);
    filename.front() = name[p1];
    return filename;
}

std::string s_texture::getCacheName(bigTexRecap *tex)
{
    return getCacheName(tex->texName);
}

#ifdef __linux__
#include <fcntl.h>


void s_texture::preQuickLoadCache(bigTexRecap *tex)
{
    if (cacheTexture) {
        const std::string filename = cacheDir + getCacheName(tex);
        tex->quickLoader = open(filename.c_str(), O_RDONLY);
        if (tex->quickLoader != -1)
            posix_fadvise(tex->quickLoader, 0, 0, POSIX_FADV_WILLNEED);
    }
}

void s_texture::abortQuickLoadCache(bigTexRecap *tex)
{
    if (tex->quickLoader == -1)
        return;
    close(tex->quickLoader);
    tex->quickLoader = -1;
}

bool s_texture::quickLoadCache(bigTexRecap *tex, const BigTextureCache &cache, void *stor, void *data, int width)
{
    if (cacheTexture) {
        if (tex->quickLoader == -1)
            return false;
        cLog::get()->write("Loading cached data for '" + tex->texName + "'", LOG_TYPE::L_DEBUG);
        if (cache.jpegLayers < 4) {
            cLog::get()->write("Performance Issue : Height is not multiple of 64 for '" + tex->texName + "', this significatively increase both loading time and cache size.", LOG_TYPE::L_WARNING);
        }
        // Read the jpeg data
        read(tex->quickLoader, stor, cache.jpegSize);
        int vwidth, vheight, unused;
        auto pixels = stbi_load_from_memory((stbi_uc *) stor, cache.jpegSize, &vwidth, &vheight, &unused, tex->formatIdx + 1);
        auto srcBegin = pixels;
        size_t fullSize = vwidth * vheight * (tex->formatIdx + 1);
        // Define from which layer to start
        int layerWidth = cache.width;
        size_t layerSize = cache.width * cache.height * (tex->formatIdx + 1);
        while (width < layerWidth) {
            srcBegin += layerSize;
            fullSize -= layerSize;
            layerSize /= 4;
            layerWidth /= 2;
        }
        fullSize /= 8; // Assume output is a multiple of 8 pixels.
        int64_t *src = (int64_t *) srcBegin;
        int64_t *dst = (int64_t *) data;
        while (fullSize--)
            *(dst++) = *(src++);
        stbi_image_free(pixels);
        // Read the raw cached data
        read(tex->quickLoader, dst, cache.rawSize);
        close(tex->quickLoader);
        tex->quickLoader = -1;
        return true;
    }
    return false;
}
#else
void s_texture::preQuickLoadCache(bigTexRecap *tex) {}
void s_texture::abortQuickLoadCache(bigTexRecap *tex) {}

bool s_texture::quickLoadCache(bigTexRecap *tex, const BigTextureCache &cache, void *stor, void *data, int width)
{
    if (cacheTexture) {
        std::ifstream file(cacheDir + getCacheName(tex), std::ifstream::binary);
        if (file) {
            cLog::get()->write("Loading cached data for '" + tex->texName + "'", LOG_TYPE::L_DEBUG);
            if (cache.jpegLayers < 4) {
                cLog::get()->write("Performance Issue : Height is not multiple of 64 for '" + tex->texName + "', this significatively increase both loading time and cache size.", LOG_TYPE::L_WARNING);
            }
            // Read the jpeg data
            file.read((char *) stor, cache.jpegSize);
            int vwidth, vheight, unused;
            auto pixels = stbi_load_from_memory((stbi_uc *) stor, cache.jpegSize, &vwidth, &vheight, &unused, tex->formatIdx + 1);
            auto srcBegin = pixels;
            size_t fullSize = vwidth * vheight * (tex->formatIdx + 1);
            // Define from which layer to start
            int layerWidth = cache.width;
            size_t layerSize = cache.width * cache.height * (tex->formatIdx + 1);
            while (width > layerWidth) {
                srcBegin += layerSize;
                fullSize -= layerSize;
                layerSize /= 4;
                layerWidth /= 2;
            }
            fullSize /= 8; // Assume output is a multiple of 8 pixels.
            int64_t *src = (int64_t *) srcBegin;
            int64_t *dst = (int64_t *) data;
            while (fullSize--)
                *(dst++) = *(src++);
            stbi_image_free(pixels);
            // Read the raw cached data
            file.read(reinterpret_cast<char *>(dst), cache.rawSize);
            return true;
        }
    }
    return false;
}
#endif

void s_texture::quickSaveCache(CacheSaveData &info, void *firstLayer, void *mipmaps)
{
    if (cacheTexture) {
        info.file.open(cacheDir + getCacheName(info.tex), std::ofstream::binary | std::ofstream::trunc);
        if (info.file) {
            cLog::get()->write("Caching data for '" + info.tex->texName + "'", LOG_TYPE::L_DEBUG);
            const int width = info.cache->width;
            const int height = info.cache->height;
            const int nbChannels = info.tex->formatIdx + 1;
            const int firstLayerSize = height * width * nbChannels;
            int mipmapSize = 0;
            for (int w=width, h=height; (w | h) > 1;) {
                if (w > 1)
                    w /= 2;
                if (h > 1)
                    h /= 2;
                mipmapSize += w * h * nbChannels;
            }
            // Unify image content to a single memory location
            char *unified = (char *) malloc(firstLayerSize + mipmapSize);
            memcpy(unified, firstLayer, firstLayerSize);
            memcpy(unified + firstLayerSize, mipmaps, mipmapSize);
            // Determine the number of layers
            int tmp = height;
            int vheight = height;
            int layers = 1;
            while (!(tmp & 3)) {
                tmp /= 4;
                vheight += tmp;
                if (++layers == 4)
                    break;
            }
            if (layers < 4) {
                cLog::get()->write("Performance Issue : Height is not multiple of 64 for '" + info.tex->texName + "', this significatively increase both loading time and cache size.", LOG_TYPE::L_WARNING);
            } else
                layers = 4;
            info.cache->jpegLayers = layers;
            info.cache->jpegSize = 0;
            tmp = width >> layers;
            info.cache->rawSize = firstLayerSize + mipmapSize - width * vheight * nbChannels;
            layers = 0;
            while (tmp) {
                tmp >>= 1;
                ++layers;
            }
            info.cache->rawLayers = layers;
            tmp -= info.cache->rawSize;
            // This function inverse the Y axis while saving the image, inverse it before this happened
            {
                int64_t *src = (int64_t *) unified;
                int64_t tmp;
                // Only work for textures with pair width and 4 channels, or width multiple of 8
                const int64_t lineSize = width * nbChannels / sizeof(int64_t);
                int64_t *dst = src + (vheight - 1) * lineSize;
                int j = vheight / 2;
                while (j--) {
                    int i = lineSize;
                    while (i--) {
                        tmp = *src;
                        *(src++) = *dst;
                        *(dst++) = tmp;
                    }
                    dst -= 2 * lineSize;
                }
            }
            tje_encode_with_func(reinterpret_cast<tje_write_func*>(&subQuickSaveCache), &info, 3, width, vheight, nbChannels, (unsigned char *) unified);
            // Write raw content
            info.file.write(unified + firstLayerSize + mipmapSize - info.cache->rawSize, info.cache->rawSize);
            info.cache->cached = true;
            free(unified);
        }
    }
}

void s_texture::subQuickSaveCache(CacheSaveData *info, char *data, int size)
{
    info->file.write(data, size);
    info->cache->jpegSize += size;
}
