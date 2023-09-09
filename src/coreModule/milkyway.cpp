/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014-2016 of the LSS Team & Association Sirius
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

#include "coreModule/milkyway.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>
#include "ojmModule/objl.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

MilkyWay::MilkyWay()
{
	sphere = ObjLMgr::instance->selectDefault();
	switchTexFader = false;
	intensityMilky.set(0.f);
	pollum.set(0.f);
	createSC_context();
	initModelMatrix();
}

void MilkyWay::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	auto tmp = PipelineLayout::DEFAULT_SAMPLER;
	tmp.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setTextureLocation(0, &tmp);
	layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 8);
	layout->buildLayout();
	layout->build();
	layoutTwoTex = std::make_unique<PipelineLayout>(vkmgr);
	layoutTwoTex->setGlobalPipelineLayout(context.layouts.front().get());
	layoutTwoTex->setTextureLocation(0, &tmp);
	layoutTwoTex->setTextureLocation(1, &tmp);
	layoutTwoTex->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, 64);
	layoutTwoTex->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 64, 12);
	layoutTwoTex->buildLayout();
	layoutTwoTex->build();
	pipelineMilky = new Pipeline[2]{{vkmgr, *context.render, PASS_BACKGROUND, layoutTwoTex.get()}, {vkmgr, *context.render, PASS_BACKGROUND, layout.get()}};
	for (int i = 0; i < 2; ++i) {
		pipelineMilky[i].setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineMilky[i].setDepthStencilMode();
		pipelineMilky[i].setCullMode(true);
		pipelineMilky[i].setFrontFace();
		pipelineMilky[i].setBlendMode(BLEND_NONE);
		sphere->bind(pipelineMilky[i]);
		pipelineMilky[i].removeVertexEntry(2);
		pipelineMilky[i].bindShader("milkyway.vert.spv");
		pipelineMilky[i].setSpecializedConstant(7, context.isFloat64Supported);
		pipelineMilky[i].bindShader("milkyway.geom.spv");
		pipelineMilky[i].bindShader(i == 0 ? "milkywayTwoTex.frag.spv" : "milkywayOneTex.frag.spv");
		pipelineMilky[i].build();
	}
	for (int i = 0; i < 3; ++i) {
		cmds[i] = context.frame[i]->create(1);
		context.frame[i]->setName(cmds[i], "MilkyWay " + std::to_string(i));
	}
}

void MilkyWay::initModelMatrix()
{
	modelMilkyway = Mat4d::scaling(1.1) *
	                Mat4d::xrotation(M_PI)*
	                Mat4d::yrotation(M_PI)*
	                Mat4d::zrotation(M_PI/180*270);

	modelZodiacal = Mat4d::scaling(1.0) *
		            Mat4d::xrotation(M_PI*23.5/180.0)*
		            Mat4d::yrotation(M_PI);
}

MilkyWay::~MilkyWay()
{
	if (sphere) delete sphere;
	if (pipelineMilky) delete[] pipelineMilky;
}

