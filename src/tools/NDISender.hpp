#ifndef NDISENDER_HPP_
#define NDISENDER_HPP_

#include "EntityCore/Resource/FrameSender.hpp"

class NDISender : public FrameSender {
public:
    NDISender(VulkanMgr &vkmgr, std::vector<std::unique_ptr<Texture>> &frames, VkFence *fences);
    virtual ~NDISender();
protected:
    virtual void submitFrame(void *data) override;
private:
    // Your local datas
};

#endif /* end of include guard: NDISENDER_HPP_ */
