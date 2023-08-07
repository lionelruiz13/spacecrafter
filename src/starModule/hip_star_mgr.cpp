/*
 * Spacecrafter
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2013 of the LSS team
 *
 * The big star catalogue extension to Stellarium:
 * Author and Copyright: Johannes Gajdosik, 2006
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
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
 */

// class used to manage groups of Stars

#include <string>
#include <list>
#include <errno.h>
#include <string.h>

#include <iostream>

#include "coreModule/projector.hpp"
#include "coreModule/time_mgr.hpp"
#include "navModule/navigator.hpp"
#include "starModule/geodesic_grid.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "starModule/string_array.hpp"
#include "starModule/zone_array.hpp"
#include "tools/app_settings.hpp"
//#include "tools/fmath.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include "tools/object_base.hpp"
#include "tools/object.hpp"
#include "tools/s_font.hpp"
#include "tools/s_texture.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/translator.hpp"
#include "tools/utility.hpp"
#include "coreModule/ubo_cam.hpp"
#include "EntityCore/EntityCore.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "tools/context.hpp"

static BigStarCatalog::StringArray spectral_array;
static BigStarCatalog::StringArray component_array;

bool HipStarMgr::flagSciNames = true;
double HipStarMgr::current_JDay = 2451545.0;  // Default to J2000 so that constellation art shows up in correct positions at init
std::map<int,std::string> HipStarMgr::common_names_map;
std::map<int,std::string> HipStarMgr::common_names_map_i18n;
std::map<std::string,int> HipStarMgr::common_names_index;
std::map<std::string,int> HipStarMgr::common_names_index_i18n;

std::map<int,std::string> HipStarMgr::sci_names_map_i18n;
std::map<std::string,int> HipStarMgr::sci_names_index_i18n;

std::string HipStarMgr::convertToSpectralType(int index)
{
	if (index < 0 || index >= spectral_array.getSize()) {
		std::stringstream oss;
		oss << "convertToSpectralType: bad index: " << index << ", max: " << spectral_array.getSize();
		cLog::get()->write(oss.str(), LOG_TYPE::L_ERROR);
		return "";
	}
	return spectral_array[index];
}

std::string HipStarMgr::convertToComponentIds(int index)
{
	if (index < 0 || index >= component_array.getSize()) {
		std::stringstream oss;
		oss << "convertToComponentIds: bad index: " << index << ", max: " << component_array.getSize() << std::endl;
		cLog::get()->write(oss.str(), LOG_TYPE::L_ERROR);
		return "";
	}
	return component_array[index];
}

void HipStarMgr::initTriangle(int lev,int index, const Vec3d &c0, const Vec3d &c1, const Vec3d &c2)
{
	ZoneArrayMap::const_iterator it(zone_arrays.find(lev));
	if (it!=zone_arrays.end()) it->second->initTriangle(index,c0,c1,c2);
}



Vec3f HipStarMgr::color_table[128];

void HipStarMgr::iniColorTable()
{
	for (int i=0; i<128; i++)
		HipStarMgr::color_table[i] = v3fNull;
}

void HipStarMgr::readColorTable ()
{
	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigDir() + "stars.ini");

	for (int i=0; i<128; i++) {
		char entry[256];
		sprintf(entry,"color_%03i",i);
		const std::string s(conf.getStr("colors",entry));
		if (!s.empty()) {
			const Vec3f c(Utility::strToVec3f(s));
			HipStarMgr::color_table[i] = c;
		} else {
			cLog::get()->write("Star color not define", LOG_TYPE::L_WARNING);
			HipStarMgr::color_table[i] = v3fNull;
		}
	}
}

void HipStarMgr::setColorStarTable(int p, Vec3f a)
{
	if (p<0 || p>127) {
		cLog::get()->write("ColorTable index Error", LOG_TYPE::L_ERROR);
		return;
	} else
		HipStarMgr::color_table[p]= a;
}

#define GAMMA 0.45

static double Gamma(double gamma,double x)
{
	return ((x<=0.0) ? 0.0 : exp(gamma*log(x)));
}

static Vec3f Gamma(double gamma,const Vec3f &x)
{
	return Vec3f(Gamma(gamma,x[0]),Gamma(gamma,x[1]),Gamma(gamma,x[2]));
}

static void InitColorTableFromConfigFile(const InitParser &conf)
{
	std::map<float,Vec3f> color_map;

	if (color_map.size() > 1) {
		for (int i=0; i<128; i++) {
			const float b_v = BigStarCatalog::IndexToBV(i);
			std::map<float,Vec3f>::const_iterator greater(color_map.upper_bound(b_v));
			if (greater == color_map.begin()) {
				HipStarMgr::color_table[i] = greater->second;
			} else {
				std::map<float,Vec3f>::const_iterator less(greater);
				--less;
				if (greater == color_map.end()) {
					HipStarMgr::color_table[i] = less->second;
				} else {
					HipStarMgr::color_table[i] = Gamma(0.45,
					                                   ((b_v-less->first)*greater->second
					                                    + (greater->first-b_v)*less->second)
					                                   *(1.f/(greater->first-less->first)));
				}
			}
		}
	}
}


