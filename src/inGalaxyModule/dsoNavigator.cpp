/*
 * Copyright (C) 2020 of the Association Andromède
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

#include "inGalaxyModule/dsoNavigator.hpp"

#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Core/FrameMgr.hpp"

#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include "tools/context.hpp"
#include <cassert>

DsoNavigator::DsoNavigator(const std::string& tex_file, const std::string &tex3d_file, int depth)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    vertexArray = std::make_unique<VertexArray>(vkmgr);
    vertexArray->createBindingEntry(3*sizeof(float));
    vertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);
    vertexArray->createBindingEntry(sizeof(dso), VK_VERTEX_INPUT_RATE_INSTANCE);
    for (int i = 0; i < 8; ++i)
        vertexArray->addInput(VK_FORMAT_R32G32B32A32_SFLOAT); // model
    vertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT); // texOffset, coefScale, lod
    vertex = vertexArray->createBuffer(0, 8, context.globalBuffer.get());
    Vec3f *ptr = (Vec3f *) context.transfer->planCopy(vertex->get());
    ptr[0].set(-1,1,1); ptr[1].set(1,1,1);
    ptr[2].set(-1,-1,1); ptr[3].set(1,-1,1);
    ptr[4].set(-1,-1,-1); ptr[5].set(1,-1,-1);
    ptr[6].set(-1,1,-1); ptr[7].set(1,1,-1);
    index = context.indexBufferMgr->acquireBuffer(3*2*6*sizeof(uint16_t));
    uint16_t tmp[3*2*6] = {2,0,1, 1,3,2,
                           4,2,3, 3,5,4,
                           6,4,5, 5,7,6,
                           0,6,7, 7,1,0,
                           0,2,4, 4,6,0,
                           1,7,5, 5,3,1};
    memcpy(context.transfer->planCopy(index), tmp, 3*2*6*sizeof(uint16_t));

    texture = std::make_unique<s_texture>(tex3d_file, TEX_LOAD_TYPE_PNG_SOLID, true, 0, depth, 1, 2);
    colorTexture = std::make_unique<s_texture>(tex_file, TEX_LOAD_TYPE_PNG_SOLID);

    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 1);
    layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 2);
    layout->setTextureLocation(3, &PipelineLayout::DEFAULT_SAMPLER);
    layout->setTextureLocation(4, &PipelineLayout::DEFAULT_SAMPLER);
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
    pipeline->setCullMode(true);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    pipeline->setTessellationState(3);
    pipeline->disableSampleShading();
    pipeline->bindVertex(*vertexArray);
    pipeline->bindShader("obj3D.vert.spv");
    pipeline->bindShader("obj3D.tesc.spv");
    pipeline->bindShader("obj3D.tese.spv");
    pipeline->bindShader("obj3D.frag.spv");
    float maxLod = texture->getTexture().getMipmapCount() - 1;
    pipeline->setSpecializedConstant(0, &maxLod, sizeof(maxLod));
    int width, height;
	colorTexture->getDimensions(width, height);
    texScale = (width | height) ? (((float) height) / ((float) width)) : 0.f;
    pipeline->setSpecializedConstant(1, &texScale, sizeof(texScale));
    pipeline->build();

    set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
    uModelViewMatrix = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
    set->bindUniform(uModelViewMatrix, 0);
    uclipping_fov = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
    set->bindUniform(uclipping_fov, 1);
    uCamRotToLocal = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
    set->bindUniform(uCamRotToLocal, 2);
    set->bindTexture(texture->getTexture(), 3);
    set->bindTexture(colorTexture->getTexture(), 4);

    insert(Mat4f::translation(Vec3f(299.78,-163.55,-63.53)) * Mat4f::yrotation(3.1415926f/2.f) * Mat4f::scaling(Vec3f(1, 1, 0.5)), 0, 1);

    context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
    for (int i = 0; i < 3; ++i)
        context.frame[i]->setName(cmds[i], "DsoNav " + std::to_string(i));
}

DsoNavigator::~DsoNavigator() {}

void DsoNavigator::build()
{
    Context &context = *Context::instance;
    if (!needRebuild[context.frameIdx])
        return;
    needRebuild[context.frameIdx] = false;
    VkCommandBuffer &cmd = cmds[context.frameIdx];
    context.frame[context.frameIdx]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
    pipeline->bind(cmd);
    layout->bindSet(cmd, *set);
    VertexArray::bind(cmd, {vertex.get(), instance.get()});
    vkCmdBindIndexBuffer(cmd, index.buffer, index.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, 3*2*6, instanceCount, 0, 0, 0);
    context.frame[context.frameIdx]->compile(cmd);
}

//! Sort dso in depth-first order, linear in time when already sorted
void DsoNavigator::computePosition(Vec3f posI, const Projector *prj)
{
    bool changed = false;
    if ((int) dsoData.size() != instanceCount) {
        instanceCount = dsoData.size();
        instance.reset();
        instance = vertexArray->createBuffer(1, instanceCount, nullptr, Context::instance->globalBuffer.get());
        needRebuild[0] = true;
        needRebuild[1] = true;
        needRebuild[2] = true;
        changed = true;
    }
    if (instanceCount == 0) return;
    float lengthSquared = (dsoPos[instanceCount - 1] - posI).lengthSquared();
    Vec3f tmpPos;
    dso tmpData;
    int swapI;
    bool invertMove = false;
    const float coef = 2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
    float rad = 1.f / dsoData[instanceCount - 1].data[1];
    float tmpLod = (lengthSquared > rad*rad) ? std::floor(-std::log2(atanf(rad / sqrt(lengthSquared-rad*rad)) * coef)) : 0;
    if (dsoData[instanceCount - 1].data[2] != tmpLod) {
        changed = true;
        dsoData[instanceCount - 1].data[2] = tmpLod;
    }
    for (int i = instanceCount - 2; i >= 0 || invertMove; --i) {
        float lengthSquared2 = (dsoPos[i + invertMove] - posI).lengthSquared();
        if (invertMove) {
            if (lengthSquared < lengthSquared2) {
                changed = true;
                tmpPos = dsoPos[i];
                dsoPos[i] = dsoPos[i + 1];
                dsoPos[i + 1] = tmpPos;
                tmpData = dsoData[i];
                dsoData[i] = dsoData[i + 1];
                dsoData[i + 1] = tmpData;
                i += 2;
                if (i < instanceCount)
                    continue;
            }
            i = swapI;
            lengthSquared = (dsoPos[i] - posI).lengthSquared();
            invertMove = false;
        } else {
            rad = 1.f / dsoData[i].data[1];
            tmpLod = (lengthSquared2 > rad*rad) ? std::floor(-std::log2(atanf(rad / sqrt(lengthSquared2-rad*rad)) * coef)) : 0;
            if (dsoData[i].data[2] != tmpLod) {
                changed = true;
                dsoData[i].data[2] = tmpLod;
            }
            if (lengthSquared > lengthSquared2) {
                changed = true;
                tmpPos = dsoPos[i];
                dsoPos[i] = dsoPos[i + 1];
                dsoPos[i + 1] = tmpPos;
                tmpData = dsoData[i];
                dsoData[i] = dsoData[i + 1];
                dsoData[i + 1] = tmpData;
                if (i < instanceCount - 2) {
                    swapI = i;
                    i += 2;
                    invertMove = true;
                }
            }
            lengthSquared = lengthSquared2;
        }
    }
    if (changed) {
        dso *dst = (dso *) Context::instance->transfer->planCopy(instance->get());
        dso *src = dsoData.data();
        int i = dsoData.size();
        while (i--)
            *(dst++) = *(src++);
    }
}

void DsoNavigator::insert(const Mat4f &model, int textureID, float unscale)
{
    dsoData.push_back({model, model.inverse(), Vec3f(texScale * textureID, unscale, 0)});
    dsoPos.emplace_back(model.r[12], model.r[13], model.r[14]);
}

void DsoNavigator::draw(const Navigator * nav, const Projector* prj)
{
    if (instanceCount == 0) return;
    Context &context = *Context::instance;
    if (needRebuild[context.frameIdx])
        build();
    Mat4f mat = nav->getHelioToEyeMat().convert();
    *uModelViewMatrix = mat;
    *uclipping_fov = prj->getClippingFov();
    *uCamRotToLocal = mat.inverse();
    context.frame[context.frameIdx]->toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
}
