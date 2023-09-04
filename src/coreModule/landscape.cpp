/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

#include "coreModule/landscape.hpp"
#include "coreModule/fog.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "tools/s_texture.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"

#include "tools/context.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"

#ifdef WIN32
#include <malloc.h>
#ifndef alloca
#define alloca _alloca
#endif
#else
#include <alloca.h>
#endif

//define word string in a same place
#define L_TYPE 			"type"
#define L_SPHERICAL		"spherical"
#define L_FISHEYE		"fisheye"
#define L_PATH 			"path"
#define L_NIGHT_TEX		"night_texture"
#define L_NAME			"name"
#define L_TEXTURE		"texture"
#define L_MIPMAP		"mipmap"
#define L_LIM_SHADE		"limited_shade"

int Landscape::slices = 20;
int Landscape::stacks = 10;

const float minShadeValue = 0.1f;
const float maxShadeValue = 0.9f;

Pipeline *Landscape::pipeline;
PipelineLayout *Landscape::layout;
std::unique_ptr<VertexArray> Landscape::vertexModel;

static float setLimitedShade(float _value )
{
	return Utility::clamp(_value, minShadeValue, maxShadeValue);
}

Landscape::Landscape(float _radius) : radius(_radius), sky_brightness(1.)
{
	map_tex = nullptr;
	map_tex_night = nullptr;

	valid_landscape = 0;
	cLog::get()->write( "Landscape generic created", LOG_TYPE::L_INFO);
	haveNightTex = false;
	m_limitedShade = false;

	fog =nullptr;
	fog = std::make_unique<Fog>(0.95f);
	assert(fog!=nullptr);
}

Landscape::~Landscape()
{
}

void Landscape::setSkyBrightness(float b)
{
	sky_brightness = b;
	fog->setSkyBrightness(b);
}

//! Set whether fog is displayed
void Landscape::fogSetFlagShow(bool b)
{
	fog->setFlagShow(b);
}
//! Get whether fog is displayed
bool Landscape::fogGetFlagShow() const
{
	return fog->getFlagShow();
}

void Landscape::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	assert(!vertexModel);
	vertexModel = std::make_unique<VertexArray>(vkmgr);
	vertexModel->createBindingEntry(5*sizeof(float));
	vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexModel->addInput(VK_FORMAT_R32G32_SFLOAT);

	layout = new PipelineLayout(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 2);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 3);
	layout->buildLayout();
	layout->build();
	pipeline = new Pipeline[2]{{vkmgr, *context.render, PASS_FOREGROUND, layout}, {vkmgr, *context.render, PASS_FOREGROUND, layout}};
	for (int i = 0; i < 2; ++i) {
		pipeline[i].setCullMode(true);
		pipeline[i].setDepthStencilMode();
		pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
		pipeline[i].bindVertex(*vertexModel);
		pipeline[i].bindShader("landscape.vert.spv");
		pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
		pipeline[i].bindShader("landscape.geom.spv");
		pipeline[i].bindShader((i == 0) ? "landscapeNightTexture.frag.spv" : "landscape.frag.spv");
		pipeline[i].build();
	}
	Fog::createSC_context();
}


Landscape* Landscape::createFromFile(const std::string& landscape_file, const std::string& section_name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	std::string s;
	s = pd.getStr(section_name, L_TYPE);
	Landscape* ldscp = nullptr;
	if (s==L_SPHERICAL) {
		ldscp = new LandscapeSpherical();
		ldscp->format = pd.getStr(section_name, L_TYPE);
	}
	else if (s==L_FISHEYE) {
		ldscp = new LandscapeFisheye();
		ldscp->format = pd.getStr(section_name, L_TYPE);
	}
	else {
		cLog::get()->write( "Unknown landscape type: " + s, LOG_TYPE::L_ERROR);
		// to avoid making this a fatal error, will load as a basic Landscape
		ldscp = new Landscape();
	}
	ldscp->load(landscape_file, section_name);
	return ldscp;
}


