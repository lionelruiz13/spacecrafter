/*
 * Spacecrafter
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2020 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

// Class to manage fonts

#include <vector>
#include "tools/log.hpp"
#include "tools/s_font.hpp"
#include "tools/utility.hpp"
#include "tools/call_system.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/draw_helper.hpp"
#include "coreModule/projector.hpp"
#include "EntityCore/Resource/TileMap.hpp"

std::vector<renderedString_struct> s_font::tempCache, s_font::tempCache2;
std::string s_font::lastUncached;
int s_font::nbFontInstances = 0;
bool s_font::needFlush = false;
std::list<s_font *> s_font::fontList;

std::vector<std::pair<std::vector<struct s_print>, std::vector<struct s_printh>>> s_font::printData;

std::string s_font::baseFontName;

TileMap *s_font::tileMap = nullptr;

void s_font::initBaseFont(const std::string& ttfFileName)
{
	baseFontName = ttfFileName;
	//on teste la fonte de base, si elle n'est pas opérationnelle, on stoppe tout.
	TTF_Font *tmp = TTF_OpenFont( baseFontName.c_str(), 12);
	if(!tmp) {
		cLog::get()->write("s_font: TTF_OpenFont error: "+ std::string(TTF_GetError()), LOG_TYPE::L_ERROR);
		cLog::get()->write("s_font: BaseFont file is not usable or operational: system aborded", LOG_TYPE::L_ERROR);
		exit(-1);
	}
	TTF_CloseFont(tmp);
}

s_font::s_font(float size_i, const std::string& ttfFileName)
{
	fontName = ttfFileName; // way to acces ttf file
	fontSize = size_i; // pixel height
	myFont = TTF_OpenFont( fontName.c_str(), fontSize);
	if(!myFont) {
		cLog::get()->write("s_font: TTF_OpenFont error: "+ std::string(TTF_GetError()), LOG_TYPE::L_ERROR);
		cLog::get()->write("s_font: replace font with baseFontName " + baseFontName, LOG_TYPE::L_WARNING);
		fontName = baseFontName;
		myFont = TTF_OpenFont( baseFontName.c_str(), fontSize);
	} else
	cLog::get()->write("s_font: loading font " + fontName, LOG_TYPE::L_INFO);
	nbFontInstances++;
	if (!tileMap) {
        auto &context = *Context::instance;
		tileMap = new TileMap(*VulkanMgr::instance, *context.stagingMgr, "s_font");
		tileMap->createMap(8192, 1024);
        context.transferSync->imageBarrier(**tileMap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COPY_BIT_KHR, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR);
        context.transferSync->build();
	}
	//std::cout << "Created new font with size: " << fontSize << " and TTF name : " << fontName << std::endl;
	fontList.push_front(this);
	self = fontList.begin();
}

void s_font::rebuild(float size_i, const std::string& ttfFileName)
{
	// dont's wast time to reload what is already loaded
	if ((fontSize == size_i) && (fontName == ttfFileName))
		return;

	// create new and swap it if correct
	TTF_Font *tmpFont =  nullptr;
	tmpFont = TTF_OpenFont( ttfFileName.c_str(), size_i);
	if(!tmpFont) {
		cLog::get()->write("s_font: TTF_OpenFont error: "+ std::string(TTF_GetError()), LOG_TYPE::L_ERROR);
		cLog::get()->write("s_font: no rebuild possible", LOG_TYPE::L_WARNING);
		return;
	} else {
		clearCache();
		TTF_CloseFont(myFont);
		fontName = ttfFileName;
		fontSize = size_i;
		myFont = tmpFont;
		cLog::get()->write("s_font: rebuild font succes", LOG_TYPE::L_INFO);
	}
}

s_font::~s_font()
{
	clearCache();
	TTF_CloseFont(myFont);
	myFont = nullptr;
	if (--nbFontInstances == 0) { // if it's the last s_font instance, clear per-frame caches
		// There must be no command using those textures
		tempCache.clear();
		tempCache2.clear();
		if (tileMap) {
			delete tileMap;
			tileMap = nullptr;
		}
	}
	fontList.erase(self);
}


void s_font::createSC_context()
{
	printData.resize(3);
	for (auto &value : printData) {
		value.first.reserve(2048); // max 2048 print per frame
		value.second.reserve(256); // max 256 printHorizontal per frame
	}
}

void s_font::beginPrint()
{
	printData[Context::instance->frameIdx].first.clear();
	printData[Context::instance->frameIdx].second.clear();
	if (needFlush) {
		// Clear every subtextures, both cached and uncached.
		// Needing to flush will cause graphical glitch for one frame (due to missing texture)
		// Clearing everything may cause other graphical glitch of text for this frame only, but avoid missing text display for further prints
		cLog::get()->write("s_font : Out of text surface, reset every text allocation to get space", LOG_TYPE::L_WARNING);
		for (auto f : fontList)
			f->clearCache();
		tempCache.insert(tempCache.end(), tempCache2.begin(), tempCache2.end());
		tempCache2.clear();
		needFlush = false;
	} else {
	    tempCache.swap(tempCache2);
	}
    for (int i = tempCache.size(); i--;) { // inverse release order reduce overhead
        if (tempCache[i].stringTexture.width)
            tileMap->releaseSurface(tempCache[i].stringTexture);
        if (tempCache[i].haveBorder)
            tileMap->releaseSurface(tempCache[i].borderTexture);
    }
	tempCache.clear();
    lastUncached.clear();
}

//! print out a string
void s_font::print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP, int upsidedown, bool cache)
{
	if (s.empty())
		return;

	renderedString_struct currentRender;
	// If not cached, create texture
	if(renderCache[s].textureW == 0 ) {
		if (lastUncached == s) {
			currentRender = tempCache.back();
		} else {
			currentRender = renderString(s, false);
			if(cache) {
				renderCache[s] = currentRender;
			} else {
                tempCache.push_back(currentRender); // to hold texture while it is used
                lastUncached = s;
			}
		}
	} else {
		// read from cache
		currentRender = renderCache[s];
	}

	float h = (upsidedown) ? -currentRender.textureH : currentRender.textureH;
	float w = currentRender.textureW;
	// ===== Variables 'passed' up to down : [x, y, h, w, Texture *string, Color, MVP] ===== //
	Context &context = *Context::instance;
	auto &tmp = printData[context.frameIdx].first;
	if (tmp.capacity() == tmp.size())
		return;
	tmp.push_back({DRAW_PRINT, x, y - currentRender.stringH, h, w, Color, &renderCache[s].stringTexture, MVP});
	context.helper->draw(&tmp.back());
	if (tmp.capacity() == tmp.size())
		cLog::get()->write("Limit of " + std::to_string(tmp.capacity()) + " print per frame reach, next attempt will be skipped\n", LOG_TYPE::L_WARNING);
}

float s_font::getStrLen(const std::string& s)
{
	if (s.empty())
		return 0;

	if( renderCache[s].textureW != 0 ) return renderCache[s].stringW;

	int w,h;
	if(TTF_SizeText(myFont,s.c_str(),&w,&h)) {
		cLog::get()->write("s_font: TTF_SizeText error: "+ std::string(TTF_GetError()), LOG_TYPE::L_ERROR);
		return w;
	} else {// perhaps print the current TTF_GetError(), the string can't be rendered...
		return w;
	}
}


//! remove cached texture for string
void s_font::clearCache(const std::string& s)
{
	if( renderCache[s].textureW != 0 ) {
        if (renderCache[s].stringTexture.width)
            tileMap->releaseSurface(renderCache[s].stringTexture);
        if (renderCache[s].haveBorder)
            tileMap->releaseSurface(renderCache[s].borderTexture);
		renderCache.erase(s);
	}

}

//! remove ALL cached textures
void s_font::clearCache()
{
    for (auto &c : renderCache) {
        if (c.second.stringTexture.width)
            tileMap->releaseSurface(c.second.stringTexture);
        if (c.second.haveBorder)
            tileMap->releaseSurface(c.second.borderTexture);
    }
	renderCache.clear();
}

//! Render a string to a texture
renderedString_struct s_font::renderString(const std::string &s, bool withBorder) const
{

	renderedString_struct rendering;
	SDL_Color color={255,255,255,0};
	SDL_Surface *text = TTF_RenderUTF8_Blended(myFont, s.c_str(), color ); //write in white

	// Calculate opengl texture size required
	rendering.stringW = text->w;
	rendering.stringH = text->h;
	rendering.haveBorder =  false;

	const unsigned short decalageX = 2;
	const unsigned short decalageY = 1;
	rendering.textureW = text->w+2*decalageX;
	rendering.textureH = text->h+2*decalageY;

	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine */
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	 rmask = 0xff000000;
	 gmask = 0x00ff0000;
	 bmask = 0x0000ff00;
	 amask = 0x000000ff;
	#else
	 rmask = 0x000000ff;
	 gmask = 0x0000ff00;
	 bmask = 0x00ff0000;
	 amask = 0xff000000;
	#endif

	SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)rendering.textureW, (int)rendering.textureH, 32, rmask, gmask, bmask, amask);
	renderedString_struct nothing;
	nothing.textureW = nothing.textureH = nothing.stringW = nothing.stringH = 0;
	nothing.haveBorder =false;
	nothing.stringTexture.width = 0;
	if(!surface)  {
		cLog::get()->write("s_font "+ fontName +": error SDL_CreateRGBSurface" + std::string(SDL_GetError()) , LOG_TYPE::L_ERROR);
		SDL_FreeSurface(text);
		return nothing;
	}

	//why (decalageX;decalageY) ? la texture initiale commence en (decalageX;decalageY)
	SDL_Rect tmp;
	tmp.x=decalageX;
	tmp.y=decalageY;
	tmp.w=text->w;
	tmp.h=text->h;
	SDL_BlitSurface(text, NULL, surface, &tmp);

	// get the number of channels in the SDL surface
	VkFormat texture_format;
	if (surface->format->Rmask == 0x000000ff)
		texture_format = VK_FORMAT_R8G8B8A8_UNORM;
	else
		texture_format = VK_FORMAT_B8G8R8A8_UNORM;

	//==== CREATE TEXTURE ====//
	rendering.stringTexture = tileMap->acquireSurface(rendering.textureW, rendering.textureH);
	if (rendering.stringTexture.width) {
		tileMap->writeSurface(rendering.stringTexture, surface->pixels);
	} else
		needFlush = true;

	if (withBorder) {
		// ***********************************
		//
		// création de la bordure
		//
		// ***********************************
		SDL_Surface *border = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)rendering.textureW, (int)rendering.textureH, 32, rmask, gmask, bmask, amask);
		if(!border)  {
			cLog::get()->write("s_font "+ fontName +": error SDL_CreateRGBSurface" + std::string(SDL_GetError()) , LOG_TYPE::L_ERROR);
			SDL_FreeSurface(text);
			return rendering;
		}

		//SDL_Rect tmp;
		int shiftx, shifty;
		for(int pass=0; pass < 4 ; pass++) {
			if(pass<2) shiftx = -1;
				else shiftx = 1;
			if(pass%2) shifty = -1;
				else shifty = 1;

			//why (decalageX;decalageY) ? la texture initiale commence en (decalageX;decalageY)
			tmp.x=decalageX+shiftx;
			tmp.y=decalageY+shifty;
			tmp.w=text->w;
			tmp.h=text->h;
			SDL_BlitSurface(text, NULL, border,  &tmp);
		}

		if (border->format->Rmask == 0x000000ff)
			texture_format = VK_FORMAT_R8G8B8A8_UNORM; // GL_RGBA
		else
			texture_format = VK_FORMAT_B8G8R8A8_UNORM; // GL_BGRA

		//==== CREATE TEXTURE ====//
		rendering.borderTexture = tileMap->acquireSurface(rendering.textureW, rendering.textureH);
		if (rendering.borderTexture.width) {
			tileMap->writeSurface(rendering.borderTexture, border->pixels);
			rendering.haveBorder =true;
		} else
			needFlush = true;

		SDL_FreeSurface(border);
	}

	SDL_FreeSurface(surface);
	SDL_FreeSurface(text);
	return rendering;
}


