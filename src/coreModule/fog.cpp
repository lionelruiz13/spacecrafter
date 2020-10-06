/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014-2020 Association Sirius
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

#include "coreModule/fog.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "tools/s_texture.hpp"
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
#include "vulkanModule/ResourceTracker.hpp"

//std::unique_ptr<shaderProgram> Fog::shaderFog;
s_texture* Fog::fog_tex;
VertexArray *Fog::vertexModel;
PipelineLayout *Fog::layout;
Pipeline *Fog::pipeline;
int Fog::vUniformID1;
int Fog::vUniformID2;
Set *Fog::set;

Fog::Fog(float _radius, ThreadContext *context) : radius(_radius)
{
	m_fogGL = std::make_unique<VertexArray>(context->surface);
	m_fogGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_fogGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	uMV = std::make_unique<Uniform>(context->surface, sizeof(*pMV), true);
	pMV = static_cast<typeof(pMV)>(uMV->data);
	uFrag = std::make_unique<Uniform>(context->surface, sizeof(float) * 2, true);
	psky_brightness = static_cast<float *>(uFrag->data);
	pFader = psky_brightness + 1;
	cmdMgr = context->commandMgr;
	globalSet = context->global->globalSet;
}


Fog::~Fog()
{}


void Fog::createSC_context(ThreadContext *context)
{
	//context = _context;
	// shaderFog = std::make_unique<shaderProgram>();
	// shaderFog-> init( "fog.vert","fog.frag");

	// shaderFog->setUniformLocation("sky_brightness");
	// shaderFog->setUniformLocation("fader");
	//
	// shaderFog->setUniformLocation("ModelViewMatrix");

	vertexModel = context->global->tracker->track(new VertexArray(context->surface));
	vertexModel->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	vertexModel->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);

	layout = context->global->tracker->track(new PipelineLayout(context->surface));
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setTextureLocation(0);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 1, 1, true);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1, true);
	layout->buildLayout();
	layout->build();
	pipeline = context->global->tracker->track(new Pipeline(context->surface, layout));
	pipeline->setDepthStencilMode();
	pipeline->setFrontFace();
	pipeline->setCullMode(true);
	pipeline->setBlendMode(BLEND_ADD);
	pipeline->bindVertex(vertexModel);
	pipeline->bindShader("fog.vert.spv");
	pipeline->bindShader("fog.frag.spv");
	pipeline->build();
	set = context->global->tracker->track(new Set(context->surface, context->setMgr, layout));
	fog_tex = new s_texture("fog.png",TEX_LOAD_TYPE_PNG_SOLID_REPEAT,false);
	set->bindTexture(fog_tex->getTexture(), 0);
	Uniform uniformModel1(context->surface, 16, true);
	vUniformID1 = set->bindVirtualUniform(&uniformModel1, 1);
	Uniform uniformModel2(context->surface, 2, true);
	vUniformID2 = set->bindVirtualUniform(&uniformModel2, 2);
	set->update();
}

void Fog::initShader()
{
	std::vector<float> dataTex;
	std::vector<float> dataPos;

	createFogMesh(radius, radius*sinf(alt_angle*M_PI/180.), 128,1, &dataTex, &dataPos);

	m_fogGL->build(dataPos.size() / 3);
	m_fogGL->fillVertexBuffer(BufferType::POS3D, dataPos);
	m_fogGL->fillVertexBuffer(BufferType::TEXTURE, dataTex);
	set->setVirtualUniform(uMV.get(), vUniformID1);
	set->setVirtualUniform(uFrag.get(), vUniformID2);
	commandIndex = cmdMgr->initNew(pipeline);
	cmdMgr->bindSet(layout, globalSet, 0);
	cmdMgr->bindSet(layout, set, 1);
	cmdMgr->bindVertex(m_fogGL.get());
	cmdMgr->draw(dataPos.size()/3);
	cmdMgr->compile();

	dataTex.clear();
	dataPos.clear();
}


// Draw the horizon fog
void Fog::draw(const Projector* prj, const Navigator* nav) const
{
	if (!fader.getInterstate()) return;

	//StateGL::BlendFunc(GL_ONE, GL_ONE);

	// StateGL::enable(GL_BLEND);
	// StateGL::enable(GL_CULL_FACE);
	// glCullFace(GL_FRONT);

	// shaderFog->use();
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, fog_tex->getID());
	// shaderFog->setUniform("fader", fader.getInterstate());
	// shaderFog->setUniform("sky_brightness", sky_brightness);
	*pFader = fader.getInterstate();
	*psky_brightness = sky_brightness;
	*pMV = (nav->getLocalToEyeMat() * Mat4d::translation(Vec3d(0.,0.,radius*sinf(angle_shift*M_PI/180.)))).convert();
	// shaderFog->setUniform("ModelViewMatrix",matrix);

	cmdMgr->setSubmission(commandIndex);
	//Renderer::drawArrays(shaderFog.get(), m_fogGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,0,nbVertex);

	// glCullFace(GL_BACK);
	// StateGL::disable(GL_CULL_FACE);
	// glActiveTexture(GL_TEXTURE0);
}


void Fog::createFogMesh(GLdouble radius, GLdouble height, GLint slices, GLint stacks, std::vector<float>* dataTex, std::vector<float>* dataPos)
{
	nbVertex=0;
	GLdouble da, r, dz;
	GLfloat z ;
	GLint i;

	da = 2.0 * M_PI / slices;
	dz = height / stacks;

	GLfloat ds = 1.0 / slices;
	GLfloat dt = 1.0 / stacks;
	GLfloat t = 0.0;
	z = 0.0;
	r = radius;
	GLfloat s = 0.0;
	for (i = 0; i <= slices; i++) {
		GLfloat x, y;
		if (i == slices) {
			x = sinf(0.0);
			y = cosf(0.0);
		}
		else {
			x = sinf(i * da);
			y = cosf(i * da);
		}
		dataTex->push_back(s);
		dataTex->push_back(t);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z);
		nbVertex++;

		dataTex->push_back(s);
		dataTex->push_back(t+dt);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z+dz);
		nbVertex++;
		s += ds;
	}
}
