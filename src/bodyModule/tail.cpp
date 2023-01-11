/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2022 Immersive Adventure
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

#include "tail.hpp"
#include "coreModule/coreLink.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/vecmath.hpp"
#include "body_smallbody.hpp"

#define NB_MAX_TAILS 1024
#define NB_TAIL_LINES 16
#define NB_TAIL_LENGTH 16
#define NB_TAIL_HEAD (1 + NB_TAIL_LINES * NB_TAIL_LINES / 4)
#define NB_TAIL_LINE_INDICES (1 + NB_TAIL_LINES / 4 + NB_TAIL_LENGTH) * 2
#define context (*Context::instance)
#define vkmgr (*VulkanMgr::instance)

struct TailContext {
    struct VertexData {
        Vec3f normal;
        float timeOffset;
    };

    struct InstanceData {
        Vec3f offset; // ModelViewMatrix already applied
        Vec3f expandDirection; // ModelViewMatrix already applied
        Vec3f expandCorrection; // ModelViewMatrix already applied
        Vec3f coefRadius;
        Vec3f color;
        Vec3f ModelViewMatrix[3];
    };

    TailContext() :
        pattern(vkmgr),
        layout(vkmgr),
        pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH)
    {
        pattern.createBindingEntry(4*sizeof(float));
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32_SFLOAT);
        pattern.createBindingEntry(3*8*sizeof(float), VK_VERTEX_INPUT_RATE_INSTANCE);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        pattern.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        vertex = pattern.createBuffer(0, NB_TAIL_HEAD + NB_TAIL_LINES * NB_TAIL_LENGTH, context.globalBuffer.get());
        instance = pattern.createBuffer(1, NB_MAX_TAILS, Context::instance->globalBuffer.get());
        // Write the vertices !
        Vec2f angles[NB_TAIL_LINES];
        for (int i = 0; i < NB_TAIL_LINES; ++i) {
            angles[i] = Vec2f(cos(i * (2 * M_PI / NB_TAIL_LINES)), sin(i * (2 * M_PI / NB_TAIL_LINES)));
        }
        auto *vptr = context.transfer->planCopy<VertexData>(vertex->get());
        *(vptr++) = VertexData{{0, 0, 1}, 0};
        for (int i = 0; i++ < NB_TAIL_LINES / 4;) {
            const Vec2f angle = angles[i];
            for (int j = 0; j < NB_TAIL_LINES; ++j)
                *(vptr++) = {{angles[j] * angle[1], angle[0]}, 0};
        }
        for (int i = 0; i++ < NB_TAIL_LENGTH;) {
            const float timeOffset = i / float(NB_TAIL_LENGTH);
            for (int j = 0; j < NB_TAIL_LINES; ++j) {
                *(vptr++) = {{angles[j], 0}, timeOffset};
            }
        }
        // Generate the indices
        index = context.indexBufferMgr->acquireBuffer(NB_TAIL_LINES * NB_TAIL_LINE_INDICES * sizeof(uint16_t));
        auto *ptr = context.transfer->planCopy<uint16_t>(index);
        for (int i = 0; i++ < NB_TAIL_LINES;) {
            *(ptr++) = 0;
            for (int j = 0; j < NB_TAIL_LINES * (NB_TAIL_LINES / 4 + NB_TAIL_LENGTH); j += NB_TAIL_LINES) {
                *(ptr++) = j + i;
                *(ptr++) = j + (i+1) % NB_TAIL_LINES;
            }
            *(ptr++) = UINT16_MAX;
        }
        // Define the layout
        layout.setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float));
        layout.build();
        // Define the pipeline
        pipeline.bindLayout(layout);
        pipeline.bindVertex(pattern);
        pipeline.bindShader("body_tail.vert.spv");
        pipeline.bindShader("body_tail.frag.spv");
        pipeline.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);
        pipeline.setCullMode(true);
        pipeline.setDepthStencilMode();
        pipeline.build("Comet Tail");
    }
    VertexArray pattern;
    PipelineLayout layout;
    Pipeline pipeline;
    std::unique_ptr<VertexBuffer> vertex, instance;
    SubBuffer index;
    std::vector<InstanceData> datas;
    unsigned int drawOffset;
    float fov;
};

std::weak_ptr<TailContext> Tail::globalRef;
TailContext *Tail::global;

