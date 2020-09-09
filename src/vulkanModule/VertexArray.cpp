#include "VirtualSurface.hpp"
#include "CommandMgr.hpp"
#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "Buffer.hpp"

static unsigned int getLayout(const BufferType& bt)
{
    switch (bt) {
        case BufferType::POS2D    : return 0;
        case BufferType::POS3D    : return 0;
        case BufferType::TEXTURE  : return 1;
        case BufferType::NORMAL   : return 2;
        case BufferType::COLOR    : return 3;
        case BufferType::COLOR4   : return 3;
        case BufferType::MAG      : return 4;
        case BufferType::SCALE    : return 5;
        default: assert(false); return 0;
    }
}

static unsigned int getDataSize(const BufferType& bt)
{
    switch (bt) {
        case BufferType::POS2D    : return 2;
        case BufferType::POS3D    : return 3;
        case BufferType::TEXTURE  : return 2;
        case BufferType::NORMAL   : return 3;
        case BufferType::COLOR    : return 3;
        case BufferType::COLOR4   : return 4;
        case BufferType::MAG      : return 1;
        case BufferType::SCALE    : return 1;
        default: assert(false); return 0;
    }
}

static VkFormat getFormat(unsigned int size)
{
    switch (size) {
        case 1: return VK_FORMAT_R32_SFLOAT;
        case 2: return VK_FORMAT_R32G32_SFLOAT;
        case 3: return VK_FORMAT_R32G32B32_SFLOAT;
        case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default: assert(false); return VK_FORMAT_UNDEFINED;
    }
}

VertexArray::VertexArray(VirtualSurface *_master, CommandMgr *_mgr) : master(_master), mgr(_mgr), vertexBuffer(nullptr), instanceBuffer(nullptr), indexBuffer(nullptr), vertice(offset) {}

VertexArray::~VertexArray() {}

VertexArray::VertexArray(VertexArray &model) : master(model.master), mgr(model.mgr), instanceBuffer(nullptr), bindingDesc(model.bindingDesc), bindingDesc2(model.bindingDesc2), vertice(offset), blockSize(model.blockSize), indexBufferSize(model.indexBufferSize)
{
    // VertexArray mustn't be build for copy, at least for now
    assert(!model.instanceBuffer);
    offset = model.offset;
    attributeDesc.assign(model.attributeDesc.begin(), model.attributeDesc.end());
    attributeDesc2.assign(model.attributeDesc2.begin(), model.attributeDesc2.end());
    vertexBuffer = model.vertexBuffer;
    indexBuffer = model.indexBuffer;
    pVertexData = model.pVertexData;
    pIndexData = model.pIndexData;
}

void VertexArray::build(int maxVertices)
{
    vertexBuffer = std::make_shared<VertexBuffer>(master, maxVertices, bindingDesc, attributeDesc);
    pVertexData = static_cast<float *>(vertexBuffer->data);
}

void VertexArray::buildInstanceBuffer(int maxInstances)
{
    instanceBuffer = std::make_unique<VertexBuffer>(master, maxInstances, bindingDesc2, attributeDesc2);
}

void VertexArray::bind(CommandMgr *cmdMgr)
{
    if (cmdMgr == nullptr)
        cmdMgr = mgr;
    if (vertexUpdate) {
        vertexBuffer->update();
        vertexUpdate = false;
    }
    cmdMgr->bindVertex(vertexBuffer.get());
    if (instanceBuffer) {
        if (instanceUpdate) {
            instanceBuffer->update();
            instanceUpdate = false;
        }
        cmdMgr->bindVertex(vertexBuffer.get(), 1);
    }
    if (indexBuffer) {
        if (indexUpdate) {
            indexBuffer->update();
            indexUpdate = false;
        }
        cmdMgr->bindIndex(indexBuffer.get(), VK_INDEX_TYPE_UINT32);
    }
}

void VertexArray::update()
{
    if (vertexUpdate) {
        vertexBuffer->update();
        vertexUpdate = false;
    }
    if (instanceBuffer) {
        if (instanceUpdate) {
            instanceBuffer->update();
            instanceUpdate = false;
        }
    }
    if (indexBuffer) {
        if (indexUpdate) {
            indexBuffer->update();
            indexUpdate = false;
        }
    }
}

