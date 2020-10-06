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

#include <fstream>
#include <algorithm>
#include "coreModule/illuminate_mgr.hpp"
#include "coreModule/illuminate.hpp"
#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"
#include "vulkanModule/VertexBuffer.hpp"

//a copy of zone_array.hpp
#define NR_OF_HIP 120416

IlluminateMgr::IlluminateMgr(HipStarMgr * _hip_stars, Navigator* _navigator, ConstellationMgr* _asterism, ThreadContext *context)
{
	hip_stars = _hip_stars;
	navigator = _navigator;
	asterism = _asterism;

	illuminateGrid.subdivise(3);

	defaultTex = new s_texture("star_illuminate.png", TEX_LOAD_TYPE_PNG_BLEND3 );
	if (defaultTex ==nullptr)
		cLog::get()->write("Error loading texture illuminateTex", LOG_TYPE::L_ERROR);

	currentTex = defaultTex;
	createSC_context(context);
}

IlluminateMgr::~IlluminateMgr()
{
	illuminateGrid.clear();

	if (defaultTex) delete defaultTex;
	defaultTex = nullptr;
}

// Load individual Illuminate for script
void IlluminateMgr::load(int num, double size, double rotation)
{
	if (num>NR_OF_HIP)
		return;
	Object selected_object = hip_stars->searchHP(num).get();
	Vec3f color = selected_object.getRGB();
	//std::cout << num << " ra/de " << ra << " " << de << " mag " << mag << " color " << color[0]<< ":"<< color[1]<< ":"<< color[2]<< std::endl;
	//std::cout <<num << " only" <<std::endl;
	load(num, color, size, rotation);
}

void IlluminateMgr::loadConstellation(const std::string& abbreviation, double size, double rotation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAbbreviation(abbreviation, HPStars);
	for (auto i: HPStars)
		this->load(i,size,rotation);
}

void IlluminateMgr::loadConstellation(const std::string& abbreviation, const Vec3f& color, double size, double rotation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAbbreviation(abbreviation, HPStars);
	for (auto i: HPStars)
		this->load(i, color, size, rotation);
}

void IlluminateMgr::loadAllConstellation(double size, double rotation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAll(HPStars);
	for (auto i: HPStars)
		this->load(i,size,rotation);
}


void IlluminateMgr::removeConstellation(const std::string& abbreviation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAbbreviation(abbreviation, HPStars);
	for (auto i: HPStars)
		this->remove(i);
}


void IlluminateMgr::removeAllConstellation()
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAll(HPStars);
	for (auto i: HPStars)
		this->remove(i);
}

void IlluminateMgr::load(int num, const Vec3f& _color, double _size, double rotation)
{
	if (num>NR_OF_HIP)
		return;
	Object selected_object = hip_stars->searchHP(num).get();
	//Vec3f color = selected_object.getRGB();
	double ra, de;
	selected_object.getRaDeValue(navigator,&ra,&de);
	double size = _size;
	//setup size
	if (size<1.0) {
		float mag = selected_object.getMag(navigator);
		if (mag<0) mag=10;
		size = defaultSize + 4.0 * (10-mag);
	}
	//std::cout << num << " ra/de " << ra << " " << de << " mag " << mag << " color " << color[0]<< ":"<< color[1]<< ":"<< color[2]<< std::endl;
	//std::cout << num << " with color" << std::endl;
	loadIlluminate(num, ra, de, size, _color[0], _color[1], _color[2], rotation );
}


// Load individual Illuminate
void IlluminateMgr::loadIlluminate(unsigned int name, double ra, double de,  double angular_size, double r, double g, double b, double tex_rotation)
{
	if (angular_size<1.0)
		angular_size=defaultSize;

	auto e = std::make_unique<Illuminate>(name, ra, de, angular_size, r, b, g, tex_rotation);
	illuminateGrid.insert(std::move(e), e->getXYZ());
}

// Clear user added Illuminate
void IlluminateMgr::remove(unsigned int name)
{
	illuminateGrid.remove_if([name](auto &value){return value->getName() == name;});
}

// remove all user added Illuminate
void IlluminateMgr::removeAll()
{
	illuminateGrid.clear();
}

// Draw all the Illuminate
void IlluminateMgr::draw(Projector* prj, const Navigator * nav)
{
	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Vec3f pXYZ;

	// illumPos.clear();
	// illumTex.clear();
	// illumColor.clear();
	float *illumData = static_cast<float *>(m_illumGL->getVertexBuffer().data);

	float max_fov = std::max( prj->getFov(), prj->getFov()*prj->getViewportWidth()/prj->getViewportHeight());
	illuminateGrid.intersect(nav->getPrecEquVision(), max_fov*M_PI/180.f);

	int nbVertex = 0;
	for (const auto &it : illuminateGrid) {
		it->draw(prj, illumData);
		if (++nbVertex == MAX_ILLUMINATE) // stop if vertexArray capacity is reached
			break;
	}
	if (nbVertex == 0)
		return;
	m_pDrawDataIllum[0] = nbVertex * 6;
	m_drawDataIllum->update();


	//int nbrIllumToTrace = illumPos.size()/12;
	// std::cout << "Illuminate à tracer: il y a " << nbrIllumToTrace << std::endl;
	// std::cout << "illumPos   size : " << illumPos.size() << std::endl;
	// std::cout << "illumTex   size : " << illumTex.size() << std::endl;
	// std::cout << "illumColor size : " << illumColor.size() << std::endl;
	//m_shaderIllum->use();

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, currentTex->getID());

	*pModelViewMatrix = prj->getMatJ2000ToEye();

	// m_illumGL->fillVertexBuffer(BufferType::POS3D, illumPos);
	// m_illumGL->fillVertexBuffer(BufferType::TEXTURE, illumTex);
	// m_illumGL->fillVertexBuffer(BufferType::COLOR, illumColor);
	cmdMgr->setSubmission(commandIndex, false, cmdMgrTarget);
	//Renderer::drawMultiArrays(m_shaderIllum.get(), m_illumGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, nbrIllumToTrace, 4);
}


