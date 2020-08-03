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


class VertexArray;
class shaderProgram;
class Projector;


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
	//! crée une fonte de taille size_i utilisant le fichier fonte nommé ttfFileName
	s_font(float size_i, const std::string& ttfFileName);
	virtual ~s_font();

	//! affiche un texte s droit au point M(x,y) de couleur Color à la position MVP avec upsidedown indiquant s'il est à l'endroit ou à l'envers
	void print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP ,int upsidedown);
	//! afficher un texte parallège à l'horizon en altitude azimuth
	//! cache indique si l'on doit garder le texte en mémoire  
	void printHorizontal(const Projector * prj, float altitude, float azimuth, const std::string& str, Vec3f& texColor, TEXT_ALIGN testPos, bool cache);
	//! supprime du cache le texte s
	void clearCache(const std::string& s);
	//! supprime tous les textes de la font
	void clearCache();
	//! indique la taille en pixel du texte s
	float getStrLen(const std::string& s);
	//! crée tout le contexte graphique des fontes
	static void createSC_context();
	//! met en place la fonte de secours
	static void initBaseFont(const std::string& ttfFileName);
protected:
	renderedString_struct renderString(const std::string &s, bool withBorder) const;
	renderedStringHash_t renderCache;

	float fontSize;
	std::string fontName;
	static std::string baseFontName;
	TTF_Font *myFont =  nullptr;

	static std::unique_ptr<shaderProgram> shaderHorizontal;
	static std::unique_ptr<shaderProgram> shaderPrint;
	static std::unique_ptr<VertexArray> m_fontGL;
};

#endif  //_S_FONT_H
