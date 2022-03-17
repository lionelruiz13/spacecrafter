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
#include "tools/s_texture.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/vecmath.hpp"
#include "coreModule/callbacks.hpp"
#include "tools/fader.hpp"
#include "bodyModule/orbit.hpp"

#include "EntityCore/Resource/SharedBuffer.hpp"
#include "EntityCore/Resource/Texture.hpp"

class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class OjmL;
class Observer;

class Ring2D {
public:
	Ring2D(float _r_min, float _r_max, int slices, int stacks, bool h, VertexArray &vertexBase);
	~Ring2D();

	void draw(VkCommandBuffer &cmd);
private:
	void computeRing(int slices, int stacks, bool h);
	std::unique_ptr<VertexBuffer> m_dataGL; //currentModel
	float r_min;
	float r_max;
};


// Class to manage rings for planets like saturn
class Ring {

public:
	Ring(double radius_min,double radius_max,const std::string &texname, const Vec3i &init);
	~Ring(void);

	void draw(VkCommandBuffer &cmd, const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz,Vec3f& lightDirection,Vec3f& planetPosition, float planetRadius);

	double getOuterRadius(void) const {
		return radius_max*mc;
	}

	double getInnerRadius(void) const {
		return radius_min*mc;
	}

	auto &getTexTexture(void) const {
		return tex->getTexture();
	}

	void multiplyRadius(float f) {
		mc =f;
	}

	void preload();
private:
	const double radius_min;
	const double radius_max;
	std::unique_ptr<s_texture> tex;

	//std::unique_ptr<shaderProgram> shaderRing;	// Shader moderne
	void initialize();
	void createSC_context();
	void createAsteroidRing();

	std::unique_ptr<Pipeline> pipeline, pipelineAsteroid;
	std::unique_ptr<PipelineLayout> layout, layoutAsteroid;
	std::unique_ptr<VertexArray> vertex, vertexAsteroid;
	std::unique_ptr<VertexBuffer> instanceAsteroid;
	std::unique_ptr<OjmL> ojmlAsteroid;
	VertexBuffer *bufferAsteroid;
	SubBuffer indexAsteroid;
	std::unique_ptr<Set> set, setAsteroid;
	struct RingUniform {
		Mat4f ModelViewMatrix;
		Mat4f ModelViewMatrixInverse;
		Vec3f clipping_fov;
		float RingScale;
		Vec3f PlanetPosition;
		float PlanetRadius;
		Vec3f LightDirection;
		float SunnySideUp;
		float fadingFactor;
	};
	std::unique_ptr<SharedBuffer<RingUniform>> uniform;

	std::unique_ptr<Ring2D> lowUP;
	std::unique_ptr<Ring2D> lowDOWN;
	std::unique_ptr<Ring2D> mediumUP;
	std::unique_ptr<Ring2D> mediumDOWN;
	std::unique_ptr<Ring2D> highUP;
	std::unique_ptr<Ring2D> highDOWN;
	std::unique_ptr<BufferMgr> asyncStagingBuffer;
	std::thread threadAsteroid;

	Vec3i init;
	float mc = 1.0;
	bool initialized = false;
	bool fullyInitialized = false;
	bool asteroidComputed = false;
	bool asteroidReady = false;
};


#endif // _RING_H_
