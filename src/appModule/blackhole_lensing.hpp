/*
 * Spacecrafter astronomy simulation and visualization
 * Copyright (C) 2026
 */

#ifndef BLACKHOLE_LENSING_HPP
#define BLACKHOLE_LENSING_HPP

#include "tools/no_copy.hpp"
#include "tools/vecmath.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

#include <array>
#include <memory>
#include <mutex>
#include <vector>

class Pipeline;
class PipelineLayout;
class Set;
class Texture;
class VertexArray;
class VertexBuffer;

class BlackHoleLensing : public NoCopy {
public:
    BlackHoleLensing(const std::vector<std::unique_ptr<Texture>> &sceneTextures,
                     float viewportWidth,
                     float viewportHeight);
    ~BlackHoleLensing();

    static void beginFrame();
    static void submit(const Vec2f &center, float eventRadius, float strength, bool distortionEnabled);
    struct DiskModel {
        Vec4f camera;
        Vec4f diskX;
        Vec4f diskY;
        Vec4f projection;
        Vec4f diskColor;
        Vec4f photonColor;
    };
    static void submitDiskModel(const DiskModel &model, Texture &density, double distance);
    static void recordTransfer(VkCommandBuffer cmd);
    void draw();

private:
    std::unique_ptr<Pipeline> createScenePipeline(bool lensing);
    void ensureDiskResources(bool trajectoriesNeeded);
    struct LensUniform {
        Vec4f centerRadiusStrength;
        Vec4f viewportActive;
    };

    struct LensState {
        Vec2f center = Vec2f(0.f, 0.f);
        float eventRadius = 0.f;
        float strength = 0.f;
        bool distortionEnabled = true;
        bool active = false;
        struct Disk {
            DiskModel model;
            Texture *density;
            double distance;
        };
        std::vector<Disk> disks;
    };

    static std::mutex stateMutex;
    static LensState pendingState;
    static Texture *pendingLutUpload;

    float viewportWidth;
    float viewportHeight;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<Pipeline> copyPipeline;
    std::unique_ptr<Texture> trajectoryLut;
    std::unique_ptr<PipelineLayout> lutLayout;
    std::unique_ptr<Pipeline> lutPipeline;
    std::vector<std::vector<std::unique_ptr<SharedBuffer<DiskModel>>>> diskModelUniforms;
    std::vector<std::vector<std::unique_ptr<Set>>> lutSets;
    std::unique_ptr<SharedBuffer<LensUniform>> uniform;
    std::vector<std::unique_ptr<Set>> sets;
    std::vector<VkDescriptorImageInfo> inputAttachmentInfo;
    std::array<int, 3> commands{};
    std::array<int, 3> diskCommands{};
};

#endif
