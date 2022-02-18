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

#include "volumObj3D.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/s_texture.hpp"
#include "tools/context.hpp"
#include "tools/log.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"

std::weak_ptr<VolumObj3D::Shared> VolumObj3D::refShared;

VolumObj3D::VolumObj3D(const std::string& tex_color_file, const std::string &tex_absorbtion_file, bool z_reflection) : transform(*Context::instance->uniformMgr), ray(*Context::instance->uniformMgr), shared(refShared.lock())
{
    if (!shared)
        refShared = shared = std::make_shared<Shared>();
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;

    int mapDepth;
    {
        int tmpSize = tex_absorbtion_file.find_last_of('.');
        int tmpPos = tex_absorbtion_file.find_last_of('d', tmpSize) + 1;
        mapDepth = std::stoi(tex_absorbtion_file.substr(tmpPos, tmpSize - tmpPos));
        tmpSize = tex_color_file.find_last_of('.');
        tmpPos = tex_color_file.find_last_of('d', tmpSize) + 1;
        int colorDepth = std::stoi(tex_color_file.substr(tmpPos, tmpSize - tmpPos));
        mapTexture = std::make_unique<s_texture>(tex_absorbtion_file, TEX_LOAD_TYPE_PNG_SOLID, false, false, mapDepth, 1, 1);
        colorTexture = std::make_unique<s_texture>(tex_color_file, TEX_LOAD_TYPE_PNG_SOLID, false, false, colorDepth, 4, 1);
    }
    set = std::make_unique<Set>(vkmgr, *context.setMgr, shared->layout.get(), -1, true, true);
    set->bindUniform(transform, 0);
    set->bindUniform(ray, 1);
    set->bindTexture(mapTexture->getTexture(), 2);
    set->bindTexture(colorTexture->getTexture(), 3);
    for (int i = 0; i < 3; ++i) {
        cmds[i] = context.frame[i]->create(1);
        context.frame[i]->setName(cmds[i], "VolumObj3D " + std::to_string(i));
    }
    // Assume width and height are equal
    ray->texCoef = Vec3f(1, 1, (z_reflection) ? 2 : 1);
    ray->rayPoints = 512;
    setModel(Mat4f::scaling(0.01), Vec3f(1, 1, 1/8.));
    isLoaded = true;
    int size;
    mapTexture->getDimensions(size, size);
    if (size < 8)
        isLoaded = false;
    colorTexture->getDimensions(size, size);
    if (size < 8)
        isLoaded = false;
    if (!isLoaded)
        cLog::get()->write("Volumetric texture missing", LOG_TYPE::L_WARNING);
}

VolumObj3D::~VolumObj3D()
{
    for (int i = 0; i < 3; ++i) {
        Context::instance->frame[i]->destroy(cmds[i]);
    }
}

VolumObj3D::Shared::Shared()
{
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;

    vertexArray = std::make_unique<VertexArray>(vkmgr);
    vertexArray->createBindingEntry(3*sizeof(float));
    vertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);

    vertex = vertexArray->createBuffer(0, 8, context.globalBuffer.get());
    Vec3f *ptr = (Vec3f *) context.transfer->planCopy(vertex->get());
    // Volumetric 3D object
    ptr[0].set(-1,1,1); ptr[1].set(1,1,1);
    ptr[2].set(-1,-1,1); ptr[3].set(1,-1,1);
    ptr[4].set(-1,-1,-1); ptr[5].set(1,-1,-1);
    ptr[6].set(-1,1,-1); ptr[7].set(1,1,-1);

    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkSamplerCreateInfo sampler = PipelineLayout::DEFAULT_SAMPLER;
    sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    layout->setTextureLocation(2, &sampler);
    layout->setTextureLocation(3, &sampler);
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
    pipeline->setTessellationState(3);
    pipeline->disableSampleShading();
    pipeline->setDepthStencilMode();
    pipeline->setCullMode(true);
    pipeline->bindVertex(*vertexArray);
    pipeline->bindShader("volumObj3D.vert.spv");
    pipeline->bindShader("volumObj3D.tesc.spv");
    pipeline->bindShader("volumObj3D.tese.spv");
    pipeline->bindShader("volumObj3D.frag.spv");
    pipeline->build();

    index = context.indexBufferMgr->acquireBuffer(3*2*6*sizeof(uint16_t));
    uint16_t tmp[3*2*6] = {2,0,1, 1,3,2,
                           4,2,3, 3,5,4,
                           6,4,5, 5,7,6,
                           0,6,7, 7,1,0,
                           0,2,4, 4,6,0,
                           1,7,5, 5,3,1};
    memcpy(context.transfer->planCopy(index), tmp, 3*2*6*sizeof(uint16_t));
}

VolumObj3D::Shared::~Shared()
{
    Context::instance->indexBufferMgr->releaseBuffer(index);
}

void VolumObj3D::setModel(const Mat4f &_model, const Vec3f &scale)
{
    model = _model * Mat4f::scaling(scale);
    ray->rayCoef = scale;
}

void VolumObj3D::draw(const Navigator * nav, const Projector* prj)
{
    Mat4f mat = nav->getHelioToEyeMat().convert() * model;
    transform->ModelViewMatrix = mat;
    transform->NormalMatrix = mat.inverseUntranslated();
    transform->clipping_fov = prj->getClippingFov();

    Context &context = *Context::instance;
    VkCommandBuffer &cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
    shared->pipeline->bind(cmd);
    shared->layout->bindSet(cmd, *set);
    shared->vertex->bind(cmd);
    vkCmdBindIndexBuffer(cmd, shared->index.buffer, shared->index.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, 3*2*6, 1, 0, 0, 0);
    context.frame[context.frameIdx]->compile(cmd);
    context.frame[context.frameIdx]->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

Mat4f VolumObj3D::drawExternal(const Navigator * nav, const Projector* prj)
{
    Mat4f mat = nav->getHelioToEyeMat().convert() * model;
    transform->ModelViewMatrix = mat;
    transform->NormalMatrix = mat.inverseUntranslated();
    transform->clipping_fov = prj->getClippingFov();
    return mat;
}

void VolumObj3D::recordVolumetricObject(VkCommandBuffer cmd)
{
    shared->pipeline->bind(cmd);
    shared->layout->bindSet(cmd, *set);
    shared->vertex->bind(cmd);
    vkCmdBindIndexBuffer(cmd, shared->index.buffer, shared->index.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, 3*2*6, 1, 0, 0, 0);
}
