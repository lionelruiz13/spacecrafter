/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2005 Robert Spearman
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2018 LSS Team & Immersive Adventure
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

// manage an assortment of script loaded images

#include <iostream>
#include <algorithm>
#include "mediaModule/image_mgr.hpp"
#include "mediaModule/image.hpp"
#include "tools/s_texture.hpp"



ImageMgr::ImageMgr()
{
	strToPos["viewport"] = IMG_POSITION::POS_VIEWPORT;
	strToPos["horizontal"] = IMG_POSITION::POS_HORIZONTAL;
	strToPos["equatorial"] = IMG_POSITION::POS_EQUATORIAL;
	strToPos["j2000"] = IMG_POSITION::POS_J2000;
	strToPos["dome"] = IMG_POSITION::POS_DOME;
	currentImg = nullptr;
}

void ImageMgr::createImageShader()
{
	Image::createShaderUnified();
	Image::createShaderImageViewport();
	Image::createSC_context();
}

ImageMgr::~ImageMgr()
{
	currentImg = nullptr;
	dropAllImages();
}


bool ImageMgr::loadImage(VideoTexture imgTex, const std::string& name, const std::string& coordinate, IMG_PROJECT project)
{
	// if name already exists, replace with new image
	this->drop_image(name);
	IMG_POSITION img_pos = convertStrToPosition(coordinate);
	std::unique_ptr<Image> img = std::make_unique<Image>(imgTex, name, img_pos, project);

	if (!img || img->imageLoaded()) {
		active_images.push_back(std::move(img));
		return true;
	}
	return false;
}


IMG_POSITION ImageMgr::convertStrToPosition( const std::string & _coordinate) const
{
	std::string coordinate = _coordinate;
	transform(coordinate.begin(), coordinate.end(), coordinate.begin(), ::tolower);
	auto it=strToPos.find(coordinate);
	if(it!=strToPos.end())
		return it->second;
	else
		return IMG_POSITION::POS_VIEWPORT;
}


bool ImageMgr::loadImage(const std::string& filename, const std::string& name, const std::string& coordinate,  IMG_PROJECT project, bool mipmap)
{
	// if name already exists, replace with new image
	this->drop_image(name);
	IMG_POSITION img_pos = convertStrToPosition(coordinate);
	std::unique_ptr<Image> img = std::make_unique<Image>(filename, name, img_pos, project, mipmap);

	if (!img || img->imageLoaded()) {
		active_images.push_back(std::move(img));
		return true;
	}
	return false;
}

void ImageMgr::drop_image(const std::string &name)
{
	currentImg = nullptr;
	active_images.remove_if([& name ](const std::unique_ptr<Image> & ptr ) {
		return ptr->getName() == name ;
	});
}

void ImageMgr::dropAllNoPersistent()
{
	currentImg= nullptr;
	active_images.remove_if([](const std::unique_ptr<Image> &  img) {
		return !img->imageIsPersistent();
	}) ;
}

void ImageMgr::dropAllImages()
{
	currentImg = nullptr;
	active_images.clear();
}

bool ImageMgr::setImage(const std::string& name)
{
	auto iter = std::find_if(active_images.begin(), active_images.end(), [&name](const std::unique_ptr<Image> &ptr) {
		return ptr->getName() == name;
	});
	if (iter != active_images.end()) {
		currentImg = (*iter).get();
		return true;
	}
	currentImg = nullptr;
	return false;
}

void ImageMgr::update(int delta_time)
{
	for (auto iter = active_images.begin(); iter != active_images.end(); ++iter) {
		(*iter)->update(delta_time);
	}
}

void ImageMgr::draw(const Navigator * nav, const Projector * prj)
{
	for (auto iter = active_images.begin(); iter != active_images.end(); ++iter) {
		(*iter)->draw(nav, prj);
	}
}

void ImageMgr::setPersistent(bool value)
{
	if (currentImg != nullptr)
		currentImg -> setPersistent(value);
}

void ImageMgr::setAlpha(float alpha, float duration)
{
	if (currentImg != nullptr)
		currentImg -> setAlpha(alpha, duration);
}

void ImageMgr::setScale(float scale, float duration)
{
	if (currentImg != nullptr)
		currentImg -> setScale(scale, duration);
}

void ImageMgr::setRotation(float rotation, float duration)
{
	if (currentImg != nullptr)
		currentImg -> setRotation(rotation, duration);
}

void ImageMgr::setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x, bool decelerate_x, bool accelerate_y, bool decelerate_y)
{
	if (currentImg != nullptr)
		currentImg -> setLocation(xpos, deltax, ypos, deltay, duration, accelerate_x, decelerate_x, accelerate_y, decelerate_y);
}

void ImageMgr::setRatio(float ratio, float duration)
{
	if (currentImg != nullptr)
		currentImg -> setRatio(ratio, duration);
}

void ImageMgr::setTransparency(bool v)
{
	if (currentImg != nullptr)
		currentImg -> setTransparency(v);
}

void ImageMgr::setKeyColor(const Vec3f& _color, float _intensity)
{
	if (currentImg != nullptr)
		currentImg -> setKeyColor(_color, _intensity);
}
