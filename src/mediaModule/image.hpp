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

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include <vector>
#include <memory>


#include "tools/no_copy.hpp"
#include "mediaModule/media_base.hpp"
#include "vulkanModule/Context.hpp"

class s_texture;
class Navigator;
class Projector;
class VertexArray;
class ImageTexture;
class Pipeline;

class Image : public NoCopy {

public:

	Image() = delete;
	Image(const std::string& filename, const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project, bool mipmap);
	Image(VideoTexture imgTex, const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project);
	virtual ~Image();

	void setAlpha(float alpha, float duration);
	void setScale(float scale, float duration);
	void setRotation(float rotation, float duration);
	void setLocation(float xpos, bool deltax, float ypos, bool deltay, float duration, bool accelerate_x = false, bool decelerate_x = false, bool accelerate_y = false, bool decelerate_y = false);
	void setRatio(float ratio, float duration);
	void setPersistent(bool value) {
		isPersistent = value;
	}

	//! indique si on active la transparence sur la KeyColor
	void setTransparency(bool v) {
		transparency = v;
	}

	//! KeyColor a utiliser pour la transparence
	void setKeyColor(const Vec3f&color, float intensity) {
		noColor = Vec4f(color[0], color[1], color[2],intensity);
	}

	bool update(int delta_time);  // update properties
	void draw(const Navigator* nav, const Projector* prj);
	static void beginDraw();
	static void endDraw();

	const std::string getName() const {
		return image_name;
	};

	bool imageLoaded() {
		return (image_ratio != -1); // was texture loaded from disk?
	}

	bool imageIsPersistent() {
		return isPersistent;
	}

	static void createShaderUnified();
	static void createShaderImageViewport();
	static void createSC_context(ThreadContext *_context);

	// static std::unique_ptr<shaderProgram> shaderImageViewport;
	// static std::unique_ptr<shaderProgram> shaderUnified;

private:
	void createLocalContext();
	void setPipeline(Pipeline *pipeline);
	void drawViewport(const Navigator * nav, const Projector * prj);
	void drawUnified(bool drawUp, const Navigator * nav, const Projector * prj);
	void initialise(const std::string& name, IMG_POSITION pos_type, IMG_PROJECT project, bool mipmap = false);
	void initCache(const Projector * prj);

	ImageTexture* imageTexture = nullptr;
	//RGB
	// s_texture* image_RGB = nullptr;
	//ou
	// s_texture* image_Y = nullptr;
	// s_texture* image_U = nullptr;
	// s_texture* image_V = nullptr;
	//pour distinguer quelle texture ont doit utiliser:
	// bool useRGB;

	std::string image_name;
	IMG_POSITION image_pos_type;
	bool isPersistent= false;

	float image_scale, image_alpha, image_rotation;
	float image_ratio, image_xpos, image_ypos;

	bool flag_alpha, flag_scale, flag_rotation, flag_location, flag_progressive_x=0, flag_progressive_y=0, flag_ratio=0;
	float coef_alpha, coef_scale, coef_rotation, coef_ratio;
	float mult_alpha, mult_scale, mult_rotation;
	float start_alpha, start_scale, start_rotation, start_ratio;
	float end_alpha, end_scale, end_rotation, end_ratio;

	int mid_time_x, mid_time_y, my_timer, my_timer_ratio, end_time, end_time_ratio;

	float coef_location, mult_location, x_move, y_move;
	double coef_xmove, coef_ymove;
	float start_xpos, start_ypos, end_xpos, end_ypos;

	//OpenGL vars
	std::vector<float> vecImgTex, vecImgPos;
	static ThreadContext *context;
	static CommandMgr *cmdMgr;
	static PipelineLayout *m_layoutViewport, *m_layoutUnifiedRGB, *m_layoutUnifiedYUV;
	static Pipeline *m_pipelineViewport;
	// RGB, RBG with transparency, YUV, YUV with transparency
	static std::array<Pipeline *, 4> m_pipelineUnified;
	static VertexArray *m_imageViewportGL, *m_imageUnifiedGL;
	static int commandIndex;
	static Set *m_setViewport, *m_setUnifiedRGB, *m_setUnifiedYUV;
	static Pipeline *pipelineUsed;
	std::unique_ptr<VertexArray> vertex;
	int vertexSize;

	//active la transparence
	bool transparency;
	// indique quelle couleur est à effacer de l'image nocolor[3] indique le delta de couleur
	Vec4f noColor;

	//données utiles au cache
	bool initialised = false;
	int vieww, viewh;

	//combien de fois l'image est elle affichée sur le dôme ?
	int howManyDisplay = 1;

	float cx, cy, radius, prj_ratio;
	float xbase, ybase;

	Mat4d mat;
	Vec3d gridpt, onscreen;
	Vec3d imagev, ortho1, ortho2;
	int grid_size;
	bool needFlip = false;
};

#endif // _IMAGE_H