HipStarMgr::HipStarMgr(int width,int height) :
	starTexture(),
	hip_index(new BigStarCatalog::HipIndexStruct[NR_OF_HIP+1]),
	mag_converter(new MagConverter(*this)),
	fontSize(13.)
{
	fader.setDuration(3);
	setMagConverterMaxScaled60DegMag(6.5f);
	if (hip_index == 0 || mag_converter == 0) {
		std::cerr << "ERROR: HipStarMgr::HipStarMgr: no memory" << std::endl;
		exit(1);
	}
	max_geodesic_grid_level = -1;
	last_max_search_level = -1;

	// shaderStars = std::make_unique<shaderProgram>();
	// shaderStars-> init("stars.vert","stars.geom", "stars.frag");
	//
	// shaderFBO = std::make_unique<shaderProgram>();
	// shaderFBO-> init("fbo.vert","fbo.frag");

	createShaderParams( width, height);
}

//TODO fix float[NBR_MAX_STARS];
void HipStarMgr::createShaderParams(int width,int height)
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	float dataTex[]= {0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0};
	float dataPos[]= {-1.0,-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0};

	renderPassClear = std::make_unique<RenderMgr>(vkmgr);
	renderPassClear->addDependency(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, false);
	// Sync color with load op
	renderPassClear->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
	// Sync depth with load op
	renderPassClear->addDependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, false);
	int colorID = renderPassClear->attach(VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	int depthID = renderPassClear->attach(VK_FORMAT_D24_UNORM_S8_UINT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, false);
	renderPassClear->setupClear(colorID, {0.f, 0.f, 0.f, 0.f});
	renderPassClear->bindColor(colorID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	renderPassClear->bindDepth(depthID, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	renderPassClear->setupClear(depthID, 1.f);
	renderPassClear->pushLayer();
	renderPassClear->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, false);
	renderPassClear->build(1);

	renderPassReuse = std::make_unique<RenderMgr>(vkmgr);
	renderPassReuse->addDependency(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, 0);
	// Sync color with load op
	renderPassReuse->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
	// Sync depth with load op
	renderPassReuse->addDependency(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, false);
	colorID = renderPassReuse->attach(VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	depthID = renderPassReuse->attach(VK_FORMAT_D24_UNORM_S8_UINT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, false);
	renderPassReuse->addDependency(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	renderPassReuse->bindColor(colorID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	renderPassReuse->bindDepth(depthID, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	renderPassReuse->setupClear(depthID, 1.f);
	renderPassReuse->pushLayer();
	renderPassReuse->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, false);
	renderPassReuse->build(1);

	depthBuffer = std::make_unique<Texture>(vkmgr, vkmgr.getScreenRect().extent.width, vkmgr.getScreenRect().extent.height);
	depthBuffer->use();
	framebufferClear = std::make_unique<FrameMgr>(vkmgr, *renderPassClear, 0, width, height, "hip_star single");
	framebufferClear->bind(colorID, *context.starColorAttachment);
	framebufferClear->bind(depthID, *depthBuffer);
	framebufferClear->build();
	framebufferReuse = std::make_unique<FrameMgr>(vkmgr, *renderPassReuse, 0, width, height, "hip_star combine");
	framebufferReuse->bind(colorID, *context.starColorAttachment);
	framebufferReuse->bind(depthID, *depthBuffer);
	framebufferReuse->build();

	syncUse = std::make_unique<SyncEvent>();
	syncUse->imageBarrier(*context.starColorAttachment,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR);
	syncUse->build();
	syncClear = std::make_unique<SyncEvent>(&vkmgr);
	syncClear->imageBarrier(*context.starColorAttachment,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
	syncClear->build();
	syncReuse = std::make_unique<SyncEvent>(&vkmgr);
	syncReuse->imageBarrier(*context.starColorAttachment,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT_KHR | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR);
	syncReuse->build();

	m_drawFBO_GL = std::make_unique<VertexArray>(vkmgr);
	m_drawFBO_GL->createBindingEntry(4 * sizeof(float));
	m_drawFBO_GL->addInput(VK_FORMAT_R32G32_SFLOAT); // POS2D
	m_drawFBO_GL->addInput(VK_FORMAT_R32G32_SFLOAT); // TEXTURE
	vertexFBO = m_drawFBO_GL->createBuffer(0, 4, context.globalBuffer.get());
	float *data = (float *) context.transfer->planCopy(vertexFBO->get());
	vertexFBO->fillEntry(2, 4, dataPos, data);
	vertexFBO->fillEntry(2, 4, dataTex, data + 2);

	// shader for the stars
	m_starsGL = std::make_unique<VertexArray>(vkmgr);
	m_starsGL->createBindingEntry(6 * sizeof(float));
	m_starsGL->addInput(VK_FORMAT_R32G32_SFLOAT); // POS2D
	m_starsGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // COLOR
	m_starsGL->addInput(VK_FORMAT_R32_SFLOAT); // MAG
	vertexStars = m_starsGL->createBuffer(0, NBR_MAX_STARS, context.globalBuffer.get());
	staging[0] = context.stagingMgr->acquireBuffer(vertexStars->get().size);
	staging[1] = context.stagingMgr->acquireBuffer(vertexStars->get().size);

	auto samplerInfo = PipelineLayout::DEFAULT_SAMPLER;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	m_layoutStars = std::make_unique<PipelineLayout>(vkmgr);
	m_layoutStars->setTextureLocation(0, &samplerInfo);
	m_layoutStars->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, 1);
	m_layoutStars->buildLayout();
	m_layoutStars->build();
	m_setStars = std::make_unique<Set>(vkmgr, *context.setMgr, m_layoutStars.get());
	m_setStars->bindUniform(*UBOCam::ubo, 1);

	m_layoutFBO = std::make_unique<PipelineLayout>(vkmgr);
	samplerInfo.magFilter = samplerInfo.minFilter = VK_FILTER_NEAREST;
	m_layoutFBO->setTextureLocation(0, &samplerInfo);
	m_layoutFBO->buildLayout();
	m_layoutFBO->build();
	m_setFBO = std::make_unique<Set>(vkmgr, *context.setMgr, m_layoutFBO.get());
	m_setFBO->bindTexture(*context.starColorAttachment, 0);

	VkPipelineColorBlendAttachmentState blendMode = BLEND_ADD;
	blendMode.colorBlendOp = blendMode.alphaBlendOp = VK_BLEND_OP_MAX;
	VkPipelineViewportStateCreateInfo viewportState {};
	VkViewport viewport{};
	VkRect2D scissor{};
	viewport.width = width;
    viewport.height = -height;
	viewport.y = height;
	viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
	scissor.offset = {0, 0};
	scissor.extent = {(uint32_t) width, (uint32_t) height};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
	pipelineStarsClear = std::make_unique<Pipeline>(vkmgr, *renderPassClear, 0, m_layoutStars.get());
	pipelineStarsClear->bindVertex(*m_starsGL);
	pipelineStarsClear->setBlendMode(blendMode);
	pipelineStarsClear->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipelineStarsClear->bindShader("stars.vert.spv");
	pipelineStarsClear->bindShader("stars.geom.spv");
	pipelineStarsClear->bindShader("stars.frag.spv");
	pipelineStarsClear->setViewportState(&viewportState);
	pipelineStarsClear->build();

	pipelineStarsReuse = std::make_unique<Pipeline>(vkmgr, *renderPassReuse, 0, m_layoutStars.get());
	pipelineStarsReuse->bindVertex(*m_starsGL);
	pipelineStarsReuse->setBlendMode(blendMode);
	pipelineStarsReuse->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipelineStarsReuse->bindShader("stars.vert.spv");
	pipelineStarsReuse->bindShader("stars.geom.spv");
	pipelineStarsReuse->bindShader("stars.frag.spv");
	pipelineStarsReuse->setViewportState(&viewportState);
	pipelineStarsReuse->build();

	m_pipelineFBO = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_BACKGROUND, m_layoutFBO.get());
	m_pipelineFBO->setDepthStencilMode();
	m_pipelineFBO->bindVertex(*m_drawFBO_GL);
	m_pipelineFBO->setBlendMode(BLEND_ADD);
	m_pipelineFBO->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	m_pipelineFBO->bindShader("fbo.vert.spv");
	m_pipelineFBO->bindShader("fbo.frag.spv");
	m_pipelineFBO->build();

	previousSync.emplace(STAR_UNINITIALIZED);
}

HipStarMgr::~HipStarMgr(void)
{
	ZoneArrayMap::iterator it(zone_arrays.end());
	while (it!=zone_arrays.begin()) {
		--it;
		delete it->second;
		it->second = nullptr;
	}
	zone_arrays.clear();
	if (mag_converter) {
		delete mag_converter;
		mag_converter = 0;
	}
	if (hip_index) delete[] hip_index;
	if (starTexture) delete starTexture;
	// if (font) delete font;

	// dataColor.clear();
	// dataMag.clear();
	// dataPos.clear();

	// deleteShader();
}

// void HipStarMgr::deleteShader()
// {
// 	if (shaderStars)
// 		delete shaderStars;
// 	shaderStars =  nullptr;

// 	if (shaderFBO)
// 		delete shaderFBO;
// 	shaderFBO = nullptr;

	// glDeleteBuffers(1,&drawFBO.tex);
	// glDeleteBuffers(1,&drawFBO.pos);
	// glDeleteVertexArrays(1,&drawFBO.vao);
// }

std::string HipStarMgr::getCommonName(int hip)
{
	std::map<int,std::string>::const_iterator it(common_names_map_i18n.find(hip));
	if (it!=common_names_map_i18n.end()) return it->second;
	return "";
}

std::string HipStarMgr::getSciName(int hip)
{
	std::map<int,std::string>::const_iterator it(sci_names_map_i18n.find(hip));
	if (it!=sci_names_map_i18n.end()) return it->second;
	return "";
}

void HipStarMgr::init(const InitParser &conf)
{
	load_data(conf);
	InitColorTableFromConfigFile(conf);
	// Load star texture no mipmap:
	starTexture = new s_texture("star16x16.png",TEX_LOAD_TYPE_PNG_SOLID,false);  // Load star texture no mipmap
	m_setStars->bindTexture(starTexture->getTexture(), 0);
	// starFont = new s_font(font_size, font_name);
	// if (!starFont) {
	// 	cLog::get()->write("HipStarMgr: Can't create starFont", LOG_TYPE::L_ERROR);
	// 	assert(0);
	// }
}

void HipStarMgr::setGrid(GeodesicGrid* geodesic_grid)
{
	geodesic_grid->visitTriangles(max_geodesic_grid_level,initTriangleFunc,this);
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		it->second->scaleAxis();
	}
}

/***************************************************************************
 Load star catalogue data from files.
 If a file is not found, it will be skipped.
***************************************************************************/
void HipStarMgr::load_data(const InitParser &baseConf)
{
	// Please do not init twice:
	assert(max_geodesic_grid_level < 0);

	cLog::get()->write( "Loading star data..." , LOG_TYPE::L_INFO);

	InitParser conf;
	conf.load(AppSettings::Instance()->getConfigDir() + "stars.ini");

	for (int i=0; i<9; i++) {
		char key_name[64];
		sprintf(key_name,"cat_file_name_%02d",i);
		const std::string cat_file_name = conf.getStr("stars",key_name).c_str();
		if (!cat_file_name.empty()) {
			cLog::get()->write(_("Loading catalog ") + std::string(cat_file_name) , LOG_TYPE::L_INFO);
			BigStarCatalog::ZoneArray *const z = BigStarCatalog::ZoneArray::create(*this,cat_file_name);
			if (z) {
				if (max_geodesic_grid_level < z->level) {
					max_geodesic_grid_level = z->level;
				}
				BigStarCatalog::ZoneArray *&pos(zone_arrays[z->level]);
				if (pos) {
					std::cerr << cat_file_name << ", " << z->level << ": duplicate level" << std::endl;
					delete z;
				} else {
					pos = z;
				}
			}
		}
	}

	for (int i=0; i<=NR_OF_HIP; i++) {
		hip_index[i].a = 0;
		hip_index[i].z = 0;
		hip_index[i].s = 0;
	}
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin());
	        it != zone_arrays.end(); it++) {
		it->second->updateHipIndex(hip_index);
	}

	const std::string cat_hip_sp_file_name = conf.getStr("stars","cat_hip_sp_file_name").c_str();
	if (cat_hip_sp_file_name.empty()) {
		std::cerr << "ERROR: stars:cat_hip_sp_file_name not found" << std::endl;
	} else {
		try {
			spectral_array.initFromFile(AppSettings::Instance()->getDataRoot() + "stars/" + cat_hip_sp_file_name);
		} catch (std::exception& e) {
			std::cerr << "ERROR while loading data from " << ("stars/" + cat_hip_sp_file_name) << ": " << e.what();
		}
	}

	const std::string cat_hip_cids_file_name = conf.getStr("stars","cat_hip_cids_file_name").c_str();
	if (cat_hip_cids_file_name.empty()) {
		std::cerr << "ERROR: stars:cat_hip_cids_file_name not found" << std::endl;
	} else {
		try {
			component_array.initFromFile(AppSettings::Instance()->getDataRoot() + "stars/" + cat_hip_cids_file_name);
		} catch (std::exception& e) {
			std::cerr << "ERROR while loading data from " << ("stars/" + cat_hip_cids_file_name) << ": " << e.what() << std::endl;
		}
	}

	last_max_search_level = max_geodesic_grid_level;
	std::ostringstream oss;
	oss <<  "finished, max_geodesic_level: " << max_geodesic_grid_level;
	cLog::get()->write( oss.str() , LOG_TYPE::L_INFO);
	cLog::get()->mark();
}

