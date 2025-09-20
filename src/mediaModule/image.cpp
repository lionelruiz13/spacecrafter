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
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/insert_all.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "ojmModule/objl.hpp"

PipelineLayout *Image::m_layoutUnifiedRGB;
PipelineLayout *Image::m_layoutUnifiedYUV;
PipelineLayout *Image::m_layoutSphereRGB;
PipelineLayout *Image::m_layoutSphereYUV;
std::array<Pipeline *, 4> Image::m_pipelineViewport;
std::array<Pipeline *, 4> Image::m_pipelineUnified;
std::array<Pipeline *, 4> Image::m_pipelineSphere;
std::unique_ptr<VertexArray> Image::m_imageViewportGL;
std::unique_ptr<VertexArray> Image::m_imageUnifiedGL;
std::unique_ptr<VertexArray> Image::m_imageSphereGL;
int Image::cmds[3];
VkCommandBuffer Image::cmd = VK_NULL_HANDLE;
Pipeline *Image::pipelineUsed = nullptr;

Image::Image(const std::string& filename, const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project, bool mipmap)
{
	// load image using alpha channel in image, otherwise no transparency
	// other than through setAlpha method -- could allow alpha load option from command
	s_texture* imageRGB = new s_texture(filename, TEX_LOAD_TYPE_PNG_ALPHA, mipmap);
	imageTexture = new RBGImageTexture(imageRGB, (pos_type == IMG_POSITION::POS_SPHERICAL) ? m_layoutSphereRGB : m_layoutUnifiedRGB);
	initialise(name, pos_type,project, mipmap);
}

Image::Image(VideoTexture imgTex, const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project)
{
	imageTexture = new YUVImageTexture(imgTex.y, imgTex.u, imgTex.v, (pos_type == IMG_POSITION::POS_SPHERICAL) ? m_layoutSphereYUV : m_layoutUnifiedYUV);
	imageTexture->setupSync(imgTex.sync);
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
	switch (pos_type) {
		case IMG_POSITION::POS_VIEWPORT:
			if (needFlip) {
				insert_all(vecImgTex,0,1,0,0,1,1,1,0);
			}
			else {
				insert_all(vecImgTex,0,0,0,1,1,0,1,1);
			}
			vertex = m_imageViewportGL->createBuffer(0, 4, Context::instance->tinyMgr.get());
			imgData = (float *) Context::instance->tinyMgr->getPtr(vertex->get());
			vertex->fillEntry(2, 4, vecImgTex.data(), imgData + 2);
			break;
		case IMG_POSITION::POS_SPHERICAL:
			break;
		default:
			vertexSize = 0;
	}
}

