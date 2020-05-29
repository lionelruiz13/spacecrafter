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

// manage an image for display from scripts

#include <iostream>
#include "coreModule/projector.hpp"
#include "mediaModule/image.hpp"
#include "navModule/navigator.hpp"
#include "tools/fmath.hpp"
#include "tools/stateGL.hpp"
#include "tools/s_texture.hpp"




shaderProgram* Image::shaderImageViewport=nullptr;
//~ shaderProgram* Image::shaderImageHorizontal=nullptr;
//~ shaderProgram* Image::shaderImageDome=nullptr;
//~ shaderProgram* Image::shaderImageEquatorial=nullptr;
shaderProgram* Image::shaderUnified=nullptr;

DataGL Image::sImage;

Image::Image(const Image* n, int i) {
		//cas n=n;
	if (this == n)
		return;
	this->image_tex = new s_texture(n->image_tex);
	this->image_name = n-> image_name;
	this->image_pos_type = n-> image_pos_type;
	this->isPersistent = n-> isPersistent;

	this->needFlip = n-> needFlip;

	//data
	this->image_alpha = n->image_alpha;
	this->image_scale = n-> image_scale;
	this->image_rotation = n-> image_rotation;

	//flag
	this->flag_alpha = n->flag_alpha;
	this->flag_scale = n->flag_scale;
	this->flag_rotation = n->flag_rotation;
	this->flag_location = n->flag_location;

	//coeff, mult
	this->mult_alpha = n->mult_alpha;
	this->mult_scale = n-> mult_scale;
	this->mult_rotation = n-> mult_rotation;
	this->coef_alpha = n->coef_alpha;
	this->coef_scale = n-> coef_scale;
	this->coef_rotation = n-> coef_rotation;

	//start, end
	this->start_alpha = n->start_alpha;
	this->start_scale = n-> start_scale;
	this->start_rotation = n-> start_rotation;
	this->end_alpha = n->end_alpha;
	this->end_scale = n-> end_scale;
	this->end_rotation = n-> end_rotation;

	//img param
	this->image_ratio = n-> image_ratio;
	this->image_xpos = n-> image_xpos;
	this->image_ypos = n-> image_ypos+i;

	//bool
	this->flag_alpha = n-> flag_alpha;
	this->flag_scale = n->flag_scale;
	this->flag_rotation = n-> flag_rotation;
	this->flag_location = n-> flag_location;

	if (this->image_ypos>360)
		this->image_ypos = this->image_ypos -360;
	//~ cout << "Clone pos " << this->image_xpos << " " <<this->image_ypos << " " <<  this->image_ratio << " " << this->image_rotation << " " << this->image_alpha << " " << endl;
}


Image::Image(const std::string& filename, const std::string& name, IMAGE_POSITIONING pos_type, bool mipmap)
{
	// load image using alpha channel in image, otherwise no transparency
	// other than through setAlpha method -- could allow alpha load option from command
	image_tex = new s_texture(filename, TEX_LOAD_TYPE_PNG_ALPHA, mipmap);
	initialise(name, pos_type,mipmap);
}

Image::Image(s_texture *_imgTex, const std::string& name, IMAGE_POSITIONING pos_type)
{
	image_tex = _imgTex;
	needFlip = true;
	isPersistent = true;
	initialise(name, pos_type);
}

void Image::initialise(const std::string& name, IMAGE_POSITIONING pos_type, bool mipmap)
{
	flag_alpha = flag_scale = flag_location = flag_rotation = 0;
	image_pos_type = pos_type;
	image_alpha = 0;  // begin not visible
	image_rotation = 0;
	image_xpos = image_ypos = 0; // centered is default
	image_scale = 1;
	image_name = name;

	int img_w, img_h;
	image_tex->getDimensions(img_w, img_h);

	if (img_h == 0) 
		image_ratio = -1; // no image loaded
	else
		image_ratio = (float)img_w/img_h;
}

