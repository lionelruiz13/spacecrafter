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

#include "tools/context.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Core/FrameMgr.hpp"

//a copy of zone_array.hpp
#define NR_OF_HIP 120416

IlluminateMgr::IlluminateMgr(std::shared_ptr<HipStarMgr> _hip_stars, Navigator *_navigator, std::shared_ptr<ConstellationMgr> _asterism)
{
	hip_stars = _hip_stars;
	navigator = _navigator;
	asterism = _asterism;

	illuminateGrid.subdivise(3);

	defaultTex = std::make_shared<s_texture>("star_illuminate.png", TEX_LOAD_TYPE_PNG_BLEND3);
	if (defaultTex == nullptr)
		cLog::get()->write("Error loading texture illuminateTex", LOG_TYPE::L_ERROR);

	currentTex = defaultTex;
	createSC_context();
}

IlluminateMgr::~IlluminateMgr()
{
	illuminateGrid.clear();
	Context::instance->indexBufferMgr->releaseBuffer(index);
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
	if (_size <= 0){
		hip_stars->hideStar(num);
		remove(num);
		return;
	} else
		hip_stars->showStar(num);
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
void IlluminateMgr::loadIlluminate(unsigned int name, double ra, double de,  double angular_size, double r, double g, double b, float tex_rotation)
{
	if (angular_size<1.0)
		angular_size=defaultSize;

	auto e = std::make_unique<Illuminate>(name, ra, de, angular_size, r, b, g, tex_rotation);
	illuminateGrid.insert(std::move(e), e->getXYZ(), angular_size/2/60*M_PI/180);
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
	hip_stars->showAllStar();
}

// Draw all the Illuminate
void IlluminateMgr::draw(Projector* prj, const Navigator * nav)
{
	Context &context = *Context::instance;
	float *illumData = (float *) context.transfer->beginPlanCopy(MAX_ILLUMINATE * 8 * sizeof(float));
	if (!illumData)
		return;
	auto cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_BACKGROUND);
	float max_fov = std::max( prj->getFov(), prj->getFov()*prj->getViewportWidth()/prj->getViewportHeight());
	illuminateGrid.intersect(nav->getPrecEquVision(), max_fov*M_PI/180.f);

	int nbVertex = 0;
	for (const auto &it : illuminateGrid) {
		it->draw(prj, illumData);
		if (++nbVertex == MAX_ILLUMINATE) // stop if vertexArray capacity is reached
			break;
	}
	context.transfer->endPlanCopy(vertex->get(), nbVertex*4*8*sizeof(float));
	if (nbVertex == 0)
		return;

	m_pipelineIllum->bind(cmd);
	m_layoutIllum->bindSets(cmd, {*m_setIllum, *context.uboSet});
	vertex->bind(cmd);
	vkCmdBindIndexBuffer(cmd, index.buffer, index.offset, VK_INDEX_TYPE_UINT32);
	auto tmp1 = prj->getMatJ2000ToEye();
	m_layoutIllum->pushConstant(cmd, 0, &tmp1);
	vkCmdDrawIndexed(cmd, nbVertex * 6, 1, 0, 0, 0);
	context.frame[context.frameIdx]->compile(cmd);
	context.frame[context.frameIdx]->toExecute(cmd, PASS_BACKGROUND);
}

void IlluminateMgr::buildSet()
{
	m_setIllum = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, m_layoutIllum.get(), -1, true, true);
	m_setIllum->bindTexture(currentTex->getTexture(), 0);
}

void IlluminateMgr::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	m_illumGL = std::make_unique<VertexArray>(vkmgr);
	m_illumGL->createBindingEntry(8 * sizeof(float));
	m_illumGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	m_illumGL->addInput(VK_FORMAT_R32G32_SFLOAT);
	m_illumGL->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertex = m_illumGL->createBuffer(0, MAX_ILLUMINATE * 4, context.globalBuffer.get());
	index = context.indexBufferMgr->acquireBuffer(MAX_ILLUMINATE * 6 * sizeof(uint32_t));

	{ // initialize index buffer
		uint32_t *tmpIndex = (uint32_t *) context.transfer->planCopy(index);
		for (int i = 0; i < MAX_ILLUMINATE * 4; i += 4) {
			*(tmpIndex++) = i + 0;
			*(tmpIndex++) = i + 1;
			*(tmpIndex++) = i + 2;

			*(tmpIndex++) = i + 2;
			*(tmpIndex++) = i + 1;
			*(tmpIndex++) = i + 3;
		}
	}
	m_layoutIllum = std::make_unique<PipelineLayout>(vkmgr);
	m_layoutIllum->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	m_layoutIllum->buildLayout();
	m_layoutIllum->setGlobalPipelineLayout(context.layouts.front().get());
	m_layoutIllum->setPushConstant(VK_SHADER_STAGE_GEOMETRY_BIT, 0, sizeof(Mat4f));
	m_layoutIllum->build();

	m_pipelineIllum = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_BACKGROUND, m_layoutIllum.get());
	m_pipelineIllum->bindVertex(*m_illumGL);
	m_pipelineIllum->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	m_pipelineIllum->setDepthStencilMode();
	m_pipelineIllum->bindShader("illuminate.vert.spv");
	m_pipelineIllum->bindShader("illuminate.geom.spv");
	m_pipelineIllum->bindShader("illuminate.frag.spv");
	m_pipelineIllum->build();

	for (int i = 0; i < 3; ++i) {
		cmds[i] = context.frame[i]->create(1);
		context.frame[i]->setName(cmds[i], "Illuminate " + std::to_string(i));
	}
	buildSet();
}

void IlluminateMgr::changeTex(const std::string& fileName)
{
	auto tmp = std::make_shared<s_texture>(fileName, TEX_LOAD_TYPE_PNG_BLEND3);
	if (tmp==nullptr) {
		cLog::get()->write("illuminate: error when loading user texture "+ fileName, LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
		return;
	}
	currentTex = tmp;
	buildSet();
}

void IlluminateMgr::removeTex()
{
	if (currentTex != defaultTex) {
		currentTex = defaultTex;
		buildSet();
	}
}
