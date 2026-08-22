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
    BlackHoleLensing(const std::vector<std::unique_ptr<Texture>> &sceneTextures, float viewportWidth, float viewportHeight);
    ~BlackHoleLensing();

    static void beginFrame();
    static void submit(const Vec2f &center, float eventRadius, float strength, bool distortionEnabled);
    void draw();

private:
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
    };

    static std::mutex stateMutex;
    static LensState pendingState;

    float viewportWidth;
    float viewportHeight;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<SharedBuffer<LensUniform>> uniform;
    std::vector<std::unique_ptr<Set>> sets;
    std::array<int, 3> commands{};
};

#endif