void Image::initCache(Projector * prj)
{
	if (initialised)
		return;

	// données qui ne sont calculées qu'une fois
	vieww = prj->getViewportWidth();
	viewh = prj->getViewportHeight();

	Vec3d center = prj->getViewportCenter();
	cx = center[0];
	cy = center[1];
	radius = center[2];

	// If radius is set, then use that to determine viewport size
	// so that truncated fisheye works with viewport images as one would expect
	if (radius > 0) {
		vieww = viewh = radius * 2;
	}

	// calculations to keep image proportions when scale up to fit view
	prj_ratio = (float)vieww/viewh;

	if (image_ratio > prj_ratio) {
		xbase = vieww/2;
		ybase = xbase/image_ratio;
	} else {
		ybase = viewh/2;
		xbase = ybase*image_ratio;
	}
	initialised = true;
}

Image::~Image()
{
	if (image_tex) delete image_tex;

	vecImgPos.clear();
	vecImgTex.clear();

	// deleteShaderImageViewport();
	// deleteShaderUnified();
}


void Image::createShaderImageViewport()
{
	//VIEWPORT
	shaderImageViewport = new shaderProgram();
	shaderImageViewport->init("imageViewport.vert","imageViewport.frag");
	shaderImageViewport->setUniformLocation("fader");
	shaderImageViewport->setUniformLocation("MVP");

	glGenVertexArrays(1,&sImage.vao);
	glBindVertexArray(sImage.vao);

	glGenBuffers(1,&sImage.pos);
	glGenBuffers(1,&sImage.tex);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	//~ //HORIZONTAL
	//~ shaderImageHorizontal = new shaderProgram();
	//~ shaderImageHorizontal->init("imageHorizontal.vert", "imageHorizontal.frag");
	//~ shaderImageHorizontal->setUniformLocation("fader");
	//~ shaderImageHorizontal->setUniformLocation("MVP");

	//~ shaderImageHorizontal->setUniformLocation("ModelViewProjectionMatrix");
	//~ shaderImageHorizontal->setUniformLocation("inverseModelViewProjectionMatrix");
	//~ shaderImageHorizontal->setUniformLocation("ModelViewMatrix");

	//~ //DOME
	//~ shaderImageDome = new shaderProgram();
	//~ shaderImageDome->init("imageDome.vert", "imageDome.frag");
	//~ shaderImageDome->setUniformLocation("fader");
	//~ shaderImageDome->setUniformLocation("MVP");

	//~ shaderImageDome->setUniformLocation("ModelViewProjectionMatrix");
	//~ shaderImageDome->setUniformLocation("inverseModelViewProjectionMatrix");
	//~ shaderImageDome->setUniformLocation("ModelViewMatrix");

	//~ //EQUATORIAL
	//~ shaderImageEquatorial = new shaderProgram();
	//~ shaderImageEquatorial->init("imageEquatorial.vert", "imageEquatorial.frag");
	//~ shaderImageEquatorial->setUniformLocation("fader");
	//~ shaderImageEquatorial->setUniformLocation("MVP");

	//UNIFIED
}

void Image::createShaderUnified()
{
	shaderUnified = new shaderProgram();
	shaderUnified->init("imageUnified.vert","imageUnified.frag");
	shaderUnified->setUniformLocation("fader");
	shaderUnified->setUniformLocation("MVP");
	shaderUnified->setUniformLocation("transparency");
	shaderUnified->setUniformLocation("noColor");

	shaderUnified->setUniformLocation("ModelViewProjectionMatrix");
	shaderUnified->setUniformLocation("inverseModelViewProjectionMatrix");
	shaderUnified->setUniformLocation("ModelViewMatrix");

	shaderUnified->setSubroutineLocation(GL_VERTEX_SHADER,"custom_project");
	shaderUnified->setSubroutineLocation(GL_VERTEX_SHADER,"custom_project_fixed_fov");
}

