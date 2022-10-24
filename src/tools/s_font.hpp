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

#include "tools/s_font_common.hpp"
#include "tools/s_texture.hpp"
#include "tools/draw_helper.hpp"
#include "EntityCore/SubTexture.hpp"

class VertexArray;
class Projector;
class Texture;
class Pipeline;
class TileMap;

struct renderedString_struct {
	SubTexture stringTexture;  // Rendered string texture reference
	SubTexture borderTexture;  // Rendered string bordered texture
	float textureW; 	   // Width of texture in pixels
	float textureH; 	   // Height of texture in pixels
	float stringW; 	       // Width of string portion in pixels
	float stringH; 	       // Height of string portion in pixels
	bool haveBorder;	   // if the text has a bordered texture
};

typedef std::map< std::string, renderedString_struct > renderedStringHash_t;
typedef renderedStringHash_t::const_iterator renderedStringHashIter_t;

/**
 * \file s_font.hpp
 * \brief transforms strings into texture
 * \author Olivier NIVOIX
 * \version 2
 *
 * \class s_font
 *
 * \brief this class, based on the SDL2_ttf library, transforms a text character string into an available texture
 * for OpenGL to display it in two ways:
 * - display parallel to the horizon
 * - right display, in the sky.
 */
class s_font {

public:
	//! creates a font of size_i using the font file named ttfFileName
	s_font(float size_i, const std::string& ttfFileName);
	virtual ~s_font();
	//! modify the parameters of the font
	void rebuild(float size_i, const std::string& ttfFileName);

	//! display a text s right at the point M(x,y) of color Color at the position MVP with upsidedown indicating if it is upright or upside down
	void print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP ,int upsidedown, bool cached = true);
	//! display a text parallel to the horizon in altitude azimuth
	//! cache indicates whether to keep the text in memory
	void printHorizontal(const Projector *prj, float altitude, float azimuth, const std::string &str, const Vec4f &texColor, TEXT_ALIGN textPos, bool cache);
	//! remove text s from cache, after which it may be overwritten by any further print* call which occur before the final submission of the current frame
	void clearCache(const std::string& s);
	//! remove every texts from font cache, after which it may be overwritten by any further print* call which occur before the final submission of the current frame
	void clearCache();
	//! indicates the size in pixels of the text s
	float getStrLen(const std::string& s);
	//! creates the whole graphic context of the fonts
	static void createSC_context();
	//! sets up the backup font
	static void initBaseFont(const std::string& ttfFileName);
	//! Initialize printer
	static void beginPrint();
	static TileMap *tileMap;
protected:
	renderedString_struct renderString(const std::string &s, bool withBorder) const;
	renderedStringHash_t renderCache;
	static std::vector<renderedString_struct> tempCache, tempCache2; // to hold texture while it is used
	static std::string lastUncached;
	static std::vector<std::pair<std::vector<struct s_print>, std::vector<struct s_printh>>> printData;
	static std::list<s_font *> fontList;
	std::list<s_font *>::iterator self;

	float fontSize;
	std::string fontName;
	static std::string baseFontName;
	TTF_Font *myFont =  nullptr;

	// static int activeID; // target id
	// static int commandIndexHorizontal, commandIndexPrint;
	// commandIndex[0] is not multisampled
	// all other were multisampled
	// static std::vector<std::pair<int, int>> commandIndex; // pair of {horizontal, print}
	// static Set *set;
	// static VertexArray *vertexHorizontal;
	// static VertexArray *vertexPrint;
	// static ThreadedCommandBuilder *cmdMgr;
	// static Pipeline *pipelineHorizontal;
	// static Pipeline *pipelinePrint;
	// static PipelineLayout *layoutHorizontal;
	// static PipelineLayout *layoutPrint;
	static int nbFontInstances;
	static bool needFlush;
};

#endif  //_S_FONT_H
