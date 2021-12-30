/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
 * Copyright (C) 2017 Immersive Adventure
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
#include "bodyModule/bodyShader.hpp"

#include "tools/context.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/Set.hpp"

drawState_t BodyShader::shaderBump;
drawState_t BodyShader::shaderNight;
drawState_t BodyShader::myEarth;
drawState_t BodyShader::shaderRinged;
drawState_t BodyShader::shaderNormal;
drawState_t BodyShader::shaderNormalTes;
// std::unique_ptr<shaderProgram> BodyShader::shaderMoonNight;
// std::unique_ptr<shaderProgram> BodyShader::shaderMoonNormal;
// std::unique_ptr<shaderProgram> BodyShader::shaderMoonBump;
drawState_t BodyShader::myMoon;

drawState_t BodyShader::shaderArtificial;


void BodyShader::createShader()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	auto tmp = PipelineLayout::DEFAULT_SAMPLER;
    tmp.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// ========== body_night ========== //
	context.layouts.push_back(std::make_unique<PipelineLayout>(vkmgr));
	shaderNight.layout = context.layouts.back().get();
	shaderNight.layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	shaderNight.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	shaderNight.layout->setTextureLocation(2, &tmp);
	shaderNight.layout->setTextureLocation(3, &PipelineLayout::DEFAULT_SAMPLER);
	shaderNight.layout->setTextureLocation(4, &tmp);
	shaderNight.layout->buildLayout();
	shaderNight.layout->setGlobalPipelineLayout(context.layouts.front().get());
	shaderNight.layout->build();

	context.pipelines.push_back(std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderNight.layout));
	shaderNight.pipeline = context.pipelines.back().get();
	shaderNight.pipeline->setCullMode(true);
	shaderNight.pipeline->setBlendMode(BLEND_NONE);
	shaderNight.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	shaderNight.pipeline->bindVertex(*context.ojmVertexArray);
	shaderNight.pipeline->bindShader("body_night.vert.spv");
	shaderNight.pipeline->bindShader("body_night.frag.spv");
	shaderNight.pipeline->build();

	// ========== my_earth ========== //
	context.layouts.push_back(std::make_unique<PipelineLayout>(vkmgr));
	myEarth.layout = context.layouts.back().get();
	myEarth.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	myEarth.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	myEarth.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 2); // globalTescGeom
	myEarth.layout->setTextureLocation(5, &tmp);
	myEarth.layout->setTextureLocation(6, &PipelineLayout::DEFAULT_SAMPLER);
	myEarth.layout->setTextureLocation(7, &tmp);
	myEarth.layout->setTextureLocation(8, &tmp);
	myEarth.layout->setTextureLocation(9, &tmp);
	myEarth.layout->setTextureLocation(10, &tmp);
	myEarth.layout->setTextureLocation(11, &tmp, VK_SHADER_STAGE_GEOMETRY_BIT);
	myEarth.layout->buildLayout();
	myEarth.layout->setGlobalPipelineLayout(context.layouts.front().get());
	myEarth.layout->build();

	myEarth.pipeline = new Pipeline[2]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myEarth.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myEarth.layout}};
	context.pipelineArray.push_back(myEarth.pipeline);
	for (int i = 0; i < 2; ++i) {
		myEarth.pipeline[i].setCullMode(true);
		myEarth.pipeline[i].setFrontFace(); // Body with tesselation don't have the same front face...
		myEarth.pipeline[i].setBlendMode(BLEND_NONE);
		myEarth.pipeline[i].bindVertex(*context.ojmVertexArray);
		myEarth.pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
		myEarth.pipeline[i].setTessellationState(3);
		myEarth.pipeline[i].bindShader("body_tes.vert.spv");
		myEarth.pipeline[i].bindShader("body_tes.tesc.spv");
		myEarth.pipeline[i].bindShader("body_tes.tese.spv");
		myEarth.pipeline[i].bindShader("my_earth.geom.spv");
		myEarth.pipeline[i].bindShader("my_earth.frag.spv");
		VkBool32 Clouds = (i == 1);
		myEarth.pipeline[i].setSpecializedConstant(1, &Clouds, sizeof(Clouds));
		myEarth.pipeline[i].build();
	}

	// ========== body_bump ========== //
	shaderBump.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(shaderBump.layout);
	shaderBump.layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	shaderBump.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	shaderBump.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2); // (vec3) UmbraColor
	shaderBump.layout->setTextureLocation(3, &tmp);
	shaderBump.layout->setTextureLocation(4, &tmp);
	shaderBump.layout->setTextureLocation(5, &PipelineLayout::DEFAULT_SAMPLER);
	shaderBump.layout->buildLayout();
	shaderBump.layout->setGlobalPipelineLayout(context.layouts.front().get());
	shaderBump.layout->build();

	shaderBump.pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderBump.layout);
	context.pipelines.emplace_back(shaderBump.pipeline);
	shaderBump.pipeline->setCullMode(true);
	shaderBump.pipeline->setBlendMode(BLEND_NONE);
	shaderBump.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	shaderBump.pipeline->bindVertex(*context.ojmVertexArray);
	shaderBump.pipeline->bindShader("body_bump.vert.spv");
	shaderBump.pipeline->bindShader("body_bump.frag.spv");
	shaderBump.pipeline->build();

	// ========== body_ringed ========== //
	shaderRinged.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(shaderRinged.layout);
	shaderRinged.layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	shaderRinged.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	shaderRinged.layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 2); // ModelViewMatrixInverse
	shaderRinged.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 3); // ringFrag
	shaderRinged.layout->setTextureLocation(4, &tmp);
	shaderRinged.layout->setTextureLocation(5, &PipelineLayout::DEFAULT_SAMPLER);
	shaderRinged.layout->setTextureLocation(6, &tmp);
	shaderRinged.layout->buildLayout();
	shaderRinged.layout->setGlobalPipelineLayout(context.layouts.front().get());
	shaderRinged.layout->build();

	shaderRinged.pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderRinged.layout);
	context.pipelines.emplace_back(shaderRinged.pipeline);
	shaderRinged.pipeline->setCullMode(true);
	shaderRinged.pipeline->setBlendMode(BLEND_NONE);
	shaderRinged.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	shaderRinged.pipeline->bindVertex(*context.ojmVertexArray);
	shaderRinged.pipeline->bindShader("body_ringed.vert.spv");
	shaderRinged.pipeline->bindShader("body_ringed.frag.spv");
	shaderRinged.pipeline->build();

	// ========== body_normal ========== //
	shaderNormal.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(shaderNormal.layout);
	shaderNormal.layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	shaderNormal.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	shaderNormal.layout->setTextureLocation(2, &tmp);
	shaderNormal.layout->setTextureLocation(3, &PipelineLayout::DEFAULT_SAMPLER);
	shaderNormal.layout->buildLayout();
	shaderNormal.layout->setGlobalPipelineLayout(context.layouts.front().get());
	shaderNormal.layout->build();

	shaderNormal.pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderNormal.layout);
	context.pipelines.emplace_back(shaderNormal.pipeline);
	shaderNormal.pipeline->setCullMode(true);
	shaderNormal.pipeline->setBlendMode(BLEND_NONE);
	shaderNormal.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	shaderNormal.pipeline->bindVertex(*context.ojmVertexArray);
	shaderNormal.pipeline->bindShader("body_normal.vert.spv");
	shaderNormal.pipeline->bindShader("body_normal.frag.spv");
	shaderNormal.pipeline->build();

	// ========== body_normal_tes ========== //
	shaderNormalTes.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(shaderNormalTes.layout);
	shaderNormalTes.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	shaderNormalTes.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	shaderNormalTes.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 2); // globalTescGeom
	shaderNormalTes.layout->setTextureLocation(5, &tmp, VK_SHADER_STAGE_GEOMETRY_BIT); // heightmapTexture
	shaderNormalTes.layout->setTextureLocation(6, &tmp); // mapTexture
	shaderNormalTes.layout->setTextureLocation(7, &PipelineLayout::DEFAULT_SAMPLER); // shadowTexture
	shaderNormalTes.layout->buildLayout();
	shaderNormalTes.layout->setGlobalPipelineLayout(context.layouts.front().get());
	shaderNormalTes.layout->build();

	shaderNormalTes.pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderNormalTes.layout);
	context.pipelines.emplace_back(shaderNormalTes.pipeline);
	shaderNormalTes.pipeline->setCullMode(true);
	shaderNormalTes.pipeline->setFrontFace(); // Body with tesselation don't have the same front face...
	shaderNormalTes.pipeline->setBlendMode(BLEND_NONE);
	shaderNormalTes.pipeline->bindVertex(*context.ojmVertexArray);
	shaderNormalTes.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	shaderNormalTes.pipeline->setTessellationState(3);
	shaderNormalTes.pipeline->bindShader("body_tes.vert.spv");
	shaderNormalTes.pipeline->bindShader("body_tes.tesc.spv");
	shaderNormalTes.pipeline->bindShader("body_tes.tese.spv");
	shaderNormalTes.pipeline->bindShader("body_normal_tes.geom.spv");
	shaderNormalTes.pipeline->bindShader("body_normal_tes.frag.spv");
	shaderNormalTes.pipeline->build();

	// ========== body_artificial ========== //
	shaderArtificial.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(shaderArtificial.layout);
	shaderArtificial.layout->setGlobalPipelineLayout(context.layouts.front().get());
	shaderArtificial.layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	shaderArtificial.layout->buildLayout();
	shaderArtificial.layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0); // NormalMatrix
	shaderArtificial.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 1); // artGeom
	shaderArtificial.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2); // LightInfo
	shaderArtificial.layout->buildLayout();
	shaderArtificial.layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 44);
	shaderArtificial.layout->build();

	shaderArtificial.pipeline = new Pipeline[2]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderArtificial.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderArtificial.layout}};
	context.pipelineArray.push_back(shaderArtificial.pipeline);
	for (short i = 0; i < 2; ++i) {
		shaderArtificial.pipeline[i].setCullMode(true);
		shaderArtificial.pipeline[i].setBlendMode(BLEND_NONE);
		shaderArtificial.pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		shaderArtificial.pipeline[i].bindVertex(*context.ojmVertexArray);
		shaderArtificial.pipeline[i].bindShader("body_artificial.vert.spv");
		shaderArtificial.pipeline[i].bindShader("body_artificial.geom.spv");
		shaderArtificial.pipeline[i].bindShader("body_artificial.frag.spv");
		VkBool32 useTexture = (i == 0);
		shaderArtificial.pipeline[i].setSpecializedConstant(0, &useTexture, sizeof(useTexture));
		shaderArtificial.pipeline[i].build();
	}

	// ========== my_moon ========== //
	myMoon.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(myMoon.layout);
	myMoon.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	myMoon.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // moonFrag
	myMoon.layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 2); // globalTescGeom
	myMoon.layout->setTextureLocation(5, &tmp); // mapTexture
	myMoon.layout->setTextureLocation(6, &tmp); // normalTexture
	myMoon.layout->setTextureLocation(7, &PipelineLayout::DEFAULT_SAMPLER); // shadowTexture
	myMoon.layout->setTextureLocation(8, &tmp, VK_SHADER_STAGE_GEOMETRY_BIT); // heightmapTexture
	myMoon.layout->buildLayout();
	myMoon.layout->setGlobalPipelineLayout(context.layouts.front().get());
	myMoon.layout->build();

	myMoon.pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myMoon.layout);
	context.pipelines.emplace_back(myMoon.pipeline);
	myMoon.pipeline->setCullMode(true);
	myMoon.pipeline->setFrontFace(); // Body with tesselation don't have the same front face...
	myMoon.pipeline->setBlendMode(BLEND_NONE);
	myMoon.pipeline->bindVertex(*context.ojmVertexArray);
	myMoon.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
	myMoon.pipeline->setTessellationState(3);
	myMoon.pipeline->bindShader("body_tes.vert.spv");
	myMoon.pipeline->bindShader("body_tes.tesc.spv");
	myMoon.pipeline->bindShader("body_tes.tese.spv");
	myMoon.pipeline->bindShader("my_moon.geom.spv");
	myMoon.pipeline->bindShader("my_moon.frag.spv");
	myMoon.pipeline->build();

	// shaderNight = std::make_unique<shaderProgram>();
	// shaderNight->init( "body_night.vert", "body_night.frag");
	// shaderNight->setUniformLocation("Clouds");
	// shaderNight->setUniformLocation("CloudNormalTexture");
	// shaderNight->setUniformLocation("CloudTexture");

	//commum
	// shaderNight->setUniformLocation("planetRadius");
	// shaderNight->setUniformLocation("planetScaledRadius");
	// shaderNight->setUniformLocation("planetOneMinusOblateness");
	//
	// shaderNight->setUniformLocation("SunHalfAngle");
	// shaderNight->setUniformLocation("LightPosition");
	// shaderNight->setUniformLocation("ModelViewProjectionMatrix");
	// shaderNight->setUniformLocation("ModelViewMatrix");
	// shaderNight->setUniformLocation("NormalMatrix");
	// shaderNight->setUniformLocation("ViewProjection");
	// shaderNight->setUniformLocation("Model");
	//
	// shaderNight->setUniformLocation("MoonPosition1");
	// shaderNight->setUniformLocation("MoonRadius1");
	// shaderNight->setUniformLocation("MoonPosition2");
	// shaderNight->setUniformLocation("MoonRadius2");
	// shaderNight->setUniformLocation("MoonPosition3");
	// shaderNight->setUniformLocation("MoonRadius3");
	// shaderNight->setUniformLocation("MoonPosition4");
	// shaderNight->setUniformLocation("MoonRadius4");
	//
	// shaderNight->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderNight->setUniformLocation("clipping_fov");

	// shaderMoonNight = std::make_unique<shaderProgram>();
	// shaderMoonNight->init( "body_moon_night.vert", "body_moon_night.frag");
	// // shaderMoonNight->setUniformLocation("Clouds");
	// // shaderMoonNight->setUniformLocation("CloudNormalTexture");
	// // shaderMoonNight->setUniformLocation("CloudTexture");

	// //commum
	// shaderMoonNight->setUniformLocation("planetRadius");
	// shaderMoonNight->setUniformLocation("planetScaledRadius");
	// shaderMoonNight->setUniformLocation("planetOneMinusOblateness");

	// shaderMoonNight->setUniformLocation("SunHalfAngle");
	// shaderMoonNight->setUniformLocation("LightPosition");
	// shaderMoonNight->setUniformLocation("ModelViewProjectionMatrix");
	// shaderMoonNight->setUniformLocation("ModelViewMatrix");
	// shaderMoonNight->setUniformLocation("NormalMatrix");
	// shaderMoonNight->setUniformLocation("ViewProjection");
	// shaderMoonNight->setUniformLocation("Model");

	// shaderMoonNight->setUniformLocation("MoonPosition1");
	// shaderMoonNight->setUniformLocation("MoonRadius1");

	// shaderMoonNight->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderMoonNight->setUniformLocation("clipping_fov");

	// myEarth= std::make_unique<shaderProgram>();
	// myEarth->init( "my_earth.vert", "my_earth.tesc","my_earth.tese", "my_earth.geom", "my_earth.frag");
	// // myEarth->setUniformLocation("Clouds");
	// // myEarth->setUniformLocation("CloudNormalTexture");
	// // myEarth->setUniformLocation("CloudTexture");
	// myEarth->setUniformLocation("TesParam");
	//
	// //commum
	// myEarth->setUniformLocation("planetRadius");
	// myEarth->setUniformLocation("planetScaledRadius");
	// myEarth->setUniformLocation("planetOneMinusOblateness");
	//
	// myEarth->setUniformLocation("SunHalfAngle");
	// myEarth->setUniformLocation("LightPosition");
	// myEarth->setUniformLocation("ModelViewProjectionMatrix");
	// myEarth->setUniformLocation("ModelViewMatrix");
	// myEarth->setUniformLocation("NormalMatrix");
	// myEarth->setUniformLocation("ViewProjection");
	// myEarth->setUniformLocation("Model");
	//
	// myEarth->setUniformLocation("MoonPosition1");
	// myEarth->setUniformLocation("MoonRadius1");
	// myEarth->setUniformLocation("MoonPosition2");
	// myEarth->setUniformLocation("MoonRadius2");
	// myEarth->setUniformLocation("MoonPosition3");
	// myEarth->setUniformLocation("MoonRadius3");
	// myEarth->setUniformLocation("MoonPosition4");
	// myEarth->setUniformLocation("MoonRadius4");
	//
	// //fisheye
	// myEarth->setUniformLocation("inverseModelViewProjectionMatrix");
	// myEarth->setUniformLocation("clipping_fov");
	//
	// shaderBump = std::make_unique<shaderProgram>();
	// shaderBump->init( "body_bump.vert", "body_bump.frag");
	// shaderBump->setUniformLocation("UmbraColor"); // custom frag
	//
	// //commum
	// shaderBump->setUniformLocation("planetRadius");
	// shaderBump->setUniformLocation("planetScaledRadius");
	// shaderBump->setUniformLocation("planetOneMinusOblateness");
	//
	// shaderBump->setUniformLocation("SunHalfAngle");
	// shaderBump->setUniformLocation("LightPosition");
	// shaderBump->setUniformLocation("ModelViewProjectionMatrix");
	// shaderBump->setUniformLocation("ModelViewMatrix");
	// shaderBump->setUniformLocation("NormalMatrix");
	//
	// shaderBump->setUniformLocation("MoonPosition1");
	// shaderBump->setUniformLocation("MoonRadius1");
	// shaderBump->setUniformLocation("MoonPosition2");
	// shaderBump->setUniformLocation("MoonRadius2");
	// shaderBump->setUniformLocation("MoonPosition3");
	// shaderBump->setUniformLocation("MoonRadius3");
	// shaderBump->setUniformLocation("MoonPosition4");
	// shaderBump->setUniformLocation("MoonRadius4");
	//
	// //fisheye
	// shaderBump->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderBump->setUniformLocation("clipping_fov");
	//
	// shaderRinged = std::make_unique<shaderProgram>();
	// shaderRinged->init( "body_ringed.vert", "body_ringed.frag");
	// shaderRinged->setUniformLocation("RingInnerRadius"); // custom frag
	// shaderRinged->setUniformLocation("RingOuterRadius"); // custom frag
	// //shaderRinged->setUniformLocation("UmbraColor");
	//
	// //commum
	// shaderRinged->setUniformLocation("planetRadius");
	// shaderRinged->setUniformLocation("planetScaledRadius");
	// shaderRinged->setUniformLocation("planetOneMinusOblateness");
	//
	// shaderRinged->setUniformLocation("SunHalfAngle");
	// shaderRinged->setUniformLocation("LightPosition");
	// shaderRinged->setUniformLocation("ModelViewProjectionMatrix");
	// shaderRinged->setUniformLocation("ModelViewMatrix");
	// shaderRinged->setUniformLocation("ModelViewMatrixInverse"); // custom vert
	// shaderRinged->setUniformLocation("NormalMatrix");
	//
	// shaderRinged->setUniformLocation("MoonPosition1");
	// shaderRinged->setUniformLocation("MoonRadius1");
	// shaderRinged->setUniformLocation("MoonPosition2");
	// shaderRinged->setUniformLocation("MoonRadius2");
	// shaderRinged->setUniformLocation("MoonPosition3");
	// shaderRinged->setUniformLocation("MoonRadius3");
	// shaderRinged->setUniformLocation("MoonPosition4");
	// shaderRinged->setUniformLocation("MoonRadius4");
	//
	// //fisheye
	// shaderRinged->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderRinged->setUniformLocation("clipping_fov");
	//
	// shaderNormal = std::make_unique<shaderProgram>();
	// shaderNormal->init("body_normal.vert", "body_normal.frag");
	// //shaderNormal->setUniformLocation("UmbraColor");
	//
	// //commum
	// shaderNormal->setUniformLocation("planetRadius");
	// shaderNormal->setUniformLocation("planetScaledRadius");
	// shaderNormal->setUniformLocation("planetOneMinusOblateness");
	//
	// shaderNormal->setUniformLocation("SunHalfAngle");
	// shaderNormal->setUniformLocation("LightPosition");
	// // shaderNormal->setUniformLocation("ModelViewProjectionMatrix");
	// shaderNormal->setUniformLocation("ModelViewMatrix");
	// shaderNormal->setUniformLocation("NormalMatrix");
	//
	// shaderNormal->setUniformLocation("MoonPosition1");
	// shaderNormal->setUniformLocation("MoonRadius1");
	// shaderNormal->setUniformLocation("MoonPosition2");
	// shaderNormal->setUniformLocation("MoonRadius2");
	// shaderNormal->setUniformLocation("MoonPosition3");
	// shaderNormal->setUniformLocation("MoonRadius3");
	// shaderNormal->setUniformLocation("MoonPosition4");
	// shaderNormal->setUniformLocation("MoonRadius4");
	//
	// //fisheye
	// // shaderNormal->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderNormal->setUniformLocation("clipping_fov");
	//
	// shaderNormalTes = std::make_unique<shaderProgram>();
	// shaderNormalTes->init( "body_normal_tes.vert", "body_normal_tes.tesc", "body_normal_tes.tese", "body_normal_tes.geom", "body_normal_tes.frag");
	// //shaderNormalTes->setUniformLocation("UmbraColor");
	// shaderNormalTes->setUniformLocation("TesParam");
	//
	// //commum
	// shaderNormalTes->setUniformLocation("planetRadius");
	// shaderNormalTes->setUniformLocation("planetScaledRadius");
	// shaderNormalTes->setUniformLocation("planetOneMinusOblateness");
	//
	// shaderNormalTes->setUniformLocation("SunHalfAngle");
	// shaderNormalTes->setUniformLocation("LightPosition");
	// shaderNormalTes->setUniformLocation("ModelViewProjectionMatrix");
	// shaderNormalTes->setUniformLocation("ModelViewMatrix");
	// shaderNormalTes->setUniformLocation("NormalMatrix");
	// shaderNormalTes->setUniformLocation("ViewProjection");
	// shaderNormalTes->setUniformLocation("Model");
	//
	// shaderNormalTes->setUniformLocation("MoonPosition1");
	// shaderNormalTes->setUniformLocation("MoonRadius1");
	// shaderNormalTes->setUniformLocation("MoonPosition2");
	// shaderNormalTes->setUniformLocation("MoonRadius2");
	// shaderNormalTes->setUniformLocation("MoonPosition3");
	// shaderNormalTes->setUniformLocation("MoonRadius3");
	// shaderNormalTes->setUniformLocation("MoonPosition4");
	// shaderNormalTes->setUniformLocation("MoonRadius4");
	//
	// //fisheye
	// shaderNormalTes->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderNormalTes->setUniformLocation("clipping_fov");



	// shaderArtificial = std::make_unique<shaderProgram>();
	// shaderArtificial->init( "body_artificial.vert", "body_artificial.geom", "body_artificial.frag");
	//
	// //commum
	// shaderArtificial->setUniformLocation("radius");
	// shaderArtificial->setUniformLocation("useTexture");
	// shaderArtificial->setUniformLocation("ProjectionMatrix");
	// shaderArtificial->setUniformLocation("ModelViewMatrix");
	// shaderArtificial->setUniformLocation("NormalMatrix");
	// shaderArtificial->setUniformLocation("MVP");
	//
	// shaderArtificial->setUniformLocation("Light.Position");
	// shaderArtificial->setUniformLocation("Light.Intensity");
	// shaderArtificial->setUniformLocation("Material.Ka");
	// shaderArtificial->setUniformLocation("Material.Kd");
	// shaderArtificial->setUniformLocation("Material.Ks");
	// shaderArtificial->setUniformLocation("Material.Ns");
	//
	// //fisheye
	// shaderArtificial->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderArtificial->setUniformLocation("clipping_fov");


	// shaderMoonBump = std::make_unique<shaderProgram>();
	// shaderMoonBump->init( "moon_bump.vert","", "","", "moon_bump.frag");
	// shaderMoonBump->setUniformLocation("UmbraColor");
	// //commum
	// shaderMoonBump->setUniformLocation("planetRadius");
	// shaderMoonBump->setUniformLocation("planetScaledRadius");
	// shaderMoonBump->setUniformLocation("planetOneMinusOblateness");

	// shaderMoonBump->setUniformLocation("SunHalfAngle");
	// shaderMoonBump->setUniformLocation("LightPosition");
	// shaderMoonBump->setUniformLocation("ModelViewProjectionMatrix");
	// shaderMoonBump->setUniformLocation("ModelViewMatrix");
	// shaderMoonBump->setUniformLocation("NormalMatrix");

	// shaderMoonBump->setUniformLocation("MoonPosition1");
	// shaderMoonBump->setUniformLocation("MoonRadius1");

	// //fisheye
	// shaderMoonBump->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderMoonBump->setUniformLocation("clipping_fov");



	// myMoon = std::make_unique<shaderProgram>();
	// myMoon->init( "my_moon.vert","my_moon.tesc", "my_moon.tese","my_moon.geom", "my_moon.frag");
	// myMoon->setUniformLocation("UmbraColor");
	// myMoon->setUniformLocation("TesParam");
	//
	// //commum
	// myMoon->setUniformLocation("planetRadius");
	// myMoon->setUniformLocation("planetScaledRadius");
	// myMoon->setUniformLocation("planetOneMinusOblateness");
	//
	// myMoon->setUniformLocation("SunHalfAngle");
	// myMoon->setUniformLocation("LightPosition");
	// myMoon->setUniformLocation("ModelViewProjectionMatrix");
	// myMoon->setUniformLocation("ModelViewMatrix");
	// myMoon->setUniformLocation("NormalMatrix");
	//
	// myMoon->setUniformLocation("MoonPosition1");
	// myMoon->setUniformLocation("MoonRadius1");
	//
	// //fisheye
	// myMoon->setUniformLocation("inverseModelViewProjectionMatrix");
	// myMoon->setUniformLocation("clipping_fov");


	// shaderMoonNormal = std::make_unique<shaderProgram>();
	// shaderMoonNormal->init( "moon_normal.vert", "moon_normal.frag");
	// //shaderMoonNormal->setUniformLocation("UmbraColor");

	// //commum
	// shaderMoonNormal->setUniformLocation("planetRadius");
	// shaderMoonNormal->setUniformLocation("planetScaledRadius");
	// shaderMoonNormal->setUniformLocation("planetOneMinusOblateness");

	// shaderMoonNormal->setUniformLocation("SunHalfAngle");
	// shaderMoonNormal->setUniformLocation("LightPosition");
	// shaderMoonNormal->setUniformLocation("ModelViewProjectionMatrix");
	// shaderMoonNormal->setUniformLocation("ModelViewMatrix");
	// shaderMoonNormal->setUniformLocation("NormalMatrix");

	// shaderMoonNormal->setUniformLocation("MoonPosition1");
	// shaderMoonNormal->setUniformLocation("MoonRadius1");

	// //fisheye
	// shaderMoonNormal->setUniformLocation("inverseModelViewProjectionMatrix");
	// shaderMoonNormal->setUniformLocation("clipping_fov");
}


// void BodyShader::deleteShader()
// {
// 	if (shaderNight) delete shaderNight;
// 	if (shaderMoonNight) delete shaderMoonNight;
// 	if (myEarth) delete myEarth;
// 	if (shaderBump) delete shaderBump;
// 	if (shaderRinged) delete shaderRinged;
// 	if (shaderNormal) delete shaderNormal;

// 	if (shaderArtificial) delete shaderArtificial;
// 	if (shaderMoonBump) delete shaderMoonBump;
// 	if (myMoon) delete myMoon;
// 	if (shaderMoonNormal) delete shaderMoonNormal;
// }
