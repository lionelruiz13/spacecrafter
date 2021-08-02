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

#include <iostream>
#include "coreModule/projector.hpp"
#include "mediaModule/image.hpp"
#include "mediaModule/imageTexture.hpp"
#include "navModule/navigator.hpp"
#include "tools/s_texture.hpp"
//
// #include "vulkanModule/VertexArray.hpp"
//
#include "vulkanModule/ResourceTracker.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/VertexArray.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/ThreadedCommandBuilder.hpp"

// std::unique_ptr<shaderProgram> Image::shaderImageViewport;
// std::unique_ptr<shaderProgram> Image::shaderUnified;
// std::unique_ptr<VertexArray> Image::m_imageUnifiedGL;
// std::unique_ptr<VertexArray> Image::m_imageViewportGL;
ThreadContext *Image::context;
PipelineLayout *Image::m_layoutViewport;
PipelineLayout *Image::m_layoutUnifiedRGB;
PipelineLayout *Image::m_layoutUnifiedYUV;
Pipeline *Image::m_pipelineViewport;
std::array<Pipeline *, 4> Image::m_pipelineUnified;
VertexArray *Image::m_imageViewportGL;
VertexArray *Image::m_imageUnifiedGL;
int Image::commandIndex = -1;
Set *Image::m_setViewport;
Set *Image::m_setUnifiedRGB;
Set *Image::m_setUnifiedYUV;
ThreadedCommandBuilder *Image::cmdMgr;
Pipeline *Image::pipelineUsed = nullptr;

Image::Image(const std::string& filename, const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project, bool mipmap)
{
	// load image using alpha channel in image, otherwise no transparency
	// other than through setAlpha method -- could allow alpha load option from command
	s_texture* imageRGB = new s_texture(filename, TEX_LOAD_TYPE_PNG_ALPHA, mipmap);
	imageTexture = new RBGImageTexture(imageRGB);
	initialise(name, pos_type,project, mipmap);
}

Image::Image(VideoTexture imgTex, const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project)
{
	s_texture* imageY = new s_texture(name+"_y", imgTex.y);
	s_texture* imageU = new s_texture(name+"_u", imgTex.u);
	s_texture* imageV = new s_texture(name+"_v", imgTex.v);
	imageTexture = new YUVImageTexture(imageY, imageU, imageV);
	needFlip = true;
	isPersistent = true;
	initialise(name, pos_type, project);
}

void Image::initialise(const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project, bool mipmap)
{
	flag_alpha = flag_scale = flag_location = flag_rotation = ratio.onTransition = 0;
	image_pos_type = pos_type;
	image_alpha = 0;  // begin not visible
	image_rotation = 0;
	image_xpos = image_ypos = 0; // centered is default
	image_scale = 1;
	image_name = name;

	switch (project) {
		case IMG_PROJECT::ONCE :
			howManyDisplay = 1;
			break;
		case IMG_PROJECT::TWICE :
			howManyDisplay = 2;
			break;
		case IMG_PROJECT::THRICE :
			howManyDisplay = 3;
			break;
	}

	int img_w, img_h;
	imageTexture->getDimensions(img_w, img_h);
	// if (useRGB)
	// 	image_RGB->getDimensions(img_w, img_h);
	// else
	// 	image_Y->getDimensions(img_w, img_h);
	if (img_h == 0)
		image_ratio = -1; // no image loaded
	else
		image_ratio = (float)img_w/img_h;
	if (pos_type == IMG_POSITION::POS_VIEWPORT) {
		vertex = std::make_unique<VertexArray>(*m_imageViewportGL);
		vertex->build(4);
	} else {
		vertex = std::make_unique<VertexArray>(*m_imageUnifiedGL);
		vertexSize = 0;
	}
}

void Image::initCache(const Projector * prj)
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
	}
	else {
		ybase = viewh/2;
		xbase = ybase*image_ratio;
	}
	initialised = true;
}


Image::~Image()
{
	// if (image_RGB) delete image_RGB;
	delete imageTexture;
	vecImgPos.clear();
	vecImgTex.clear();
}


void Image::createShaderImageViewport()
{
	// shaderImageViewport = std::make_unique<shaderProgram>();
	// shaderImageViewport->init("imageViewport.vert","imageViewport.frag");
	// shaderImageViewport->setUniformLocation({"fader", "MVP"});
}


