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
#include "renderGL/stateGL.hpp"
#include "coreModule/projector.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

#include "vulkanModule/TextureMgr.hpp"

std::unique_ptr<shaderProgram> s_font::shaderHorizontal;
std::unique_ptr<shaderProgram> s_font::shaderPrint;
std::unique_ptr<VertexArray> s_font::m_fontGL;

std::string s_font::baseFontName;

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
	//cout << "Created new font with size: " << fontSize << " and TTF name : " << fontName << endl;
}

s_font::~s_font()
{
	clearCache();
	TTF_CloseFont(myFont);
	myFont = nullptr;
}


void s_font::createSC_context()
{
	//HORIZONTAL
	shaderHorizontal = std::make_unique<shaderProgram>();
	shaderHorizontal->init("sfontHorizontal.vert","sfontHorizontal.frag");
	shaderHorizontal->setUniformLocation("Color");

	//PRINT
	shaderPrint = std::make_unique<shaderProgram>();
	shaderPrint->init("sfontPrint.vert","sfontPrint.frag");
	shaderPrint->setUniformLocation({"MVP","Color"});

	m_fontGL = std::make_unique<VertexArray>();
	m_fontGL->registerVertexBuffer(BufferType::POS2D, BufferAccess::DYNAMIC);
	m_fontGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::DYNAMIC);
}


//! print out a string
void s_font::print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP, int upsidedown)
{
	if (s.empty())
		return;

	renderedString_struct currentRender;
	// If not cached, create texture
	if(renderCache[s].textureW == 0 ) {
		currentRender = renderString(s, false);
		renderCache[s] = currentRender;
	} else {
		// read from cache
		currentRender = renderCache[s];
	}

	StateGL::enable(GL_BLEND);

	// Draw
	std::vector<float> vecPos;
	std::vector<float> vecTex;

	float h = currentRender.textureH;
	float w = currentRender.textureW;

	if(!upsidedown) {
		y -= currentRender.stringH;  // adjust for base of text in texture
		insert_all(vecPos, x, y, x, y+h, x+w, y, x+w, y+h);
		insert_all(vecTex, 0, 0, 0, 1, 1, 0, 1, 1);

	} else {
		y -= currentRender.stringH;  // adjust for base of text in texture
		insert_all(vecPos, x+w, y+h, x, y+h, x+w, y, x, y);
		insert_all(vecTex, 1, 0, 0, 0, 1, 1, 0, 1);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_2D, currentRender.stringTexture);
	// Avoid edge visibility
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_fontGL->fillVertexBuffer(BufferType::POS2D, vecPos);
	m_fontGL->fillVertexBuffer(BufferType::TEXTURE,vecTex);

	shaderPrint->use();
	shaderPrint->setUniform("MVP", MVP);
	shaderPrint->setUniform("Color", Color);
	Renderer::drawArrays(shaderPrint.get(), m_fontGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, 4);

	vecPos.clear();
	vecTex.clear();
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
		glDeleteTextures( 1, &renderCache[s].stringTexture);
		if (renderCache[s].haveBorder)
			glDeleteTextures( 1, &renderCache[s].borderTexture);
		renderCache.erase(s);
	}

}

//! remove ALL cached textures
void s_font::clearCache()
{
	for ( renderedStringHashIter_t iter = renderCache.begin(); iter != renderCache.end(); ++iter ) {
		if( (*iter).second.textureW != 0 ) {
			glDeleteTextures( 1, &((*iter).second.stringTexture));
			if ((*iter).second.haveBorder)
				glDeleteTextures( 1, &((*iter).second.borderTexture));
		}
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
	nothing.stringTexture = 0;
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
	GLenum texture_format;
	if (surface->format->Rmask == 0x000000ff)
		texture_format = GL_RGBA;
	else
		texture_format = GL_BGRA;

	glGenTextures( 1, &rendering.stringTexture);
	glBindTexture( GL_TEXTURE_2D, rendering.stringTexture);
    // disable mipmapping on the new texture
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, texture_format, (GLint)rendering.textureW, (GLint)rendering.textureH, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels );


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
			texture_format = GL_RGBA;
		else
			texture_format = GL_BGRA;

		glGenTextures( 1, &rendering.borderTexture);
		glBindTexture( GL_TEXTURE_2D, rendering.borderTexture);

		// disable mipmapping on the new texture
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, texture_format, (GLint)rendering.textureW, (GLint)rendering.textureH, 0, texture_format, GL_UNSIGNED_BYTE, border->pixels );
		rendering.haveBorder =true;
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
	if(renderCache[str].textureW == 0) {
		rendering = renderString(str, true);
		if(cache)
			renderCache[str] = rendering;
	} else {
		rendering = renderCache[str];
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

	int steps = 2+int(psi*15);

	std::vector<Vec2f> meshPoints;  // screen x,y
	std::vector<float> vecPos;
	std::vector<float> vecTex;

	// Pre-calculate points (more efficient)
	for (int i=0; i<=steps; i++) {
		float angle, p, q;
		angle = theta - i*psi/steps;
		p = sin(angle);
		q = cos(angle);

		meshPoints.push_back(Vec2f(center[0]+p*(d-rendering.textureH), center[1]+q*(d-rendering.textureH)));
		meshPoints.push_back(Vec2f(center[0]+p*d,center[1]+q*d));
	}

	Vec3f Color (texColor[0], texColor[1], texColor[2]);
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i=0; i<=steps; i++) {
		insert_vec2(vecPos,meshPoints[i*2]);
		insert_vec2(vecPos,meshPoints[i*2+1]);
		insert_all(vecTex, (float)i/steps, 0.f , (float)i/steps, 1.f);
	}

	shaderHorizontal->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rendering.stringTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rendering.borderTexture);

	shaderHorizontal->setUniform("Color", Color);
	m_fontGL->fillVertexBuffer(BufferType::POS2D, vecPos);
	m_fontGL->fillVertexBuffer(BufferType::TEXTURE,vecTex);

	Renderer::drawArrays(shaderHorizontal.get(), m_fontGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,vecPos.size()/2);

	vecPos.clear();
	vecTex.clear();

	if (!cache) {
		glDeleteTextures( 1, &rendering.stringTexture);
		glDeleteTextures( 1, &rendering.borderTexture);
	}
}
