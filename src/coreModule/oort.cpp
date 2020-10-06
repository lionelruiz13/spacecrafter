/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/oort.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
// #include "renderGL/OpenGL.hpp"
// #include "renderGL/shader.hpp"
// #include "renderGL/Renderer.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/VertexArray.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"

#define NB_POINTS 200000

Oort::Oort(ThreadContext *context)
{
	color = Vec3f(1.0,1.0,0.0);
	fader = false;
	createSC_context(context);
}

Oort::~Oort()
{}

void Oort::createSC_context(ThreadContext *_context)
{
	context = _context;
	// shaderOort = std::make_unique<shaderProgram>();
	// shaderOort->init("oort.vert","oort.frag");
	// shaderOort->setUniformLocation({"Mat","color","intensity"});

	m_dataGL = std::make_unique<VertexArray>(context->surface);
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setDepthStencilMode();
	pipeline->bindVertex(m_dataGL.get());
	pipeline->bindShader("oort.vert.spv");
	pipeline->bindShader("oort.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
	uMat = std::make_unique<Uniform>(context->surface, sizeof(*pMat));
	pMat = static_cast<typeof(pMat)>(uMat->data);
	set->bindUniform(uMat.get(), 0);
	uFrag = std::make_unique<Uniform>(context->surface, sizeof(float) * 4);
	pColor = static_cast<typeof(pColor)>(uFrag->data);
	pIntensity = static_cast<float *>(uFrag->data) + 3;
	set->bindUniform(uFrag.get(), 1);
}


void Oort::populate(unsigned int nbr) noexcept
{
	float radius, theta, phi, r_theta, r_phi;
	Vec3f tmp;
	Vec3d tmp_local;
	std::vector<float> dataOort;
	for(unsigned int i=0; i<nbr ; i++) {
		r_theta = (float) (rand()%3600);
		r_phi = (float) (rand()%1400);
		theta = r_theta /10.;
		phi   = -70. + r_phi /10.;
		if (abs(phi)>60) phi = phi*(1+(abs(phi)-60)/35);
		radius = 60. + (float) (rand()%5000);
		if (radius<2570) phi = phi*(radius-0)/2570;
		if (radius>4000) radius = radius*(1+(radius-4000)/4000);

		Utility::spheToRect(theta*M_PI/180,phi*M_PI/180, tmp);
		tmp = tmp * radius;
		insert_vec3(dataOort, tmp);
		//~ printf("%5.3f %5.3f %5.3f \n", tmp_local[0], tmp_local[1], tmp_local[2]);
	}

	nbAsteroids = dataOort.size()/3 ;
	//on charge les points dans un vbo
	m_dataGL->build(nbAsteroids);
	m_dataGL->fillVertexBuffer(BufferType::POS3D, dataOort);
}

void Oort::build()
{
	CommandMgr *cmdMgr = context->commandMgr;
	commandIndex = cmdMgr->initNew(pipeline.get());
	cmdMgr->bindSet(layout.get(), context->global->globalSet, 0);
	cmdMgr->bindSet(layout.get(), set.get(), 1);
	cmdMgr->bindVertex(m_dataGL.get());
	cmdMgr->draw(nbAsteroids);
	cmdMgr->compile();
}

void Oort::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate()) return;

	// gestion de l'intensité
	if ((abs(distance) < 1e13) || (abs(distance) > 5.E15))
	return;

	intensity = std::min(1.0, (abs(distance/1.e13)-1));
	if (abs(distance) > 1.E15) intensity = 1.25-0.25*(abs(distance/1.E15));
	//~ printf("distance : %f\n", distance);
	//~ printf("intensity : %f\n", intensity);

	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	*pMat = nav->getHelioToEyeMat().convert();
	*pColor = color;
	*pIntensity = intensity*fader.getInterstate();

	// shaderOort->use();
	// shaderOort->setUniform("Mat",matrix);
	// shaderOort->setUniform("color", color);
	// shaderOort->setUniform("intensity", intensity*fader.getInterstate());

	// m_dataGL->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, nbAsteroids );
	// m_dataGL->unBind();
	// shaderOort->unuse();
	//Renderer::drawArrays(shaderOort.get(), m_dataGL.get(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, nbAsteroids );
	context->commandMgr->setSubmission(commandIndex);
}