void Image::deleteShaderUnified()
{
	if (shaderUnified) delete shaderUnified;
	shaderUnified = nullptr;
}

void Image::deleteShaderImageViewport()
{
	if (shaderImageViewport) delete shaderImageViewport;
	shaderImageViewport = nullptr;
	
	glDeleteBuffers(1,&sImage.tex);
	glDeleteBuffers(1,&sImage.pos);
	glDeleteVertexArrays(1,&sImage.vao);
}

void Image::setAlpha(float alpha, float duration)
{
	if (duration<=0) {
		image_alpha = alpha;
		flag_alpha = 0;
		return;
	}

	flag_alpha = 1;

	start_alpha = image_alpha;
	end_alpha = alpha;

	coef_alpha = 1.0f/(1000.f*duration);
	mult_alpha = 0;
}


void Image::setScale(float scale, float duration)
{
	if (duration<=0) {
		image_scale = scale;
		flag_scale = 0;
		return;
	}

	flag_scale = 1;

	start_scale = image_scale;
	end_scale = scale;

	coef_scale = 1.0f/(1000.f*duration);
	mult_scale = 0;
}

void Image::setRotation(float rotation, float duration)
{
	if (duration<=0) {
		image_rotation = rotation;
		flag_rotation = 0;
		return;
	}

	flag_rotation = 1;

	start_rotation = image_rotation;
	end_rotation = rotation;
	coef_rotation = 1.0f/(1000.f*duration);
	mult_rotation = 0;
}


void Image::setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x, bool decelerate_x, bool accelerate_y, bool decelerate_y)
{
	// xpos and ypos are interpreted when drawing based on image position type
	if (duration<=0) {
		if (deltax) image_xpos = xpos;
		if (deltay) image_ypos = ypos;
		flag_location = 0;
		return;
	}

	flag_location = 1;

	start_xpos = image_xpos;
	start_ypos = image_ypos;

	my_timer = 0;

	// only move if changing value
	if (deltax) end_xpos = xpos;
	else end_xpos = image_xpos;

	if (deltay) end_ypos = ypos;
	else end_ypos = image_ypos;
	
	// the new script begin here
	x_move = end_xpos - start_xpos;
	y_move = end_ypos - start_ypos;
	flag_progressive_x = accelerate_x or decelerate_x;
	flag_progressive_y = accelerate_y or decelerate_y;
	end_time = int(duration * 1000.f);
	if (flag_progressive_x) {
		if (accelerate_x and not decelerate_x) {
			mid_time_x = end_time;
			coef_xmove = x_move / float(end_time * end_time);
		} else if (accelerate_x and decelerate_x) {
			mid_time_x = int(duration * 500.f + 0.5f);
			coef_xmove = x_move / float(2 * mid_time_x * mid_time_x);
		} else {
			mid_time_x = 0;
			coef_xmove = x_move / float(end_time * end_time);
		}
	} if (flag_progressive_y) {
		if (accelerate_y and not decelerate_y) {
			mid_time_y = end_time;
			coef_ymove = y_move / float(end_time * end_time);
		} else if (accelerate_y and decelerate_y) {
			mid_time_y = int(duration * 500.f + 0.5f);
			coef_ymove = y_move / float(2 * mid_time_y * mid_time_y);
		} else {
			mid_time_y = 0;
			coef_ymove = y_move / float(end_time * end_time);
		}
	}
	x_move = x_move / (1000.f*duration);
	y_move = y_move / (1000.f*duration);
}

void Image::setRatio(float ratio, float duration)
{
	if (duration <= 0) {
		flag_ratio = 0;
		image_ratio = ratio;
		return;
	}
	flag_ratio = 1;
	end_time_ratio = int(duration * 1000.f);
	start_ratio = image_ratio;
	end_ratio = ratio;
	coef_ratio = (ratio-start_ratio)/end_time_ratio;
	my_timer_ratio = 0;
}