//! Load common names from file
int HipStarMgr::loadCommonNames(const std::string& commonNameFile)
{
	common_names_map.clear();
	common_names_map_i18n.clear();
	common_names_index.clear();
	common_names_index_i18n.clear();

	cLog::get()->write("Loading star names from " + commonNameFile);

	FILE *cnFile=fopen(commonNameFile.c_str(),"r");
	if (!cnFile) {
		std::cerr << "Warning " << commonNameFile << " not found." << std::endl;
		return 0;
	}

	// Assign names to the matching stars, now support spaces in names
	char line[256];
	while (fgets(line, sizeof(line), cnFile)) {
		line[sizeof(line)-1] = '\0';
		unsigned int hip;
		if (sscanf(line,"%u",&hip)!=1) {
			std::cerr << "ERROR: StarMgr::loadCommonNames(" << commonNameFile << "): bad line: \"" << line << '"' << std::endl;
			return 0;
		}
		unsigned int i = 0;
		while (line[i]!='|' && i<sizeof(line)-2) ++i;
		i++;
		std::string englishCommonName =  line+i;
		// remove newline
		englishCommonName.erase(englishCommonName.length()-1, 1);

		// remove underscores
		for (std::string::size_type j=0; j<englishCommonName.length(); ++j) {
			if (englishCommonName[j]=='_') englishCommonName[j]=' ';
		}
		const std::string commonNameI18n = _(englishCommonName.c_str());
		std::string commonNameI18n_cap = commonNameI18n;
		transform(commonNameI18n.begin(), commonNameI18n.end(), commonNameI18n_cap.begin(), ::toupper);

		common_names_map[hip] = englishCommonName;
		common_names_index[englishCommonName] = hip;
		common_names_map_i18n[hip] = commonNameI18n;
		common_names_index_i18n[commonNameI18n_cap] = hip;

	}
	fclose(cnFile);
	return 1;
}