void VertexArray::update(VkCommandBuffer cmdBuffer)
{
    vertexBuffer->update(cmdBuffer);
}

void VertexArray::registerVertexBuffer(const BufferType& bt, const BufferAccess& ba)
{
    VkVertexInputAttributeDescription desc;
    desc.binding = 0;
    desc.location = getLayout(bt);
    desc.format = getFormat(getDataSize(bt));
    desc.offset = bindingDesc.stride;
    offset[desc.location] = blockSize;
    blockSize += getDataSize(bt);
    bindingDesc.stride = blockSize * sizeof(float);
    attributeDesc.push_back(desc);
}

void VertexArray::fillVertexBuffer(const std::vector<float> data)
{
    memcpy(pVertexData, data.data(), data.size() * sizeof(float));
}

void VertexArray::fillVertexBuffer(const BufferType& bt, const std::vector<float> data)
{
    if (attributeDesc.size() == 1) { // Optimize if there is only one BufferType
        fillVertexBuffer(data);
        return;
    }
    fillVertexBuffer(bt, data.size(), data.data());
}

void VertexArray::fillVertexBuffer(const BufferType& bt, unsigned int size, const float* data)
{
    float *tmp = pVertexData + offset[getLayout(bt)];
    const uint8_t dataSize = getDataSize(bt);
    size /= dataSize;
    for (unsigned int i = 0; i < size; ++i) {
        for (uint8_t j = 0; j < dataSize; ++j)
            tmp[j] = *(data++); // *(data++); work like data[i * dataSize + j]; here
        tmp += size * blockSize;
    }
    vertexUpdate = true;
}

void VertexArray::registerInstanceBuffer(const BufferAccess& ba, VkFormat format, unsigned int stride)
{
    VkVertexInputAttributeDescription desc;
    desc.binding = 1;
    desc.location = attributeDesc2.size();
    desc.format = format;
    desc.offset = bindingDesc2.stride;
    blockSize += stride;
    bindingDesc2.stride = blockSize * sizeof(float);
    attributeDesc2.push_back(desc);
}

float *VertexArray::getInstanceBufferPtr()
{
    return (static_cast<float *>(instanceBuffer->data));
}

void VertexArray::registerIndexBuffer(const BufferAccess& ba, unsigned int size, size_t blockSize)
{
    indexBuffer = std::make_shared<Buffer>(master, size * blockSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    pIndexData = static_cast<unsigned int *>(indexBuffer->data);
}

void VertexArray::fillIndexBuffer(const std::vector<unsigned int> data)
{
    fillIndexBuffer(data.size(), data.data());
}

int VertexArray::getVertexOffset() const
{
    return static_cast<long>(pVertexData - static_cast<float *>(vertexBuffer->data)) / blockSize;
}

void VertexArray::setVertexOffset(int offset)
{
    pVertexData = static_cast<float *>(vertexBuffer->data) + offset * blockSize;
}

int VertexArray::getIndexOffset() const
{
    return static_cast<long>(pIndexData - static_cast<unsigned int *>(indexBuffer->data));
}

void VertexArray::fillIndexBuffer(unsigned int size, const unsigned int* data)
{
    int dec = getVertexOffset();
    if (dec > 0) {
        for (unsigned int i = 0; i < size; i++) {
            pIndexData[i] = data[i] + dec;
        }
    } else {
        memcpy(pIndexData, data, size * sizeof(unsigned int));
    }
    indexUpdate = true;
    indexBufferSize = size;
}

void VertexArray::assumeVerticeChanged()
{
    vertexUpdate = true;
}

void VertexArray::assign(VertexArray *vertex, int maxVertices, int maxIndex)
{
    vertexBuffer = vertex->vertexBuffer;
    indexBuffer = vertex->indexBuffer;
    pVertexData = vertex->pVertexData;
    pIndexData = vertex->pIndexData;
    vertex->pVertexData += maxVertices * blockSize;
    vertex->pIndexData += maxIndex;
}

VertexArray::Vertice &VertexArray::operator[](int pos)
{
    vertice.setData(pVertexData + pos * blockSize);
    return vertice;
}

VertexArray::Vertice::Vertice(std::array<uint8_t, 6> &_offset) : offset(_offset) {}

float *VertexArray::Vertice::operator[](const BufferType &bt)
{
    return data + offset[getLayout(bt)];
}