bool Image::update(int delta_time)
{
	if (image_ratio <= 0) return 0;
	
	if (flag_alpha) {
		mult_alpha += coef_alpha*delta_time;

		if ( mult_alpha >= 1) {
			mult_alpha = 1;
			flag_alpha = 0;
		}

		image_alpha = start_alpha + mult_alpha*(end_alpha-start_alpha);
	}
	
	if (flag_scale) {

		mult_scale += coef_scale*delta_time;

		if ( mult_scale >= 1) {
			mult_scale = 1;
			flag_scale = 0;
		}

		// this transition is parabolic for better visual results
		if (start_scale > end_scale) {
			image_scale = start_scale + (1 - (1-mult_scale)*(1-mult_scale))*(end_scale-start_scale);
		} else image_scale = start_scale + mult_scale*mult_scale*(end_scale-start_scale);
	}
	
	if (flag_rotation) {
		mult_rotation += coef_rotation*delta_time;

		if ( mult_rotation >= 1) {
			mult_rotation = 1;
			flag_rotation = 0;
		}

		image_rotation = start_rotation + mult_rotation*(end_rotation-start_rotation);
	}
	if (flag_location) {
		my_timer += delta_time;
		if (flag_progressive_x) {
			if (my_timer < mid_time_x) {
				image_xpos = start_xpos + my_timer * my_timer * coef_xmove;
			} else if (my_timer < end_time) {
				image_xpos = end_xpos - (end_time - my_timer) * (end_time - my_timer) * coef_xmove;
			} else {
				image_xpos = end_xpos;
				flag_location = 0;
			}
		} else {
			if (my_timer < end_time) {
				image_xpos = start_xpos + my_timer*x_move;
			} else {
				image_xpos = end_xpos;
				flag_location = 0;
			}
		}
		if (flag_progressive_y) {
			if (my_timer < mid_time_y) {
				image_ypos = start_ypos + my_timer * my_timer * coef_ymove;
			} else if (my_timer < end_time) {
				image_ypos = end_ypos - (end_time - my_timer) * (end_time - my_timer) * coef_ymove;
			} else {
				image_ypos = end_ypos;
				flag_location = 0;
			}
		} else {
			if (my_timer < end_time) {
				image_ypos = start_ypos + my_timer*y_move;
			} else {
				image_ypos = end_ypos;
				flag_location = 0;
			}
		}
	}
	
	if (flag_ratio) {
		my_timer_ratio += delta_time;
		if (my_timer_ratio < end_time_ratio)
			image_ratio = start_ratio + my_timer_ratio*coef_ratio;
		else {
			image_ratio = end_ratio;
			flag_ratio = 0;
		}
	}
	return 1;
}

void Image::draw(const Navigator * nav, Projector * prj)
{
	if (image_ratio < 0 || image_alpha == 0) return;
	//  printf("draw image %s alpha %f\n", image_name.c_str(), image_alpha);

	initCache(prj);

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, image_tex->getID());

	switch (image_pos_type) {
		case IMAGE_POSITIONING::POS_VIEWPORT:
			drawViewport(nav, prj);
			break;

		case IMAGE_POSITIONING::POS_HORIZONTAL:
			mat = nav->getLocalToEyeMat();
			//~ draw_horizontal(nav, prj);
			drawUnified(false, nav, prj);
			break;

		case IMAGE_POSITIONING::POS_DOME:
			mat = nav->getDomeFixedMat();
			//~ draw_dome(nav, prj);
			drawUnified(false, nav, prj);
			break;

		case IMAGE_POSITIONING::POS_J2000:
			mat = nav->getJ2000ToEyeMat();
			//~ draw_equatorial_J2000(nav, prj);
			drawUnified(true, nav, prj);
			break;

		case IMAGE_POSITIONING::POS_EQUATORIAL:
			mat = nav->getEarthEquToEyeMat();
			//~ draw_equatorial_J2000(nav, prj);
			drawUnified(true, nav, prj);
			break;

		default:
			return;
			break;
	}

	vecImgPos.clear();
	vecImgTex.clear();

	//~ glUseProgram(0);
	StateGL::disable(GL_BLEND);
}

