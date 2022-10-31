/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009-2010 Digitalis Education Solutions, Inc.
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

#include <iostream>
#include <iomanip>
#include <random>

#include "bodyModule/ring.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "navModule/observer.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "../planetsephems/sideral_time.h"
#include "ojmModule/ojml.hpp"

#include "tools/context.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"

#ifdef WIN32
#include <malloc.h>
#ifndef alloca
#define alloca _alloca
#endif
#else
#include <alloca.h>
#endif

#define NB_ASTEROIDS 400000

double Ring::fadingFactor = 40;

Ring::Ring(double radius_min,double radius_max,const std::string &texname, const Vec3i &_init)
	:radius_min(radius_min),radius_max(radius_max)
{
	init = _init;
	tex = std::make_unique<s_texture>(texname, TEX_LOAD_TYPE_PNG_ALPHA, true);
}

void Ring::initialize()
{
	if (initialized) {
		if (asteroidComputed) {
			if (threadAsteroid.joinable()) {
				threadAsteroid.join();
				auto staging = asyncStagingBuffer->fastAcquireBuffer(instanceAsteroid->get().size);
				Context::instance->transfer->planCopyBetween(staging, instanceAsteroid->get());
				asteroidReady = true;
			} else {
				// We have already joined the threadAsteroid, clear ressources
				asyncStagingBuffer.reset();
				fullyInitialized = true;
			}
		}
		return;
	}
	// tex->use(); This no longer exist
	createSC_context();

	if (bufferAsteroid)
		threadAsteroid = std::thread(&Ring::createAsteroidRing, this);

	lowUP = std::make_unique<Ring2D>((float) radius_min, (float) radius_max, init[0], 4, true, *vertex);
	lowDOWN = std::make_unique<Ring2D>((float) radius_min, (float) radius_max, init[0], 4, false, *vertex);

	mediumUP = std::make_unique<Ring2D>((float) radius_min, (float) radius_max, init[1], 8, true, *vertex);
	mediumDOWN = std::make_unique<Ring2D>((float) radius_min, (float) radius_max, init[1], 8, false, *vertex);

	highUP = std::make_unique<Ring2D>((float) radius_min, (float) radius_max, init[2], 16, true, *vertex);
	highDOWN = std::make_unique<Ring2D>((float) radius_min, (float) radius_max, init[2], 16, false, *vertex);

	initialized = true;
	fullyInitialized = !bufferAsteroid;
}

void Ring::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	layout->buildLayout();
	layout->build();

	vertex = std::make_unique<VertexArray>(vkmgr);
	vertex->createBindingEntry(3 * sizeof(float));
	vertex->addInput(VK_FORMAT_R32G32_SFLOAT);
	vertex->addInput(VK_FORMAT_R32_SFLOAT);

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setCullMode(true);
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	pipeline->bindVertex(*vertex);
	pipeline->bindShader("ring_planet.vert.spv");
	pipeline->bindShader("ring_planet.frag.spv");
	pipeline->build();

	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get(), -1, false, true);
	uniform = std::make_unique<SharedBuffer<RingUniform>>(*context.uniformMgr);
	set->bindUniform(uniform, 0);
	set->bindTexture(tex->getTexture(), 1);

	ojmlAsteroid = std::make_unique<OjmL>(AppSettings::Instance()->getModel3DDir()+"sat_ice.ojm");
	bufferAsteroid = ojmlAsteroid->getVertexBuffer();
	indexAsteroid = ojmlAsteroid->getIndexBuffer();
	if (bufferAsteroid == nullptr) {
		cLog::get()->write("Failed to load ojml for ring asteroids", LOG_TYPE::L_ERROR);
		return;
	}
	vertexAsteroid = std::make_unique<VertexArray>(vkmgr, context.ojmAlignment);
	vertexAsteroid->createBindingEntry(8*sizeof(float));
	vertexAsteroid->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexAsteroid->createBindingEntry(6*sizeof(float), VK_VERTEX_INPUT_RATE_INSTANCE);
	vertexAsteroid->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexAsteroid->addInput(VK_FORMAT_R32G32B32_SFLOAT);

	layoutAsteroid = std::make_unique<PipelineLayout>(vkmgr);
	layoutAsteroid->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layoutAsteroid->buildLayout();
	layoutAsteroid->build();

	float asteroid_radius = (radius_max - radius_min) / 500.f;
	pipelineAsteroid = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layoutAsteroid.get());
	//pipelineAsteroid->setCullMode(true);
	pipelineAsteroid->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineAsteroid->bindVertex(*vertexAsteroid);
	pipelineAsteroid->bindShader("ring_test.vert.spv");
	pipelineAsteroid->setSpecializedConstant(0, &asteroid_radius, sizeof(asteroid_radius));
	pipelineAsteroid->bindShader("ring_test.frag.spv");
	pipelineAsteroid->build();

	setAsteroid = std::make_unique<Set>(vkmgr, *context.setMgr, layoutAsteroid.get(), -1, true, true);
	setAsteroid->bindUniform(uniform, 0);
}

