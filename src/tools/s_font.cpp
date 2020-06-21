/*
 * Spacecrafter
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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
#include "tools/stateGL.hpp"
//#include "tools/fmath.hpp"
#include "coreModule/projector.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"

std::unique_ptr<shaderProgram> s_font::shaderHorizontal;
std::unique_ptr<shaderProgram> s_font::shaderPrint;
std::unique_ptr<VertexArray> s_font::m_fontGL;


s_font::s_font(float size_i, const std::string& ttfFileName) //: lineHeightEstimate(0)
{
	myFont=nullptr;
	fontName = ttfFileName; // way to acces ttf file
	fontSize = size_i; // pixel height
	//to find error, i need to fix the way
	//printf("TTF_OpenFont file: %s  size: %f\n", fontName.c_str() , fontSize);
	myFont = TTF_OpenFont( fontName.c_str(), fontSize);
	if(!myFont) {
		cLog::get()->write("s_font: TTF_OpenFont error: "+ std::string(TTF_GetError()), LOG_TYPE::L_ERROR);
		exit(-1);
	}
	//cout << "Created new font with size: " << fontSize << " and TTF name : " << fontName << endl;
}

s_font::~s_font()
{

	clearCache();
	TTF_CloseFont(myFont);

	// LOL !!!!!!!!!!!!!!!!!!!!!!
	// while(!vecPos.empty()) {
	// 	vecPos.pop_back();
	// }

	// while(!vecTex.empty()) {
	// 	vecTex.pop_back();
	// }
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
void s_font::print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP, int upsidedown/*, int cache*/)
{
	//bool cache = true;
	if(s == "") return;

	renderedString_struct currentRender;

	// If not cached, create texture
	if( /*!cache ||*/ renderCache[s].textureW == 0 ) {
		currentRender = renderString(s, false);
		//if( cache ) {
			renderCache[s] = currentRender;
		//}
	} else {
		// read from cache
		currentRender = renderCache[s];
	}

	//if(cache==-1) return; // do not draw, just wanted to cache

	StateGL::enable(GL_BLEND);
	
	// Draw
	std::vector<float> vecPos;
	std::vector<float> vecTex;

	float h = currentRender.textureH;
	float w = currentRender.textureW;

	if(!upsidedown) {
		y -= currentRender.stringH;  // adjust for base of text in texture

		// vecPos.push_back(x);
		// vecPos.push_back(y);
		// vecPos.push_back(x);
		// vecPos.push_back(y+h);
		// vecPos.push_back(x+w);
		// vecPos.push_back(y);
		// vecPos.push_back(x+w);
		// vecPos.push_back(y+h);
		insert_all(vecPos, x, y, x, y+h, x+w, y, x+w, y+h);

		// vecTex.push_back(0);
		// vecTex.push_back(0);
		// vecTex.push_back(0);
		// vecTex.push_back(1);
		// vecTex.push_back(1);
		// vecTex.push_back(0);
		// vecTex.push_back(1);
		// vecTex.push_back(1);
		insert_all(vecTex, 0, 0, 0, 1, 1, 0, 1, 1);

	} else {
		y -= currentRender.stringH;  // adjust for base of text in texture

		// vecPos.push_back(x+w);
		// vecPos.push_back(y+h);
		// vecPos.push_back(x);
		// vecPos.push_back(y+h);
		// vecPos.push_back(x+w);
		// vecPos.push_back(y);
		// vecPos.push_back(x);
		// vecPos.push_back(y);
		insert_all(vecPos, x+w, y+h, x, y+h, x+w, y, x, y);

		// vecTex.push_back(1);
		// vecTex.push_back(0);
		// vecTex.push_back(0);
		// vecTex.push_back(0);
		// vecTex.push_back(1);
		// vecTex.push_back(1);
		// vecTex.push_back(0);
		// vecTex.push_back(1);
		insert_all(vecTex, 1, 0, 0, 0, 1, 1, 0, 1);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_2D, currentRender.stringTexture);
	//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Avoid edge visibility
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_fontGL->fillVertexBuffer(BufferType::POS2D, vecPos);
	m_fontGL->fillVertexBuffer(BufferType::TEXTURE,vecTex);

	shaderPrint->use();
	shaderPrint->setUniform("MVP", MVP);
	shaderPrint->setUniform("Color", Color);

	m_fontGL->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0,4);
	m_fontGL->unBind();
	shaderPrint->unuse();

	vecPos.clear();
	vecTex.clear();
	//if(!cache) glDeleteTextures( 1, &currentRender.stringTexture);
}