void Image::drawViewport(const Navigator * nav, Projector * prj)
{
	float w = image_scale*xbase;
	float h = image_scale*ybase;
	if (image_ratio<1) {
		w *= image_ratio;
	} else {
		h /= image_ratio;
	}

	//	  cout << "drawing image viewport " << image_name << endl;
	// at x or y = 1, image is centered on projection edge centered in viewport at 0,0
	Mat4f MVP = prj->getMatProjectionOrtho2D();

	Mat4f TRANSFO= Mat4f::translation( Vec3f(cx, cy, 0) );
	//TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 1 * nav->getHeading()*M_PI/180. );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 0 *M_PI/180. );
	TRANSFO = TRANSFO*Mat4f::translation( Vec3f(image_xpos*vieww/2, image_ypos*viewh/2, 0) );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), (-image_rotation-90) *M_PI/180. );

	// l'image video est inversée
	if (needFlip) {
		vecImgTex.push_back(0);
		vecImgTex.push_back(1);
		vecImgTex.push_back(0);
		vecImgTex.push_back(0);
		vecImgTex.push_back(1);
		vecImgTex.push_back(1);
		vecImgTex.push_back(1);
		vecImgTex.push_back(0);
	} else {
		vecImgTex.push_back(0);
		vecImgTex.push_back(0);
		vecImgTex.push_back(0);
		vecImgTex.push_back(1);
		vecImgTex.push_back(1);
		vecImgTex.push_back(0);
		vecImgTex.push_back(1);
		vecImgTex.push_back(1);
	}

	vecImgPos.push_back(w);
	vecImgPos.push_back(-h);
	vecImgPos.push_back(-w);
	vecImgPos.push_back(-h);
	vecImgPos.push_back(w);
	vecImgPos.push_back(h);
	vecImgPos.push_back(-w);
	vecImgPos.push_back(h);

	glBindVertexArray(sImage.vao);

	glBindBuffer(GL_ARRAY_BUFFER,sImage.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgPos.size(),vecImgPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,sImage.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgTex.size(),vecImgTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	//UTILISATION DU SHADER
	shaderImageViewport->use();

	//SET UNIFORM
	shaderImageViewport->setUniform("fader", image_alpha);
	shaderImageViewport->setUniform("MVP", MVP*TRANSFO);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shaderImageViewport->unuse();
}

