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

#include <iostream>
#include "mediaModule/imageTexture.hpp"
#include "tools/s_texture.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "mediaModule/media_base.hpp"
#include "EntityCore/Resource/SyncEvent.hpp"

ImageTexture::ImageTexture(PipelineLayout *layout)
{
	set = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, layout, -1, false, true);
}

void ImageTexture::bindSet(VkCommandBuffer cmd, PipelineLayout *layout)
{
	layout->bindSet(cmd, *set);
	if (sync) {
		sync->inUse = true;
	}
}

void ImageTexture::setupSync(std::shared_ptr<VideoSync> &_sync)
{
	sync = _sync;
}

RBGImageTexture::RBGImageTexture(s_texture* img, PipelineLayout *layout) : ImageTexture(layout)
{
	image = img;
	type = "useRBG";
	isyuv = false;
	set->bindTexture(image->getTexture(), 0);
}

RBGImageTexture::~RBGImageTexture()
{
	if (image!=nullptr) delete image;
}

void RBGImageTexture::getDimensions(int &img_w, int &img_h)
{
	image->getDimensions(img_w, img_h);
}

YUVImageTexture::YUVImageTexture(s_texture* imgY, s_texture* imgU, s_texture* imgV, PipelineLayout *layout) : ImageTexture(layout)
{
	imageY = imgY;
	imageU = imgU;
	imageV = imgV;
	type = "useYUV";
	isyuv = true;
	set->bindTexture(imageY->getTexture(), 0);
	set->bindTexture(imageU->getTexture(), 1);
	set->bindTexture(imageV->getTexture(), 2);
}

YUVImageTexture::~YUVImageTexture()
{
	if (imageY != nullptr)	delete imageY;
	if (imageU != nullptr)	delete imageU;
	if (imageV != nullptr)	delete imageV;
}

void YUVImageTexture::getDimensions(int &img_w, int &img_h)
{
	imageY->getDimensions(img_w, img_h);
}
