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

#include "bodyModule/body_color.hpp"
#include "bodyModule/halo.hpp"
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
constexpr int DISK_SLICES = 256;
constexpr int DISK_STACKS = 12;
constexpr int DISK_STRIP_VERTICES = (DISK_SLICES + 1) * 2;
constexpr int DISK_VERTEX_FLOATS = 4;
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
    if (!overlaySet)
        createOverlayContext(VulkanMgr::instance->getScreenRect().extent.height);
    return *overlaySet;
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

void BlackHole::drawBody(VkCommandBuffer cmd, const Projector*, const Navigator*, const Mat4d&, float screen_sz, bool)
{
    if (!diskEnabled)
        drawOverlay(cmd, screen_sz);
}

void BlackHole::drawRings(VkCommandBuffer cmd, const Projector* prj, const Observer*, const Mat4d& mat, double screen_sz, Vec3f&, Vec3f&, float)
{
    drawDisk(cmd, prj, mat, screen_sz);
    drawOverlay(cmd, screen_sz);
}

void BlackHole::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{
    if (isVisible && flags.flag_halo && getOnScreenSize(prj, nav) < 10)
        halo->drawHalo(nav, prj, eye);
}

void BlackHole::drawAxis(VkCommandBuffer, const Projector*, const Mat4d&)
{
}

void BlackHole::drawPlanetGrid(VkCommandBuffer, const Projector*, const Mat4d&)
{
}

void BlackHole::createDiskContext()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    diskLayout = std::make_unique<PipelineLayout>(vkmgr);
    diskLayout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    diskLayout->setTextureLocation(1, &PipelineLayout::DEFAULT_SAMPLER);
    diskLayout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    diskLayout->buildLayout();
    diskLayout->setGlobalPipelineLayout(context.layouts.front().get());
    diskLayout->build();

    diskVertex = std::make_unique<VertexArray>(vkmgr);
    diskVertex->createBindingEntry(DISK_VERTEX_FLOATS * sizeof(float));
    diskVertex->addInput(VK_FORMAT_R32G32_SFLOAT);
    diskVertex->addInput(VK_FORMAT_R32_SFLOAT);
    diskVertex->addInput(VK_FORMAT_R32_SFLOAT);
    buildDiskMesh();

    diskSet = std::make_unique<Set>(vkmgr, *context.setMgr, diskLayout.get(), -1, false, true);
    diskUniform = std::make_unique<SharedBuffer<DiskUniform>>(*context.uniformMgr);
    diskVisualUniform = std::make_unique<SharedBuffer<DiskVisualUniform>>(*context.uniformMgr);
    diskSet->bindUniform(diskUniform, 0);
    diskSet->bindTexture(diskTex->getTexture(), 1);
    diskSet->bindUniform(diskVisualUniform, 2);

    diskPipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, diskLayout.get());
    diskPipeline->setCullMode(false);
    diskPipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    diskPipeline->setBlendMode(BLEND_ADD);
    diskPipeline->setDepthStencilMode(VK_TRUE, VK_FALSE);
    diskPipeline->bindVertex(*diskVertex);
    diskPipeline->bindShader("blackhole_disk.vert.spv");
    diskPipeline->setSpecializedConstant(7, context.isFloat64Supported);
    diskPipeline->setSpecializedConstant(8, Context::projectionType);
    diskPipeline->bindShader("blackhole_ring.frag.spv");
    diskPipeline->build("Black hole disk");
}

void BlackHole::buildDiskMesh()
{
    diskBuffer = diskVertex->createBuffer(0, DISK_STACKS * DISK_STRIP_VERTICES, Context::instance->globalBuffer.get());
    float *data = static_cast<float *>(Context::instance->transfer->planCopy(diskBuffer->get()));

    const double rSize = diskOuterRadius - diskInnerRadius;
    for (int i = 0; i < DISK_STACKS; ++i) {
        const double texR0 = static_cast<double>(i) / DISK_STACKS;
        const double texR1 = static_cast<double>(i + 1) / DISK_STACKS;
        const double r0 = diskInnerRadius + rSize * texR0;
        const double r1 = diskInnerRadius + rSize * texR1;

        for (int j = 0; j <= DISK_SLICES; ++j) {
            const double angle = static_cast<double>(j) / DISK_SLICES;
            const double theta = angle * 2.0 * M_PI;
            const double c = cos(theta);
            const double s = sin(theta);

            *(data++) = static_cast<float>(r0 * c);
            *(data++) = static_cast<float>(r0 * s);
            *(data++) = static_cast<float>(texR0);
            *(data++) = static_cast<float>(angle);
            *(data++) = static_cast<float>(r1 * c);
            *(data++) = static_cast<float>(r1 * s);
            *(data++) = static_cast<float>(texR1);
            *(data++) = static_cast<float>(angle);
        }
    }
}