void MilkyWay::defineZodiacalState(const std::string& tex_file, float _intensity)
{
	if (zodiacal.tex==nullptr) { //fist time to read this texture
		zodiacal.tex = std::make_unique<s_texture>(tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
		zodiacal.intensity = std::clamp(_intensity, 0.f, 1.f);
		zodiacal.name = tex_file;
		buildZodiacal();
	} else {
		cLog::get()->write("Milkyway: zodicalState already exist, function aborded" , LOG_TYPE::L_WARNING);
	}
}

void MilkyWay::defineInitialMilkywayState(const std::string& path_file,const std::string& tex_file, const std::string& iris_tex_file, float _intensity)
{
	if (defaultMilky.tex==nullptr) {
		defaultMilky.tex = std::make_unique<s_texture>(path_file + tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
		defaultMilky.intensity = std::clamp(_intensity, 0.f, 1.f);
		defaultMilky.name = path_file +tex_file;
		currentMilky.tex = std::make_unique<s_texture>(path_file + tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
		currentMilky.intensity =  std::clamp(_intensity, 0.f, 1.f);
		currentMilky.name = path_file +tex_file;
		intensityMilky.set(currentMilky.intensity);

		if (useIrisMilky && !iris_tex_file.empty()) {
			irisMilky.tex = std::make_unique<s_texture>(path_file + iris_tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
			irisMilky.intensity =  std::clamp(_intensity, 0.f, 1.f);
			irisMilky.name = path_file + iris_tex_file;
			cLog::get()->write("Milkyway: define irisMilky, name "+ iris_tex_file, LOG_TYPE::L_DEBUG);
		} else
			cLog::get()->write("Milkyway: no irisMilky define" , LOG_TYPE::L_DEBUG);
		buildMilkyway();
	} else {
		cLog::get()->write("Initial textures already define, function aborded", LOG_TYPE::L_WARNING);
	}
}

void MilkyWay::changeMilkywayStateWithoutIntensity(const std::string& full_tex_file)
{
	this->changeMilkywayState(full_tex_file,intensityMilky.final());
}

void MilkyWay::changeMilkywayState(const std::string& tex_file, float _intensity)
{
	nextMilky.tex = std::make_unique<s_texture>(tex_file, TEX_LOAD_TYPE_PNG_BLEND1, true);
	nextMilky.intensity = _intensity;
	nextMilky.name = tex_file;
	onTextureTransition = true;
	switchTexFader = true;
	setIntensity(nextMilky.intensity);
	buildMilkyway();
}

void MilkyWay::restoreDefaultMilky()
{
	nextMilky.tex = std::make_unique<s_texture>(defaultMilky.name, TEX_LOAD_TYPE_PNG_BLEND1, true);
	nextMilky.name = defaultMilky.name;
	onTextureTransition = true;
	switchTexFader = true;
	setIntensity(defaultMilky.intensity);
	buildMilkyway();
	// cout << "Value of Intensity " << intensityMilky << endl;
}

void MilkyWay::endTexTransition()
{
	currentMilky.tex = std::move(nextMilky.tex);
	currentMilky.name = nextMilky.name;
	onTextureTransition = false;
	switchTexFader = false;
	buildMilkyway();
}

void MilkyWay::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav, double julianDay)
{
	if (showFader.getInterstate() <= 0)
		return;

	// .045 chosen so that ad_lum = 1 at standard NELM of 6.5
	float ad_lum=eye->adaptLuminance(.045);

	// NB The tone reproducer code is simply incorrect, so this function fades out the Milky Way by the time eye limiting mag gets to ~5
	if (ad_lum < 0) {
		ad_lum = 0;
	} else if (ad_lum < .9987)
		ad_lum = -ad_lum*3.6168 +ad_lum*ad_lum*9.6253 -ad_lum*ad_lum*ad_lum*5.0121;

	struct {
		float cmag;
		float pollum;
		float texTransit;
	} frag;
	frag.cmag = ad_lum * showFader.getInterstate() * intensityMilky;
	frag.pollum = pollum;

	if (onTextureTransition && (switchTexFader.getInterstate()>0.99))
		endTexTransition();

	Context &context = *Context::instance;
	VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_BACKGROUND);

	Mat4f matrix = (nav->getJ2000ToEyeMat() * modelMilkyway ).convert();

	if (onTextureTransition) {
		frag.texTransit = switchTexFader.getInterstate();
		pipelineMilky[0].bind(cmd);
		layoutTwoTex->bindSets(cmd, {*context.uboSet, *setMilky});
		layoutTwoTex->pushConstant(cmd, 0, &matrix);
		layoutTwoTex->pushConstant(cmd, 1, &frag);
	} else {
		pipelineMilky[1].bind(cmd);
		layout->pushConstant(cmd, 0, &matrix);
		layout->pushConstant(cmd, 1, &frag);
		if (displayIrisMilky && currentMilky.name == defaultMilky.name) {
			layout->bindSets(cmd, {*context.uboSet, *setIrisMilky});
		} else
			layout->bindSets(cmd, {*context.uboSet, *setMilky});
	}

	sphere->bind(cmd);
	sphere->draw(cmd, 4096);
	//nextZodiacalLight
	if (zodiacal.tex != nullptr && zodiacalFader.getInterstate() && allowZodiacal) {
		pipelineZodiacal->bind(cmd);
		frag.cmag = ad_lum * zodiacal.intensity * zodiacalFader.getInterstate();

		//	365.2422 c'est la période de révolution terrestre
		//	27.5 c'est le shift de la texture ça n'a aucun sens
		matrix = (nav->getJ2000ToEyeMat() * modelZodiacal *
		          Mat4d::zrotation(2*M_PI*(-julianDay+27.5)/365.2422)).convert();
		layout->pushConstant(cmd, 0, &matrix);
		layout->pushConstant(cmd, 1, &frag);
		layout->bindSet(cmd, *setZodiacal, 1);
		sphere->draw(cmd, 4096);
	}
	context.frame[context.frameIdx]->compile(cmd);
	context.frame[context.frameIdx]->toExecute(cmd, PASS_BACKGROUND);
}

void MilkyWay::buildMilkyway()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	if (onTextureTransition) {
		setMilky = std::make_unique<Set>(vkmgr, *context.setMgr, layoutTwoTex.get(), -1, false, true);
		setMilky->bindTexture(nextMilky.tex->getTexture(), 1);
		setIrisMilky.reset();
	} else {
		if (useIrisMilky && currentMilky.name == defaultMilky.name) {
			setIrisMilky = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get(), -1, false, true);
			setIrisMilky->bindTexture(irisMilky.tex->getTexture(), 0);
		}
		setMilky = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get(), -1, false, true);
	}
	setMilky->bindTexture(currentMilky.tex->getTexture(), 0);
}

void MilkyWay::buildZodiacal()
{
	// init context
	pipelineZodiacal = std::make_unique<Pipeline>(*VulkanMgr::instance, *Context::instance->render, PASS_BACKGROUND, layout.get());
	pipelineZodiacal->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineZodiacal->setDepthStencilMode();
	pipelineZodiacal->setCullMode(true);
	pipelineZodiacal->setFrontFace();
	sphere->bind(*pipelineZodiacal);
	pipelineZodiacal->removeVertexEntry(2);
	pipelineZodiacal->bindShader("milkyway.vert.spv");
	pipelineZodiacal->bindShader("milkyway.geom.spv");
	pipelineZodiacal->bindShader("zodiacal.frag.spv");
	pipelineZodiacal->build();

	setZodiacal = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, layout.get(), -1, false, true);
	if (zodiacal.tex != nullptr)
		setZodiacal->bindTexture(zodiacal.tex->getTexture(), 0);
}
