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
#include "vulkanModule/ResourceTracker.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"
#include "vulkanModule/VertexArray.hpp"

ThreadContext *StarViewer::context;
PipelineLayout *StarViewer::layout;
Pipeline *StarViewer::pipeline, *StarViewer::pipelineCorona, *StarViewer::pipelineHalo;

StarViewer::StarViewer(const Vec3f &pos, const Vec3f &color, const float _radius)
{
    createLocalContext();
    build();
    model = Mat4f::translation(pos);
    radius = _radius;
    pFrag->color = color;
    pFrag->radius = radius;
    pVert->radius = radius;
}

StarViewer::~StarViewer() {}

void StarViewer::createSC_context(ThreadContext *_context)
{
    context = _context;

    layout = context->global->tracker->track(new PipelineLayout(context->surface));
    layout->setGlobalPipelineLayout(context->global->globalLayout);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    layout->buildLayout();
    layout->build();

    VertexArray vertex(context->surface);
    vertex.registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    vertex.registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
    vertex.registerVertexBuffer(BufferType::NORMAL, BufferAccess::STATIC);

    pipeline = context->global->tracker->track(new Pipeline(context->surface, layout));
    pipeline->setBlendMode(BLEND_NONE);
    //pipeline->setDepthStencilMode();
    pipeline->setCullMode(true);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline->bindVertex(&vertex);
    pipeline->removeVertexEntry(1);
    pipeline->removeVertexEntry(2);
    pipeline->bindShader("big_star.vert.spv");
    pipeline->bindShader("big_star.frag.spv");
    pipeline->build();

    VertexArray vertexHalo(context->surface);
    vertexHalo.registerVertexBuffer(BufferType::POS2D, BufferAccess::STREAM);
    vertexHalo.registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STREAM);

    pipelineCorona = context->global->tracker->track(new Pipeline(context->surface, context->global->globalLayout));
    pipelineCorona->setDepthStencilMode();
    pipelineCorona->setBlendMode(BLEND_ADD);
    pipelineCorona->bindVertex(&vertexHalo);
    pipelineCorona->bindShader("big_star_halo.vert.spv");
    pipelineCorona->bindShader("big_star_corona.frag.spv");
    pipelineCorona->build();

    // pipelineHalo = context->global->tracker->track(new Pipeline(context->surface, context->global->globalLayout));
    // pipelineHalo->setDepthStencilMode();
    // pipelineHalo->setBlendMode(BLEND_ADD);
    // pipelineHalo->bindVertex(&vertexHalo);
    // pipelineHalo->bindShader("big_star_halo.vert.spv");
    // pipelineHalo->bindShader("big_star_halo.frag.spv");
    // pipelineHalo->build();
}

void StarViewer::createLocalContext()
{
    commandIndex = context->commandMgr->getCommandIndex();
    set = std::make_unique<Set>(context->surface, context->setMgr, layout);
    uVert = std::make_unique<Uniform>(context->surface, sizeof(*pVert));
    pVert = static_cast<typeof(pVert)>(uVert->data);
    set->bindUniform(uVert.get(), 0);
    uFrag = std::make_unique<Uniform>(context->surface, sizeof(*pFrag));
    pFrag = static_cast<typeof(pFrag)>(uFrag->data);
    set->bindUniform(uFrag.get(), 1);

    vertexHalo = std::make_unique<VertexArray>(context->surface);
    vertexHalo->registerVertexBuffer(BufferType::POS2D, BufferAccess::STREAM);
    vertexHalo->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
    vertexHalo->build(4);
    std::vector<float> tmp{-1., -1., -1., 1., 1., -1., 1., 1.};
    vertexHalo->fillVertexBuffer(BufferType::TEXTURE, tmp);

    objl = std::make_unique<ObjL>();
    objl->init(AppSettings::Instance()->getModel3DDir() + "Sphere", "Sphere", context);

    drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
}

void StarViewer::build()
{
    CommandMgr *cmdMgr = context->commandMgr;
    cmdMgr->init(commandIndex, pipelineCorona, renderPassType::USE_DEPTH_BUFFER);
    cmdMgr->bindSet(context->global->globalLayout, context->global->globalSet);
    cmdMgr->bindVertex(vertexHalo.get());
    cmdMgr->draw(4);
    cmdMgr->bindPipeline(pipeline);
    cmdMgr->bindSet(layout, set.get(), 1);
    objl->bind(cmdMgr);
    cmdMgr->indirectDrawIndexed(drawData.get());
    // cmdMgr->bindPipeline(pipelineHalo);
    // cmdMgr->bindVertex(vertexHalo.get());
    // cmdMgr->draw(4, 1, 4);
    cmdMgr->compile();
}

void StarViewer::draw(const Navigator * nav, const Projector* prj, const Mat4f &mat)
{
    Vec3f pos = mat * model * v3fNull;
    float screen_size = getOnScreenSize(prj, pos);
    pos = mat.transpose() * (pos / 2) - mat.transpose() * pos;
    pos.normalize();
    pFrag->cam_view = pos;
    Mat4d mat2(mat.r[0], mat.r[1], mat.r[2], mat.r[3], mat.r[4], mat.r[5], mat.r[6], mat.r[7], mat.r[8], mat.r[9], mat.r[10], mat.r[11], mat.r[12], mat.r[13], mat.r[14], mat.r[15]);
    Vec3d screenPos;
    if (!prj->projectCustomCheck(v3fNull, screenPos, mat2, (int)(screen_size*2)))
        return;

    pVert->ModelViewMatrix = mat * model;
    pVert->clipping_fov = prj->getClippingFov();
    objl->draw(screen_size, drawData->data);
    insert_all(vertices,
        screenPos[0] - screen_size*2, screenPos[1] - screen_size*2,
        screenPos[0] - screen_size*2, screenPos[1] + screen_size*2,
        screenPos[0] + screen_size*2, screenPos[1] - screen_size*2,
        screenPos[0] + screen_size*2, screenPos[1] + screen_size*2);
    vertexHalo->fillVertexBuffer(BufferType::POS2D, vertices);
    vertices.clear();

    context->commandMgr->setSubmission(commandIndex);
}

// Return the radius of a circle containing the object on screen
float StarViewer::getOnScreenSize(const Projector* prj, const Vec3f &pos)
{
    return atanf(radius/sqrt(pos.lengthSquared() - radius * radius))*2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
}