//! Load scientific names from file
void HipStarMgr::loadSciNames(const std::string& sciNameFile)
{
	sci_names_map_i18n.clear();
	sci_names_index_i18n.clear();

	cLog::get()->write("Loading star sci names from " + sciNameFile);

	FILE *snFile;
	snFile=fopen(sciNameFile.c_str(),"r");
	if (!snFile) {
		std::cerr << "Warning " << sciNameFile << " not found." << std::endl;
		return;
	}

	// Assign names to the matching stars, now support spaces in names
	char line[256];
	while (fgets(line, sizeof(line), snFile)) {
		line[sizeof(line)-1] = '\0';
		unsigned int hip;
		if (sscanf(line,"%u",&hip)!=1) {
			std::cerr << "ERROR: StarMgr::loadSciNames(" << sciNameFile << "): bad line: \"" << line << '"' << std::endl;
			return;
		}
		if (sci_names_map_i18n.find(hip)!=sci_names_map_i18n.end())
			continue;
		unsigned int i = 0;
		while (line[i]!='|' && i<sizeof(line)-2) ++i;
		i++;
		char *tempc = line+i;
		std::string sci_name = tempc;
		sci_name.erase(sci_name.length()-1, 1);
		std::string sci_name_i18n = sci_name;

		// remove underscores
		for (std::string::size_type j=0; j<sci_name_i18n.length(); ++j) {
			if (sci_name_i18n[j]==L'_') sci_name_i18n[j]=L' ';
		}

		std::string sci_name_i18n_cap = sci_name_i18n;
		transform(sci_name_i18n.begin(), sci_name_i18n.end(), sci_name_i18n_cap.begin(), ::toupper);

		sci_names_map_i18n[hip] = sci_name_i18n;
		sci_names_index_i18n[sci_name_i18n_cap] = hip;
	}

	fclose(snFile);
}

