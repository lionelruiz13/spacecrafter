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
#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include <cassert>

DsoNavigator::DsoNavigator(ThreadContext *_context, const std::string& tex_file)
{
    context = _context;
    commandIndex = context->commandMgrDynamic->getCommandIndex();
    vertex = std::make_unique<VertexArray>(context->surface);
    vertex->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    for (int i = 0; i < 8; ++i)
        vertex->registerInstanceBuffer(BufferAccess::STREAM, VK_FORMAT_R32G32B32A32_SFLOAT); // model
    vertex->registerInstanceBuffer(BufferAccess::STREAM, VK_FORMAT_R32G32B32_SFLOAT); // texOffset, coefScale
    vertex->setInstanceBufferStride(sizeof(*pInstance));
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

    texture = std::make_unique<Texture>(context->surface, context->global->textureMgr, "dso3d.png", false, true, true, VK_FORMAT_R16_UNORM, 1, true, true, 2);
    if (!texture->isValid()) {
        cLog::get()->write("Uninitialized 3d texture for dsoNavigator", LOG_TYPE::L_ERROR, LOG_FILE::VULKAN);
        return;
    }
    colorTexture = std::make_unique<s_texture>(tex_file, TEX_LOAD_TYPE_PNG_SOLID);

    layout = std::make_unique<PipelineLayout>(context->surface);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 1);
    layout->setUniformLocation(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 2);
    layout->setTextureLocation(3, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT, &texture->getInfo()->sampler);
    layout->setTextureLocation(4, nullptr, VK_SHADER_STAGE_FRAGMENT_BIT, &colorTexture->getTexture()->getInfo()->sampler);
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
    pipeline->setCullMode(true);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    pipeline->setTessellationState(3);
    pipeline->disableSampleShading();
    pipeline->bindVertex(vertex.get());
    pipeline->bindShader("obj3D.vert.spv");
    pipeline->bindShader("obj3D.tesc.spv");
    pipeline->bindShader("obj3D.tese.spv");
    pipeline->bindShader("obj3D.frag.spv");
    float maxLod = texture->getMipmapCount() - 1;
    pipeline->setSpecializedConstant(0, &maxLod, sizeof(maxLod));
    int width, height;
	colorTexture->getDimensions(width, height);
    texScale = (width | height) ? (((float) height) / ((float) width)) : 0.f;
    pipeline->setSpecializedConstant(1, &texScale, sizeof(texScale));
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
    set->bindTexture(colorTexture->getTexture(), 4);

    insert(Mat4f::translation(Vec3f(299.78,-163.55,-63.53)) * Mat4f::yrotation(3.1415926f/2.f) * Mat4f::scaling(Vec3f(1, 1, 0.5)), 0, 1);
}

DsoNavigator::~DsoNavigator() {}

void DsoNavigator::build(int nbDso)
{
    context->commandMgr->waitGraphicQueueIdle();
	context->commandMgr->waitCompletion(0);
	context->commandMgr->waitCompletion(1);
	context->commandMgr->waitCompletion(2);
    vertex->buildInstanceBuffer(nbDso);
    pInstance = reinterpret_cast<typeof(pInstance)>(vertex->getInstanceBufferPtr());
    CommandMgr *cmdMgr = context->commandMgrDynamic;
    cmdMgr->init(commandIndex, pipeline.get(), renderPassType::USE_DEPTH_BUFFER);
    cmdMgr->bindVertex(vertex.get());
    cmdMgr->bindSet(layout.get(), set.get());
    cmdMgr->drawIndexed(3*2*6, nbDso);
    cmdMgr->compile();
}

//! Sort dso in depth-first order, linear in time when already sorted
void DsoNavigator::computePosition(Vec3f posI, const Projector *prj)
{
    if ((int) dsoData.size() != instanceCount) {
        memcpy(reinterpret_cast<void*>(dsoData.data()), pInstance, instanceCount * sizeof(dso));
        instanceCount = dsoData.size();
        build(instanceCount);
        memcpy(reinterpret_cast<void*>(pInstance), dsoData.data(), instanceCount * sizeof(dso));
    }
    if (instanceCount == 0) return;
    float lengthSquared = (dsoPos[instanceCount - 1] - posI).lengthSquared();
    Vec3f tmpPos;
    dso tmpData;
    int swapI;
    bool invertMove = false;
    const float coef = 2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
    float rad = 1.f / pInstance[instanceCount - 1].data[1];
    pInstance[instanceCount - 1].data[2] = (lengthSquared > rad*rad) ? std::floor(-std::log2(atanf(rad / sqrt(lengthSquared-rad*rad)) * coef)) : 0;
    for (int i = instanceCount - 2; i >= 0 || invertMove; --i) {
        float lengthSquared2 = (dsoPos[i + invertMove] - posI).lengthSquared();
        if (invertMove) {
            if (lengthSquared < lengthSquared2) {
                tmpPos = dsoPos[i];
                dsoPos[i] = dsoPos[i + 1];
                dsoPos[i + 1] = tmpPos;
                tmpData = pInstance[i];
                pInstance[i] = pInstance[i + 1];
                pInstance[i + 1] = tmpData;
                i += 2;
                if (i < instanceCount)
                    continue;
            }
            i = swapI;
            lengthSquared = (dsoPos[i] - posI).lengthSquared();
            invertMove = false;
        } else {
            rad = 1.f / pInstance[i].data[1];
            pInstance[i].data[2] = (lengthSquared2 > rad*rad) ? std::floor(-std::log2(atanf(rad / sqrt(lengthSquared2-rad*rad)) * coef)) : 0;
            if (lengthSquared > lengthSquared2) {
                tmpPos = dsoPos[i];
                dsoPos[i] = dsoPos[i + 1];
                dsoPos[i + 1] = tmpPos;
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

void DsoNavigator::insert(const Mat4f &model, int textureID, float unscale)
{
    if (!texture->isValid()) return;
    dsoData.push_back({model, model.inverse(), Vec3f(texScale * textureID, unscale, 0)});
    dsoPos.emplace_back(model.r[12], model.r[13], model.r[14]);
}

void DsoNavigator::draw(const Navigator * nav, const Projector* prj)
{
    if (instanceCount == 0) return;
    Mat4f mat = nav->getHelioToEyeMat().convert();
    *pModelViewMatrix = mat;
    *pclipping_fov = prj->getClippingFov();
    *pCamRotToLocal = mat.inverse();
    context->commandMgrDynamic->setSubmission(commandIndex, false, context->commandMgr);
}