void BlackHole::drawDisk(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat, double)
{
    if (!diskEnabled)
        return;
    if (!diskPipeline)
        createDiskContext();
    if (!diskPipeline || diskPipeline->get() == VK_NULL_HANDLE)
        return;

    diskUniform->get().ModelViewMatrix = mat.convert();
    diskUniform->get().clipping_fov = prj->getClippingFov();
    diskUniform->get().RingScale = diskScale;
    diskUniform->get().fadingFactor = 100000.f;
    diskVisualUniform->get().diskColor = visual.diskColor;
    diskVisualUniform->get().diskIntensity = visual.diskIntensity;
    diskVisualUniform->get().photonColor = visual.photonColor;
    diskVisualUniform->get().turbulence = visual.turbulence;

    diskPipeline->bind(cmd);
    diskLayout->bindSets(cmd, {*diskSet, *Context::instance->uboSet});
    diskBuffer->bind(cmd);
    for (int i = 0; i < DISK_STACKS; ++i)
        vkCmdDraw(cmd, DISK_STRIP_VERTICES, 1, i * DISK_STRIP_VERTICES, 0);
}

void BlackHole::createOverlayContext(float viewportHeight)
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;

    overlayVertex = std::make_unique<VertexArray>(vkmgr, sizeof(Vec2f));
    overlayVertex->createBindingEntry(sizeof(Vec2f));
    overlayVertex->addInput(VK_FORMAT_R32G32_SFLOAT);
    overlayBuffer = overlayVertex->createBuffer(0, 1, context.tinyMgr.get());
    overlayScreenPos = static_cast<std::pair<float, float> *>(context.tinyMgr->getPtr(overlayBuffer->get()));

    overlayLayout = std::make_unique<PipelineLayout>(vkmgr);
    overlayLayout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0);
    overlayLayout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    overlayLayout->buildLayout();
    overlayLayout->setGlobalPipelineLayout(context.layouts.front().get());
    overlayLayout->build();

    overlaySet = std::make_unique<Set>(vkmgr, *context.setMgr, overlayLayout.get());
    overlayRmag = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
    overlayUniform = std::make_unique<SharedBuffer<OverlayUniform>>(*context.uniformMgr);
    overlaySet->bindUniform(overlayRmag, 0);
    overlaySet->bindUniform(overlayUniform, 1);

    overlayPipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, overlayLayout.get());
    overlayPipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    overlayPipeline->setDepthStencilMode(VK_FALSE, VK_FALSE);
    overlayPipeline->bindVertex(*overlayVertex);
    overlayPipeline->bindShader("blackhole_overlay.vert.spv");
    overlayPipeline->bindShader("blackhole_overlay.geom.spv");
    overlayPipeline->bindShader("blackhole_overlay.frag.spv");
    overlayPipeline->setSpecializedConstant(0, viewportHeight);
    overlayPipeline->build("Black hole overlay");
}

void BlackHole::drawOverlay(VkCommandBuffer cmd, double screen_sz)
{
    if (!overlayPipeline)
        createOverlayContext(VulkanMgr::instance->getScreenRect().extent.height);

    if (!overlayPipeline || overlayPipeline->get() == VK_NULL_HANDLE || !overlayScreenPos)
        return;

    const float outerRadius = diskEnabled ? static_cast<float>(diskOuterRadius * diskScale) : static_cast<float>(radius);
    const float eventRadius = std::max(5.f, static_cast<float>(screen_sz) * static_cast<float>(radius) / std::max(outerRadius, 0.000001f) * 0.90f);
    const float overlayRadius = std::max(28.f, eventRadius * 3.45f);

    BlackHoleLensing::submit(Vec2f(screenPos.first, screenPos.second), eventRadius, visual.lensingStrength);

    *overlayRmag = overlayRadius;
    overlayUniform->get().photonColorAndEventRadius = Vec4f(visual.photonColor[0], visual.photonColor[1], visual.photonColor[2], eventRadius);
    overlayUniform->get().lensColorAndStrength = Vec4f(visual.lensColor[0], visual.lensColor[1], visual.lensColor[2], visual.lensingStrength);
    overlayUniform->get().controls = Vec4f(visual.diskIntensity, 0.f, 0.f, 0.f);
    *overlayScreenPos = screenPos;

    overlayPipeline->bind(cmd);
    overlayLayout->bindSets(cmd, {*overlaySet, *Context::instance->uboSet});
    overlayBuffer->bind(cmd);
    vkCmdDraw(cmd, 1, 1, 0, 0);
}