int HipStarMgr::drawStar(const Projector *prj,const Vec3d &XY, const float rc_mag[2], const Vec3f &color)
{
	if (rc_mag[0]<=0.f || rc_mag[1]<=0.f || nbStarsToDraw[drawIdx] >= NBR_MAX_STARS) return -1;

	float mag = 2.f*rc_mag[0];

	// Roll off star size limit as fov decreases to match planet halo scale
	RangeMap<float> rmap(180, 1, -starSizeLimit, -(starSizeLimit + objectSizeLimit));
	float rolloff = -rmap.Map(prj->getFov());
	if( mag > rolloff )
		mag = rolloff;

	*(vertexData++) = XY[0];
	*(vertexData++) = XY[1];
	*(vertexData++) = color[0]*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX);
	*(vertexData++) = color[1]*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX);
	*(vertexData++) = color[2]*rc_mag[1]*(1.-twinkle_amount*rand()/RAND_MAX);
	*(vertexData++) = mag;

	nbStarsToDraw[drawIdx] += 1;

	return 0;
}

int HipStarMgr::getMaxSearchLevel(const ToneReproductor *eye, const Projector *prj) const
{
	int rval = -1;
	mag_converter->setFov(prj->getFov());
	mag_converter->setEye(eye);
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		const float mag_min = 0.001f*it->second->mag_min;
		float rcmag[2];
		if (mag_converter->computeRCMag(mag_min, eye, rcmag) < 0) break;
		rval = it->first;
	}
	return rval;
}

void MagConverter::setFov(float fov)
{
	if (fov > max_fov) fov = max_fov;
	else if (fov < min_fov) fov = min_fov;
	fov_factor = 108064.73f / (fov*fov);
}

void MagConverter::setEye(const ToneReproductor *eye)
{
	min_rmag
	    = std::sqrt(eye->adaptLuminance(
	                    std::exp(-0.92103f*(max_scaled_60deg_mag + mag_shift + 12.12331f))
	                    * (108064.73f / 3600.f))) * 30.f;
}

int MagConverter::computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2]) const
{
	if (mag > max_mag) {
		rc_mag[0] = rc_mag[1] = 0.f;
		return -1;
	}

	// rmag:
	rc_mag[0] = std::sqrt(
	                eye->adaptLuminance(
	                    std::exp(-0.92103f*(mag + mag_shift + 12.12331f)) * fov_factor))
	            * 30.f;

	if (rc_mag[0] < min_rmag) {
		rc_mag[0] = rc_mag[1] = 0.f;
		return -1;
	}

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rc_mag[0]<1.2f) {
		if (rc_mag[0] * mgr.getScale() < 0.1f) {
			rc_mag[0] = rc_mag[1] = 0.f;
			return -1;
		}
		rc_mag[1] = rc_mag[0] * rc_mag[0] / 1.44f;
		if (rc_mag[1] * mgr.getMagScale() < 0.1f) {
			rc_mag[0] = rc_mag[1] = 0.f;
			return -1;
		}
		rc_mag[0] = 1.2f;
	} else {
		// cmag:
		rc_mag[1] = 1.f;
		if (rc_mag[0] > mgr.getStarSizeLimit()) {
			rc_mag[0] = mgr.getStarSizeLimit();
		}
	}
	// Global scaling
	rc_mag[0] *= mgr.getScale();
	rc_mag[1] *= mgr.getMagScale();

	return 0;
}