void Image::initCache(const Projector * prj)
{
	if (initialised)
		return;

	// data that is calculated only once
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

void Image::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	// VertexArray
	m_imageUnifiedGL = std::make_unique<VertexArray>(vkmgr);
	m_imageUnifiedGL->createBindingEntry(5 * sizeof(float));
	m_imageUnifiedGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // POS3D
	m_imageUnifiedGL->addInput(VK_FORMAT_R32G32_SFLOAT); // TEXTURE
	m_imageSphereGL = std::make_unique<VertexArray>(vkmgr, context.ojmAlignment);
	m_imageSphereGL->createBindingEntry(8 * sizeof(float));
	m_imageSphereGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // POS3D
	m_imageSphereGL->addInput(VK_FORMAT_R32G32_SFLOAT); // TEXTURE
	m_imageViewportGL = std::make_unique<VertexArray>(vkmgr);
	m_imageViewportGL->createBindingEntry(4 * sizeof(float));
	m_imageViewportGL->addInput(VK_FORMAT_R32G32_SFLOAT); // POS2D
	m_imageViewportGL->addInput(VK_FORMAT_R32G32_SFLOAT); // TEXTURE
	// PipelineLayout
	m_layoutUnifiedRGB = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(m_layoutUnifiedRGB);
	m_layoutUnifiedRGB->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedRGB->buildLayout();
	m_layoutUnifiedRGB->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 76);
	m_layoutUnifiedRGB->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 76, 20);
	m_layoutUnifiedRGB->build();
	m_layoutUnifiedYUV = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(m_layoutUnifiedYUV);
	m_layoutUnifiedYUV->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedYUV->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedYUV->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutUnifiedYUV->buildLayout();
	m_layoutUnifiedYUV->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 76);
	m_layoutUnifiedYUV->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 76, 20);
	m_layoutUnifiedYUV->build();
	auto tmpSampler = PipelineLayout::DEFAULT_SAMPLER;
	tmpSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	m_layoutSphereRGB = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(m_layoutSphereRGB);
	m_layoutSphereRGB->setTextureLocation(0, &tmpSampler);
	m_layoutSphereRGB->buildLayout();
	m_layoutSphereRGB->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 76);
	m_layoutSphereRGB->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 76, 20);
	m_layoutSphereRGB->build();
	m_layoutSphereYUV = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(m_layoutSphereYUV);
	m_layoutSphereYUV->setTextureLocation(0, &tmpSampler);
	m_layoutSphereYUV->setTextureLocation(1, &tmpSampler);
	m_layoutSphereYUV->setTextureLocation(2, &tmpSampler);
	m_layoutSphereYUV->buildLayout();
	m_layoutSphereYUV->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 76);
	m_layoutSphereYUV->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 76, 20);
	m_layoutSphereYUV->build();

	// Pipeline
	for (int i = 0; i < 4; ++i) {
		m_pipelineUnified[i] = new Pipeline(vkmgr, *context.render, PASS_FOREGROUND, i < 2 ? m_layoutUnifiedRGB : m_layoutUnifiedYUV);
		context.pipelines.emplace_back(m_pipelineUnified[i]);
		m_pipelineUnified[i]->setDepthStencilMode();
		m_pipelineUnified[i]->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
		m_pipelineUnified[i]->setCullMode(true);
		m_pipelineUnified[i]->bindVertex(*m_imageUnifiedGL);
		m_pipelineUnified[i]->bindShader("imageUnified.vert.spv");
		m_pipelineUnified[i]->setSpecializedConstant(7, context.isFloat64Supported);

		m_pipelineViewport[i] = new Pipeline(vkmgr, *context.render, PASS_FOREGROUND, i < 2 ? m_layoutUnifiedRGB : m_layoutUnifiedYUV);
		context.pipelines.emplace_back(m_pipelineViewport[i]);
		m_pipelineViewport[i]->setDepthStencilMode();
		m_pipelineViewport[i]->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
		m_pipelineViewport[i]->bindVertex(*m_imageViewportGL);
		m_pipelineViewport[i]->bindShader("imageViewport.vert.spv");
		m_pipelineViewport[i]->setSpecializedConstant(7, context.isFloat64Supported);

		m_pipelineSphere[i] = new Pipeline(vkmgr, *context.render, PASS_FOREGROUND, i < 2 ? m_layoutSphereRGB : m_layoutSphereYUV);
		context.pipelines.emplace_back(m_pipelineSphere[i]);
		m_pipelineSphere[i]->setDepthStencilMode();
		m_pipelineSphere[i]->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		m_pipelineSphere[i]->setCullMode(true);
		m_pipelineSphere[i]->setFrontFace();
		m_pipelineSphere[i]->bindVertex(*m_imageSphereGL);
		m_pipelineSphere[i]->bindShader("imageUnified.vert.spv");
		m_pipelineSphere[i]->setSpecializedConstant(7, context.isFloat64Supported);
	}
	m_pipelineUnified[0]->bindShader("imageUnifiedRGB.frag.spv");
	m_pipelineUnified[1]->bindShader("imageUnifiedRGBTransparency.frag.spv");
	m_pipelineUnified[2]->bindShader("imageUnifiedYUV.frag.spv");
	m_pipelineUnified[3]->bindShader("imageUnifiedYUVTransparency.frag.spv");
	m_pipelineSphere[0]->bindShader("imageUnifiedRGB.frag.spv");
	m_pipelineSphere[1]->bindShader("imageUnifiedRGBTransparency.frag.spv");
	m_pipelineSphere[2]->bindShader("imageUnifiedYUV.frag.spv");
	m_pipelineSphere[3]->bindShader("imageUnifiedYUVTransparency.frag.spv");
	m_pipelineViewport[0]->bindShader("imageUnifiedRGB.frag.spv");
	m_pipelineViewport[1]->bindShader("imageUnifiedRGBTransparency.frag.spv");
	m_pipelineViewport[2]->bindShader("imageUnifiedYUV.frag.spv");
	m_pipelineViewport[3]->bindShader("imageUnifiedYUVTransparency.frag.spv");
	for (int i = 0; i < 4; ++i) {
		m_pipelineUnified[i]->build();
		m_pipelineSphere[i]->build();
		m_pipelineViewport[i]->build();
	}
	// CommandBuffer
	for (int i = 0; i < 3; ++i) {
		cmds[i] = context.frame[i]->create(1);
		context.frame[i]->setName(cmds[i], "Image " + std::to_string(i));
	}
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

		case IMG_POSITION::POS_SPHERICAL:
			drawSpherical(nav, prj);
			break;

		default:
			return;
			break;
	}

	vecImgPos.clear();
}