void Ring::createAsteroidRing()
{
	Context &context = *Context::instance;
	const float asteroid_radius = (radius_max - radius_min) / 500.f;

	int width, height;
	tex->getDimensions(width, height);
    bool nonPersistant = false;
	uint8_t *pData = (uint8_t *) tex->acquireContent(nonPersistant);
	uint8_t *pDataLoop = pData + 3; // Use alpha from R G B A

	std::vector<float> probability;
	probability.reserve(width + 1);
	float sum_probability = 0;
	probability.push_back(sum_probability);
	for (int i = 0; i < width; ++i) {
		float alpha = *pDataLoop / 255.f;
		sum_probability += std::max(alpha*alpha - 0.01f, 0.f);
		// In the texture, 0.15 correspond to no asteroids
		probability.push_back(sum_probability);
		std::cout << "Color";
		for (int i = -3; i < 1; ++i)
			std::cout << ' ' << (int) pDataLoop[i];
		std::cout << "\n";
		pDataLoop += 4;
	}
	std::cout << "\e[94mRing density : " << sum_probability << "\e[0m\n";
	// Just hope nobody is currently creating/freeing a buffer from the ojmBufferMgr...
	instanceAsteroid = vertexAsteroid->createBuffer(1, NB_ASTEROIDS, nullptr, context.ojmBufferMgr.get());
	// This is not possible, but... Anyway...
	asyncStagingBuffer = std::make_unique<BufferMgr>(*VulkanMgr::instance, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, instanceAsteroid->get().size, "ring buffer");
	Vec3f *tmp = static_cast<Vec3f *>(asyncStagingBuffer->getPtr());
	std::default_random_engine generator;
	auto distance_distribution = std::uniform_real_distribution<float>(0.f, sum_probability);
	auto z_shift_distribution = std::uniform_real_distribution<float>(-0.8f*asteroid_radius, 0.8f*asteroid_radius);
	for (int i = 0; i < NB_ASTEROIDS; ++i) {
		float distance = distance_distribution(generator);
		int j;
		for (j = 0; j < width; ++j) {
			if (probability[j+1] >= distance) {
				distance = (j + (distance - probability[j]) / (probability[j+1] - probability[j])) / width;
				break;
			}
		}
		distance = radius_min + distance * (radius_max - radius_min);
		float z_shift = z_shift_distribution(generator) + z_shift_distribution(generator);
		*(tmp++) = Vec3f(Mat4f::zrotation(i * 3.141592653589793238 * 2 / NB_ASTEROIDS) * Vec4f(distance, 0., z_shift, 1.));
		uint8_t *tmpColor = pData + j * 4;
		float factor = std::min(tmpColor[3] / 128.f, 1.f) / 255.f;
		(tmp++)->set(tmpColor[0] * factor, tmpColor[1] * factor, tmpColor[2] * factor);
	}
    tex->releaseContent(pData);
	asteroidComputed = true;
}

Ring::~Ring(void)
{
}