void Image::createShaderUnified()
{
	// shaderUnified = std::make_unique<shaderProgram>();
	// shaderUnified->init("imageUnified.vert","imageUnified.frag");
	// shaderUnified->setUniformLocation({"fader","ModelViewMatrix", "noColor", "clipping_fov"});
	// shaderUnified->setUniformLocation("ModelViewMatrix");
	// // a cause des textures YUV
	// shaderUnified->setSubroutineLocation(GL_FRAGMENT_SHADER, "useRGB");
	// shaderUnified->setSubroutineLocation(GL_FRAGMENT_SHADER, "useYUV");
	// // a cause de la transparency
	// shaderUnified->setSubroutineLocation(GL_FRAGMENT_SHADER, "useTransparency");
	// shaderUnified->setSubroutineLocation(GL_FRAGMENT_SHADER, "useNoTransparency");
}


void Image::createSC_context(ThreadContext *_context)
{
	context = _context;
	// VertexArray
	m_imageUnifiedGL = context->global->tracker->track(new VertexArray(context->surface));
	m_imageUnifiedGL->registerVertexBuffer(BufferType::POS3D,BufferAccess::STREAM);
	m_imageUnifiedGL->registerVertexBuffer(BufferType::TEXTURE,BufferAccess::STREAM);
	m_imageViewportGL = context->global->tracker->track(new VertexArray(context->surface));
	m_imageViewportGL->registerVertexBuffer(BufferType::POS2D,BufferAccess::STREAM);
	m_imageViewportGL->registerVertexBuffer(BufferType::TEXTURE,BufferAccess::STREAM);
	// PipelineLayout
	m_layoutViewport = context->global->tracker->track(new PipelineLayout(context->surface));
	m_layoutViewport->setTextureLocation(0);
	m_layoutViewport->buildLayout(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	m_layoutViewport->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	m_layoutViewport->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 4);
	m_layoutViewport->build();
	m_layoutUnifiedRGB = context->global->tracker->track(new PipelineLayout(context->surface));
	m_layoutUnifiedRGB->setGlobalPipelineLayout(m_layoutViewport);
	m_layoutUnifiedRGB->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 76);
	m_layoutUnifiedRGB->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 76, 20);
	m_layoutUnifiedRGB->build();
	m_layoutUnifiedYUV = context->global->tracker->track(new PipelineLayout(context->surface));
	m_layoutUnifiedYUV->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedYUV->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedYUV->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedYUV->buildLayout(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	m_layoutUnifiedYUV->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 76);
	m_layoutUnifiedYUV->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 76, 20);
	m_layoutUnifiedYUV->build();
	// Pipeline
	m_pipelineViewport = context->global->tracker->track(new Pipeline(context->surface, m_layoutViewport));
	m_pipelineViewport->setDepthStencilMode();
	m_pipelineViewport->setRenderPassCompatibility(renderPassCompatibility::RESOLVE);
	m_pipelineViewport->bindVertex(m_imageViewportGL);
	m_pipelineViewport->bindShader("imageViewport.vert.spv");
	m_pipelineViewport->bindShader("imageViewport.frag.spv");
	m_pipelineViewport->build();
	for (int i = 0; i < 4; ++i) {
		m_pipelineUnified[i] = context->global->tracker->track(new Pipeline(context->surface, i < 2 ? m_layoutUnifiedRGB : m_layoutUnifiedYUV));
		m_pipelineUnified[i]->setDepthStencilMode();
		m_pipelineUnified[i]->setCullMode(true);
		m_pipelineUnified[i]->setRenderPassCompatibility(renderPassCompatibility::RESOLVE);
		m_pipelineUnified[i]->bindVertex(m_imageUnifiedGL);
		m_pipelineUnified[i]->bindShader("imageUnified.vert.spv");
	}
	m_pipelineUnified[0]->bindShader("imageUnifiedRGB.frag.spv");
	m_pipelineUnified[1]->bindShader("imageUnifiedRGBTransparency.frag.spv");
	m_pipelineUnified[2]->bindShader("imageUnifiedYUV.frag.spv");
	m_pipelineUnified[3]->bindShader("imageUnifiedYUVTransparency.frag.spv");
	for (int i = 0; i < 4; ++i)
		m_pipelineUnified[i]->build();
	// Set
	m_setViewport = context->global->tracker->track(new Set());
	m_setUnifiedRGB = m_setViewport;
	m_setUnifiedYUV = context->global->tracker->track(new Set());
	// CommandBuffer
	cmdMgr = context->commandMgrSingleUseInterface;
	commandIndex = cmdMgr->getCommandIndex();
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

	my_timer = 0;// count time elapsed from the beginning of the command

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
	end_time = int(duration * 1000.f); // movement duration in milliseconds
	if (flag_progressive_x) {
		if (accelerate_x and not decelerate_x) {
			mid_time_x = end_time; // switch from acceleration to deceleration at the end of the movement
			coef_xmove = x_move / float(end_time * end_time); // the movement must be completed at end_time²
		}
		else if (accelerate_x and decelerate_x) {
			mid_time_x = int(duration * 500.f + 0.5f); // switch from acceleration to deceleration at the middle of the movement
			coef_xmove = x_move / float(2 * mid_time_x * mid_time_x); // the movement must be the middle of the complete movement at mid_time_x²
		}
		else {   // (not accelerate_x and decelerate_x)
			mid_time_x = 0; // switch from acceleration to deceleration at the beginning of the movement
			coef_xmove = x_move / float(end_time * end_time); // the movement must be completed at end_time²
		}
	}
	if (flag_progressive_y) {
		if (accelerate_y and not decelerate_y) {
			mid_time_y = end_time; // switch from acceleration to deceleration at the end of the movement
			coef_ymove = y_move / float(end_time * end_time); // the movement must be completed at end_time²
		}
		else if (accelerate_y and decelerate_y) {
			mid_time_y = int(duration * 500.f + 0.5f); // switch from acceleration to deceleration at the middle of the movement
			coef_ymove = y_move / float(2 * mid_time_y * mid_time_y); // the movement must be the middle of the complete movement at mid_time_y²
		}
		else {   // (not accelerate_y and decelerate_y)
			mid_time_y = 0; // switch from acceleration to deceleration at the beginning of the movement
			coef_ymove = y_move / float(end_time * end_time); // the movement must be completed at end_time²
		}
	}
	x_move = x_move / (1000.f*duration);
	y_move = y_move / (1000.f*duration);
}


