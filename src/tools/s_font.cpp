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
#include "tools/s_font.hpp"
#include "tools/utility.hpp"
#include "tools/stateGL.hpp"
#include "tools/fmath.hpp"
#include "coreModule/projector.hpp"



shaderProgram* s_font::shaderHorizontal=nullptr;
shaderProgram* s_font::shaderPrint=nullptr;

DataGL s_font::sFont;

s_font::s_font(float size_i, const std::string& ttfFileName) //: lineHeightEstimate(0)
{
	myFont=nullptr;
	fontName = ttfFileName; // way to acces ttf file
	fontSize = size_i; // pixel height
	//to find error, i need to fix the way
	//printf("TTF_OpenFont file: %s  size: %f\n", fontName.c_str() , fontSize);
	myFont = TTF_OpenFont( fontName.c_str(), fontSize);
	if(!myFont) {
		printf("TTF_OpenFont error: %s\n", TTF_GetError());
		exit(-1);
	}
	//cout << "Created new font with size: " << fontSize << " and TTF name : " << fontName << endl;
}

s_font::~s_font()
{

	clearCache();
	TTF_CloseFont(myFont);

	while(!vecPos.empty()) {
		vecPos.pop_back();
	}

	while(!vecTex.empty()) {
		vecTex.pop_back();
	}
}


