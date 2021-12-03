/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2020   AssociationSirius
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



#ifndef _IMAGE_TEXTURE_HPP
#define _IMAGE_TEXTURE_HPP

#include <string>
#include <memory>
#include <vulkan/vulkan.h>

class s_texture;
class Navigator;
class Projector;
class Set;
class PipelineLayout;
class SyncEvent;
struct VideoSync;

class ImageTexture {
public :
	ImageTexture(PipelineLayout *layout);
	virtual ~ImageTexture() {}
	virtual void getDimensions(int &img_w, int &img_h) = 0;
	const std::string& getType() const {
		return type;
	}
	bool isYUV() const {
		return isyuv;
	}
	void bindSet(VkCommandBuffer cmd, PipelineLayout *layout);
	void setupSync(std::shared_ptr<VideoSync> &sync);
protected:
	std::string type;
	bool isyuv;
	std::unique_ptr<Set> set;
	std::shared_ptr<VideoSync> sync;
};


class RBGImageTexture: public ImageTexture {
public:
	RBGImageTexture(s_texture* img, PipelineLayout *layout);
	virtual ~RBGImageTexture();
	virtual void getDimensions(int &img_w, int &img_h)  override;
private:
	s_texture* image = nullptr;
};


class YUVImageTexture: public ImageTexture {
public:
	YUVImageTexture(s_texture* imgY, s_texture* imgU, s_texture* imgV, PipelineLayout *layout);
	virtual ~YUVImageTexture();
	virtual void getDimensions(int &img_w, int &img_h)  override;
private:
	s_texture* imageY = nullptr;
	s_texture* imageU = nullptr;
	s_texture* imageV = nullptr;
};

#endif // _IMAGE_TEXTURE_HPP