//! Draw all the stars
double HipStarMgr::preDraw(GeodesicGrid* grid, ToneReproductor* eye, Projector* prj, Navigator* nav, TimeMgr* timeMgr, float altitude, bool atmosphere)
{
	starNameToDraw.clear();
	double twinkle_param=1.;
	nbStarsToDraw[drawIdx] = 0;
	if (altitude>2000)
		twinkle_param=std::max(0.,1.-(altitude-2000.)/50000.);
	current_JDay = timeMgr->getJulian();

	// If stars are turned off don't waste time below projecting all stars just to draw disembodied labels
	if (fader.isZero())
		return 0.;
	vertexData = (float *) Context::instance->stagingMgr->getPtr(staging[drawIdx]);

	int max_search_level = getMaxSearchLevel(eye, prj);
	const GeodesicSearchResult* geodesic_search_result = grid->search(prj->unprojectViewport(),max_search_level);

	mag_converter->setFov(prj->getFov());
	mag_converter->setEye(eye);

	// Set temporary static variable for optimization
	if (flagStarTwinkle) twinkle_amount = twinkleAmount*twinkle_param;
	else twinkle_amount = 0;
	const float names_brightness = fader * names_fader;

	float rcmag_table[2*256];

	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		const float mag_min = 0.001f*it->second->mag_min;

		const float k = (0.001f*it->second->mag_range)/it->second->mag_steps;
		for (int i=it->second->mag_steps-1; i>=0; i--) {
			const float mag = mag_min+k*i;
			if (mag_converter->computeRCMag(mag, eye, rcmag_table + 2*i) < 0) {
				if (i==0) {
					return 0.; //goto exit_loop;
				}
			}
			rcmag_table[2*i] *= fader;
		}
		last_max_search_level = it->first;

		unsigned int max_mag_star_name = 0;
		if (names_fader.isNonZero()) {
			int x = (int)((maxMagStarName-mag_min)/k);
			if (x > 0) max_mag_star_name = x;
		}
		int zone=0;
		for (GeodesicSearchInsideIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->draw(zone,true,rcmag_table,prj,nav,max_mag_star_name,names_brightness, starNameToDraw,  selected_star, atmosphere, isolateSelected && !selected_star.empty(), hide_stars);
		}
		for (GeodesicSearchBorderIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->draw(zone,false,rcmag_table,prj,nav,max_mag_star_name,names_brightness, starNameToDraw,  selected_star, atmosphere, isolateSelected && !selected_star.empty(), hide_stars);
		}

	}
	return 1.;
}

double HipStarMgr::draw(GeodesicGrid* grid, ToneReproductor* eye, Projector* prj, TimeMgr* timeMgr, float altitude)
{
	if (nbStarsToDraw[drawIdx]==0)
		return 0.;

	previousSync.emplace(starTrace ? STAR_STORE : STAR_CLEAR);
	previousSync.emplace(starTrace ? STAR_STORE : STAR_CLEAR);
	Context &context = *Context::instance;

	// 6 * sizeof(float) is the vertex stride
	context.transfer->planCopyBetween(staging[drawIdx], vertexStars->get(), nbStarsToDraw[drawIdx] * 6 * sizeof(float));
	if (cmds[context.frameIdx] == -1) {
		cmds[context.frameIdx] = context.frame[context.frameIdx]->create(1);
		context.frame[context.frameIdx]->setName(cmds[context.frameIdx], "HipStar draw FBO");
	}
	VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_BACKGROUND);
	m_pipelineFBO->bind(cmd);
	m_layoutFBO->bindSet(cmd, *m_setFBO);
	vertexFBO->bind(cmd);
	vkCmdDraw(cmd, 4, 1, 0, 0);
	// srcDependency and resetDependency can't be performed inside a RenderPass, report them later
	context.frame[context.frameIdx]->compile(cmd);
	context.frame[context.frameIdx]->toExecute(cmd, PASS_BACKGROUND);
	this->drawStarName(prj);
	context.starUsed[context.frameIdx] = this;
	context.waitFrameSync[1].stageMask |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
	context.signalFrameSync[1].stageMask |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR;
	drawIdx = !drawIdx;
	return 0.;
}

void HipStarMgr::updateFramebuffer(VkCommandBuffer cmd)
{
	previousSync.pop(lastSync);
	previousSync.pop(nextSync);
	switch (lastSync) {
		case STAR_UNINITIALIZED:
			renderPassClear->begin(0, cmd); // 0 is the index of the single FrameMgr created from this renderPass
			pipelineStarsClear->bind(cmd);
			break;
		case STAR_CLEAR:
			// syncClear->dstDependency(cmd);
			// syncClear->resetDependency(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
			renderPassClear->begin(0, cmd); // 0 is the index of the single FrameMgr created from this renderPass
			pipelineStarsClear->bind(cmd);
			break;
		case STAR_STORE:
			// syncReuse->dstDependency(cmd);
			// syncReuse->resetDependency(cmd, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
			renderPassReuse->begin(0, cmd); // 0 is the index of the single FrameMgr created from this renderPass
			pipelineStarsReuse->bind(cmd);
			break;
	}
	m_layoutStars->bindSet(cmd, *m_setStars);
	vertexStars->bind(cmd);
	vkCmdDraw(cmd, nbStarsToDraw[submitIdx], 1, 0, 0);
	submitIdx = !submitIdx;
	vkCmdEndRenderPass(cmd);
	syncUse->placeBarrier(cmd);
}

