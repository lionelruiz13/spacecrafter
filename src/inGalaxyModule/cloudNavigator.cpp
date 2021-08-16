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

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/VertexArray.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"
#include "vulkanModule/Texture.hpp"

#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include <cassert>

CloudNavigator::CloudNavigator(ThreadContext *_context)
{
    context = _context;
    assert(sizeof(*pInstance) == 4*4 + 4*4*4 + 4*4*4);
    commandIndex = context->commandMgrDynamic->getCommandIndex();
    vertex = std::make_unique<VertexArray>(context->surface);
    vertex->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    vertex->registerInstanceBuffer(BufferAccess::STREAM, VK_FORMAT_R32G32B32A32_SFLOAT); // color
    for (int i = 0; i < 8; ++i)
        vertex->registerInstanceBuffer(BufferAccess::STREAM, VK_FORMAT_R32G32B32A32_SFLOAT); // model
    vertex->registerIndexBuffer(BufferAccess::STATIC, 3*2*6, 2, VK_INDEX_TYPE_UINT16);
    vertex->build(8);
    Vec3f *ptr = reinterpret_cast<Vec3f *>(vertex->getStagingVertexBufferPtr());
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
    vertex->assumeVerticeChanged();
    vertex->fillIndexBuffer(3*6, reinterpret_cast<uint32_t*>(&(tmp[0])));

    texture = std::make_unique<Texture>(context->surface, context->global->textureMgr, "volum_cloud.png", false, true, true, VK_FORMAT_R8_UNORM, 1, true);

    layout = std::make_unique<PipelineLayout>(context->surface);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 1);
    layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 2);
    layout->setTextureLocation(3, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT, texture->isValid() ? &texture->getInfo()->sampler : nullptr);
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
    pipeline->setDepthStencilMode(VK_TRUE, VK_FALSE);
    pipeline->setCullMode(true);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    pipeline->setTessellationState(3);
    pipeline->disableSampleShading();
    pipeline->bindVertex(vertex.get());
    pipeline->bindShader("cloud3D.vert.spv");
    pipeline->bindShader("cloud3D.tesc.spv");
    pipeline->bindShader("cloud3D.tese.spv");
    pipeline->bindShader("cloud3D.frag.spv");
    float maxLod = texture->getMipmapCount() - 1;
    pipeline->setSpecializedConstant(0, &maxLod, sizeof(maxLod));
    pipeline->build();

    set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
    uModelViewMatrix = std::make_unique<Uniform>(context->surface, sizeof(*pModelViewMatrix));
    pModelViewMatrix = static_cast<typeof(pModelViewMatrix)>(uModelViewMatrix->data);
    set->bindUniform(uModelViewMatrix.get(), 0);
    uclipping_fov = std::make_unique<Uniform>(context->surface, sizeof(*pclipping_fov));
    pclipping_fov = static_cast<typeof(pclipping_fov)>(uclipping_fov->data);
    set->bindUniform(uclipping_fov.get(), 1);
    uCamRotToLocal = std::make_unique<Uniform>(context->surface, sizeof(*pCamRotToLocal));
    pCamRotToLocal = static_cast<typeof(pCamRotToLocal)>(uCamRotToLocal->data);
    set->bindUniform(uCamRotToLocal.get(), 2);
    set->bindTexture(texture.get(), 3);
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
    context->commandMgr->waitGraphicQueueIdle();
	context->commandMgr->waitCompletion(0);
	context->commandMgr->waitCompletion(1);
	context->commandMgr->waitCompletion(2);
    vertex->buildInstanceBuffer(nbClouds);
    pInstance = reinterpret_cast<typeof(pInstance)>(vertex->getInstanceBufferPtr());
    CommandMgr *cmdMgr = context->commandMgrDynamic;
    cmdMgr->init(commandIndex, pipeline.get(), renderPassType::USE_DEPTH_BUFFER_DONT_SAVE);
    cmdMgr->bindVertex(vertex.get());
    cmdMgr->bindSet(layout.get(), set.get());
    cmdMgr->drawIndexed(3*2*6, nbClouds);
    cmdMgr->compile();
}

//! Sort clouds in depth-first order, linear in time when already sorted
void CloudNavigator::computePosition(Vec3f posI)
{
    if ((int) cloudData.size() != instanceCount) {
        memcpy(reinterpret_cast<void*>(cloudData.data()), pInstance, instanceCount * sizeof(cloud));
        instanceCount = cloudData.size();
        build(instanceCount);
        memcpy(reinterpret_cast<void*>(pInstance), cloudData.data(), instanceCount * sizeof(cloud));
    }
    if (instanceCount == 0) return;
    float lengthSquared = (cloudPos[instanceCount - 1] - posI).lengthSquared();
    Vec3f tmpPos;
    cloud tmpData;
    int swapI;
    bool invertMove = false;
    for (int i = instanceCount - 2; i >= 0 || invertMove; --i) {
        float lengthSquared2 = (cloudPos[i + invertMove] - posI).lengthSquared();
        if (invertMove) {
            if (lengthSquared < lengthSquared2) {
                tmpPos = cloudPos[i];
                cloudPos[i] = cloudPos[i + 1];
                cloudPos[i + 1] = tmpPos;
                tmpData = pInstance[i];
                pInstance[i] = pInstance[i + 1];
                pInstance[i + 1] = tmpData;
                i += 2;
                if (i < instanceCount)
                    continue;
            }
            i = swapI;
            lengthSquared = (cloudPos[i] - posI).lengthSquared();
            invertMove = false;
        } else {
            if (lengthSquared > lengthSquared2) {
                tmpPos = cloudPos[i];
                cloudPos[i] = cloudPos[i + 1];
                cloudPos[i + 1] = tmpPos;
                tmpData = pInstance[i];
                pInstance[i] = pInstance[i + 1];
                pInstance[i + 1] = tmpData;
                if (i < instanceCount - 2) {
                    swapI = i;
                    i += 2;
                    invertMove = true;
                }
            }
            lengthSquared = lengthSquared2;
        }
    }
}

void CloudNavigator::insert(const Vec4f &color, const Mat4f &model)
{
    cloudData.push_back({color, model, model.inverse()});
    cloudPos.emplace_back(model.r[12], model.r[13], model.r[14]);
}

void CloudNavigator::draw(const Navigator * nav, const Projector* prj)
{
    if (instanceCount == 0) return;
    Mat4f mat = nav->getHelioToEyeMat().convert();
    *pModelViewMatrix = mat;
    *pclipping_fov = prj->getClippingFov();
    *pCamRotToLocal = mat.inverse();
    context->commandMgrDynamic->setSubmission(commandIndex, false, context->commandMgr);
}

void CloudNavigator::draw(const Mat4f &mat, const Projector* prj)
{
    if (instanceCount == 0) return;
    *pModelViewMatrix = mat;
    *pclipping_fov = prj->getClippingFov();
    *pCamRotToLocal = mat.inverse();
    context->commandMgrDynamic->setSubmission(commandIndex, false, context->commandMgr);
}