//~ void Image::draw_horizontal(const Navigator * nav, Projector * prj)
//~ {
	//~ mat = nav->getLocalToEyeMat();
	//~ Mat4f matrix=mat.convert();
	//~ Mat4f proj = prj->getMatProjection().convert();

	//~ // altitude = xpos, azimuth = ypos (0 at North), image top towards zenith when rotation = 0
	//~ imagev = Mat4d::zrotation(-1*(image_ypos-90)*M_PI/180.) * Mat4d::xrotation(image_xpos*M_PI/180.) * Vec3d(0,1,0);
	//~ ortho1 = Mat4d::zrotation(-1*(image_ypos-90)*M_PI/180.) * Vec3d(1,0,0);
	//~ ortho2 = imagev^ortho1;

	//~ grid_size = int(image_scale/5.);  // divisions per row, column
	//~ if (grid_size < 5) grid_size = 5;

	//~ for (int i=0; i<grid_size; i++) {

		//~ for (int j=0; j<=grid_size; j++) {

			//~ for (int k=0; k<=1; k++) {
				//~ if (image_ratio<1) {
					//~ // image height is maximum angular dimension
					//~ gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         //~ Mat4d::rotation( ortho1, image_scale*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ Mat4d::rotation( ortho2, image_scale*image_ratio*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ imagev;
				//~ } else {
					//~ // image width is maximum angular dimension
					//~ gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         //~ Mat4d::rotation( ortho1, image_scale/image_ratio*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ Mat4d::rotation( ortho2, image_scale*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ imagev;
				//~ }
				//~ vecImgTex.push_back((i+k)/(float)grid_size);
				
				//~ // l'image video est inversée
				//~ if (needFlip)
					//~ vecImgTex.push_back((grid_size-j)/(float)grid_size);
				//~ else
					//~ vecImgTex.push_back(j/(float)grid_size);

				//~ vecImgPos.push_back(gridpt[0]);
				//~ vecImgPos.push_back(gridpt[1]);
				//~ vecImgPos.push_back(gridpt[2]);
			//~ }
		//~ }
	//~ }

	//~ glBindVertexArray(sImage.vao);

	//~ shaderImageHorizontal->use();

	//~ shaderImageHorizontal->setUniform("ModelViewProjectionMatrix",proj*matrix);
	//~ shaderImageHorizontal->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	//~ shaderImageHorizontal->setUniform("ModelViewMatrix",matrix);
	//~ shaderImageHorizontal->setUniform("fader", image_alpha);
	//~ shaderImageHorizontal->setUniform("MVP", proj*matrix);

	//~ glBindBuffer(GL_ARRAY_BUFFER,sImage.pos);
	//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgPos.size(),vecImgPos.data(),GL_DYNAMIC_DRAW);
	//~ glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	//~ glBindBuffer(GL_ARRAY_BUFFER,sImage.tex);
	//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgTex.size(),vecImgTex.data(),GL_DYNAMIC_DRAW);
	//~ glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	//~ for(int i=0; i< grid_size; i++)
		//~ glDrawArrays(GL_TRIANGLE_STRIP, ((grid_size+1) * 2) *i, (grid_size+1) * 2 );

	//~ vecImgPos.clear();
	//~ vecImgTex.clear();
//~ }

//~ void Image::draw_dome(const Navigator * nav, Projector * prj)
//~ {
	//~ mat = nav->geTdomeMat();
	//~ Mat4f matrix=mat.convert();
	//~ Mat4f proj = prj->getMatProjection().convert();

	//~ // altitude = xpos, azimuth = ypos (0 at North), image top towards zenith when rotation = 0
	//~ imagev = Mat4d::zrotation(-1*(image_ypos-90)*M_PI/180.) * Mat4d::xrotation(image_xpos*M_PI/180.) * Vec3d(0,1,0);
	//~ ortho1 = Mat4d::zrotation(-1*(image_ypos-90)*M_PI/180.) * Vec3d(1,0,0);
	//~ ortho2 = imagev^ortho1;

	//~ grid_size = int(image_scale/5.);  // divisions per row, column
	//~ if (grid_size < 5) grid_size = 5;

	//~ for (int i=0; i<grid_size; i++) {

		//~ for (int j=0; j<=grid_size; j++) {
			//~ for (int k=0; k<=1; k++) {
				//~ // TODO: separate x, y scales?
				//~ if (image_ratio<1) {
					//~ // image height is maximum angular dimension
					//~ gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         //~ Mat4d::rotation( ortho1, image_scale*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ Mat4d::rotation( ortho2, image_scale*image_ratio*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ imagev;
				//~ } else {
					//~ // image width is maximum angular dimension
					//~ gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         //~ Mat4d::rotation( ortho1, image_scale/image_ratio*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ Mat4d::rotation( ortho2, image_scale*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ imagev;
				//~ }

				//~ vecImgTex.push_back((i+k)/(float)grid_size);
				//~ // l'image video est inversée
				//~ if (needFlip)
					//~ vecImgTex.push_back((grid_size-j)/(float)grid_size);
				//~ else
					//~ vecImgTex.push_back(j/(float)grid_size);

				//~ vecImgPos.push_back(gridpt[0]);
				//~ vecImgPos.push_back(gridpt[1]);
				//~ vecImgPos.push_back(gridpt[2]);
			//~ }
		//~ }
	//~ }

	//~ //UTILISATION DU SHADER
	//~ shaderImageDome->use();

	//~ //SET UNIFORM
	//~ shaderImageDome->setUniform("fader", image_alpha);
	//~ glBindVertexArray(sImage.vao);

	//~ shaderImageDome->setUniform("ModelViewProjectionMatrix",proj*matrix);
	//~ shaderImageDome->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	//~ shaderImageDome->setUniform("ModelViewMatrix",matrix);
	//~ shaderImageDome->setUniform("fader", image_alpha);
	//~ shaderImageDome->setUniform("MVP", proj*matrix);

		//~ glBindBuffer(GL_ARRAY_BUFFER,sImage.pos);
		//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgPos.size(),vecImgPos.data(),GL_DYNAMIC_DRAW);
		//~ glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

		//~ glBindBuffer(GL_ARRAY_BUFFER,sImage.tex);
		//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgTex.size(),vecImgTex.data(),GL_DYNAMIC_DRAW);
		//~ glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	//~ for(int i=0; i< grid_size; i++)
		//~ glDrawArrays(GL_TRIANGLE_STRIP, ((grid_size+1) * 2) *i, (grid_size+1) * 2 );

	//~ vecImgPos.clear();
	//~ vecImgTex.clear();