void Image::setRatio(float new_ratio, float duration)
{
	if (duration <= 0) {
		ratio.onTransition = 0;
		image_ratio = new_ratio;
		return;
	}
	ratio.onTransition = 1;
	ratio.duration = int(duration * 1000.f);
	ratio.start = image_ratio;
	ratio.end = new_ratio;
	ratio.coef = (new_ratio-ratio.start)/ratio.duration;
	ratio.timer = 0; // count time elapsed from the beginning of the command
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
			if (end_scale < start_scale) {

			}
		}

		// this transition is parabolic for better visual results
		if (start_scale > end_scale) {
			image_scale = start_scale + (1 - (1-mult_scale)*(1-mult_scale))*(end_scale-start_scale);
		}
		else image_scale = start_scale + mult_scale*mult_scale*(end_scale-start_scale);
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
		my_timer += delta_time; // update local timer
		if (flag_progressive_x) { // progressive movement in X-axis
			if (my_timer < mid_time_x) { // acceleration phase
				image_xpos = start_xpos + my_timer * my_timer * coef_xmove; // square function
			} else if (my_timer < end_time) { // deceleration phase
				image_xpos = end_xpos - (end_time - my_timer) * (end_time - my_timer) * coef_xmove; // (end - x)² function
			} else { // movement completed
				image_xpos = end_xpos;
				flag_location = 0;
			}
		} else { // linear movement in X-axis
			if (my_timer < end_time) {
				image_xpos = start_xpos + my_timer*x_move; // linear function
			} else {
				image_xpos = end_xpos;
				flag_location = 0;
			}
		}
		if (flag_progressive_y) { // progressive movement in Y-axis
			if (my_timer < mid_time_y) { // acceleration phase
				image_ypos = start_ypos + my_timer * my_timer * coef_ymove; // square function
			} else if (my_timer < end_time) { // deceleration phase
				image_ypos = end_ypos - (end_time - my_timer) * (end_time - my_timer) * coef_ymove; // (end - x)² function
			} else { // movement completed
				image_ypos = end_ypos;
				flag_location = 0;
			}
		} else { // linear movement in Y-axis
			if (my_timer < end_time) {
				image_ypos = start_ypos + my_timer*y_move; // linear function
			} else {
				image_ypos = end_ypos;
				flag_location = 0;
			}
		}
	}

	if (ratio.onTransition) {
		ratio.timer += delta_time; // update local timer
		if (ratio.timer < ratio.duration)
			image_ratio = ratio.start + ratio.timer*ratio.coef; // linear function
		else {
			image_ratio = ratio.end;
			ratio.onTransition = 0;
		}
	}
	return 1;
}