void Image::beginDraw()
{
	cmd = Context::instance->frame[Context::instance->frameIdx]->begin(cmds[Context::instance->frameIdx], PASS_FOREGROUND);
	pipelineUsed = nullptr;
}

void Image::endDraw()
{
	Context::instance->frame[Context::instance->frameIdx]->compile(cmd);
	Context::instance->frame[Context::instance->frameIdx]->toExecute(cmd, PASS_FOREGROUND);
	cmd = VK_NULL_HANDLE;
}

void Image::setPipeline(Pipeline *pipeline)
{
	if (pipelineUsed != pipeline) {
		pipelineUsed = pipeline;
		pipeline->bind(cmd);
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
	PipelineLayout *layout;
	if (imageTexture->isYUV()) {
		setPipeline(m_pipelineViewport[transparency ? 3 : 2]);
		layout = m_layoutUnifiedYUV;
	} else {
		setPipeline(m_pipelineViewport[transparency ? 1 : 0]);
		layout = m_layoutUnifiedRGB;
	}
	imageTexture->bindSet(cmd, layout);

	//	  cout << "drawing image viewport " << image_name << endl;
	// at x or y = 1, image is centered on projection edge centered in viewport at 0,0
	Mat4f MVP = prj->getMatProjectionOrtho2D();

	Mat4f TRANSFO= Mat4f::translation( Vec3f(cx, cy, 0) );
	//TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 1 * nav->getHeading()*M_PI/180. );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 0 *M_PI/180. );
	TRANSFO = TRANSFO*Mat4f::translation( Vec3f(image_xpos*vieww/2, image_ypos*viewh/2, 0) );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), (-image_rotation-90) *M_PI/180. );

	insert_all(vecImgPos, w, -h, -w, -h, w, h, -w, h);
	vertex->fillEntry(2, 4, vecImgPos.data(), imgData);
	MVP = MVP * TRANSFO;
	layout->pushConstant(cmd, 0, &MVP);
	if (transparency) {
		float tmpBuff[5];
		tmpBuff[0] = image_alpha;
		*reinterpret_cast<Vec4f *>(tmpBuff + 1) = noColor;
		layout->pushConstant(cmd, 1, &tmpBuff, 0, 20);
	} else
		layout->pushConstant(cmd, 1, &image_alpha, 0, 4);
	vertex->bind(cmd);
	vkCmdDraw(cmd, 4, 1, 0, 0);
	// imageTexture->unbindSet(cmd);
}