//~ }

//~ void Image::draw_equatorial_J2000(const Navigator * nav, Projector * prj)
//~ {
	//~ // equatorial is in current equatorial coordinates
	//~ // j2000 is in J2000 epoch equatorial coordinates (precessed)
	//~ // ypos is right ascension, xpos is declination
	//~ imagev = Mat4d::zrotation((image_ypos-90)*M_PI/180.) * Mat4d::xrotation((image_xpos)*M_PI/180.) * Vec3d(0,1,0);
	//~ ortho1 = Mat4d::zrotation(((image_ypos-90))*M_PI/180.) * Vec3d(1,0,0);
	//~ ortho2 = imagev^ortho1;

	//~ grid_size = int(image_scale/5.);  // divisions per row, column
	//~ if (grid_size < 5) grid_size = 5;

	//~ //UTILISATION DU SHADER
	//~ shaderImageEquatorial->use();

	//~ //SET UNIFORM
	//~ shaderImageEquatorial->setUniform("fader", image_alpha);

	//~ glBindVertexArray(sImage.vao);

	//~ for (int i=0; i<grid_size; i++) {
		//~ for (int j=0; j<=grid_size; j++) {
			//~ for (int k=0; k<=1; k++) {

				//~ if (image_ratio<1) {
					//~ // image height is maximum angular dimension
					//~ gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         //~ Mat4d::rotation( ortho1, image_scale*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ Mat4d::rotation( ortho2, image_scale/image_ratio*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ imagev;
				//~ } else {
					//~ // image width is maximum angular dimension
					//~ gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         //~ Mat4d::rotation( ortho1, image_scale/image_ratio*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ Mat4d::rotation( ortho2, image_scale*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         //~ imagev;
				//~ }

				//~ if ((image_pos_type == IMAGE_POSITIONING::POS_J2000 && prj->projectJ2000(gridpt, onscreen)) ||
				        //~ (image_pos_type == IMAGE_POSITIONING::POS_EQUATORIAL && prj->projectEarthEqu(gridpt, onscreen))) {

					//~ vecImgTex.push_back((i+k)/(float)grid_size);
					//~ // l'image video est inversée
					//~ if (needFlip)
						//~ vecImgTex.push_back((grid_size-j)/(float)grid_size);
					//~ else
						//~ vecImgTex.push_back(j/(float)grid_size);
					//~ //vecImgTex.push_back(j/(float)grid_size);

					//~ vecImgPos.push_back(onscreen[0]);
					//~ vecImgPos.push_back(onscreen[1]);
				//~ }
			//~ }
		//~ }

		//~ glBindBuffer(GL_ARRAY_BUFFER,sImage.pos);
		//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgPos.size(),vecImgPos.data(),GL_DYNAMIC_DRAW);
		//~ glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

		//~ glBindBuffer(GL_ARRAY_BUFFER,sImage.tex);
		//~ glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgTex.size(),vecImgTex.data(),GL_DYNAMIC_DRAW);
		//~ glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

		//~ glDrawArrays(GL_TRIANGLE_STRIP, 0, vecImgPos.size()/2 );

		//~ vecImgPos.clear();
		//~ vecImgTex.clear();
	//~ }
