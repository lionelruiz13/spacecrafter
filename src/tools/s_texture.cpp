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
#include "stb_image_resize.h"
#include <exception>
#include "tools/call_system.hpp"
#include <cassert>
#include <vulkan/vulkan.h>

#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/MemoryManager.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "EntityCore/Tools/SafeQueue.hpp"

#define MAX_LOW_RES 1024*512*4

std::string s_texture::texDir = "./";
std::map<std::string, std::weak_ptr<s_texture::texRecap>> s_texture::texCache;
std::list<s_texture::bigTexRecap> s_texture::bigTextures;
PushQueue<s_texture::bigTexRecap *, 31> s_texture::bigTextureQueue;
PushQueue<std::shared_ptr<s_texture::texRecap>> s_texture::textureQueue;
std::atomic<long> s_texture::currentAllocation(0); // Allocations planned but not done yet
bool s_texture::loadInLowResolution = false;
int s_texture::lowResMax = MAX_LOW_RES;
std::vector<std::shared_ptr<s_texture::texRecap>> s_texture::releaseMemory[3];
std::vector<std::unique_ptr<Texture>> s_texture::releaseTexture[3];
short s_texture::releaseIdx = 0;
short s_texture::releaseTexIdx = 0;

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

s_texture::s_texture(const std::string& _textureName, int _loadType, bool mipmap, int resolution, int depth, int nbChannels, int channelSize) : textureName(_textureName),
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
		succes = preload(textureName, mipmap, resolution, depth, nbChannels, channelSize);
	else
		succes = preload(texDir + textureName, mipmap, resolution, depth, nbChannels, channelSize);

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
	unsigned char image_data[4] = {255,0,0,255};

	auto &tex = texCache["\0"];
	if (tex.expired()) {
		texture = std::make_shared<texRecap>();
		texture->size = 4;
		texture->width = 1;
		texture->height = 1;
		texture->depth = 1;
		texture->texture = std::make_unique<Texture>(*VulkanMgr::instance, *Context::instance->stagingMgr, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "Fallback texture");
		texture->texture->init(1, 1, image_data);
		tex = texture;
	} else {
		texture = tex.lock();
	}
}

