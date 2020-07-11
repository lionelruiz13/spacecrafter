/*
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2020 of the LSS Team & Association Sirius
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

// Class to manage s_fonts

#ifndef _S_FONT_H
#define _S_FONT_H


#include <map>
#include <vector>
#include <memory>
#include <SDL2/SDL_ttf.h>

#include "tools/utility.hpp"
#include "tools/s_texture.hpp"


class VertexArray;
class shaderProgram;
class Projector;

enum class TEXT_POSITION : char {LEFT, RIGHT, CENTER};

typedef struct {
	GLuint stringTexture;  // Rendered string texture reference - remember to delete when done
	GLuint borderTexture;  // Rendered string bordered texture -  remember to delete when done
	float textureW; 	   // Width of texture in pixels
	float textureH; 	   // Height of texture in pixels
	float stringW; 	       // Width of string portion in pixels
	float stringH; 	       // Height of string portion in pixels
	bool haveBorder;	   // if the text has a bordered texture
} renderedString_struct;

typedef std::map< std::string, renderedString_struct > renderedStringHash_t;
typedef renderedStringHash_t::const_iterator renderedStringHashIter_t;

class s_font {

public:
	s_font(float size_i, const std::string& ttfFileName);
	virtual ~s_font();

	void print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP ,int upsidedown);
	void printHorizontal(const Projector * prj, float altitude, float azimuth, const std::string& str, Vec3f& texColor, TEXT_POSITION testPos, bool cache);

	void clearCache(const std::string& s);
	void clearCache();

	float getStrLen(const std::string& s);

	static void createSC_context();

protected:

	renderedString_struct renderString(const std::string &s, bool withBorder) const;
	renderedStringHash_t renderCache;

	std::string fontName;
	TTF_Font *myFont;
	float fontSize;

	static std::unique_ptr<shaderProgram> shaderHorizontal;
	static std::unique_ptr<shaderProgram> shaderPrint;
	static std::unique_ptr<VertexArray> m_fontGL;
};

#endif  //_S_FONT_H
