/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2026
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 */

#include "bodyModule/body_blackhole.hpp"
#include "bodyModule/blackhole_trajectory_lut.hpp"

#include "bodyModule/body_color.hpp"
#include "bodyModule/orbit_3d.hpp"
#include "bodyModule/trail.hpp"
#include "appModule/blackhole_lensing.hpp"
#include "coreModule/projector.hpp"
#include "tools/context.hpp"
#include "tools/file_path.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr int HORIZON_SLICES = 64;
constexpr int HORIZON_STACKS = 32;
constexpr int HORIZON_STRIP_VERTICES = (HORIZON_SLICES + 1) * 2;
}

BlackHole::BlackHole(std::shared_ptr<Body> parent,
                     const std::string& englishName,
                     bool flagHalo,
                     double radius,
                     double oblateness,
                     std::unique_ptr<BodyColor> myColor,
                     float sol_local_day,
                     float albedo,
                     std::unique_ptr<Orbit> orbit,
                     bool close_orbit,
                     ObjL* currentObj,
                     double orbit_bounding_radius,
                     const BodyTexture &bodyTexture,
                     double diskInnerRadius,
                     double diskOuterRadius,
                     const std::string &diskTexture,
                     const BlackHoleVisual &visual) :
    Body(std::move(parent),
         englishName,
         BLACKHOLE,
         flagHalo,
         radius,
         oblateness,
         std::move(myColor),
         sol_local_day,
         albedo,
         std::move(orbit),
         close_orbit,
         currentObj,
         orbit_bounding_radius,
         bodyTexture),
    diskInnerRadius(diskInnerRadius),
    diskOuterRadius(diskOuterRadius),
    visual(visual)
{
    diskEnabled = diskOuterRadius > diskInnerRadius && !diskTexture.empty();
    if (diskEnabled)
        diskTex = std::make_unique<s_texture>(FilePath(diskTexture, FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_ALPHA, true);

    trail = std::make_unique<Trail>(this, 1460);
    orbitPlot = std::make_unique<Orbit3D>(this);
}

BlackHole::~BlackHole() = default;

void BlackHole::selectShader()
{
    changed = false;
}

float BlackHole::computeMagnitude(const Vec3d) const
{
    return 99.f;
}

double BlackHole::calculateBoundingRadius()
{
    boundingRadius = std::max<double>(radius, diskEnabled ? diskOuterRadius * diskScale : radius);
    return boundingRadius;
}

float BlackHole::getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only)
{
    const double rad = (diskEnabled && !orb_only) ? diskOuterRadius * diskScale : static_cast<double>(radius);
    double temp = getEarthEquPos(nav).lengthSquared() - rad * rad;
    if (temp < 0.)
        temp = 0.000001;
    return atanf(rad / sqrt(temp)) * 2.f * 180.f / M_PI / prj->getFov() * prj->getViewportHeight();
}

void BlackHole::setSphereScale(float s, bool initial_scale)
{
    radius = initialRadius * s;
    diskScale = s;
    if (initial_scale)
        initialScale = s;
}

void BlackHole::drawOrbit(VkCommandBuffer, VkCommandBuffer, const Observer*, const Navigator*, const Projector*)
{
}

Set &BlackHole::getSet(float)
{
    if (!horizonSet)
        createHorizonContext();
    return *horizonSet;
}

bool BlackHole::hasRings()
{
    return diskEnabled;
}

void BlackHole::drawTrail(VkCommandBuffer, const Navigator*, const Projector*)
{
}

void BlackHole::drawHints(const Navigator*, const Projector*)
{
}

void BlackHole::drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator*, const Mat4d& mat, float screen_sz, bool)
{
    if (!diskEnabled && (!visual.distortionEnabled || visual.lensingStrength <= 0.f))
        drawHorizon(cmd, prj, mat);
    if (!diskEnabled)
        submitLensing(prj, mat, screen_sz);
}

