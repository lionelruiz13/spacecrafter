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

#include "inGalaxyModule/starViewer.hpp"
#include "ojmModule/objl.hpp"
#include "tools/app_settings.hpp"
#include "coreModule/projector.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

PipelineLayout *StarViewer::layout;
Pipeline *StarViewer::pipeline, *StarViewer::pipelineCorona;
std::unique_ptr<VertexArray> StarViewer::modelHalo;

StarViewer::StarViewer(const Vec3f &pos, const Vec3f &color, const float _radius)
{
    createLocalContext();
    model = Mat4f::translation(pos);
    radius = _radius;
    (*uFrag)->color = color;
    (*uFrag)->radius = radius;
    (*uVert)->radius = radius;
}

StarViewer::~StarViewer() {}

void StarViewer::createSC_context()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    layout = new PipelineLayout(vkmgr);
    context.layouts.emplace_back(layout);
    layout->setGlobalPipelineLayout(context.layouts.front().get());
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    layout->buildLayout();
    layout->build();

    pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout);
    context.pipelines.emplace_back(pipeline);
    pipeline->setBlendMode(BLEND_NONE);
    //pipeline->setDepthStencilMode();
    pipeline->setCullMode(true);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline->bindVertex(*context.ojmVertexArray);
    pipeline->removeVertexEntry(1);
    pipeline->removeVertexEntry(2);
    pipeline->bindShader("big_star.vert.spv");
    pipeline->bindShader("big_star.frag.spv");
    pipeline->build();

    modelHalo = std::make_unique<VertexArray>(vkmgr);
    modelHalo->createBindingEntry(4 * sizeof(float));
    modelHalo->addInput(VK_FORMAT_R32G32_SFLOAT); // Pos2D
    modelHalo->addInput(VK_FORMAT_R32G32_SFLOAT); // Texture

    pipelineCorona = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, context.layouts.front().get());
    context.pipelines.emplace_back(pipelineCorona);
    pipelineCorona->setDepthStencilMode();
    pipelineCorona->setBlendMode(BLEND_ADD);
    pipelineCorona->bindVertex(*modelHalo);
    pipelineCorona->bindShader("big_star_halo.vert.spv");
    pipelineCorona->bindShader("big_star_corona.frag.spv");
    pipelineCorona->build();
}

void StarViewer::createLocalContext()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    for (int i = 0; i < 3; ++i) {
        cmds[i] = context.frame[i]->create(1);
        context.frame[i]->setName(cmds[i], "StarViewer " + std::to_string(i));
    }
    set = std::make_unique<Set>(vkmgr, *context.setMgr, layout);
    uVert = std::make_unique<SharedBuffer<s_vert>>(*context.uniformMgr);
    set->bindUniform(uVert, 0);
    uFrag = std::make_unique<SharedBuffer<s_frag>>(*context.uniformMgr);
    set->bindUniform(uFrag, 1);

    vertexHalo = modelHalo->createBuffer(0, 4, context.tinyMgr.get());
    pVertexHalo = (float *) context.tinyMgr->getPtr(vertexHalo->get());
    pVertexHalo[2] = -1.;
    pVertexHalo[3] = -1.;
    pVertexHalo[6] = -1.;
    pVertexHalo[7] = 1.;
    pVertexHalo[10] = 1.;
    pVertexHalo[11] = -1.;
    pVertexHalo[14] = 1.;
    pVertexHalo[15] = 1.;

    objl = std::make_unique<ObjL>();
    objl->init(AppSettings::Instance()->getModel3DDir() + "Sphere", "Sphere");
}

void StarViewer::draw(const Navigator * nav, const Projector* prj, const Mat4f &mat)
{
    Vec3f pos = mat * model * v3fNull;
    float screen_size = getOnScreenSize(prj, pos);
    pos = mat.transpose() * (pos / 2) - mat.transpose() * pos;
    pos.normalize();
    (*uFrag)->cam_view = pos;
    Mat4d mat2(mat.r[0], mat.r[1], mat.r[2], mat.r[3], mat.r[4], mat.r[5], mat.r[6], mat.r[7], mat.r[8], mat.r[9], mat.r[10], mat.r[11], mat.r[12], mat.r[13], mat.r[14], mat.r[15]);
    Vec3d screenPos;
    if (!prj->projectCustomCheck(v3fNull, screenPos, mat2, (int)(screen_size*2)))
        return;

    (*uVert)->ModelViewMatrix = mat * model;
    (*uVert)->clipping_fov = prj->getClippingFov();
    Context &context = *Context::instance;
    VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
    pipelineCorona->bind(cmd);
    context.layouts.front()->bindSet(cmd, *context.uboSet);
    vertexHalo->bind(cmd);
    vkCmdDraw(cmd, 4, 1, 0, 0);
    pipeline->bind(cmd);
    layout->bindSet(cmd, *set, 1);
    objl->bind(cmd);
    objl->draw(cmd, screen_size);
    pVertexHalo[0] = screenPos[0] - screen_size*2;
    pVertexHalo[1] = screenPos[1] - screen_size*2;
    pVertexHalo[4] = screenPos[0] - screen_size*2;
    pVertexHalo[5] = screenPos[1] + screen_size*2;
    pVertexHalo[8] = screenPos[0] + screen_size*2;
    pVertexHalo[9] = screenPos[1] - screen_size*2;
    pVertexHalo[12] = screenPos[0] + screen_size*2;
    pVertexHalo[13] = screenPos[1] + screen_size*2;

    context.frame[context.frameIdx]->compile(cmd);
    context.frame[context.frameIdx]->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

// Return the radius of a circle containing the object on screen
float StarViewer::getOnScreenSize(const Projector* prj, const Vec3f &pos)
{
    return atanf(radius/sqrt(pos.lengthSquared() - radius * radius))*2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
}
