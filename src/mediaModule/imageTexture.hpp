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

class s_texture;
class Navigator;
class Projector;

class ImageTexture
{
public :
	ImageTexture(){}
	virtual ~ImageTexture(){}
	virtual void getDimensions(int &img_w, int &img_h) = 0;
	virtual void bindImageTexture() = 0;
	const std::string& getType() const {
		return type;
	}
protected:
	std::string type;
};


class RBGImageTexture: public ImageTexture {
public:
	RBGImageTexture(s_texture* img);
	virtual ~RBGImageTexture();
	virtual void getDimensions(int &img_w, int &img_h)  override;
	virtual void bindImageTexture() override;
private:
	s_texture* image = nullptr;
};


class YUVImageTexture: public ImageTexture {
public:
	YUVImageTexture(s_texture* imgY, s_texture* imgU, s_texture* imgV );
	virtual ~YUVImageTexture();
	virtual void getDimensions(int &img_w, int &img_h)  override;
	virtual void bindImageTexture() override;
private:
	s_texture* imageY = nullptr;
	s_texture* imageU = nullptr;
	s_texture* imageV = nullptr;	
};

#endif // _IMAGE_TEXTURE_HPP