Tail::Tail(float deltaTraceJD, float ejectionForce, const Vec3f &coefRadius, const Vec3f &color) :
    deltaTraceJD(deltaTraceJD), ejectionForce(ejectionForce), coefRadius(coefRadius), color(color)
{
    if (globalRef.expired()) {
        globalRef = shared = std::make_shared<TailContext>();
        global = shared.get();
    } else {
        shared = globalRef.lock();
    }
}

Tail::~Tail()
{
}

void Tail::draw(const Navigator *nav, SmallBody *body, const Vec3f &eye_planet, const Vec3f &eye_sun, float radius, float JD)
{
    auto direction = eye_planet - eye_sun;
    const float distToSun = direction.length();
    Vec2f tailFactor = body->getComaDiameterAndTailLengthAU(distToSun) * radius;
    if (tailFactor[0] > tailFactor[1])
        return;
    direction /= distToSun;

    if (JD != lastJD) {
        Vec3f currentPos = body->get_heliocentric_ecliptic_pos();
        Vec3f halfPos = body->getPositionAtDate(JD-deltaTraceJD/2);
        Vec3f pastPos = body->getPositionAtDate(JD-deltaTraceJD);

        Vec3f velocity = (halfPos - currentPos) * 2;
        Vec3f velocityCorrection = velocity + (halfPos - pastPos) * 2;
        velocity += velocityCorrection;

        currentPos.normalize();
        pastPos.normalize();
        const float factors = ejectionForce * tailFactor[1] * deltaTraceJD;
        cachedExpansionInitial = velocity + currentPos * factors;
        cachedExpansionCorrection = velocityCorrection * -2 + (pastPos - currentPos) * factors;
        cachedCoefRadius = coefRadius * tailFactor[0];
        lastJD = JD;
    }
    // We must find out a way to rotate to the initial direction
    Vec3f initialDirection = nav->getHelioToEyeMat().multiplyWithoutTranslation(cachedExpansionInitial);
    Vec3f directionCorrection = nav->getHelioToEyeMat().multiplyWithoutTranslation(cachedExpansionCorrection);
    Vec3f tmp = initialDirection;
    tmp.normalize();
    Mat4f m = Mat4f::rotation({0, 0, 1}, tmp);
    shared->datas.push_back({eye_planet, initialDirection, directionCorrection, cachedCoefRadius, color, {{m.r[0], m.r[1], m.r[2]}, {m.r[4], m.r[5], m.r[6]}, {m.r[8], m.r[9], m.r[10]}}});
}

// void Tail::draw(const Vec3f &eyePosition, const Vec3f &eyeDirection, const Vec3f &eyeCorrection, const Vec3f &coefRadius)
// {
//     Vec3f tmp = eyeDirection;
//     tmp.normalize();
//     Mat4f m = Mat4f::rotation({0, 0, 1}, tmp);
//     shared->datas.push_back({eyePosition, eyeDirection, eyeCorrection, coefRadius, {{m.r[0], m.r[1], m.r[2]}, {m.r[4], m.r[5], m.r[6]}, {m.r[8], m.r[9], m.r[10]}}});
// }

void Tail::beginDraw(float fov)
{
    if (globalRef.expired())
        return;
    global->datas.clear();
    global->drawOffset = 0;
    global->fov = fov * (M_PI / 360.f);
}

void Tail::drawBatch(VkCommandBuffer cmd)
{
    if (globalRef.expired())
        return;
    if (global->datas.size() != global->drawOffset) {
        global->pipeline.bind(cmd);
        global->layout.pushConstant(cmd, 0, &global->fov);
        global->vertex->bind(cmd);
        global->instance->bind(cmd, global->drawOffset * sizeof(TailContext::InstanceData));
        vkCmdBindIndexBuffer(cmd, global->index.buffer, global->index.offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, NB_TAIL_LINE_INDICES * NB_TAIL_LINES, global->datas.size() - global->drawOffset, 0, 0, 0);
        global->drawOffset = global->datas.size();
    }
}

void Tail::endDraw()
{
    if (globalRef.expired())
        return;
    const int size = global->datas.size() * sizeof(TailContext::InstanceData);
    if (size)
        memcpy(context.transfer->planCopy(global->instance->get(), 0, size), global->datas.data(), size);
}

bool Tail::shouldDraw()
{
    return !(globalRef.expired() || global->datas.size() == global->drawOffset);
}

// Constants :
// vec3 pos (origin)
// vec3 normals (side)
// vec3 light direction
// vec3 past direction
// float time shift (time before repercution)

// Variables :
// float radius (over timeShift)
// vec3 finalPos = mv * (pos + radius * normals + pastDirection * timeShift)