void Image::draw(const Navigator * nav, const Projector * prj)
{
	if (image_ratio < 0 || image_alpha == 0) return;

	initCache(prj);

	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//imageTexture->bindImageTexture(set);
	// if (useRGB) {
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture (GL_TEXTURE_2D, image_RGB->getID());
	// } else {
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture (GL_TEXTURE_2D, image_Y->getID());
	// glActiveTexture(GL_TEXTURE1);
	// glBindTexture (GL_TEXTURE_2D, image_U->getID());
	// glActiveTexture(GL_TEXTURE2);
	// glBindTexture (GL_TEXTURE_2D, image_V->getID());
	// }

	switch (image_pos_type) {
		case IMG_POSITION::POS_VIEWPORT:
			drawViewport(nav, prj);
			break;

		case IMG_POSITION::POS_HORIZONTAL:
			mat = nav->getLocalToEyeMat();
			drawUnified(false, nav, prj);
			break;

		case IMG_POSITION::POS_DOME:
			mat = nav->getDomeFixedMat();
			drawUnified(false, nav, prj);
			break;

		case IMG_POSITION::POS_J2000:
			mat = nav->getJ2000ToEyeMat();
			drawUnified(true, nav, prj);
			break;

		case IMG_POSITION::POS_EQUATORIAL:
			mat = nav->getEarthEquToEyeMat();
			drawUnified(true, nav, prj);
			break;

		default:
			return;
			break;
	}

	vecImgPos.clear();
	vecImgTex.clear();

	// StateGL::disable(GL_BLEND);
}

void Image::beginDraw()
{
	cmdMgr->init(commandIndex, false);
	cmdMgr->beginRenderPass(renderPassType::RESOLVE_DEFAULT, renderPassCompatibility::RESOLVE);
	pipelineUsed = nullptr;
}

void Image::endDraw()
{
	cmdMgr->compile();
	cmdMgr->setSubmission(commandIndex, true, context->commandMgr);
}

void Image::setPipeline(Pipeline *pipeline)
{
	if (pipelineUsed != pipeline) {
		pipelineUsed = pipeline;
		cmdMgr->bindPipeline(pipeline);
	}
}

void Image::drawViewport(const Navigator * nav, const Projector * prj)
{
	float w = image_scale*xbase;
	float h = image_scale*ybase;
	if (image_ratio<1) {
		w *= image_ratio;
	}
	else {
		h /= image_ratio;
	}
	setPipeline(m_pipelineViewport);
	m_setViewport->clear();
	imageTexture->bindImageTexture(m_setViewport);
	cmdMgr->pushSet(m_layoutViewport, m_setViewport);

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
		insert_all(vecImgTex,0,1,0,0,1,1,1,0);
	}
	else {
		insert_all(vecImgTex,0,0,0,1,1,0,1,1);
	}

	insert_all(vecImgPos, w, -h, -w, -h, w, h, -w, h);

	vertex->fillVertexBuffer(BufferType::POS2D,vecImgPos);
	vertex->fillVertexBuffer(BufferType::TEXTURE,vecImgTex);
	MVP = MVP * TRANSFO;
	cmdMgr->pushConstant(m_layoutViewport, VK_SHADER_STAGE_VERTEX_BIT, 0, &MVP, 64);
	cmdMgr->pushConstant(m_layoutViewport, VK_SHADER_STAGE_FRAGMENT_BIT, 64, &image_alpha, 4);
	cmdMgr->bindVertex(vertex.get());
	cmdMgr->draw(4);

	// shaderImageViewport->use();
	// shaderImageViewport->setUniform("fader", image_alpha);
	// shaderImageViewport->setUniform("MVP", MVP*TRANSFO);

	// Renderer::drawArrays(shaderImageViewport.get(), m_imageViewportGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, 0, 4);
}