void BlackHole::drawRings(VkCommandBuffer cmd, const Projector* prj, const Observer*, const Mat4d& mat, double screen_sz, Vec3f&, Vec3f&, float)
{
    if (!visual.distortionEnabled || visual.lensingStrength <= 0.f)
        drawHorizon(cmd, prj, mat);
    submitLensing(prj, mat, screen_sz);
    drawDisk(prj, mat);
}

void BlackHole::drawHalo(const Navigator*, const Projector*, const ToneReproductor*)
{
}

void BlackHole::drawAxis(VkCommandBuffer, const Projector*, const Mat4d&)
{
}

void BlackHole::drawPlanetGrid(VkCommandBuffer, const Projector*, const Mat4d&)
{
}

void BlackHole::createHorizonContext()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    horizonLayout = std::make_unique<PipelineLayout>(vkmgr);
    horizonLayout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    horizonLayout->buildLayout();
    horizonLayout->setGlobalPipelineLayout(context.layouts.front().get());
    horizonLayout->build();

    horizonVertex = std::make_unique<VertexArray>(vkmgr);
    horizonVertex->createBindingEntry(3 * sizeof(float));
    horizonVertex->addInput(VK_FORMAT_R32G32B32_SFLOAT);
    buildHorizonMesh();

    horizonSet = std::make_unique<Set>(vkmgr, *context.setMgr, horizonLayout.get(), -1, false, true);
    horizonUniform = std::make_unique<SharedBuffer<HorizonUniform>>(*context.uniformMgr);
    horizonSet->bindUniform(horizonUniform, 0);

    horizonPipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, horizonLayout.get());
    horizonPipeline->setCullMode(false);
    horizonPipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    horizonPipeline->setBlendMode(BLEND_NONE);
    horizonPipeline->setDepthStencilMode(VK_TRUE, VK_TRUE);
    horizonPipeline->bindVertex(*horizonVertex);
    horizonPipeline->bindShader("blackhole_horizon.vert.spv");
    horizonPipeline->setSpecializedConstant(7, context.isFloat64Supported);
    horizonPipeline->setSpecializedConstant(8, Context::projectionType);
    horizonPipeline->bindShader("blackhole_horizon.frag.spv");
    horizonPipeline->build("Black hole horizon");

}

void BlackHole::buildHorizonMesh()
{
    horizonBuffer = horizonVertex->createBuffer(0, HORIZON_STACKS * HORIZON_STRIP_VERTICES, Context::instance->globalBuffer.get());
    float *data = static_cast<float *>(Context::instance->transfer->planCopy(horizonBuffer->get()));

    for (int i = 0; i < HORIZON_STACKS; ++i) {
        const double phi0 = M_PI * static_cast<double>(i) / HORIZON_STACKS;
        const double phi1 = M_PI * static_cast<double>(i + 1) / HORIZON_STACKS;
        for (int j = 0; j <= HORIZON_SLICES; ++j) {
            const double theta = 2.0 * M_PI * static_cast<double>(j) / HORIZON_SLICES;
            const double c = cos(theta);
            const double s = sin(theta);
            *(data++) = static_cast<float>(sin(phi0) * c);
            *(data++) = static_cast<float>(sin(phi0) * s);
            *(data++) = static_cast<float>(cos(phi0));
            *(data++) = static_cast<float>(sin(phi1) * c);
            *(data++) = static_cast<float>(sin(phi1) * s);
            *(data++) = static_cast<float>(cos(phi1));
        }
    }
}

