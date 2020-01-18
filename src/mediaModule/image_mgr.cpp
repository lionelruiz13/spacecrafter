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
#include "mediaModule/image_mgr.hpp"
#include "tools/s_texture.hpp"



ImageMgr::ImageMgr()
{
	currentImg = nullptr;
}

void ImageMgr::createImageShader()
{
	Image::createShaderUnified();
	Image::createShaderImageViewport();	
}

ImageMgr::~ImageMgr()
{
	currentImg = nullptr;
	dropAllImages();
	Image::deleteShaderUnified();
	Image::deleteShaderImageViewport();
}

int ImageMgr::loadImage(GLuint imgTex, const std::string& name, const std::string& coordinate)
{
	// if name already exists, replace with new image
	this->drop_image(name);
	Image::IMAGE_POSITIONING img_pos = convertStrToPosition(coordinate);
	s_texture* imgVideo = new s_texture(name, imgTex);
	Image *img = new Image(imgVideo, name, img_pos);

	if (!img || img->imageLoaded()) {
		active_images.push_back(img);
		return 1;
	} else return 0;

}

Image::IMAGE_POSITIONING ImageMgr::convertStrToPosition( const std::string & coordinate) const
{
	Image::IMAGE_POSITIONING img_pos = Image::IMAGE_POSITIONING::POS_VIEWPORT;
	if (coordinate == "horizontal") img_pos = Image::IMAGE_POSITIONING::POS_HORIZONTAL;
	else if (coordinate == "equatorial") img_pos = Image::IMAGE_POSITIONING::POS_EQUATORIAL;
	else if (coordinate == "j2000") img_pos = Image::IMAGE_POSITIONING::POS_J2000;
	else if (coordinate == "dome") img_pos = Image::IMAGE_POSITIONING::POS_DOME;

	return img_pos;
}

int ImageMgr::loadImage(const std::string& filename, const std::string& name, const std::string& coordinate, bool mipmap)
{
	// if name already exists, replace with new image
	this->drop_image(name);
	Image::IMAGE_POSITIONING img_pos = convertStrToPosition(coordinate);
	Image *img = new Image(filename, name, img_pos, mipmap);

	if (!img || img->imageLoaded()) {
		active_images.push_back(img);
		return 1;
	} else return 0;

}

int ImageMgr::drop_image(const std::string& name)
{
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		if ((*iter)->getName()==name) {
			currentImg= nullptr;
			delete (*iter);
			active_images.erase(iter);
			iter--;
		}
	}
	return 0;  // not found
}

int ImageMgr::dropAllNoPersistent()
{
	currentImg= nullptr;
	// memory leak
	//~ active_images.remove_if([](Image* img){ if (!img->imageIsPersistent()) return true; else return false;});
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		//~ cout << "iter " << (*iter)->getName() << endl;
		if (!(*iter)->imageIsPersistent()) {
			delete (*iter);
			//~ (*iter)=nullptr;
			iter = active_images.erase(iter);
			iter--;
		}
	}
	return 0;
}

int ImageMgr::dropAllImages()
{
	currentImg = nullptr;
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		delete *iter;
	}
	active_images.clear();
	return 0;
}

bool ImageMgr::setImage(const std::string& name)
{
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		if ((*iter)->getName()==name) {
			currentImg = *iter;
			return true;}
	}
	currentImg = nullptr;
	return false;
}

void ImageMgr::update(int delta_time)
{
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		(*iter)->update(delta_time);
	}
}

void ImageMgr::draw(const Navigator * nav, Projector * prj)
{
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		(*iter)->draw(nav, prj);
	}
}

void ImageMgr::setPersistent(bool value) {
	if (currentImg != nullptr)
		currentImg -> setPersistent(value);
}

void ImageMgr::setAlpha(float alpha, float duration) {
	if (currentImg != nullptr)
		currentImg -> setAlpha(alpha, duration);
}

void ImageMgr::setScale(float scale, float duration) {
	if (currentImg != nullptr)
		currentImg -> setScale(scale, duration);
}

void ImageMgr::setRotation(float rotation, float duration) {
	if (currentImg != nullptr)
		currentImg -> setRotation(rotation, duration);
}

void ImageMgr::setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x, bool decelerate_x, bool accelerate_y, bool decelerate_y) {
	if (currentImg != nullptr)
		currentImg -> setLocation(xpos, deltax, ypos, deltay, duration, accelerate_x, decelerate_x, accelerate_y, decelerate_y);
}

void ImageMgr::setRatio(float ratio, float duration) {
	if (currentImg != nullptr)
		currentImg -> setRatio(ratio, duration);
}

void ImageMgr::setTransparency(bool v) {
	if (currentImg != nullptr)
		currentImg -> setTransparency(v);
}

void ImageMgr::setKeyColor(const Vec3f& _color, float _intensity) {
	if (currentImg != nullptr)
		currentImg -> setKeyColor(_color, _intensity);
}


void ImageMgr::clone(const std::string& _name, int i)
{
	if (i<2||i>3) return;
	Image* tmp = nullptr;
	for (std::list<Image*>::iterator iter = active_images.begin(); iter != active_images.end(); ++iter) {
		if ((*iter)->getName()== _name) {
			tmp = (*iter);
			break;
		}
	}
	if (tmp == nullptr) return;
	if (i==2) {
		Image* tmp2 = new Image(tmp,180.f);
		active_images.push_back(tmp2);
	}
	if (i==3) {
		Image* tmp3 = new Image(tmp,240.f);
		active_images.push_back(tmp3);
		Image* tmp2 = new Image(tmp,120.f);
		active_images.push_back(tmp2);
	}
}
