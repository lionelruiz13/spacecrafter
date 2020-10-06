/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#ifndef _RING_H_
#define _RING_H_

#include <memory>

#include "tools/object_base.hpp"
#include "tools/utility.hpp"
#include "tools/s_font.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/vecmath.hpp"
#include "coreModule/callbacks.hpp"
#include "tools/fader.hpp"
#include "bodyModule/orbit.hpp"
#include "renderGL/stateGL.hpp"

#include "vulkanModule/Context.hpp"

class VertexArray;
class Pipeline;
class PipelineLayout;
class Set;
class Uniform;
class Buffer;
class CommandMgr;

class Ring2D {
public:
	Ring2D(float _r_min, float _r_max, GLint slices, GLint stacks, bool h, VertexArray &vertexBase);
	~Ring2D();
	void initFrom(VertexArray &vertex);
	void draw(void *pDrawData);

private:
	void computeRing(GLint slices, GLint stacks, bool h);
	std::vector<float> datas;
	std::unique_ptr<VertexArray> m_dataGL; //currentModel
	struct {
		uint32_t vertexCount;
    	uint32_t instanceCount;
    	uint32_t firstVertex;
    	uint32_t firstInstance;
	} drawData{0, 1, 0, 0};
	float r_min;
	float r_max;
};


// Class to manage rings for planets like saturn
class Ring {

public:
	Ring(double radius_min,double radius_max,const std::string &texname, const Vec3i &init, ThreadContext *context);
	~Ring(void);

	void draw(const Projector* prj,const Mat4d& mat,double screen_sz,Vec3f& lightDirection,Vec3f& planetPosition, float planetRadius);

	double getOuterRadius(void) const {
		return radius_max*mc;
	}

	double getInnerRadius(void) const {
		return radius_min*mc;
	}

	auto getTexTexture(void) const {
		return tex->getTexture();
	}

	void multiplyRadius(float f) {
		mc =f;
	}

private:
	const double radius_min;
	const double radius_max;
	const s_texture *tex;

	//std::unique_ptr<shaderProgram> shaderRing;	// Shader moderne
	void createSC_context(ThreadContext *context);

	bool needRecording = true;
	CommandMgr *cmdMgr;
	Set *globalSet;
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<VertexArray> vertex;
	std::unique_ptr<Set> set;
	std::unique_ptr<Uniform> uniform;
	std::unique_ptr<Buffer> drawData;
	struct {
		Mat4f ModelViewMatrix;
		Mat4f ModelViewMatrixInverse;
		Vec3f PlanetPosition;
		float PlanetRadius;
		Vec3f LightDirection;
		float SunnySideUp;
		Vec3f clipping_fov;
		float RingScale;
	} *pUniform;

	Ring2D* lowUP;
	Ring2D* lowDOWN;
	Ring2D* mediumUP;
	Ring2D* mediumDOWN;
	Ring2D* highUP;
	Ring2D* highDOWN;

	Vec3i init;
	float mc = 1.0;
};


#endif // _RING_H_
