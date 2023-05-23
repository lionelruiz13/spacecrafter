#ifndef LAZY_OJML_HPP_
#define LAZY_OJML_HPP_

#include "EntityCore/Forward.hpp"
#include "EntityCore/SubBuffer.hpp"
#include "EntityCore/Executor/AsyncLoader.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"

class LazyOjmL : public AsyncLoader {
public:
    LazyOjmL(const std::filesystem::path &source, LoadPriority priority);
    LazyOjmL(VertexBuffer *vertex, SubBuffer index, unsigned int indexCount);

    inline void draw(VkCommandBuffer cmd) {
        vkCmdDrawIndexed(cmd, index.count, 1, index.offset, vertex->getOffset(), 0);
    }

    static void bind(VkCommandBuffer cmd);
    static void bind(Pipeline &pipeline);

    virtual void generateCache(SaveData &cache) override;
    virtual void loadCache(SaveData &cache, AL_FILE binCache) override;
    virtual void postLoad() override;
private:
    VertexBuffer *vertex;
    std::unique_ptr<VertexBuffer> vertexHandler;
    SubBuffer indexBuffer;
    SubBuffer staging;
    struct {
        unsigned int count = 0;
        unsigned int offset = 0; // Used as vertexCount before postLoad
    } index;
};

#endif /* end of include guard: LAZY_OJML_HPP_ */