bool s_texture::preload(const std::string& fullName, bool mipmap, int resolution, int depth, int _nbChannels, int _channelSize)
{
	auto &tex = texCache[fullName];
	nbChannels = _nbChannels;
	channelSize = _channelSize;
	if (tex.expired()) {
		int channels;
		texture = std::make_shared<texRecap>();
		stbi_uc *data;
		if (channelSize == 1)
		 	data = stbi_load(fullName.c_str(), &texture->width, &texture->height, &channels, nbChannels);
		else
			data = reinterpret_cast<stbi_uc*>(stbi_load_16(fullName.c_str(), &texture->width, &texture->height, &channels, nbChannels));
		if (!data) {
			cLog::get()->write("s_texture: could not load " + fullName , LOG_TYPE::L_ERROR);
			return false;
		}
		tex = texture;
		texture->size = texture->width * texture->height * nbChannels * channelSize;
		texture->depth = depth;
		texture->mipmap = mipmap;
		texture->bigTextureResolution = resolution;
		texture->width /= 1 << ((static_cast<int>(std::log2(texture->depth)) - 1) / 2);
		texture->height /= 1 << (static_cast<int>(std::log2(texture->depth)) / 2);
		if (loadInLowResolution && depth == 1 && texture->size > lowResMax) {
			// Scale image, don't modify the x/y ratio
			float scale = std::sqrt(lowResMax / (float) texture->size);
			texture->width *= scale;
			texture->height *= scale;
			texture->size = texture->width * texture->height * nbChannels * channelSize;
		}
		// if (channelSize == 1)
		// 	blend(loadType, data, texture->size);
		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (mipmap)
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		const VkImageType imgType = (depth > 1) ? VK_IMAGE_TYPE_3D : ((texture->height > 1) ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_1D);
		--_nbChannels;
		const VkFormat format = (const VkFormat[]) {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM}[(channelSize == 1) ? _nbChannels : (_nbChannels | 4)];
		texture->texture = std::make_unique<Texture>(*VulkanMgr::instance, *Context::instance->texStagingMgr, usage, fullName.substr(fullName.find(".spacecrafter/")+14), format, imgType);
		stbi_image_free(data);
		if (texture->size <= 1*1024*1024)
			getTexture(); // Enfore loading the texture now
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
	if (realWidth != texture->width) {
		// This texture have been rescaled
		stbi_uc *dataIn = data;
		data = new stbi_uc[texture->size];
		stbir_resize_uint8(dataIn, realWidth, realHeight, 0, data, texture->width, texture->height, 0, nbChannels);
		stbi_image_free(dataIn);
	}
	// Use negated height to flip this axis
	bool ret = texture->texture->init(texture->width, -texture->height, data, texture->mipmap, nbChannels, channelSize, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, texture->depth);
	if (!ret) {
		releaseUnusedMemory();
		ret = texture->texture->init(texture->width, -texture->height, data, texture->mipmap, nbChannels, channelSize, VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, texture->depth);
	}
	if (realWidth != texture->width)
		delete[] data;
	else
		stbi_image_free(data);
	if (!ret) {
		cLog::get()->write("s_texture: not enough memory for " + fullName, LOG_TYPE::L_ERROR);
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
	const long size = static_cast<long>(texture->width)*static_cast<long>(texture->height);
	if (texture->texture->isOnCPU()) {
		p = (uint8_t *) texture->texture->acquireStagingMemoryPtr();
		for (long i = 0; i < size; ++i)
			sum += p[i * 4];
		// texture->texture->releaseStagingMemoryPtr();
	} else {
		int width, height, channels;
		p = stbi_load(textureName.c_str(), &width, &height, &channels, 4);
		if (!p) {
			cLog::get()->write("Failed to compute average luminance for " + textureName, LOG_TYPE::L_ERROR);
			return (0);
		}
		for (long i = 0; i < size; ++i)
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
	bigTextures.clear();
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
}

Texture &s_texture::getTexture()
{
	if (!texture->texture->isOnGPU()) {
		if (!texture->texture->isOnCPU()) {
			load();
		}
		texture->texture->use();
		textureQueue.emplace(texture);
	}
	return *texture->texture;
}

Texture *s_texture::getBigTexture()
{
	if (texture->bigTexture) {
		if (texture->bigTexture->binding == texture->bigTextureBinding) {
			texture->bigTexture->lifetime = 3;
			return (texture->bigTexture->ready ? texture->bigTexture->texture.get() : nullptr);
		}
		texture->bigTexture = nullptr;
	}
	texture->bigTexture = acquireBigTexture(texture->bigTextureResolution);
	if (texture->bigTexture && texture->bigTexture->ready)
		return texture->bigTexture->texture.get();
	return nullptr;
}

s_texture::bigTexRecap *s_texture::acquireBigTexture(int resolution)
{
	int extPos = textureName.find_last_of('.');
	for (auto &bt : bigTextures) {
		if (!bt.acquired && bt.resolution <= resolution) {
			std::string tmpName = textureName.substr(0, extPos) + std::to_string(bt.resolution) + "K" + textureName.substr(extPos);
			if (tmpName == bt.texName) {
				bt.lifetime = 3;
				bt.ready = true;
				bt.acquired = true;
				texture->bigTextureBinding = bt.binding;
				return &bt;
			}
			if (!std::ifstream(tmpName).good()) // Check if file exist
				return nullptr;
			bt.texName = tmpName;
			texture->bigTextureBinding = bt.binding;
			bt.lifetime = 3;
			bt.ready = false;
			bt.acquired = true;
			bigTextureQueue.emplace(&bt);
			return &bt;
		}
	}
	// There is no bigTexRecap available, create another one
	for (MemoryQuerry &querry : VulkanMgr::instance->getMemoryManager()->querryMemory()) {
		if (!(querry.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
			continue;
		while (resolution * resolution * (4 * 1024 * 1024) > querry.free - currentAllocation)
			resolution /= 2;
		std::string tmpName = textureName.substr(0, extPos) + std::to_string(resolution) + "K" + textureName.substr(extPos);
		if (resolution < 4 || !std::ifstream(tmpName).good()) // Check if file exist
			return nullptr;
		auto it = bigTextures.rbegin();
		while (it != bigTextures.rend() && it->resolution < resolution)
			++it;
		auto it2 = it.base();
		bigTextures.insert(it2, bigTexRecap{1, resolution, nullptr, tmpName, 3, false, true});
		texture->bigTextureBinding = 1;
		bigTextureQueue.emplace(texture->bigTexture);
		currentAllocation += resolution * (resolution / 2) * 4 * 1024 * 1024 * 4/3;
		return (&*it2);
	}
	return nullptr;
}

void s_texture::releaseUnusedMemory()
{
	cLog::get()->write("Not enough memory, attempt to release memory", LOG_TYPE::L_WARNING);
	for (auto &bt : bigTextures) {
		if (!bt.acquired)
			bt.texture = nullptr;
	}
}

void s_texture::recordTransfer(VkCommandBuffer cmd)
{
	for (auto &t : releaseMemory[releaseIdx])
		t->texture->detach();
	releaseMemory[releaseIdx].clear();
	std::shared_ptr<texRecap> tex;
	while (textureQueue.pop(tex)) {
		tex->texture->use(cmd, true);
		releaseMemory[releaseIdx].push_back(tex);
	}
	releaseIdx = (releaseIdx + 1) % 3;
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