float s_font::getStrLen(const std::string& s/*, bool cache*/)
{

	if(s == "") return 0;
	//if(myFont==nullptr) fprintf(stderr,"myFont == NULL\n");

	if( renderCache[s].textureW != 0 ) return renderCache[s].stringW;

	int w,h;
	if(TTF_SizeText(myFont,s.c_str(),&w,&h)) {
	//	printf("ERROR TTF_SizeText(myFont,s.c_str(),&w,&h)) ==%i %i\n",w,h);
		cLog::get()->write("s_font: TTF_SizeText error: "+ std::string(TTF_GetError()), LOG_TYPE::L_ERROR);
		return w;
	} else {// perhaps print the current TTF_GetError(), the string can't be rendered...
		//printf("ERROR : TTF_SizeText(myFont,s.c_str(),&w,&h)) ==%i %i but say ==0 \n",w,h);
		return w;
	}
	/*
	Mat4f MVP;

	// otherwise calculate (and cache if desired)
	if(cache) {
		print(0, 0, s, Vec4f (1.0,1.0, 1.0, 1.0), MVP , 0); //, -1);
		return renderCache[s].stringW;
	} else {
		int w,h;
		if(TTF_SizeText(myFont,s.c_str(),&w,&h)) {
			printf("ERROR TTF_SizeText(myFont,s.c_str(),&w,&h)) ==%i %i\n",w,h);
			return w;
		} else {// perhaps print the current TTF_GetError(), the string can't be rendered...
			//printf("ERROR : TTF_SizeText(myFont,s.c_str(),&w,&h)) ==%i %i but say ==0 \n",w,h);
			return w;
		}
	}
	*/
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
			//cout << "Cleared cache for string: " << (*iter).first << endl;
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

	// opengl texture dimensions must be powers of 2
	//~ rendering.textureW = getNextPowerOf2((int)rendering.stringW);
	//~ rendering.textureH = getNextPowerOf2((int)rendering.stringH);
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
		//cLog::get()->write("s_font: TTF_SizeText error: "+ std::string(SDL_GetError()), LOG_TYPE::L_ERROR);
		//if(text) 
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

    // disable mipmapping on the new texture
    //~ glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    //~ glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// disable mipmapping on default texture
	//~ glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//~ glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//~ // Avoid edge visibility
	//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	// get the number of channels in the SDL surface
	GLenum texture_format;
	if (surface->format->Rmask == 0x000000ff)
		texture_format = GL_RGBA;
	else
		texture_format = GL_BGRA;

	// get the number of channels in the SDL surface
	// GLenum texture_format = GL_RGBA;
	// GLint nOfColors = surface->format->BytesPerPixel;
	// if (nOfColors == 4) {     // contains an alpha channel
	// 	if (surface->format->Rmask == 0x000000ff)
	// 		texture_format = GL_RGBA;
	// 	else
	// 		texture_format = GL_BGRA;
	// } else if (nOfColors == 3) {     // no alpha channel
	// 	if (surface->format->Rmask == 0x000000ff)  // THIS IS WRONG for someplatforms
	// 		texture_format = GL_RGB;
	// 	else
	// 		texture_format = GL_BGR;
	// } else {
	// 	//cerr << "Error: unable to convert surface to font texture.\n";
	// 	if(surface) SDL_FreeSurface(surface);
	// 	return nothing;
	// }

	glGenTextures( 1, &rendering.stringTexture);
	glBindTexture( GL_TEXTURE_2D, rendering.stringTexture);

    // disable mipmapping on the new texture
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, text->w, text->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, text->pixels );
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
		//cLog::get()->write("s_font: TTF_SizeText error: "+ std::string(SDL_GetError()), LOG_TYPE::L_ERROR);
		//if(text) 
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
    //glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, text->w, text->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, text->pixels );
	glTexImage2D( GL_TEXTURE_2D, 0, texture_format, (GLint)rendering.textureW, (GLint)rendering.textureH, 0, texture_format, GL_UNSIGNED_BYTE, border->pixels );
	rendering.haveBorder =true;
	//if (border)
	SDL_FreeSurface(border);
	}

	//if(surface)
	SDL_FreeSurface(surface);
	//if(text)
	SDL_FreeSurface(text);
	return rendering;
}