void HipStarMgr::syncFramebuffer(VkCommandBuffer cmd)
{
	// switch (nextSync) {
	// 	case STAR_UNINITIALIZED:
	// 		break;
	// 	case STAR_CLEAR:
	// 		syncClear->srcDependency(cmd);
	// 		break;
	// 	case STAR_STORE:
	// 		syncReuse->srcDependency(cmd);
	// 		break;
	// }
}

void HipStarMgr::drawStarName( Projector* prj )
{
	for (auto const& token : starNameToDraw) {
		prj->printGravity180(font, std::get<0>(token), std::get<1>(token), std::get<2>(token), std::get<3>(token), 4,4);
		//  prj->printGravity180(starFont,xy[0],xy[1], starname, Color, true, 4, 4);//, false);
	}
	//cout << "Number of the names to print : " << starNameToDraw.size() << endl;
}


//! Look for a star by XYZ coords
ObjectBaseP HipStarMgr::search(Vec3d pos) const
{
	assert(0);
	pos.normalize();
	std::vector<ObjectBaseP > v = searchAround(pos, 0.8, nullptr);
	ObjectBaseP nearest;
	double cos_angle_nearest = -10.0;
	for (std::vector<ObjectBaseP >::const_iterator it(v.begin()); it!=v.end(); it++) {
		const double c = (*it)->getObsJ2000Pos(0)*pos;
		if (c > cos_angle_nearest) {
			cos_angle_nearest = c;
			nearest = *it;
		}
	}
	return nearest;
}

//! Return a stl vector containing the stars located
//! inside the lim_fov circle around position v
std::vector<ObjectBaseP > HipStarMgr::searchAround(const Vec3d& vv, double lim_fov, const GeodesicGrid* grid) const
{
	std::vector<ObjectBaseP > result;
	if (!getFlagShow())
		return result;

	Vec3d v(vv);
	v.normalize();
	// find any vectors h0 and h1 (length 1), so that h0*v=h1*v=h0*h1=0
	int i;
	{
		const double a0 = fabs(v[0]);
		const double a1 = fabs(v[1]);
		const double a2 = fabs(v[2]);
		if (a0 <= a1) {
			if (a0 <= a2) i = 0;
			else i = 2;
		} else {
			if (a1 <= a2) i = 1;
			else i = 2;
		}
	}
	Vec3d h0(0.0,0.0,0.0);
	h0[i] = 1.0;
	Vec3d h1 = h0 ^ v;
	h1.normalize();
	h0 = h1 ^ v;
	h0.normalize();
	// now we have h0*v=h1*v=h0*h1=0.
	// construct a region with 4 corners e0,e1,e2,e3 inside which
	// all desired stars must be:
	double f = 1.4142136 * tan(lim_fov * M_PI/180.0);
	h0 *= f;
	h1 *= f;
	Vec3d e0 = v + h0;
	Vec3d e1 = v + h1;
	Vec3d e2 = v - h0;
	Vec3d e3 = v - h1;
	f = 1.0/e0.length();
	e0 *= f;
	e1 *= f;
	e2 *= f;
	e3 *= f;
	// search the triangles
	const GeodesicSearchResult* geodesic_search_result = grid->search(e0,e1,e2,e3,last_max_search_level);
	// iterate over the stars inside the triangles:
	f = cos(lim_fov * M_PI/180.);
	for (ZoneArrayMap::const_iterator it(zone_arrays.begin()); it!=zone_arrays.end(); it++) {
		int zone;
		for (GeodesicSearchInsideIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->searchAround(zone,v,f,result);
		}
		for (GeodesicSearchBorderIterator it1(*geodesic_search_result,it->first); (zone = it1.next()) >= 0;) {
			it->second->searchAround(zone,v,f,result);
		}
	}
	return result;
}

//! Update i18 names from english names according to passed translator.
//! The translation is done using gettext with translated strings defined in translations.h
void HipStarMgr::updateI18n(Translator& trans)
{
	common_names_map_i18n.clear();
	common_names_index_i18n.clear();
	for (std::map<int,std::string>::iterator it(common_names_map.begin()); it!=common_names_map.end(); it++) {
		const int i = it->first;
		const std::string t(trans.translateUTF8(it->second));
		common_names_map_i18n[i] = t;
		std::string t_cap = t;
		transform(t.begin(), t.end(), t_cap.begin(), ::toupper);
		common_names_index_i18n[t_cap] = i;
	}
}

ObjectBaseP HipStarMgr::search(const std::string& name) const
{
	const std::string catalogs("HP HD SAO");

	std::string n = name;
	for (std::string::size_type i=0; i<n.length(); ++i) {
		if (n[i]=='_') n[i]=' ';
	}

	std::istringstream ss(n);
	std::string cat;
	unsigned int num;

	ss >> cat;

	// check if a valid catalog reference
	if (catalogs.find(cat,0) == std::string::npos) {
		// try see if the string is a HP number
		std::istringstream cat_to_num(cat);
		cat_to_num >> num;
		if (!cat_to_num.fail()) return searchHP(num);
		return nullptr;
	}

	ss >> num;
	if (ss.fail()) return nullptr;

	if (cat == "HP") return searchHP(num);
	assert(0);
	return nullptr;
}

