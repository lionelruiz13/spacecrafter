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

#include "bodyModule/ring.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "tools/s_font.hpp"
#include "planetsephems/sideral_time.h"
#include "tools/log.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Buffer.hpp"

Ring::Ring(double radius_min,double radius_max,const std::string &texname, const Vec3i &_init, ThreadContext *context)
	:radius_min(radius_min),radius_max(radius_max)
{
	init = _init;
	tex = new s_texture(texname,TEX_LOAD_TYPE_PNG_ALPHA,1);
	int nbVertices = (init[0] * (4 + 1) + init[1] * (8 + 1) + init[2] * (16 + 1)) * 2 * 2;
	createSC_context(context);

	lowUP = new Ring2D((float) radius_min, (float) radius_max, init[0], 4, true, *vertex);
	lowDOWN = new Ring2D((float) radius_min, (float) radius_max, init[0], 4, false, *vertex);

	mediumUP = new Ring2D((float) radius_min, (float) radius_max, init[1], 8, true, *vertex);
	mediumDOWN = new Ring2D((float) radius_min, (float) radius_max, init[1], 8, false, *vertex);

	highUP = new Ring2D((float) radius_min, (float) radius_max, init[2], 16, true, *vertex);
	highDOWN = new Ring2D((float) radius_min, (float) radius_max, init[2], 16, false, *vertex);

	vertex->build(nbVertices);
	highUP->initFrom(*vertex);
	highDOWN->initFrom(*vertex);
	mediumUP->initFrom(*vertex);
	mediumDOWN->initFrom(*vertex);
	lowUP->initFrom(*vertex);
	lowDOWN->initFrom(*vertex);
	vertex->assumeVerticeChanged();
	vertex->update();
}


void Ring::createSC_context(ThreadContext *context)
{
	cmdMgr = context->commandMgr;
	globalSet = context->global->globalSet;
// 	shaderRing = std::make_unique<shaderProgram>();
// 	shaderRing->init( "ring_planet.vert","ring_planet.frag");
//
// //	shaderRing->setUniformLocation("Texture");
// 	shaderRing->setUniformLocation("LightDirection");
// 	shaderRing->setUniformLocation("PlanetRadius");
// 	shaderRing->setUniformLocation("PlanetPosition");
// 	shaderRing->setUniformLocation("SunnySideUp");
// 	shaderRing->setUniformLocation("RingScale");
//
// 	//shaderRing->setUniformLocation("ModelViewProjectionMatrix");
// 	//shaderRing->setUniformLocation("inverseModelViewProjectionMatrix");
// 	shaderRing->setUniformLocation("ModelViewMatrix");
// 	shaderRing->setUniformLocation("clipping_fov");
// 	shaderRing->setUniformLocation("NormalMatrix");
// 	shaderRing->setUniformLocation("ModelViewMatrixInverse");

	vertex = std::make_unique<VertexArray>(context->surface, cmdMgr);
	vertex->registerVertexBuffer(BufferType::POS2D, BufferAccess::STATIC);
	vertex->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);

	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layout->setTextureLocation(1);
	layout->buildLayout();
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->build();

	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setCullMode(true);
	pipeline->bindVertex(vertex.get());
	pipeline->bindShader("ring_planet.vert.spv");
	pipeline->bindShader("ring_planet.frag.spv");
	pipeline->build();

	set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
	uniform = std::make_unique<Uniform>(context->surface, sizeof(*pUniform));
	pUniform = static_cast<typeof(pUniform)>(uniform->data);
	set->bindUniform(uniform.get(), 0);
	set->bindTexture(tex->getTexture(), 1);

	drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
}

Ring::~Ring(void)
{
	if (tex) delete tex;
	tex = nullptr;
	delete lowUP;
	delete lowDOWN;
	delete mediumUP;
	delete mediumDOWN;
	delete highUP;
	delete highDOWN;
}

void Ring::draw(const Projector* prj,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius)
{
	if (needRecording) {
		if (!cmdMgr->isRecording())
			return;
		cmdMgr->bindPipeline(pipeline.get());
		cmdMgr->bindSet(layout.get(), set.get());
		cmdMgr->bindSet(layout.get(), globalSet, 1);
		cmdMgr->bindVertex(vertex.get());
		cmdMgr->indirectDraw(drawData.get());
		cmdMgr->compile();
		needRecording = false;
		return;
	}
	// Normal transparency mode
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// StateGL::enable(GL_CULL_FACE);
	// StateGL::enable(GL_BLEND);


	// shaderRing->use();

	// glBindTexture (GL_TEXTURE_2D, tex->getID());

	// solve the ring wraparound by culling: decide if we are above or below the ring plane
	const double h = mat.r[ 8]*mat.r[12]
	                 + mat.r[ 9]*mat.r[13]
	                 + mat.r[10]*mat.r[14];

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, tex->getID());

