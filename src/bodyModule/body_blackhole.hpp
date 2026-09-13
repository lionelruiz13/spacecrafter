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

#ifndef BODY_BLACKHOLE_HPP_
#define BODY_BLACKHOLE_HPP_

#include "bodyModule/body.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

class Pipeline;
class PipelineLayout;
class Set;
class VertexArray;
class VertexBuffer;
class s_texture;

struct BlackHoleVisual {
    Vec3f diskColor = Vec3f(1.0f, 0.31f, 0.045f);
    Vec3f photonColor = Vec3f(1.0f, 0.86f, 0.42f);
    Vec3f lensColor = Vec3f(0.70f, 0.12f, 0.025f);
    float diskIntensity = 1.0f;
    float turbulence = 1.0f;
    float lensingStrength = 1.0f;
    bool distortionEnabled = true;
};

class BlackHole : public Body {
public:
    BlackHole(std::shared_ptr<Body> parent,
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
              const BlackHoleVisual &visual);
    virtual ~BlackHole();

    virtual void selectShader() override;
    virtual float computeMagnitude(const Vec3d obs_pos) const override;
    virtual double calculateBoundingRadius() override;
    virtual float getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only = false) override;
    virtual void setSphereScale(float s, bool initial_scale = false) override;
    virtual void drawOrbit(VkCommandBuffer cmdBodyDepth, VkCommandBuffer cmdOrbit, const Observer* observatory, const Navigator* nav, const Projector* prj) override;

protected:
    virtual Set &getSet(float screen_sz) override;
    virtual bool hasRings() override;
    virtual void drawTrail(VkCommandBuffer cmd, const Navigator* nav, const Projector* prj) override;
    virtual void drawHints(const Navigator* nav, const Projector* prj) override;
    virtual void drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest) override;
    virtual void drawRings(VkCommandBuffer cmd, const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz, Vec3f& lightDirection, Vec3f& planetPosition, float planetRadius) override;
    virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye) override;
    virtual void drawAxis(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat) override;
    virtual void drawPlanetGrid(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat) override;

private:
    struct DiskUniform {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
        float RingScale;
        float fadingFactor;
    };

    struct DiskVisualUniform {
        Vec3f diskColor;
        float diskIntensity;
        Vec3f photonColor;
        float turbulence;
    };

    struct HorizonUniform {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
        float HorizonRadius;
    };

    struct OverlayUniform {
        Vec4f photonColorAndEventRadius;
        Vec4f lensColorAndStrength;
        Vec4f controls;
    };

    void createDiskContext();
    void createHorizonContext();
    void createOverlayContext(float viewportHeight);
    void buildDiskMesh();
    void buildHorizonMesh();
    void drawDisk(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat, double screen_sz);
    void drawHorizon(VkCommandBuffer cmd, const Projector* prj, const Mat4d& mat);
    void drawOverlay(VkCommandBuffer cmd, double screen_sz);

    std::unique_ptr<s_texture> diskTex;
    double diskInnerRadius;
    double diskOuterRadius;

    std::unique_ptr<VertexArray> diskVertex;
    std::unique_ptr<VertexBuffer> diskBuffer;
    std::unique_ptr<PipelineLayout> diskLayout;
    std::unique_ptr<Pipeline> diskPipeline;
    std::unique_ptr<Set> diskSet;
    std::unique_ptr<SharedBuffer<DiskUniform>> diskUniform;
    std::unique_ptr<SharedBuffer<DiskVisualUniform>> diskVisualUniform;

    std::unique_ptr<VertexArray> horizonVertex;
    std::unique_ptr<VertexBuffer> horizonBuffer;
    std::unique_ptr<PipelineLayout> horizonLayout;
    std::unique_ptr<Pipeline> horizonPipeline;
    std::unique_ptr<Set> horizonSet;
    std::unique_ptr<SharedBuffer<HorizonUniform>> horizonUniform;

    std::unique_ptr<VertexArray> overlayVertex;
    std::unique_ptr<VertexBuffer> overlayBuffer;
    std::unique_ptr<PipelineLayout> overlayLayout;
    std::unique_ptr<Pipeline> overlayPipeline;
    std::unique_ptr<Set> overlaySet;
    std::unique_ptr<SharedBuffer<float>> overlayRmag;
    std::unique_ptr<SharedBuffer<OverlayUniform>> overlayUniform;
    std::pair<float, float> *overlayScreenPos = nullptr;

    float diskScale = 1.f;
    bool diskEnabled = true;
    BlackHoleVisual visual;
};

#endif