//! Search the star by HP number
ObjectBaseP HipStarMgr::searchHP(int _HP) const
{
	if (0 < _HP && _HP <= NR_OF_HIP) {
		const BigStarCatalog::Star1 *const s = hip_index[_HP].s;
		if (s) {
			const BigStarCatalog::SpecialZoneArray<BigStarCatalog::Star1> *const a = hip_index[_HP].a;
			const BigStarCatalog::SpecialZoneData<BigStarCatalog::Star1> *const z = hip_index[_HP].z;
			return s->createStelObject(a,z);
		}
	}
	return ObjectBaseP();
}

ObjectBaseP HipStarMgr::searchByNameI18n(const std::string& nameI18n) const
{
	std::string objw = nameI18n;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search by HP number if it's an HP formated number
	// Please help, if you know a better way to do this:
	if (nameI18n.length() >= 2 && nameI18n[0]=='H' && nameI18n[1]=='P') {
		bool hp_ok = false;
		std::string::size_type i=2;
		// ignore spaces
		for (; i<nameI18n.length(); i++) {
			if (nameI18n[i] != ' ') break;
		}
		// parse the number
		unsigned int nr = 0;
		for (; i<nameI18n.length(); i++) {
			if ( (hp_ok = ('0' <= nameI18n[i] && nameI18n[i] <= '9')) ) {
				nr = 10*nr+(nameI18n[i]-'0');
			} else {
				break;
			}
		}
		if (hp_ok) {
			return searchHP(nr);
		}
	}

	// Search by I18n common name
	std::map<std::string,int>::const_iterator it(common_names_index_i18n.find(objw));
	if (it!=common_names_index_i18n.end()) {
		return searchHP(it->second);
	}

	// Search by sci name
	it = sci_names_index_i18n.find(objw);
	if (it!=sci_names_index_i18n.end()) {
		return searchHP(it->second);
	}

	return ObjectBaseP();
}
void HipStarMgr::setSelected(Object star) {
	auto it = selected_star.find(star.getNameI18n());
	if (it != selected_star.end()) {
		selected_star.erase(it);
	} else {
		selected_star.insert(std::pair<std::string, bool>(star.getNameI18n(), true));
	}

	int HP = getHPFromStarName(star.getNameI18n());
	if (HP >= 0) {
		selected_stars.push_back(HP);
	}
}

int HipStarMgr::getHPFromStarName(const std::string& name) const {
	std::string objw = name;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);
	// Search by HP number if it's an HP formated number
	// Please help, if you know a better way to do this:
	if (name.length() >= 2 && name[0]=='H' && name[1]=='P') {
		bool hp_ok = false;
		std::string::size_type i=2;
		// ignore spaces
		for (; i<name.length(); i++) {
			if (name[i] != ' ') break;
		}
		// parse the number
		unsigned int nr = 0;
		for (; i<name.length(); i++) {
			if ( (hp_ok = ('0' <= name[i] && name[i] <= '9')) ) {
				nr = 10*nr+(name[i]-'0');
			} else {
				break;
			}
		}
		if (hp_ok) {
			return(nr);
		}
	}
	std::map<std::string,int>::const_iterator il(common_names_index_i18n.find(objw));
	if (il!=common_names_index_i18n.end()) {
		return il->second;
	} else {
		return -1;
	}
}

ObjectBaseP HipStarMgr::searchByName(const std::string& name) const
{
	int HP = getHPFromStarName(name);
	if (HP >= 0) {
		return searchHP(HP);
	}

	return ObjectBaseP();
}

//! Find and return the list of at most maxNbItem objects auto-completing
//! the passed object I18n name
std::vector<std::string> HipStarMgr::listMatchingObjectsI18n( const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	if (maxNbItem==0) return result;

	std::string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	// Search for common names
	for (std::map<std::string,int>::const_iterator it(common_names_index_i18n.lower_bound(objw)); it!=common_names_index_i18n.end(); it++) {
		const std::string constw(it->first.substr(0,objw.size()));
		if (constw == objw) {
			if (maxNbItem == 0) break;
			result.push_back(getCommonName(it->second));
			maxNbItem--;
		} else {
			break;
		}
	}
	// Search for sci names
	for (std::map<std::string,int>::const_iterator it(sci_names_index_i18n.lower_bound(objw)); it!=sci_names_index_i18n.end(); it++) {
		const std::string constw(it->first.substr(0,objw.size()));
		if (constw == objw) {
			if (maxNbItem == 0) break;
			result.push_back(getSciName(it->second));
			maxNbItem--;
		} else {
			break;
		}
	}

	sort(result.begin(), result.end());

	return result;
}

void HipStarMgr::add_hide_stars(int hip)
{
	std::string name = getCommonName(hip);
	auto it = hide_stars.find(name);
	if (it != hide_stars.end()) {
		it->second = true;
	} else {
		hide_stars.insert(std::pair<std::string, bool>(name, true));
	}
}

void HipStarMgr::remove_hide_stars(int hip)
{
	std::string name = getCommonName(hip);
	auto it = hide_stars.find(name);
	if (it != hide_stars.end())
		it->second = false;
}