// create landscape from parameters passed in a hash (same keys as with ini file)
Landscape* Landscape::createFromHash(stringHash_t & param, int landing)
{
	// night landscape textures for spherical and fisheye landscape types or possibility to have limitedShare
	std::string night_tex="";
	if (!param[L_NIGHT_TEX].empty())
		night_tex = param[L_PATH] + param[L_NIGHT_TEX];

	float limitedShadeValue = 0;
	if (!param[L_LIM_SHADE].empty()) {
		limitedShadeValue = setLimitedShade(Utility::strToFloat(param[L_LIM_SHADE]));
	}

	std::string texture="";
	if (param[L_TEXTURE].empty())
		texture = param[L_PATH] + param["maptex"];
	else
		texture = param[L_PATH] + param[L_TEXTURE];

	bool mipmap = param[L_MIPMAP].empty() ? true : Utility::isTrue(param[L_MIPMAP]);

	// NOTE: textures should be full filename (and path)
	if (param[L_TYPE]==L_FISHEYE) {
		LandscapeFisheye* ldscp = new LandscapeFisheye();
		ldscp->format = param[L_TYPE];
		ldscp->create(param[L_NAME], texture, Utility::strToDouble(param["fov"], Utility::strToDouble(param["texturefov"], 180)),
		              Utility::strToDouble(param["rotate_z"], 0.), night_tex, limitedShadeValue, mipmap);
		return ldscp;
	}
	else if (param[L_TYPE]==L_SPHERICAL) {
		LandscapeSpherical* ldscp = new LandscapeSpherical();
		ldscp->format = param[L_TYPE];
		ldscp->create(param[L_NAME], texture, Utility::strToDouble(param["base_altitude"], -90),
		              Utility::strToDouble(param["top_altitude"], 90), Utility::strToDouble(param["rotate_z"], 0.),  night_tex, limitedShadeValue, mipmap, landing);
		return ldscp;
	}
	else {    //wrong Landscape
		Landscape* ldscp = new Landscape();
		cLog::get()->write( "Unknown landscape type in createFromHash: " + param[L_NAME], LOG_TYPE::L_ERROR);
		return ldscp;
	}
}


// Load attributes common to all landscapes
void Landscape::loadCommon(const std::string& landscape_file, const std::string& section_name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	name = pd.getStr(section_name, L_NAME);
	author = pd.getStr(section_name, "author");
	description = pd.getStr(section_name, "description");

	fog->setAltAngle(pd.getDouble(section_name, "fog_alt_angle", 30.));
	fog->setAngleShift(pd.getDouble(section_name, "fog_angle_shift", 0.));

	if (name.empty()) {
		cLog::get()->write( "No valid landscape definition found for section " + section_name +" in file " + landscape_file, LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	else {
		valid_landscape = 1;
	}
}


std::string Landscape::getFileContent(const std::string& landscape_file)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	std::string result;
	for (int i=0; i<pd.getNsec(); i++) {
		result += pd.getSecname(i) + '\n';
	}
	return result;
}


std::string Landscape::getLandscapeNames(const std::string& landscape_file)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	std::string result;
	for (int i=0; i<pd.getNsec(); i++) {
		result += pd.getStr(pd.getSecname(i), L_NAME) + '\n';
	}
	return result;
}


void Landscape::draw(const Projector* prj, const Navigator* nav)
{
	Context &context = *Context::instance;
	if (fader.isZero() || !valid_landscape) return;

	if (haveNightTex && sky_brightness < 0.25) {
		context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx + 3], PASS_FOREGROUND);
	} else {
		if (m_limitedShade)
			sky_brightness = std::max(sky_brightness, m_limitedShadeValue);
		context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx], PASS_FOREGROUND);
	}
	uFrag->get().sky_brightness = fmin(sky_brightness,1.0);
	uFrag->get().fader = fader;
	*uMV = (nav->getLocalToEyeMat() * Mat4d::zrotation(-rotate_z)).convert();

	fog->draw(prj,nav);
}

void Landscape::destroySC_context()
{
	delete[] pipeline;
	delete layout;
	Fog::destroySC_context();
}

// *********************************************************************
//
// Fisheye landscape
//
// *********************************************************************

LandscapeFisheye::LandscapeFisheye(float _radius) : Landscape(_radius)
{
	rotate_z = 0;
}


LandscapeFisheye::~LandscapeFisheye()
{
}