//! Draw text with baseline more or less parallel with horizon
void s_font::printHorizontal(const Projector * prj, float altitude, float azimuth, const std::string& str, Vec3f& texColor, TEXT_ALIGN testPos, bool cache)
{
	if (str.empty()) return;
	// Get rendered texture
	renderedString_struct rendering;
	SubTexture *subTex;
	if(renderCache[str].textureW == 0) {
		if (lastUncached == str) {
			rendering = tempCache.back();
			subTex = &tempCache.back().stringTexture;
		} else {
			rendering = renderString(str, true);
			if(cache) {
				renderCache[str] = rendering;
				subTex = &renderCache[str].stringTexture;
			} else {
	            tempCache.push_back(rendering); // to hold texture while it is used
	            lastUncached = str;
				subTex = &tempCache.back().stringTexture;
			}
		}
	} else {
		rendering = renderCache[str];
		subTex = &renderCache[str].stringTexture;
	}

	float angle, lCercle;
	switch (testPos) {
		case TEXT_ALIGN::LEFT : angle = 0; break;
		case TEXT_ALIGN::RIGHT :
			lCercle = 2.f*M_PI*prj->getViewportRadius() * (90.f-altitude)/90.f;
			angle = 360.f * rendering.textureW / lCercle;
			break;
		case TEXT_ALIGN::CENTER:
			lCercle = 2.f*M_PI*prj->getViewportRadius() * (90.f-altitude)/90.f;
			angle = 360.f * rendering.textureW / lCercle / 2.f; //because center
			break;
	}

	Vec3d startV, screen;
	Utility::spheToRect(-(azimuth-angle)*M_PI/180., altitude*M_PI/180., startV);
	prj->projectDomeFixed(startV, screen);
	float x = screen[0];
	float y = screen[1];

	Vec3d center = prj->getViewportCenter();
	float radius = center[2];
	float dx = x - center[0];
	float dy = y - center[1];
	float d = sqrt(dx*dx + dy*dy);
	// If the text is too far away to be visible in the screen return
	if(radius > 0) {
		if (d > radius + rendering.stringH) return;
	} else {
		if(std::max(prj->getViewportWidth(), prj->getViewportHeight() ) > d) return;
	}

	float theta = M_PI + atan2f(dx, dy - 1);
	float psi = (float)getStrLen(str)/(d + 1);  // total angle of rotation

	// ===== Variables 'passed' up to down : [thetha, psi, center[0:1], d, d-rendering.textureH, texColor, Texture *border, Texture *string] ===== //
	Context &context = *Context::instance;
	auto &tmp = printData[context.frameIdx].second;
	if (tmp.capacity() == tmp.size())
		return;
	tmp.push_back({DRAW_PRINTH, theta, psi, {center[0], center[1]}, d, d-rendering.textureH, texColor, subTex});
	context.helper->draw(&tmp.back());
	if (tmp.capacity() == tmp.size())
		cLog::get()->write("Limit of " + std::to_string(tmp.capacity()) + " printHorizontal per frame reach, next attempt will be skipped\n", LOG_TYPE::L_WARNING);
	// int steps = 2+int(psi*15);
	//
	// std::vector<Vec2f> meshPoints;  // screen x,y
	// std::vector<float> vecPos;
	// std::vector<float> vecTex;
	//
	// // Pre-calculate points (more efficient)
	// for (int i=0; i<=steps; i++) {
	// 	float angle, p, q;
	// 	angle = theta - i*psi/steps;
	// 	p = sin(angle);
	// 	q = cos(angle);
	//
	// 	meshPoints.push_back(Vec2f(center[0]+p*(d-rendering.textureH), center[1]+q*(d-rendering.textureH)));
	// 	meshPoints.push_back(Vec2f(center[0]+p*d,center[1]+q*d));
	// }
	//
	// Vec3f Color (texColor[0], texColor[1], texColor[2]);
	// /*
	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// */
	//
	// for (int i=0; i<=steps; i++) {
	// 	insert_vec2(vecPos,meshPoints[i*2]);
	// 	insert_vec2(vecPos,meshPoints[i*2+1]);
	// 	insert_all(vecTex, (float)i/steps, 0.f , (float)i/steps, 1.f);
	// }
	//
	// /*
	// shaderHorizontal->use();
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, rendering.stringTexture);
	// glActiveTexture(GL_TEXTURE1);
	// glBindTexture(GL_TEXTURE_2D, rendering.borderTexture);
	// */
	//
	// //shaderHorizontal->setUniform("Color", Color);
	// vertexHorizontal->fillVertexBuffer(BufferType::POS2D, vecPos);
	// vertexHorizontal->fillVertexBuffer(BufferType::TEXTURE,vecTex);
	//
	// set->clear();
	// set->bindTexture(rendering.stringTexture.get(), 0);
	// set->bindTexture(rendering.borderTexture.get(), 1);
	// cmdMgr->select(commandIndexHorizontal);
	// cmdMgr->pushSet(layoutHorizontal, set);
	// cmdMgr->pushConstant(layoutHorizontal, VK_SHADER_STAGE_FRAGMENT_BIT, 0, &Color, sizeof(Vec4f));
	// int offset = vertexHorizontal->getVertexOffset();
	// cmdMgr->draw(vecPos.size()/2, 1, offset);
	// vertexHorizontal->setVertexOffset(offset + vecPos.size()/2);
	//
	// //Renderer::drawArrays(shaderHorizontal.get(), *m_fontGL, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,vecPos.size()/2);
	//
	// vecPos.clear();
	// vecTex.clear();
	//
	// if (!cache) {
	// 	//glDeleteTextures( 1, &rendering.stringTexture);
	// 	//glDeleteTextures( 1, &rendering.borderTexture);
	// }
}