void IlluminateMgr::createSC_context(ThreadContext *context)
{
	// m_shaderIllum = std::make_unique<shaderProgram>();
	// m_shaderIllum->init("illuminate.vert", "illuminate.geom", "illuminate.frag");
	// m_shaderIllum->setUniformLocation("ModelViewMatrix");

	m_illumGL = std::make_unique<VertexArray>(context->surface);
	m_illumGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
	m_illumGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::DYNAMIC);
	m_illumGL->registerVertexBuffer(BufferType::COLOR, BufferAccess::DYNAMIC);
	m_illumGL->build(MAX_ILLUMINATE * 4);
	m_illumGL->registerIndexBuffer(BufferAccess::STATIC, MAX_ILLUMINATE * 6, 2, VK_INDEX_TYPE_UINT16);

	{ // initialize index buffer
		std::vector<uint16_t> tmpIndex;
		tmpIndex.reserve(MAX_ILLUMINATE * 6);
		for (int i = 0; i < MAX_ILLUMINATE * 4; i += 4) {
			tmpIndex.push_back(i + 0);
			tmpIndex.push_back(i + 1);
			tmpIndex.push_back(i + 2);

			tmpIndex.push_back(i + 2);
			tmpIndex.push_back(i + 1);
			tmpIndex.push_back(i + 3);
		}
		m_illumGL->fillIndexBuffer(MAX_ILLUMINATE * 3, reinterpret_cast<uint32_t *>(tmpIndex.data()));
	}
	m_layoutIllum = std::make_unique<PipelineLayout>(context->surface);
	m_layoutIllum->setTextureLocation(0);
	m_layoutIllum->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 1);
	m_layoutIllum->buildLayout();
	m_layoutIllum->setGlobalPipelineLayout(context->global->globalLayout);
	m_layoutIllum->build();

	m_pipelineIllum = std::make_unique<Pipeline>(context->surface, m_layoutIllum.get());
	m_pipelineIllum->bindVertex(m_illumGL.get());
	m_pipelineIllum->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	m_pipelineIllum->setDepthStencilMode();
	m_pipelineIllum->bindShader("illuminate.vert.spv");
	m_pipelineIllum->bindShader("illuminate.geom.spv");
	m_pipelineIllum->bindShader("illuminate.frag.spv");
	m_pipelineIllum->build();

	m_setIllum = std::make_unique<Set>(context->surface, context->setMgr, m_layoutIllum.get());
	m_setIllum->bindTexture(currentTex->getTexture(), 0);
	m_uniformIllum = std::make_unique<Uniform>(context->surface, sizeof(Mat4f));
	pModelViewMatrix = static_cast<typeof(pModelViewMatrix)>(m_uniformIllum->data);
	m_setIllum->bindUniform(m_uniformIllum.get(), 1);

	m_drawDataIllum = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	m_pDrawDataIllum = static_cast<typeof(m_pDrawDataIllum)>(m_drawDataIllum->data);
	m_pDrawDataIllum[1] = 1; // instanceCount
	m_pDrawDataIllum[2] = m_pDrawDataIllum[3] = m_pDrawDataIllum[4] = 0; // offsets

	cmdMgr = context->commandMgrDynamic;
	cmdMgrTarget = context->commandMgr;
	commandIndex = cmdMgr->initNew(m_pipelineIllum.get());
	cmdMgr->bindSet(m_layoutIllum.get(), m_setIllum.get());
	cmdMgr->bindSet(m_layoutIllum.get(), context->global->globalSet, 1);
	cmdMgr->bindVertex(m_illumGL.get());
	cmdMgr->indirectDrawIndexed(m_drawDataIllum.get());
	cmdMgr->compile();
	globalSet = context->global->globalSet;
}


void IlluminateMgr::changeTex(const std::string& fileName)
{
	this->removeTex();
	userTex = new s_texture(fileName, TEX_LOAD_TYPE_PNG_BLEND3 );
	if (userTex==nullptr) {
		cLog::get()->write("illuminate: error when loading user texture "+ fileName, LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
	}
	currentTex = userTex;
	cmdMgrTarget->waitCompletion(0);
	cmdMgrTarget->waitCompletion(1);
	cmdMgrTarget->waitCompletion(2);
	m_setIllum->bindTexture(currentTex->getTexture(), 0);
	cmdMgr->init(commandIndex, m_pipelineIllum.get());
	cmdMgr->bindSet(m_layoutIllum.get(), m_setIllum.get());
	cmdMgr->bindSet(m_layoutIllum.get(), globalSet, 1);
	cmdMgr->bindVertex(m_illumGL.get());
	cmdMgr->indirectDrawIndexed(m_drawDataIllum.get());
	cmdMgr->compile();
}

void IlluminateMgr::removeTex()
{
	if (currentTex == defaultTex) //nothing to do
		return;
	// here, userTex is used
	currentTex = defaultTex;
	delete userTex;
	userTex = nullptr;
}
