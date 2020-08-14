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

RBGImageTexture::RBGImageTexture(s_texture* img)
{
	image = img;
	type = "useRBG";
}

RBGImageTexture::~RBGImageTexture()
{
	if (image!=nullptr) delete image;
}

void RBGImageTexture::getDimensions(int &img_w, int &img_h)
{
	image->getDimensions(img_w, img_h);
}

void RBGImageTexture::bindImageTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, image->getID());
}


YUVImageTexture::YUVImageTexture(s_texture* imgY, s_texture* imgU, s_texture* imgV )
{
	imageY = imgY;
	imageU = imgU;
	imageV = imgV;
	type = "useYUV";
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

void YUVImageTexture::bindImageTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, imageY->getID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, imageU->getID());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture (GL_TEXTURE_2D, imageV->getID());
}