//~ }


void Image::drawUnified(bool drawUp, const Navigator * nav, Projector * prj)
{
	float plotDirection;
	Mat4f matrix=mat.convert();
	Mat4f proj = prj->getMatProjection().convert();

	if (drawUp)
		plotDirection = 1.0;
	else
		plotDirection = -1.0;

	// altitude = xpos, azimuth = ypos (0 at North), image top towards zenith when rotation = 0
	imagev = Mat4d::zrotation(plotDirection*(image_ypos-90)*M_PI/180.) * Mat4d::xrotation(image_xpos*M_PI/180.) * Vec3d(0,1,0);
	ortho1 = Mat4d::zrotation(plotDirection*(image_ypos-90)*M_PI/180.) * Vec3d(1,0,0);
	ortho2 = imagev^ortho1;

	grid_size = int(image_scale/5.);  // divisions per row, column
	if (grid_size < 5) grid_size = 5;

	for (int i=0; i<grid_size; i++) {

		for (int j=0; j<=grid_size; j++) {

			for (int k=0; k<=1; k++) {
				if (image_ratio<1) {
					// image height is maximum angular dimension
					gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         Mat4d::rotation( ortho1, image_scale*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         Mat4d::rotation( ortho2, image_scale*image_ratio*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         imagev;
				} else {
					// image width is maximum angular dimension
					gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
					         Mat4d::rotation( ortho1, image_scale/image_ratio*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         Mat4d::rotation( ortho2, image_scale*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
					         imagev;
				}
				vecImgTex.push_back((i+k)/(float)grid_size);

				// l'image video est inversée
				if (needFlip)
					vecImgTex.push_back((grid_size-j)/(float)grid_size);
				else
					vecImgTex.push_back(j/(float)grid_size);

				vecImgPos.push_back(gridpt[0]);
				vecImgPos.push_back(gridpt[1]);
				vecImgPos.push_back(gridpt[2]);
			}
		}
	}

	glBindVertexArray(sImage.vao);

	shaderUnified->use();

	shaderUnified->setUniform("ModelViewProjectionMatrix",proj*matrix);
	shaderUnified->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	shaderUnified->setUniform("ModelViewMatrix",matrix);
	shaderUnified->setUniform("fader", image_alpha);
	shaderUnified->setUniform("MVP", proj*matrix);
	shaderUnified->setUniform("transparency",transparency);
	shaderUnified->setUniform("noColor",noColor);

	if (image_pos_type==IMAGE_POSITIONING::POS_DOME)
		shaderUnified->setSubroutine(GL_VERTEX_SHADER,"custom_project_fixed_fov");
	else
		shaderUnified->setSubroutine(GL_VERTEX_SHADER,"custom_project");

	glBindBuffer(GL_ARRAY_BUFFER,sImage.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgPos.size(),vecImgPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,sImage.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vecImgTex.size(),vecImgTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	for(int i=0; i< grid_size; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, ((grid_size+1) * 2) *i, (grid_size+1) * 2 );

	shaderUnified->unuse();

	vecImgPos.clear();
	vecImgTex.clear();
}

