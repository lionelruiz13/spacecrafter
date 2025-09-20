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
#ifndef _IMAGE_MGR_H_
#define _IMAGE_MGR_H_

#include <list>
#include <map>
#include <string>
#include <memory>

#include "tools/vecmath.hpp"
#include "mediaModule/media_base.hpp"
#include "tools/no_copy.hpp"

class Image;
class Navigator;
class Projector;
class Set;

/**
 * @class ImageMgr
 * @brief This class manages all the image entities used in the scripts.
 *
 * The active_images container contains all the images that are independent.
 * This class is then used as an image manager.
 *
 * Some images are permanent, so they are not affected by deletion
 *
 * currentImg allows to modify the images of the container with the function
 * setImage. If currentImg is defined, then setXXXX modifies the concerned image.
 */


class ImageMgr: public NoCopy {
public:
	ImageMgr();
	virtual ~ImageMgr();

	//! tube for creating shaders for drawing images
	void createImageShader();

	//! load an image in the container
	bool loadImage(const std::string& filename, const std::string& name, const std::string& coordinate, IMG_PROJECT project, bool mipmap);
	//! load an image directly from the OpenGL
	bool loadImage(VideoTexture imgTex, const std::string& name, const std::string& coordinate, IMG_PROJECT project);
	//! removes the image name from the container
	void drop_image(const std::string &name);
	//! deletes all non-persistent images from the container
	void dropAllNoPersistent();
	//! deletes all the images in the container
	void dropAllImages();

	//! changes the persistence of currentImg
	void setPersistent(bool value);
	//! changes the target image of currentImg
	bool setImage(const std::string &name);
	//! changes the transparency of currentImg
	void setAlpha(float alpha, float duration);
	//! changes the size of currentImg
	void setScale(float scale, float duration);
	//! changes the rotation angle of currentImg
	void setRotation(float rotation, float duration);
	//! changes the position of currentImg on the dome
	void setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x = false, bool decelerate_x = false, bool accelerate_y = false, bool decelerate_y = false);
	//! changes the ratio of currentImg
	void setRatio(float ratio, float duration);

	//! enables color removal
	void setTransparency(bool v);
	//! determines the color of the image to be deleted on the plot
	void setKeyColor(const Vec3f& _color, float _intensity);

	//! update the fader of the images
	void update(int delta_time);
	//! displays all the images in the container
	void draw(const Navigator * nav, const Projector * prj);

	//! converts a string into IMG_POSITION enum
	IMG_POSITION  convertStrToPosition( const std::string & coordinate) const;
private:
	Image * currentImg=nullptr;
	std::list<std::unique_ptr<Image>> active_images;
	std::map<std::string, IMG_POSITION> strToPos;
};

#endif // _IMAGE_MGR_H