void BlackHole::drawDisk(const Projector* prj, const Mat4d& mat)
{
    if (!diskEnabled)
        return;

    const bool distorted = visual.distortionEnabled && visual.lensingStrength > 0.f;
    const double unitRadius = distorted
        ? radius * blackhole::diskRadiusCalibration * visual.lensingStrength
        : radius;
    if (unitRadius <= 0.0)
        return;
    const Vec3d camera = -(mat * Vec3d(0.0, 0.0, 0.0)) / unitRadius;
    Vec3d x = mat.multiplyWithoutTranslation(Vec3d(1.0, 0.0, 0.0));
    Vec3d y = mat.multiplyWithoutTranslation(Vec3d(0.0, 1.0, 0.0));
    x.normalize();
    y.normalize();
    const Vec3f viewport = prj->getViewportFloatCenter();
    BlackHoleLensing::DiskModel model;
    // A negative capture radius explicitly selects undistorted rays.
    model.camera = Vec4f(camera[0], camera[1], camera[2],
                        distorted ? std::max(1.0, radius * blackhole::diskRadiusCalibration / unitRadius) : -1.0);
    model.diskX = Vec4f(x[0], x[1], x[2], diskInnerRadius * diskScale / unitRadius);
    model.diskY = Vec4f(y[0], y[1], y[2], diskOuterRadius * diskScale / unitRadius);
    model.projection = Vec4f(viewport[0], VulkanMgr::instance->getScreenRect().extent.height - viewport[1],
                            viewport[2], prj->getClippingFov()[2]);
    model.diskColor = Vec4f(visual.diskColor[0], visual.diskColor[1], visual.diskColor[2], visual.diskIntensity);
    model.photonColor = Vec4f(visual.photonColor[0], visual.photonColor[1], visual.photonColor[2], visual.turbulence);
    BlackHoleLensing::submitDiskModel(model, diskTex->getTexture(), camera.length() * unitRadius);
}

void BlackHole::drawHorizon(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat)
{
    if (!horizonPipeline)
        createHorizonContext();
    if (!horizonPipeline || horizonPipeline->get() == VK_NULL_HANDLE)
        return;

    horizonUniform->get().ModelViewMatrix = mat.convert();
    horizonUniform->get().clipping_fov = prj->getClippingFov();
    horizonUniform->get().HorizonRadius = static_cast<float>(radius);

    horizonPipeline->bind(cmd);
    horizonLayout->bindSets(cmd, {*horizonSet, *Context::instance->uboSet});
    horizonBuffer->bind(cmd);
    for (int i = 0; i < HORIZON_STACKS; ++i)
        vkCmdDraw(cmd, HORIZON_STRIP_VERTICES, 1, i * HORIZON_STRIP_VERTICES, 0);
}

void BlackHole::submitLensing(const Projector* prj, const Mat4d& mat, double screen_sz)
{
    Vec3d centerProjected;
    prj->projectCustom(Vec3d(0.0, 0.0, 0.0), centerProjected, mat);

    const Vec3d samples[6] = {
        Vec3d( radius, 0.0, 0.0), Vec3d(-radius, 0.0, 0.0),
        Vec3d(0.0,  radius, 0.0), Vec3d(0.0, -radius, 0.0),
        Vec3d(0.0, 0.0,  radius), Vec3d(0.0, 0.0, -radius)
    };
    float eventRadius = 0.f;
    for (const Vec3d &sample : samples) {
        Vec3d projected;
        prj->projectCustom(sample, projected, mat);
        const float dx = static_cast<float>(projected[0] - centerProjected[0]);
        const float dy = static_cast<float>(projected[1] - centerProjected[1]);
        eventRadius = std::max(eventRadius, std::sqrt(dx * dx + dy * dy));
    }
    if (eventRadius < 1.f) {
        const float outerRadius = diskEnabled ? static_cast<float>(diskOuterRadius * diskScale) : static_cast<float>(radius);
        eventRadius = static_cast<float>(screen_sz) * static_cast<float>(radius)
                    / std::max(outerRadius, 0.000001f);
    }
    eventRadius = std::max(5.f, eventRadius);

    BlackHoleLensing::submit(Vec2f(screenPos.first, screenPos.second), eventRadius,
                             visual.lensingStrength, visual.distortionEnabled && visual.lensingStrength > 0.f);
}
