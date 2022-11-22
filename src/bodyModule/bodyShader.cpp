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
drawState_t BodyShader::myMoon;
drawState_t BodyShader::shaderArtificial;
drawState_t BodyShader::depthTrace;


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
	shaderNight.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	shaderNight.pipeline->bindShader("body_night.frag.spv");
	shaderNight.pipelineNoDepth = shaderNight.pipeline->clone("shaderNight noDepth");
	shaderNight.pipelineNoDepth->setDepthStencilMode();
	context.pipelines.emplace_back(shaderNight.pipelineNoDepth);
	shaderNight.pipeline->build("shaderNight");

	// ========== my_earth ========== //
	context.layouts.push_back(std::make_unique<PipelineLayout>(vkmgr));
	myEarth.layout = context.layouts.back().get();
	myEarth.layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	myEarth.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	myEarth.layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_VERTEX_BIT, 2); // globalTescGeom
	myEarth.layout->setTextureLocation(5, &tmp);
	myEarth.layout->setTextureLocation(6, &PipelineLayout::DEFAULT_SAMPLER);
	myEarth.layout->setTextureLocation(7, &tmp);
	myEarth.layout->setTextureLocation(8, &tmp);
	myEarth.layout->setTextureLocation(9, &tmp);
	myEarth.layout->setTextureLocation(10, &tmp);
	myEarth.layout->setTextureLocation(11, &tmp, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
	myEarth.layout->buildLayout();
	myEarth.layout->setGlobalPipelineLayout(context.layouts.front().get());
	myEarth.layout->build();

	myEarth.pipeline = new Pipeline[4]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myEarth.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myEarth.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myEarth.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, myEarth.layout}};
	context.pipelineArray.push_back(myEarth.pipeline);
	for (int i = 0; i < 4; ++i) {
		myEarth.pipeline[i].setCullMode(true);
		myEarth.pipeline[i].setFrontFace(); // Body with tesselation don't have the same front face...
		myEarth.pipeline[i].setBlendMode(BLEND_NONE);
		myEarth.pipeline[i].bindVertex(*context.ojmVertexArray);
		myEarth.pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
		myEarth.pipeline[i].setTessellationState(3);
		myEarth.pipeline[i].bindShader("body_tes.vert.spv");
		myEarth.pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
		myEarth.pipeline[i].bindShader("body_tes.tesc.spv");
		myEarth.pipeline[i].bindShader("my_earth.tese.spv");
		myEarth.pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
		myEarth.pipeline[i].bindShader("my_earth.frag.spv");
		VkBool32 Clouds = (i & 1);
		myEarth.pipeline[i].setSpecializedConstant(1, &Clouds, sizeof(Clouds));
		if (i & 2) {
			myEarth.pipeline[i].setDepthStencilMode();
			myEarth.pipeline[i].build("myEarth noDepth");
		} else
			myEarth.pipeline[i].build("myEarth");
	}
	myEarth.pipelineNoDepth = myEarth.pipeline + 2;

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
	shaderBump.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	shaderBump.pipeline->bindShader("body_bump.frag.spv");
	shaderBump.pipelineNoDepth = shaderBump.pipeline->clone("shaderBump noDepth");
	shaderBump.pipelineNoDepth->setDepthStencilMode();
	context.pipelines.emplace_back(shaderBump.pipelineNoDepth);
	shaderBump.pipeline->build("shaderBump");

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
	shaderRinged.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	shaderRinged.pipeline->bindShader("body_ringed.frag.spv");
	shaderRinged.pipeline->build("shaderRinged");

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
	shaderNormal.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	shaderNormal.pipeline->bindShader("body_normal.frag.spv");
	shaderNormal.pipelineNoDepth = shaderNormal.pipeline->clone("shaderNormal noDepth");
	shaderNormal.pipelineNoDepth->setDepthStencilMode();
	context.pipelines.emplace_back(shaderNormal.pipelineNoDepth);
	shaderNormal.pipeline->build("shaderNormal");

	// ========== body_normal_tes ========== //
	shaderNormalTes.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(shaderNormalTes.layout);
	shaderNormalTes.layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	shaderNormalTes.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // globalFrag
	shaderNormalTes.layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_VERTEX_BIT, 2); // globalTescGeom
	shaderNormalTes.layout->setTextureLocation(5, &tmp, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT); // heightmapTexture
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
	shaderNormalTes.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	shaderNormalTes.pipeline->bindShader("body_tes.tesc.spv");
	shaderNormalTes.pipeline->bindShader("body_normal_tes.tese.spv");
	shaderNormalTes.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	shaderNormalTes.pipeline->bindShader("body_normal_tes.frag.spv");
	shaderNormalTes.pipelineNoDepth = shaderNormalTes.pipeline->clone("shaderNormalTes noDepth");
	shaderNormalTes.pipelineNoDepth->setDepthStencilMode();
	context.pipelines.emplace_back(shaderNormalTes.pipelineNoDepth);
	shaderNormalTes.pipeline->build("shaderNormalTes");

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

	shaderArtificial.pipeline = new Pipeline[4]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderArtificial.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderArtificial.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderArtificial.layout}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, shaderArtificial.layout}};
	context.pipelineArray.push_back(shaderArtificial.pipeline);
	for (short i = 0; i < 4; ++i) {
		shaderArtificial.pipeline[i].setCullMode(true);
		shaderArtificial.pipeline[i].setBlendMode(BLEND_NONE);
		shaderArtificial.pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		shaderArtificial.pipeline[i].bindVertex(*context.ojmVertexArray);
		shaderArtificial.pipeline[i].bindShader("body_artificial.vert.spv");
		shaderArtificial.pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
		shaderArtificial.pipeline[i].bindShader("body_artificial.geom.spv");
		shaderArtificial.pipeline[i].bindShader((i & 1) ? "body_artificial_notex.frag.spv" : "body_artificial_tex.frag.spv");
		if (i & 2)
			shaderArtificial.pipeline[i].setDepthStencilMode();
	}
	shaderArtificial.pipeline[0].build("shaderArtificial textured");
	shaderArtificial.pipeline[1].build("shaderArtificial colored");
	shaderArtificial.pipelineNoDepth = shaderArtificial.pipeline + 2;
	shaderArtificial.pipelineNoDepth[0].build("shaderArtificial textured noDepth");
	shaderArtificial.pipelineNoDepth[1].build("shaderArtificial colored noDepth");

	// ========== my_moon ========== //
	myMoon.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(myMoon.layout);
	myMoon.layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0); // globalVertProj
	myMoon.layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1); // moonFrag
	myMoon.layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_VERTEX_BIT, 2); // globalTescGeom
	myMoon.layout->setTextureLocation(5, &tmp); // mapTexture
	myMoon.layout->setTextureLocation(6, &tmp); // normalTexture
	myMoon.layout->setTextureLocation(7, &PipelineLayout::DEFAULT_SAMPLER); // shadowTexture
	myMoon.layout->setTextureLocation(8, &tmp, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT); // heightmapTexture
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
	myMoon.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	myMoon.pipeline->bindShader("body_tes.tesc.spv");
	myMoon.pipeline->bindShader("my_moon.tese.spv");
	myMoon.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	myMoon.pipeline->bindShader("my_moon.frag.spv");
	myMoon.pipelineNoDepth = myMoon.pipeline->clone("myMoon noDepth");
	myMoon.pipelineNoDepth->setDepthStencilMode();
	context.pipelines.emplace_back(myMoon.pipelineNoDepth);
	myMoon.pipeline->build("myMoon");

	// ========== depthTrace ========== //
	depthTrace.layout = new PipelineLayout(vkmgr);
	context.layouts.emplace_back(depthTrace.layout);
	depthTrace.layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(depthTraceInfo));
	depthTrace.layout->build();

	depthTrace.pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, depthTrace.layout);
	context.pipelines.emplace_back(depthTrace.pipeline);
	depthTrace.pipeline->setCullMode(true);
	depthTrace.pipeline->setBlendMode(BLEND_NONE);
	depthTrace.pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	depthTrace.pipeline->bindVertex(*context.ojmVertexArray);
	depthTrace.pipeline->removeVertexEntry(1);
	depthTrace.pipeline->removeVertexEntry(2);
	depthTrace.pipeline->bindShader("body_depth_trace.vert.spv");
	depthTrace.pipeline->setSpecializedConstant(7, context.isFloat64Supported);
	depthTrace.pipeline->build("depthTrace");
}
