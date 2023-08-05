#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "tools/vecmath.hpp"
#include <vulkan/vulkan.h>
#include <vector>

class ToneReproductor;
class FrameMgr;

class Renderer {
public:
    Renderer();
    void init(ToneReproductor *eye);
    void beginDraw(uint8_t frameIdx);
    void beginBodyDraw();
    void endBodyDraw();
    void clearDepth();
    void drawHalo(const std::pair<float, float> &pos, const Vec3f &color, float rmag);
    float adaptLuminance(float world_luminance) const;
private:
    void allocateCommands();
    void nextCommandBuffer();
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    ToneReproductor *eye;
    FrameMgr *frame;
    std::vector<VkCommandBuffer> cmds[3];
    uint16_t cmdIdx;
    uint8_t frameIdx;
};

#endif /* end of include guard: RENDERER_HPP_ */
