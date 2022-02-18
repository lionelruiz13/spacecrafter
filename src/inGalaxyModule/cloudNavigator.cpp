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
#include "inGalaxyModule/cloudNavigator.hpp"

#include "tools/context.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"

#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include <cassert>

CloudNavigator::CloudNavigator(const std::string &filename)
{
    Context &context = *Context::instance;
    VulkanMgr &vkmgr = *VulkanMgr::instance;

    vertexArray = std::make_unique<VertexArray>(vkmgr);
    vertexArray->createBindingEntry(3*sizeof(float));
    vertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);
    vertexArray->createBindingEntry(sizeof(cloud), VK_VERTEX_INPUT_RATE_INSTANCE);
    vertexArray->addInput(VK_FORMAT_R32G32B32A32_SFLOAT); // color
    for (int i = 0; i < 8; ++i)
        vertexArray->addInput(VK_FORMAT_R32G32B32A32_SFLOAT); // model invmodel
    vertexArray->addInput(VK_FORMAT_R32_SFLOAT); // lodFactor
    vertex = vertexArray->createBuffer(0, 8, context.globalBuffer.get());
    Vec3f *ptr = (Vec3f *) context.transfer->planCopy(vertex->get());
    ptr[0].set(-1,1,1); ptr[1].set(1,1,1);
    ptr[2].set(-1,-1,1); ptr[3].set(1,-1,1);
    ptr[4].set(-1,-1,-1); ptr[5].set(1,-1,-1);
    ptr[6].set(-1,1,-1); ptr[7].set(1,1,-1);
    uint16_t tmp[3*2*6] = {2,0,1, 1,3,2,
                           4,2,3, 3,5,4,
                           6,4,5, 5,7,6,
                           0,6,7, 7,1,0,
                           0,2,4, 4,6,0,
                           1,7,5, 5,3,1};
    index = context.indexBufferMgr->acquireBuffer(3*2*6*sizeof(uint16_t));
    memcpy(context.transfer->planCopy(index), tmp, index.size);

    texture = std::make_unique<s_texture>("volum_cloud.png", TEX_LOAD_TYPE_PNG_SOLID, true, 0, 256, 1);

    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 1);
    layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 2);
    layout->setTextureLocation(3, &PipelineLayout::DEFAULT_SAMPLER);
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
    pipeline->setDepthStencilMode(VK_TRUE, VK_FALSE);
    pipeline->setCullMode(true);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    pipeline->setTessellationState(3);
    pipeline->disableSampleShading();
    pipeline->bindVertex(*vertexArray);
    pipeline->bindShader("cloud3D.vert.spv");
    pipeline->bindShader("cloud3D.tesc.spv");
    pipeline->bindShader("cloud3D.tese.spv");
    pipeline->bindShader("cloud3D.frag.spv");
    float maxLod = texture->getTexture().getMipmapCount() - 1;
    pipeline->setSpecializedConstant(0, &maxLod, sizeof(maxLod));
    float minLod = 0.; // The original texture is too big
    pipeline->setSpecializedConstant(1, &minLod, sizeof(minLod));
    pipeline->build();

    set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
    uModelViewMatrix = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
    set->bindUniform(uModelViewMatrix, 0);
    uclipping_fov = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
    set->bindUniform(uclipping_fov, 1);
    uCamRotToLocal = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
    set->bindUniform(uCamRotToLocal, 2);
    set->bindTexture(texture->getTexture(), 3);

    for (int i = 0; i < 3; ++i) {
        cmds[i] = context.frame[i]->create(1);
        context.frame[i]->setName(cmds[i], "cloud3D");
    }
    //addRule(6, Vec4f(0.5, 0.5, 0.9, 1.), 1.0);
    addRule(8, Vec4f(0, 0, 0, 1.), 0.8);
    addRule(9, Vec4f(0.5, 0.05, 0.25, 2.), 0.5);
    if (!filename.empty())
        loadCatalog(filename);
    // //=============== TEST ===============//
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(61.6645, 106.7731717, 8.732473606)) * Mat4f::scaling(1));
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(57.8494, 101.6510559, 8.009581944)) * Mat4f::scaling(1));
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(62.9656, 106.519477, 9.064920313)) * Mat4f::scaling(1));
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(59.1472, 101.1386578, 8.995018229)) * Mat4f::scaling(1));
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(58.6639, 100.3883491, 8.040727743)) * Mat4f::scaling(1));
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(63.3615, 107.8446613, 9.884306462)) * Mat4f::scaling(1));
    // insert(Vec4f(0.08, 0.5, 0.8, 2.), Mat4f::translation(Vec3f(57.7345, 101.5458561, 8.163640626)) * Mat4f::scaling(1));
}