void s_font::createShader()
{
	//HORIZONTAL
	shaderHorizontal = new shaderProgram();
	shaderHorizontal->init("sfontHorizontal.vert","sfontHorizontal.frag");
	shaderHorizontal->setUniformLocation("Color");

	//PRINT
	shaderPrint = new shaderProgram();
	shaderPrint->init("sfontPrint.vert","sfontPrint.frag");
	shaderPrint->setUniformLocation("MVP");
	shaderPrint->setUniformLocation("Color");

	glGenVertexArrays(1,&sFont.vao);
	glBindVertexArray(sFont.vao);

	glGenBuffers(1,&sFont.tex);
	glGenBuffers(1,&sFont.pos);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void s_font::deleteShader()
{
	if(shaderHorizontal) delete shaderHorizontal;
	shaderHorizontal = nullptr;
	if(shaderPrint) delete shaderPrint;
	shaderPrint = nullptr;

	glDeleteBuffers(1,&sFont.tex);
	glDeleteBuffers(1,&sFont.pos);
	glDeleteVertexArrays(1, &sFont.vao);
}

//! print out a string
//! cache == 0 means do not cache rendered string texture
//! cache == -1 means do not actually draw, just cache
void s_font::print(float x, float y, const std::string& s, Vec4f Color, Mat4f MVP, int upsidedown, int cache)
{
	if(s == "") return;

	renderedString_struct currentRender;

	// If not cached, create texture
	if( !cache || renderCache[s].textureW == 0 ) {
		currentRender = renderString(s);
		if( cache ) {
			renderCache[s] = currentRender;
		}
	} else {
		// read from cache
		currentRender = renderCache[s];
	}

	if(cache==-1) return; // do not draw, just wanted to cache

	StateGL::enable(GL_BLEND);

	// Draw
	glActiveTexture(GL_TEXTURE0);
	glBindTexture( GL_TEXTURE_2D, currentRender.stringTexture);
	//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//~ glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// Avoid edge visibility
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shaderPrint->use();

	shaderPrint->setUniform("MVP", MVP);
	shaderPrint->setUniform("Color", Color);

	glBindVertexArray(sFont.vao);

	float h = currentRender.textureH;
	float w = currentRender.textureW;

	if(!upsidedown) {
		y -= currentRender.stringH;  // adjust for base of text in texture

		vecPos.push_back(x);
		vecPos.push_back(y);
		vecPos.push_back(x);
		vecPos.push_back(y+h);
		vecPos.push_back(x+w);
		vecPos.push_back(y);
		vecPos.push_back(x+w);
		vecPos.push_back(y+h);

		vecTex.push_back(0);
		vecTex.push_back(0);
		vecTex.push_back(0);
		vecTex.push_back(1);
		vecTex.push_back(1);
		vecTex.push_back(0);
		vecTex.push_back(1);
		vecTex.push_back(1);

	} else {
		y -= currentRender.stringH;  // adjust for base of text in texture

		vecPos.push_back(x+w);
		vecPos.push_back(y+h);
		vecPos.push_back(x);
		vecPos.push_back(y+h);
		vecPos.push_back(x+w);
		vecPos.push_back(y);
		vecPos.push_back(x);
		vecPos.push_back(y);

		vecTex.push_back(1);
		vecTex.push_back(0);
		vecTex.push_back(0);
		vecTex.push_back(0);
		vecTex.push_back(1);
		vecTex.push_back(1);
		vecTex.push_back(0);
		vecTex.push_back(1);
	}

	glBindBuffer(GL_ARRAY_BUFFER,sFont.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecPos.size(),vecPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,sFont.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecTex.size(),vecTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glDrawArrays(GL_TRIANGLE_STRIP, 0,4);

	vecPos.clear();
	vecTex.clear();

	shaderPrint->unuse();

	if(!cache) glDeleteTextures( 1, &currentRender.stringTexture);
}

float s_font::getStrLen(const std::string& s, bool cache)
{

	if(s == "") return 0;
	if(myFont==nullptr) fprintf(stderr,"myFont == NULL\n");

	if( renderCache[s].textureW != 0 ) return renderCache[s].stringW;

	Mat4f MVP;

	// otherwise calculate (and cache if desired)
	if(cache) {
		print(0, 0, s, Vec4f (1.0,1.0, 1.0, 1.0), MVP , 0, -1);
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
}


//! remove cached texture for string
void s_font::clearCache(const std::string& s)
{
	if( renderCache[s].textureW != 0 ) {
		glDeleteTextures( 1, &renderCache[s].stringTexture);
		renderCache.erase(s);
	}

}

//! remove ALL cached textures
void s_font::clearCache()
{
	for ( renderedStringHashIter_t iter = renderCache.begin(); iter != renderCache.end(); ++iter ) {
		if( (*iter).second.textureW != 0 ) {
			glDeleteTextures( 1, &((*iter).second.stringTexture));
			//cout << "Cleared cache for string: " << (*iter).first << endl;
		}
	}

	renderCache.clear();
}

//! Render a string to a texture
renderedString_struct s_font::renderString(const std::string &s) const
{

	renderedString_struct rendering;
	SDL_Color color;
	color.r=255;
	color.g=255;
	color.b=255;
	SDL_Surface *text = TTF_RenderUTF8_Blended(myFont, s.c_str(), color ); //write in white

	// Calculate opengl texture size required
	rendering.stringW = text->w;
	rendering.stringH = text->h;

	// opengl texture dimensions must be powers of 2
	//~ rendering.textureW = getNextPowerOf2((int)rendering.stringW);
	//~ rendering.textureH = getNextPowerOf2((int)rendering.stringH);
	rendering.textureW = text->w;
	rendering.textureH = text->h;

	//~ Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend on the endianness (byte order) of the machine */
	//~ #if SDL_BYTEORDER == SDL_BIG_ENDIAN
	//~ rmask = 0xff000000;
	//~ gmask = 0x00ff0000;
	//~ bmask = 0x0000ff00;
	//~ amask = 0x000000ff;
	//~ #else
	//~ rmask = 0x000000ff;
	//~ gmask = 0x0000ff00;
	//~ bmask = 0x00ff0000;
	//~ amask = 0xff000000;
	//~ #endif

	//~ SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)rendering.textureW, (int)rendering.textureH, 32, rmask, gmask, bmask, amask);
	//~ renderedString_struct nothing;
	//~ nothing.textureW = nothing.textureH = nothing.stringW = nothing.stringH = 0;
	//~ nothing.stringTexture = 0;
	//~ if(!surface) return nothing;

	//~ SDL_Rect tmp;
	//~ tmp.x=0;
	//~ tmp.y=0;
	//~ tmp.w=text->w;
	//~ tmp.h=text->h;
	//~ SDL_BlitSurface(text, &tmp, surface, &tmp);

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
	//~ GLenum texture_format;
	//~ GLint nOfColors = surface->format->BytesPerPixel;
	//~ if (nOfColors == 4) {     // contains an alpha channel
		//~ if (surface->format->Rmask == 0x000000ff)
			//~ texture_format = GL_RGBA;
		//~ else
			//~ texture_format = GL_BGRA;
	//~ } else if (nOfColors == 3) {     // no alpha channel
		//~ if (surface->format->Rmask == 0x000000ff)  // THIS IS WRONG for someplatforms
			//~ texture_format = GL_RGB;
		//~ else
			//~ texture_format = GL_BGR;
	//~ } else {
		//~ cerr << "Error: unable to convert surface to font texture.\n";
		//~ if(surface) SDL_FreeSurface(surface);
		//~ return nothing;
	//~ }

	glGenTextures( 1, &rendering.stringTexture);
	glBindTexture( GL_TEXTURE_2D, rendering.stringTexture);

    // disable mipmapping on the new texture
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, text->w, text->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, text->pixels );
	//~ glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, (GLint)rendering.textureW, (GLint)rendering.textureH, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels );

	//~ if(surface) SDL_FreeSurface(surface);
	if(text) SDL_FreeSurface(text);
	return rendering;
}


//! Draw text with baseline more or less parallel with horizon
//! justify: -1 left, 0 center, 1 right align (not impemented yet)
void s_font::printHorizontal(const Projector * prj, float altitude, float azimuth, const std::string& str, Vec3f& texColor, bool cache, bool outline)
{
	if(str == "") return;

	renderedString_struct rendering;

	Vec3d startV, screen;
	Utility::spheToRect(-azimuth*C_PI/180., altitude*C_PI/180., startV);
	prj->projectDomeFixed(startV, screen);
	float x = screen[0];
	float y = screen[1];

	// Get rendered texture
	if(renderCache[str].textureW == 0) {
		rendering = renderString(str);
		if(cache) renderCache[str] = rendering;
	} else {
		rendering = renderCache[str];
	}

	float textureExtentH = rendering.stringH/rendering.textureH;
	float textureExtentW = rendering.stringW/rendering.textureW;

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

	StateGL::enable(GL_BLEND);

	shaderHorizontal->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rendering.stringTexture);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float theta = C_PI + atan2f(dx, dy - 1);
	float psi = (float)getStrLen(str)/(d + 1);  // total angle of rotation

	int steps = int(psi*15);//TODO trouver le bon compromis pour la taille
	//cout << steps << endl;
	if(steps < 10) steps = 10;

	float angle, p, q;

	std::vector<Vec2f> meshPoints;  // screen x,y

	// Pre-calculate points (more efficient)
	for (int i=0; i<=steps; i++) {

		angle = theta - i*psi/steps;
		p = sin(angle);
		q = cos(angle);

		meshPoints.push_back(Vec2f(center[0]+p*(d-rendering.stringH), center[1]+q*(d-rendering.stringH)));
		meshPoints.push_back(Vec2f(center[0]+p*d,center[1]+q*d));
	}

	int shiftx = 0;
	int shifty = 0;

	Vec3f Color (texColor[0], texColor[1], texColor[2]);

	shaderHorizontal->setUniform("Color", Color);

	glBindVertexArray(sFont.vao);


	for (int pass=0; pass<outline*4+1; pass++) {

		if(outline) {
			if(pass < 4 ) {
				Color = v3fNull;
				shaderHorizontal->setUniform("Color", Color);
				if(pass<2) shiftx = -1;
				else shiftx = 1;
				if(pass%2) shifty = -1;
				else shifty = 1;
			} else {
				Color = Vec3f(texColor[0], texColor[1], texColor[2]);
				shaderHorizontal->setUniform("Color", Color);
				shiftx = shifty = 0;
			}
		}

		for (int i=0; i<=steps; i++) {

			vecPos.push_back(meshPoints[i*2][0]+shiftx);
			vecPos.push_back(meshPoints[i*2][1]+shifty);
			vecPos.push_back(meshPoints[i*2+1][0]+shiftx);
			vecPos.push_back(meshPoints[i*2+1][1]+shifty);

			vecTex.push_back((float)i/steps*textureExtentW);
			vecTex.push_back(0.0);
			vecTex.push_back((float)i/steps*textureExtentW);
			vecTex.push_back(textureExtentH);
		}


		glBindBuffer(GL_ARRAY_BUFFER,sFont.pos);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecPos.size(),vecPos.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

		glBindBuffer(GL_ARRAY_BUFFER,sFont.tex);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecTex.size(),vecTex.data(),GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, vecPos.size()/2);

		vecPos.clear();
		vecTex.clear();
	}

	shaderHorizontal->unuse();
	if(!cache) glDeleteTextures( 1, &rendering.stringTexture);
}