//	shaderRing->setUniform("Texture",0);
	assert(offsetof(typeof(*pUniform), SunnySideUp) == sizeof(float) * (16*2 + (3+1)*2 + 3));
	pUniform->LightDirection = _lightDirection;
	pUniform->PlanetPosition = _planetPosition;
	pUniform->PlanetRadius = planetRadius;
	pUniform->RingScale = mc;


	//Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix = mat.convert();
	Mat4f inv_matrix = matrix.inverse();
//	ashaderRing->setUniform("ModelViewProjectionMatrix",proj*matrix);
//	ashaderRing->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	pUniform->ModelViewMatrix = matrix;
	pUniform->clipping_fov = prj->getClippingFov();
	//pUniform->NormalMatrix = inv_matrix.transpose();
	pUniform->ModelViewMatrixInverse = inv_matrix;

	pUniform->SunnySideUp = (h>0.0) ? 1.0 : 0.0;

	if (screen_sz < 30.f) {
		if (h>0.0) lowUP->draw(drawData->data);
		else lowDOWN->draw(drawData->data);
	}
	else {
		if (screen_sz >300.f) {
			if (h>0.0) highUP->draw(drawData->data);
			else highDOWN->draw(drawData->data);
		}
		else {
			if (h>0.0) mediumUP->draw(drawData->data);
			else mediumDOWN->draw(drawData->data);
		}
	}
	drawData->update();

	//shaderRing->unuse();
	// glActiveTexture(GL_TEXTURE0);

	//StateGL::disable(GL_CULL_FACE);
}


// class Ring2D
Ring2D::Ring2D(float _r_min, float _r_max, GLint _slices, GLint _stacks, bool h, VertexArray &base)
{
	r_min = _r_min;
	r_max = _r_max;

	computeRing(_slices, _stacks, h);

	m_dataGL = std::make_unique<VertexArray>(base);
}

Ring2D::~Ring2D()
{
	datas.clear();
}

void Ring2D::initFrom(VertexArray &vertex)
{
	drawData.firstInstance = 0;
	drawData.instanceCount = 1;
	drawData.vertexCount = datas.size() / 4; // Pos2D + TEXTURE = 4
	m_dataGL->assign(&vertex, drawData.vertexCount);
	m_dataGL->fillVertexBuffer(datas);
	drawData.firstVertex = m_dataGL->getVertexOffset();
}

void Ring2D::draw(void *pDrawData)
{
	*static_cast<typeof(&drawData)>(pDrawData) = drawData;
	// shader->use();
	// m_dataGL->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,dataVertex.size()/2);
	// m_dataGL->unBind();
	// shader->unuse();
	//Renderer::drawArrays(shader, m_dataGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,dataVertex.size()/2);
}


void Ring2D::computeRing(GLint slices, GLint stacks, bool h)
{
	double theta;
	double x,y;
	int j;

	const double dr = (r_max-r_min) / stacks;
	const double dtheta = 2.0 * M_PI / slices*(1-2*h);

	//~ if (slices < 0) slices = -slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (j = 0; j <= slices; j++) {
		theta = (j == slices) ? 0.0 : j * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// draw intermediate stacks as quad strips
	for (double r = r_min; r < r_max; r+=dr) {
		const double tex_r0 = (r-r_min)/(r_max-r_min);
		const double tex_r1 = (r+dr-r_min)/(r_max-r_min);

		for (j=0,cos_sin_theta_p=cos_sin_theta; j<=slices; j++,cos_sin_theta_p+=2) {
			theta = (j == slices) ? 0.0 : j * dtheta;

			x = r*cos_sin_theta_p[0];
			y = r*cos_sin_theta_p[1];

			//~ glColor3f(x,y,0);
			datas.push_back(x);
			datas.push_back(y);

			//~ glTexCoord2d(tex_r0, 0.5);
			datas.push_back(tex_r0);
			datas.push_back(0.5);

			x = (r+dr)*cos_sin_theta_p[0];
			y = (r+dr)*cos_sin_theta_p[1];

			//~ glColor3f(x,y,0);
			datas.push_back(x);
			datas.push_back(y);

			//~ glTexCoord2d(tex_r1, 0.5);
			datas.push_back(tex_r1);
			datas.push_back(0.5);
		}
	}
}