//! Draw text with baseline more or less parallel with horizon
//! justify: -1 left, 0 center, 1 right align (not impemented yet)
void s_font::printHorizontal(const Projector * prj, float altitude, float azimuth, const std::string& str, Vec3f& texColor, bool cache) //, bool cache, bool outline)
{
	//int outline = 1;

	if(str == "") return;

	renderedString_struct rendering;

	Vec3d startV, screen;
	Utility::spheToRect(-azimuth*M_PI/180., altitude*M_PI/180., startV);
	prj->projectDomeFixed(startV, screen);
	float x = screen[0];
	float y = screen[1];

	// Get rendered texture
	if(renderCache[str].textureW == 0) {
		rendering = renderString(str, true);
		if(cache)
			renderCache[str] = rendering;
	} else {
		rendering = renderCache[str];
	}

	//float textureExtentH = 1.0; //rendering.stringH/rendering.textureH;
	//float textureExtentW = 1.0; //rendering.stringW/rendering.textureW;

	Vec3d center = prj->getViewportCenter();
	float radius = center[2];

	float dx = x - center[0];
	float dy = y - center[1];
	float d = sqrt(dx*dx + dy*dy);

	// If the text is too far away to be visible in the screen return
	if(radius > 0) {
		if (d > radius + rendering.stringH) return;
	} else {
		if(myMax(prj->getViewportWidth(), prj->getViewportHeight() ) > d) return;
	}

	float theta = M_PI + atan2f(dx, dy - 1);
	float psi = (float)getStrLen(str)/(d + 1);  // total angle of rotation

	int steps = 2+int(psi*15);//TODO trouver le bon compromis pour la taille
	//std::cout << str << " " << steps << std::endl;
	//if(steps < 10) steps = 10;

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

	// int shiftx = 0;
	// int shifty = 0;

	Vec3f Color (texColor[0], texColor[1], texColor[2]);

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// for (int pass=0; pass<0*4+1; pass++) {
		// if(1) {
			/*if(pass < 4 ) {
				Color = v3fNull;
				shaderHorizontal->setUniform("Color", Color);
				if(pass<2) shiftx = -1;
				else shiftx = 1;
				if(pass%2) shifty = -1;
				else shifty = 1;
			// } else {*/
			// 	Color = Vec3f(texColor[0], texColor[1], texColor[2]);
			// 	shaderHorizontal->setUniform("Color", Color);
				// shiftx = shifty = 0;
			//}
		// }

	for (int i=0; i<=steps; i++) {
		// vecPos.push_back(meshPoints[i*2][0]/*+shiftx*/);
		// vecPos.push_back(meshPoints[i*2][1]/*+shifty*/);
		// vecPos.push_back(meshPoints[i*2+1][0]/*+shiftx*/);
		// vecPos.push_back(meshPoints[i*2+1][1]/*+shifty*/);
		insert_vec2(vecPos,meshPoints[i*2]);
		insert_vec2(vecPos,meshPoints[i*2+1]);

		// vecTex.push_back((float)i/steps); // *textureExtentW);
		// vecTex.push_back(0.0);
		// vecTex.push_back((float)i/steps); // *textureExtentW);
		// vecTex.push_back(1.0); // 1.0 <- textureExtentH
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

	m_fontGL->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, vecPos.size()/2);
	m_fontGL->unBind();
	shaderHorizontal->unuse();

	vecPos.clear();
	vecTex.clear();

	if (!cache) {
		glDeleteTextures( 1, &rendering.stringTexture);
		glDeleteTextures( 1, &rendering.borderTexture);
	}
}