void Image::drawSpherical(const Navigator *nav, const Projector *prj)
{
	PipelineLayout *layout;
	if (imageTexture->isYUV()) {
		setPipeline(m_pipelineSphere[transparency ? 3 : 2]);
		layout = m_layoutSphereYUV;
	} else {
		setPipeline(m_pipelineSphere[transparency ? 1 : 0]);
		layout = m_layoutSphereRGB;
	}
	imageTexture->bindSet(cmd, layout);
	struct {
		Mat4f matrix;
		Vec3f clipping_fov;
	} uVert;
	uVert.matrix = nav->getLocalToEyeMat().convert();
	uVert.clipping_fov = prj->getClippingFov();
	layout->pushConstant(cmd, 0, &uVert);
	if (transparency) {
		float tmpBuff[5];
		tmpBuff[0] = image_alpha;
		*reinterpret_cast<Vec4f *>(tmpBuff + 1) = noColor;
		layout->pushConstant(cmd, 1, &tmpBuff, 0, 20);
	} else
		layout->pushConstant(cmd, 1, &image_alpha, 0, 4);
	auto objl = ObjLMgr::instance->selectDefault();
	objl->bind(cmd);
	objl->draw(cmd, 1024);
}

static int decalages(int i, int howManyDisplay)
{
	// if no clone to display: direct 0
	//if (howManyDisplay==1) return 0;

	// display the original first: direct 0
	//if (howManyDisplay==2 && i==0) return 0;
	if (howManyDisplay==2 && i==1) return 180;

	// display the original first: direct 0
	//if (howManyDisplay==3 && i==0) return 0;
	if (howManyDisplay==3 && i==1) return 120;
	if (howManyDisplay==3 && i==2) return 240;
	//in all other cases
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
		uVert.clipping_fov[2] = M_PI_2;

	PipelineLayout *layout;
	if (imageTexture->isYUV()) {
		setPipeline(m_pipelineUnified[transparency ? 3 : 2]);
		layout = m_layoutUnifiedYUV;
	} else {
		setPipeline(m_pipelineUnified[transparency ? 1 : 0]);
		layout = m_layoutUnifiedRGB;
	}
	imageTexture->bindSet(cmd, layout);
	layout->pushConstant(cmd, 0, &uVert);
	if (transparency) {
		float tmpBuff[5];
		tmpBuff[0] = image_alpha;
		*reinterpret_cast<Vec4f *>(tmpBuff + 1) = noColor;
		layout->pushConstant(cmd, 1, &tmpBuff, 0, 20);
	} else
		layout->pushConstant(cmd, 1, &image_alpha, 0, 4);

	Context &context = *Context::instance;
	imgData = (float *) context.transfer->beginPlanCopy(vertexSize * 5 * sizeof(float));
	uint32_t currentSize = 0;
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
					*(imgData++) = gridpt[0];
					*(imgData++) = gridpt[1];
					*(imgData++) = gridpt[2];
					*(imgData++) = (i+k)/(float)grid_size;

					// the video image is inverted
					if (needFlip)
						*(imgData++) = (grid_size-j)/(float)grid_size;
					else
						*(imgData++) = j/(float)grid_size;
					++currentSize;
				}
			}
		}
	}
	if (vertexSize != currentSize) {
		vertexSize = currentSize;
		vertex.reset();
		vertex = m_imageUnifiedGL->createBuffer(0, currentSize, Context::instance->globalBuffer.get());
	}
	context.transfer->endPlanCopy(vertex->get(), currentSize * 5 * sizeof(float));
	vertex->bind(cmd);
	int rowSize = (grid_size + 1) * 2;
	for (int i=0; i<grid_size * howManyDisplay; i++) {
		vkCmdDraw(cmd, rowSize, 1, i * rowSize, 0);
	}
}