static int decalages(int i, int howManyDisplay)
{
	// si pas de clone à afficher: direct 0
	//if (howManyDisplay==1) return 0;

	// on affiche d'abord l'original: direct 0
	//if (howManyDisplay==2 && i==0) return 0;
	if (howManyDisplay==2 && i==1) return 180;

	// on affiche d'abord l'original: direct 0
	//if (howManyDisplay==3 && i==0) return 0;
	if (howManyDisplay==3 && i==1) return 120;
	if (howManyDisplay==3 && i==2) return 180;
	//dans tous les autes cas
	return 0;
}

void Image::drawUnified(bool drawUp, const Navigator * nav, const Projector * prj)
{
	float plotDirection;
	struct {
		Mat4f matrix;
		Vec3f clipping_fov;
	} uVert;
	uVert.matrix=mat.convert();

	drawUp ? plotDirection = 1.0 : plotDirection = -1.0;

	uVert.clipping_fov = prj->getClippingFov();

	if (image_pos_type==IMG_POSITION::POS_DOME)
		uVert.clipping_fov[2] = 180;

	int index = transparency ? 1 : 0;
	PipelineLayout *layout;
	Set *set;
	if (imageTexture->isYUV()) {
		setPipeline(m_pipelineUnified[index | 2]);
		layout = m_layoutUnifiedYUV;
		set = m_setUnifiedYUV;
	} else {
		setPipeline(m_pipelineUnified[index]);
		layout = m_layoutUnifiedRGB;
		set = m_setUnifiedRGB;
	}
	set->clear();
	imageTexture->bindImageTexture(set);
	cmdMgr->pushSet(layout, set);
	cmdMgr->pushConstant(layout, VK_SHADER_STAGE_VERTEX_BIT, 0, &uVert, 76);
	if (transparency) {
		float tmpBuff[5];
		tmpBuff[0] = image_alpha;
		*reinterpret_cast<Vec4f *>(tmpBuff + 1) = noColor;
		cmdMgr->pushConstant(layout, VK_SHADER_STAGE_FRAGMENT_BIT, 76, tmpBuff, 20);
	} else
		cmdMgr->pushConstant(layout, VK_SHADER_STAGE_FRAGMENT_BIT, 76, &image_alpha, 4);

	for (int i=0; i<howManyDisplay; i++) {
		// altitude = xpos, azimuth = ypos (0 at North), image top towards zenith when rotation = 0
		imagev = Mat4d::zrotation(plotDirection*(image_ypos+decalages(i,howManyDisplay)-90)*M_PI/180.) * Mat4d::xrotation(image_xpos*M_PI/180.) * Vec3d(0,1,0);
		ortho1 = Mat4d::zrotation(plotDirection*(image_ypos+decalages(i,howManyDisplay)-90)*M_PI/180.) * Vec3d(1,0,0);
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
					}
					else {
						// image width is maximum angular dimension
						gridpt = Mat4d::rotation( imagev, (image_rotation+180)*M_PI/180.) *
						         Mat4d::rotation( ortho1, image_scale/image_ratio*(j-grid_size/2.)/(float)grid_size*M_PI/180.) *
						         Mat4d::rotation( ortho2, image_scale*(i+k-grid_size/2.)/(float)grid_size*M_PI/180.) *
						         imagev;
					}
					insert_vec3(vecImgData, gridpt);

					vecImgData.push_back((i+k)/(float)grid_size);

					// l'image video est inversée
					if (needFlip)
						vecImgData.push_back((grid_size-j)/(float)grid_size);
					else
						vecImgData.push_back(j/(float)grid_size);

				}
			}
		}
	}
	if (vertexSize != vecImgData.size() / 5) {
		vertexSize = vecImgData.size() / 5;
		vertex->build(vertexSize);
	}
	vertex->fillVertexBuffer(vecImgData);
	cmdMgr->bindVertex(vertex.get());
	int rowSize = (grid_size + 1) * 2;
	for (int i=0; i<grid_size * howManyDisplay; i++) {
		cmdMgr->draw(rowSize, 1, i * rowSize);
	}
	vecImgData.clear();
}
