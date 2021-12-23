#include "NDISender.hpp"

NDISender::NDISender(VulkanMgr &vkmgr, std::vector<std::unique_ptr<Texture>> &frames, VkFence *fences) : FrameSender(vkmgr, frames, fences)
{
}

NDISender::~NDISender()
{
}

void NDISender::submitFrame(void *data)
{
    // Pointer to image data of size (width * height * 4 * sizeof(uint8_t))
    // 4 * sizeof(uint8_t) for R8G8B8A8_UNORM
}