CloudNavigator::~CloudNavigator() {}

void CloudNavigator::build(int nbClouds)
{
    Context &context = *Context::instance;

    instanceCount = nbClouds;
    instance.reset();
    instance = vertexArray->createBuffer(1, nbClouds, nullptr, context.globalBuffer.get());
}

//! Sort clouds in depth-first order, linear in time when already sorted
void CloudNavigator::computePosition(Vec3f posI, const Projector *prj)
{
    bool changed = false;
    if ((int) cloudData.size() != instanceCount) {
        build(cloudData.size());
        changed = true;
    }
    if (instanceCount == 0) return;
    float lengthSquared = (cloudPos[instanceCount - 1] - posI).lengthSquared();
    Vec3f tmpPos;
    cloud tmpData;
    int swapI;
    bool invertMove = false;
    const float coef = 2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
    const float radSquared = rad*rad;
    float tmpLod = (lengthSquared > radSquared) ? std::floor(-std::log2(atanf(rad / sqrt(lengthSquared-radSquared)) * coef)) : 0;
    if (cloudData[instanceCount - 1].lodFactor != tmpLod) {
        changed = true;
        cloudData[instanceCount - 1].lodFactor = tmpLod;
    }
    for (int i = instanceCount - 2; i >= 0 || invertMove; --i) {
        float lengthSquared2 = (cloudPos[i + invertMove] - posI).lengthSquared();
        if (invertMove) {
            if (lengthSquared < lengthSquared2) {
                tmpPos = cloudPos[i];
                cloudPos[i] = cloudPos[i + 1];
                cloudPos[i + 1] = tmpPos;
                tmpData = cloudData[i];
                cloudData[i] = cloudData[i + 1];
                cloudData[i + 1] = tmpData;
                changed = true;
                i += 2;
                if (i < instanceCount)
                    continue;
            }
            i = swapI;
            lengthSquared = (cloudPos[i] - posI).lengthSquared();
            invertMove = false;
        } else {
            tmpLod = (lengthSquared2 > radSquared) ? std::floor(-std::log2(atanf(rad / sqrt(lengthSquared2-radSquared)) * coef)) : 0;
            if (cloudData[i].lodFactor != tmpLod) {
                changed = true;
                cloudData[i].lodFactor = tmpLod;
            }
            if (lengthSquared > lengthSquared2) {
                tmpPos = cloudPos[i];
                cloudPos[i] = cloudPos[i + 1];
                cloudPos[i + 1] = tmpPos;
                tmpData = cloudData[i];
                cloudData[i] = cloudData[i + 1];
                cloudData[i + 1] = tmpData;
                changed = true;
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
        memcpy(Context::instance->transfer->planCopy(instance->get()), cloudData.data(), cloudData.size() * sizeof(cloud));
    }
}

void CloudNavigator::insert(const Vec4f &color, const Mat4f &model)
{
    cloudData.push_back({color, model, model.inverse(), 8});
    cloudPos.emplace_back(model.r[12], model.r[13], model.r[14]);
}

void CloudNavigator::draw(const Navigator * nav, const Projector* prj)
{
    draw(nav->getHelioToEyeMat().convert(), prj);
}

void CloudNavigator::draw(const Mat4f &mat, const Projector* prj)
{
    if (instanceCount == 0) return;
    Context &context = *Context::instance;
    *uModelViewMatrix = mat;
    *uclipping_fov = prj->getClippingFov();
    *uCamRotToLocal = mat.inverse();
    FrameMgr &frame = *context.frame[context.frameIdx];
    auto &cmd = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
    pipeline->bind(cmd);
    layout->bindSet(cmd, *set);
    VertexArray::bind(cmd, {vertex.get(), instance.get()});
    vkCmdBindIndexBuffer(cmd, index.buffer, index.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, 3*2*6, instanceCount, 0, 0, 0);
    frame.compile(cmd);
    frame.toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

void CloudNavigator::addRule(int index, Vec4f color, float scaling)
{
    types[index] = cloudType{color, scaling};
}

void CloudNavigator::loadCatalog(const std::string &filename)
{
    std::ifstream file(filename);
    float x, y, z;
    while (file) {
        int t = -1;
        try {
            file >> t >> x >> y >> z;
            auto &data = types.at(t);
            Vec3f pos(x, y, z);
            pos *= scaling;
            insert(data.color, Mat4f::translation(pos) * Mat4f::scaling(rad * data.scaling));
        } catch (...) {
        }
    }
}