void Ring::draw(VkCommandBuffer &cmd, const Projector* prj, float observerDistanceToBody, const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius)
{
	if (!fullyInitialized)
		initialize();

	// solve the ring wraparound by culling: decide if we are above or below the ring plane
	const double h = mat.r[ 8]*mat.r[12]
	                 + mat.r[ 9]*mat.r[13]
	                 + mat.r[10]*mat.r[14];

	static_assert(offsetof(RingUniform, SunnySideUp) == sizeof(float) * (16*2 + (3+1)*2 + 3), "Invalid alignment in Ring");
	uniform->get().LightDirection = _lightDirection;
	uniform->get().PlanetPosition = _planetPosition;
	uniform->get().PlanetRadius = planetRadius;
	uniform->get().RingScale = mc;

	Mat4f matrix = mat.convert();
	Mat4f inv_matrix = matrix.inverse();
	uniform->get().ModelViewMatrix = matrix;
	uniform->get().clipping_fov = prj->getClippingFov();
	uniform->get().ModelViewMatrixInverse = inv_matrix;

	uniform->get().SunnySideUp = (h>0.0) ? 1.0 : 0.0;

	if (asteroidReady && observerDistanceToBody < radius_max * 10) {
		uniform->get().fadingFactor = fadingFactor; // calibrated over Saturn radiux_max
		pipelineAsteroid->bind(cmd);
		layoutAsteroid->bindSet(cmd, *setAsteroid);
		VertexArray::bind(cmd, {bufferAsteroid, instanceAsteroid.get()});
		vkCmdBindIndexBuffer(cmd, indexAsteroid.buffer, indexAsteroid.offset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd, indexAsteroid.size / 4, instanceAsteroid->getVertexCount(), 0, 0, 0);
	} else {
		uniform->get().fadingFactor = 100000;
	}
	pipeline->bind(cmd);
	layout->bindSet(cmd, *set);

	if (screen_sz < 30.f) {
		if (h>0.0) lowUP->draw(cmd);
		else lowDOWN->draw(cmd);
	}
	else {
		if (screen_sz >300.f) {
			if (h>0.0) highUP->draw(cmd);
			else highDOWN->draw(cmd);
		}
		else {
			if (h>0.0) mediumUP->draw(cmd);
			else mediumDOWN->draw(cmd);
		}
	}
}

// class Ring2D
Ring2D::Ring2D(float _r_min, float _r_max, int _slices, int _stacks, bool h, VertexArray &base)
{
	r_min = _r_min;
	r_max = _r_max;

	m_dataGL = base.createBuffer(0, _stacks * (_slices + 1) * 2, Context::instance->globalBuffer.get());
	computeRing(_slices, _stacks, h);
}

Ring2D::~Ring2D()
{
}

void Ring2D::draw(VkCommandBuffer &cmd)
{
	m_dataGL->bind(cmd);
	vkCmdDraw(cmd, m_dataGL->getVertexCount(), 1, 0, 0);
}

void Ring2D::computeRing(int slices, int stacks, bool h)
{
	double theta;
	int j;

	const double dtheta = 2.0 * M_PI / slices*(1-2*h);

	//~ if (slices < 0) slices = -slices;
	double *cos_sin_theta = (double *) alloca(sizeof(double) * 2*(slices+1));
	double *cos_sin_theta_p = cos_sin_theta;
	for (j = 0; j <= slices; j++) {
		theta = (j == slices) ? 0.0 : j * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	float *datas = (float *) Context::instance->transfer->planCopy(m_dataGL->get());

	const double r_size = r_max - r_min;
	// draw intermediate stacks as quad strips
	for (int i = 0; i < stacks; ++i) {
		const double tex_r0 = ((double) i) / stacks;
		const double tex_r1 = (i + 1.) / stacks;
		const double r0 = r_min + r_size * tex_r0;
		const double r1 = r_min + r_size * tex_r1;

		for (j=0,cos_sin_theta_p=cos_sin_theta; j<=slices; ++j,cos_sin_theta_p+=2) {
			theta = (j == slices) ? 0.0 : j * dtheta;

			// Pos2D
			*(datas++) = r0*cos_sin_theta_p[0];
			*(datas++) = r0*cos_sin_theta_p[1];
			// Tex1D
			*(datas++) = tex_r0;

			// Pos2D
			*(datas++) = r1*cos_sin_theta_p[0];
			*(datas++) = r1*cos_sin_theta_p[1];
			// Tex1D
			*(datas++) = tex_r1;
		}
	}
}

void Ring::preload()
{
	if (!fullyInitialized)
		initialize();
}
