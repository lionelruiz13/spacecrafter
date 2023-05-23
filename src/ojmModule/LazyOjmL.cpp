#include "LazyOjmL.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/context.hpp"
#include "EntityCore/Executor/AsyncLoaderMgr.hpp"
#include "tools/vecmath.hpp"
#include "tools/log.hpp"
#include <sstream>

struct OjmVertex {
    Vec3f vertex;
    Vec2f uv;
    Vec3f normal;
};

struct OjmCacheHeader {
    uint64_t timestamp;
    uint64_t binFile;
    uint32_t indexCount;
    uint32_t vertexCount;
};

LazyOjmL::LazyOjmL(const std::filesystem::path &source, LoadPriority priority) :
    AsyncLoader(source, priority)
{
    once = false;
    AsyncLoaderMgr::instance->addTask(this);
}

LazyOjmL::LazyOjmL(VertexBuffer *vertex, SubBuffer index, unsigned int indexCount) :
    AsyncLoader(".", LoadPriority::DONE), vertex(std::move(vertex)), indexBuffer(index), index({indexCount, static_cast<uint32_t>(index.offset / sizeof(int))})
{
}

void LazyOjmL::bind(Pipeline &pipeline)
{
    pipeline.bindVertex(*Context::instance->ojmVertexArray);
}

void LazyOjmL::bind(VkCommandBuffer cmd)
{
    vkCmdBindIndexBuffer(cmd, Context::instance->indexBufferMgr->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    const VkDeviceSize zero = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &Context::instance->ojmBufferMgr->getBuffer(), &zero);
}

void LazyOjmL::generateCache(SaveData &cache)
{
    std::ostringstream oss;
    oss << "Generating cache of OjmL " << source;
    cLog::get()->write(oss.str(), LOG_TYPE::L_DEBUG);

    std::ifstream stream(source, std::ios_base::in);
    char line[265];
    std::vector<Vec3f> vertices;
    std::vector<Vec2f> uvs;
    std::vector<Vec3f> normals;
    std::vector<uint32_t> indices;

    if (stream.is_open()) {
        float radius = 0;
        while (!stream.eof()) {
            stream.getline(line,256);

            switch (line[0]) {
                case 'v':
                    {
                        Vec3f vertex;
                        std::stringstream ss(std::string(line+2));
                        ss >> vertex.v[0] >> vertex.v[1] >> vertex.v[2];
                        float tmp = vertex.lengthSquared();
                        if (radius < tmp)
                            radius = tmp;
                        vertices.push_back(vertex);
                    }
                    break;
                case 'u':
                    {
                        Vec2f uv;
                        std::stringstream ss(std::string(line+2));
                        ss >> uv.v[0] >> uv.v[1];
                        uvs.push_back(uv);
                    }
                    break;
                case 'n':
                    {
                        Vec3f normal;
                        std::stringstream ss(std::string(line+2));
                        ss >> normal.v[0] >> normal.v[1] >> normal.v[2];
                        normals.push_back(normal);
                    }
                    break;
                case 'j':
                    {
                        unsigned int idx;
                        std::stringstream ss(std::string(line+2));
                        for (int k = 0; k < 9; ++k) {
                            ss >> idx;
                            indices.push_back(idx);
                        }
                    }
                    break;
                case 'i':
                    {
                        unsigned int idx;
                        std::stringstream ss(std::string(line+2));
                        for (int k = 0; k < 3; ++k) {
                            ss >> idx;
                            indices.push_back(idx);
                        }
                    }
                    break;
            }
        };
        radius = sqrt(radius);
        if (radius > 1.001) {
            std::ostringstream oss;
            oss << "Downscale ojml " << source << " from " << radius << " to 1";
            cLog::get()->write(oss.str(), LOG_TYPE::L_WARNING);
            for (auto &v : vertices) {
                v /= radius;
            }
        }
    }

    std::vector<OjmVertex> vertexDatas;
    vertexDatas.reserve(vertices.size());
    for (uint32_t i = 0; i < vertices.size(); ++i) {
        vertexDatas.push_back({vertices[i], uvs[i], normals[i]});
    }
    auto cfile = AsyncLoaderMgr::instance->setBinCache(cache);
    cache.get().resize(sizeof(OjmCacheHeader));
    OjmCacheHeader &header = cache;
    header.indexCount = indices.size();
    header.vertexCount = vertexDatas.size();
    cfile.write(reinterpret_cast<const char *>(vertexDatas.data()), vertexDatas.size() * sizeof(OjmVertex));
    cfile.write(reinterpret_cast<const char *>(indices.data()), indices.size() * sizeof(uint32_t));
}

void LazyOjmL::loadCache(SaveData &cache, AL_FILE binCache)
{
    const OjmCacheHeader &header = cache;
    index.count = header.indexCount;
    index.offset = header.vertexCount;
    std::cout << "Statistics v" << header.vertexCount << " i" << header.indexCount << std::endl;
    staging = Context::instance->asyncStagingMgr->acquireBuffer(index.count * sizeof(int) + index.offset * sizeof(OjmVertex));
    AL_READ(binCache, Context::instance->asyncStagingMgr->getPtr(staging), staging.size);
}

void LazyOjmL::postLoad()
{
    Context &context = *Context::instance;
    indexBuffer = context.indexBufferMgr->acquireBuffer(index.count * sizeof(int));
    vertexHandler = context.ojmVertexArray->createBuffer(0, index.offset, context.ojmBufferMgr.get());
    vertex = vertexHandler.get();
    const int vertexSize = index.offset * sizeof(OjmVertex);
    context.transfer->planCopyBetween(staging, vertex->get(), vertexSize);
    context.transfer->planCopyBetween(staging, indexBuffer, indexBuffer.size, vertexSize, 0);
    index.offset = indexBuffer.offset / sizeof(int);
    context.transientAsyncBuffer[context.frameIdx].push_back(staging);
    std::ostringstream oss;
    oss << "Asynchronously loaded OjmL " << source;
    cLog::get()->write(oss.str(), LOG_TYPE::L_DEBUG);
}