void LandscapeFisheye::load(const std::string& landscape_file, const std::string& section_name)
{
	loadCommon(landscape_file, section_name);

	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);

	std::string type = pd.getStr(section_name, L_TYPE);
	if (type != L_FISHEYE) {
		cLog::get()->write( "type mismatch for landscape " + section_name + ": expected fisheye found " + type, LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	std::string texture = pd.getStr(section_name, L_TEXTURE);
	if (texture.empty()) {
		cLog::get()->write( "No texture for landscape " + section_name, LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	texture = AppSettings::Instance()->getLandscapeDir() + texture;

	std::string night_texture = pd.getStr(section_name, L_NIGHT_TEX, "");
	if (! night_texture.empty())
		night_texture = AppSettings::Instance()->getLandscapeDir() +night_texture;

	float limitedShadeValue = 0;
	std::string haveLimitedShade = pd.getStr(section_name, L_LIM_SHADE);
	if (!haveLimitedShade.empty()) {
		limitedShadeValue = setLimitedShade(Utility::strToFloat(haveLimitedShade));
	}

	create(name, texture,
	       pd.getDouble(section_name, "fov", pd.getDouble(section_name, "texturefov", 180)),
	       pd.getDouble(section_name, "rotate_z", 0.),
	       night_texture, limitedShadeValue,
	       pd.getBoolean(section_name, L_TEXTURE, true));
}


// create a fisheye landscape from basic parameters (no ini file needed)
void LandscapeFisheye::create(const std::string _name, const std::string _maptex, double _texturefov,
                              const float _rotate_z, const std::string _maptex_night, float limitedShade, bool _mipmap)
{
	valid_landscape = 1;  // assume ok...
	cLog::get()->write( "Landscape Fisheye " + _name + " created", LOG_TYPE::L_INFO);
	name = _name;
	map_tex = std::make_unique<s_texture>(_maptex,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);

	if (! _maptex_night.empty()) {
		map_tex_night = std::make_unique<s_texture>(_maptex_night,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
		haveNightTex = true;
	} else {
		haveNightTex = false;
		if (limitedShade>0) {
			m_limitedShade = true;
			m_limitedShadeValue = limitedShade;
		}
	}
	tex_fov = _texturefov*M_PI/180.;
	rotate_z = _rotate_z*M_PI/180.;

	initShader();
	fog->initShader();
}


void LandscapeFisheye::initShader()
{
	Context &context = *Context::instance;

	nbVertex = 2 * (slices + 1) * stacks;
	createFisheyeMesh(radius,slices,stacks, tex_fov, (float *) context.transfer->beginPlanCopy(nbVertex * 5 * sizeof(float)));
	vertex = vertexModel->createBuffer(0, nbVertex, context.globalBuffer.get());
	context.transfer->endPlanCopy(vertex->get(), nbVertex * 5 * sizeof(float));
	set = std::make_unique<Set>(*VulkanMgr::instance, *context.setMgr, layout, -1, true, true);
	uMV = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
	uFrag = std::make_unique<SharedBuffer<frag>>(*context.uniformMgr);
	set->bindUniform(uMV, 2);
	set->bindUniform(uFrag, 3);
	set->bindTexture(map_tex->getTexture(), 0);
	if (haveNightTex)
		set->bindTexture(map_tex_night->getTexture(), 1);

	context.cmdInfo.commandBufferCount = haveNightTex ? 6 : 3;
	vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_FOREGROUND);
		pipeline[1].bind(cmd);
		vertex->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		vkCmdDraw(cmd, nbVertex, 1, 0, 0);
		context.frame[i]->compile(cmd);
		if (haveNightTex) {
			auto cmd = cmds[i + 3];
			context.frame[i]->begin(cmd, PASS_FOREGROUND);
			pipeline[0].bind(cmd);
			vertex->bind(cmd);
			layout->bindSets(cmd, {*context.uboSet, *set});
			vkCmdDraw(cmd, nbVertex, 1, 0, 0);
			context.frame[i]->compile(cmd);
		}
	}
}


static inline double FisheyeTexCoordFastS(double rho_div_fov, double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	return 0.5 + rho_div_fov * costheta;
}


static inline double FisheyeTexCoordFastT(double rho_div_fov, double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	return 0.5 + rho_div_fov * sintheta;
}

void LandscapeFisheye::createFisheyeMesh(double radius, int slices, int stacks, double texture_fov, float *data)
{
	// unsigned int indice1=0;
	// unsigned int indice3=0;
	double rho,x,y,z;
	int i, j;

	int nbr=0;
	const double drho = M_PI / stacks;
	double *cos_sin_rho = (double *) alloca(sizeof(double) * 2*(stacks+1));
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		const double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const double dtheta = 2.0 * M_PI / slices;
	double *cos_sin_theta = (double *) alloca(sizeof(double) * 2*(slices+1));
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		const double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	const int imax = stacks;

	// draw intermediate stacks as quad strips
	for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0; i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {

		for (j=0,cos_sin_theta_p=cos_sin_theta; j<= slices; ++j,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = cos_sin_rho_p[2];
			if (z>=0) {
				nbr++;
				z=z-0.01; //TODO magic number to export

				*(data++) = x*radius;
				*(data++) = y*radius;
				*(data++) = z*radius;
				*(data++) = FisheyeTexCoordFastS((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
				*(data++) = FisheyeTexCoordFastT((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
			}
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = cos_sin_rho_p[0];
			if (z>=0) {
				nbr++;
				z=z-0.01; //TODO magic number to export
				*(data++) = x*radius;
				*(data++) = y*radius;
				*(data++) = z*radius;
				*(data++) = FisheyeTexCoordFastS(rho/M_PI, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
				*(data++) = FisheyeTexCoordFastT(rho/M_PI, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
			}
		}
	}
	nbVertex = nbr;
}

// *********************************************************************
//
// spherical panoramas
//
// *********************************************************************

LandscapeSpherical::LandscapeSpherical(float _radius) : Landscape(_radius),  base_altitude(-90), top_altitude(90), landingFader(false, 2)
{
	landingFader.interpolator.zeroValue = base_altitude;
	landingFader.interpolator.delta = top_altitude - base_altitude;
	landingFader.setNoDelay(true); // Switching false to true update the cached value with the new parameters
	rotate_z = 0;
}


LandscapeSpherical::~LandscapeSpherical()
{
}


void LandscapeSpherical::load(const std::string& landscape_file, const std::string& section_name)
{
	loadCommon(landscape_file, section_name);

	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);

	std::string type = pd.getStr(section_name, L_TYPE);
	if (type != L_SPHERICAL ) {
		cLog::get()->write( "Type mismatch for landscape " + section_name +", expected spherical, found " + type, LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}

	std::string texture = pd.getStr(section_name, L_TEXTURE);
	if (texture.empty()) {
		cLog::get()->write( "No texture for landscape " + section_name, LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	texture = AppSettings::Instance()->getLandscapeDir() + texture;

	std::string night_texture = pd.getStr(section_name, L_NIGHT_TEX, "");
	if (! night_texture.empty())
		night_texture = AppSettings::Instance()->getLandscapeDir() +night_texture;

	float limitedShadeValue = 0;
	std::string haveLimitedShade = pd.getStr(section_name, L_LIM_SHADE);
	if (!haveLimitedShade.empty()) {
		limitedShadeValue = setLimitedShade(Utility::strToFloat(haveLimitedShade));
	}

	create(name, texture,
	       pd.getDouble(section_name, "base_altitude", -90),
	       pd.getDouble(section_name, "top_altitude", 90),
	       pd.getDouble(section_name, "rotate_z", 0.),
	       night_texture, limitedShadeValue,
	       pd.getBoolean(section_name, L_TEXTURE, true), 1);
}

// create a spherical landscape from basic parameters (no ini file needed)
void LandscapeSpherical::create(const std::string _name, const std::string _maptex, const float _base_altitude,
                                const float _top_altitude, const float _rotate_z, const std::string _maptex_night, float limitedShade, bool _mipmap, int landing)
{
	valid_landscape = 1;  // assume ok...
	cLog::get()->write( "Landscape Spherical " + _name + " created", LOG_TYPE::L_INFO);
	name = _name;
	map_tex = std::make_unique<s_texture>(_maptex,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);

	if (!_maptex_night.empty()) {
		map_tex_night = std::make_unique<s_texture>(_maptex_night,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
		haveNightTex = true;
	} else {
		haveNightTex = false;
		if (limitedShade>0) {
			m_limitedShade = true;
			m_limitedShadeValue = limitedShade;
		}
	}

	base_altitude = ((_base_altitude >= -90 && _base_altitude <= 90) ? _base_altitude : -90);
	top_altitude = ((_top_altitude >= -90 && _top_altitude <= 90) ? _top_altitude : 90);
	rotate_z = _rotate_z*M_PI/180.;

	if (landing == 0)
		landingFader.setNoDelay(false);
	else
		landingFader.setNoDelay(true);
	initShader();
	fog->initShader();
}

void LandscapeSpherical::draw(const Projector* prj, const Navigator* nav)
{
	if (top_altitude != landingFader) {
		top_altitude = landingFader;
		createSphericalMesh(radius, 1.0, slices,stacks, base_altitude, top_altitude, (float *) Context::instance->transfer->planCopy(vertex->get()));
	}
	Landscape::draw(prj, nav);
}

void LandscapeSpherical::initShader()
{
	Context &context = *Context::instance;

	nbVertex = 2 * (slices + 1) * stacks;
	createSphericalMesh(radius, 1.0, slices,stacks, base_altitude, top_altitude, (float *) context.transfer->beginPlanCopy(nbVertex * 5 * sizeof(float)));
	vertex = vertexModel->createBuffer(0, nbVertex, context.globalBuffer.get());
	context.transfer->endPlanCopy(vertex->get(), nbVertex * 5 * sizeof(float));
	set = std::make_unique<Set>(*VulkanMgr::instance, *context.setMgr, layout, -1, true, true);
	uMV = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
	uFrag = std::make_unique<SharedBuffer<frag>>(*context.uniformMgr);
	set->bindUniform(uMV, 2);
	set->bindUniform(uFrag, 3);
	set->bindTexture(map_tex->getTexture(), 0);
	if (haveNightTex)
		set->bindTexture(map_tex_night->getTexture(), 1);

	context.cmdInfo.commandBufferCount = haveNightTex ? 6 : 3;
	vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_FOREGROUND);
		pipeline[1].bind(cmd);
		vertex->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		vkCmdDraw(cmd, nbVertex, 1, 0, 0);
		context.frame[i]->compile(cmd);
		if (haveNightTex) {
			auto cmd = cmds[i + 3];
			context.frame[i]->begin(cmd, PASS_FOREGROUND);
			pipeline[0].bind(cmd);
			vertex->bind(cmd);
			layout->bindSets(cmd, {*context.uboSet, *set});
			vkCmdDraw(cmd, nbVertex, 1, 0, 0);
			context.frame[i]->compile(cmd);
		}
	}
}


void LandscapeSpherical::createSphericalMesh(double radius, double one_minus_oblateness, int slices, int stacks,
        double bottom_altitude, double top_altitude, float *data)
{
	double bottom = M_PI / 180. * bottom_altitude;
	double angular_height = M_PI / 180. * top_altitude - bottom;

	float x, y, z;
	float s, t;
	int i, j;
	t=0.0; // from inside texture is reversed

	const float drho = angular_height / (float) stacks;
	double *cos_sin_rho = (double *) alloca(sizeof(double) * 2*(stacks+1));
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = M_PI_2 + bottom + i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const float dtheta = 2.0 * M_PI / (float) slices;
	double *cos_sin_theta = (double *) alloca(sizeof(double) * 2*(slices+1));
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triane fan on texturing (s coord. at top/bottom tip varies)
	const float ds = 1.0 / slices;
	const float dt = -1.0 / stacks; // from inside texture is reversed

	// draw intermediate  as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = -1.0 * cos_sin_rho_p[0];

			*(data++) = x*radius;
			*(data++) = y*radius;
			*(data++) = z * one_minus_oblateness * radius;
			*(data++) = 1-s;
			*(data++) = t;

			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = -1.0 * cos_sin_rho_p[2];

			*(data++) = x*radius;
			*(data++) = y*radius;
			*(data++) = z * one_minus_oblateness * radius;
			*(data++) = 1-s;
			*(data++) = t-dt;

			s += ds;
		}
		t -= dt;
	}
}

void LandscapeSpherical::setLanding(bool isLanding, float speed)
{
	landingFader.interpolator.zeroValue = -90 + ((speed * 10) * 18);
	landingFader.interpolator.delta = 90 - landingFader.interpolator.zeroValue;
	landingFader = isLanding;
}
