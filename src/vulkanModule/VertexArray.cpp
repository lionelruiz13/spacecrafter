#include "VirtualSurface.hpp"
#include "CommandMgr.hpp"
#include "VertexBuffer.hpp"
#include "VertexArray.hpp"
#include "Buffer.hpp"
#include "tools/log.hpp"

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

static int getFormatSize(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_UINT: return 1;
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_UINT: return 2;
        case VK_FORMAT_R32G32B32_SFLOAT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_UINT: return 3;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_UINT: return 4;
        default: assert(false); return 0;
    }
}

VertexArray::VertexArray(VirtualSurface *_master, CommandMgr *_mgr, bool updateOnBind) : master(_master), mgr(_mgr), vertexBuffer(nullptr), instanceBuffer(nullptr), indexBuffer(nullptr), vertice(offset), updateOnBind(updateOnBind) {}

VertexArray::~VertexArray() {}

VertexArray::VertexArray(VertexArray &model) : master(model.master), mgr(model.mgr), instanceBuffer(nullptr), bindingDesc(model.bindingDesc), bindingDesc2(model.bindingDesc2), vertice(offset), blockSize(model.blockSize), maxVertices(model.maxVertices), maxIndex(model.maxIndex), indexBufferSize(model.indexBufferSize), indexType(model.indexType), updateOnBind(model.updateOnBind)
{
    // instanceBuffer mustn't be build for copy, at least for now
    assert(!model.instanceBuffer);
    offset = model.offset;
    attributeDesc.assign(model.attributeDesc.begin(), model.attributeDesc.end());
    attributeDesc2.assign(model.attributeDesc2.begin(), model.attributeDesc2.end());
    vertexBuffer = model.vertexBuffer;
    indexBuffer = model.indexBuffer;
    vertexAccess = model.vertexAccess;
    instanceAccess = model.instanceAccess;
    indexAccess = model.indexAccess;
    pVertexData = model.pVertexData;
    pIndexData = model.pIndexData;
}

void VertexArray::build(int _maxVertices)
{
    maxVertices = _maxVertices;
    vertexBuffer = nullptr; // Release resources of previous vertexBuffer if any
    vertexBuffer = std::make_shared<VertexBuffer>(master, maxVertices, bindingDesc, attributeDesc, (vertexAccess == BufferAccess::STREAM_LOCAL), (vertexAccess==BufferAccess::STREAM));
    pVertexData = static_cast<float *>(vertexBuffer->data);
}

void VertexArray::buildInstanceBuffer(int _maxInstances)
{
    maxInstances = _maxInstances;
    instanceBuffer = nullptr; // Release resources of previous instanceBuffer if any
    instanceBuffer = std::make_unique<VertexBuffer>(master, maxInstances, bindingDesc2, attributeDesc2, (instanceAccess == BufferAccess::STREAM_LOCAL), (instanceAccess==BufferAccess::STREAM));
    pInstanceData = static_cast<float *>(instanceBuffer->data);
}

void VertexArray::bind(CommandMgr *cmdMgr)
{
    if (cmdMgr == nullptr)
        cmdMgr = mgr;
    cmdMgr->bindVertex(vertexBuffer.get(), 0, vertexBuffer->getOffset());
    if (instanceBuffer)
        cmdMgr->bindVertex(instanceBuffer.get(), 1, instanceBuffer->getOffset());
    if (indexBuffer)
        cmdMgr->bindIndex(indexBuffer.get(), indexType);
    if (updateOnBind)
        update();
}

void VertexArray::update()
{
    if (vertexUpdate) {
        switch (vertexAccess) {
            case BufferAccess::DYNAMIC:
                vertexBuffer->update();
                break;
            case BufferAccess::STATIC:
                vertexBuffer->update();
                vertexBuffer->detach();
                break;
            default: break;
        }
        vertexUpdate = false;
    }
    if (instanceBuffer) {
        if (instanceUpdate) {
            instanceBuffer->update();
            instanceUpdate = false;
            if (instanceAccess == BufferAccess::STATIC)
                instanceBuffer->detach();
        }
    }
    if (indexBuffer) {
        if (indexUpdate) {
            indexBuffer->update();
            indexUpdate = false;
            if (indexAccess == BufferAccess::STATIC)
                indexBuffer->detach();
        }
    }
}

void VertexArray::update(VkCommandBuffer cmdBuffer)
{
    vertexBuffer->update(cmdBuffer);
}

void VertexArray::updateVertex(int size)
{
    vertexBuffer->update(size);
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
    if (vertexAccess < ba) vertexAccess = ba;
}

void VertexArray::fillVertexBuffer(const std::vector<float> &data)
{
    assert(getVertexOffset() + data.size() / blockSize <= maxVertices);
    memcpy(pVertexData, data.data(), data.size() * sizeof(float));
    vertexUpdate = true;
}

void VertexArray::fillVertexBuffer(const BufferType& bt, const std::vector<float> &data)
{
    if (attributeDesc.size() == 1) { // Optimize if there is only one BufferType
        fillVertexBuffer(data);
        return;
    }
    fillVertexBuffer(bt, data.size(), data.data());
}

void VertexArray::fillVertexBuffer(const BufferType& bt, unsigned int size, const float* data)
{
    assert(getVertexOffset() + size / getDataSize(bt) <= maxVertices);
    float *tmp = pVertexData + offset[getLayout(bt)];
    const uint8_t dataSize = getDataSize(bt);
    size /= dataSize;
    for (unsigned int i = 0; i < size; ++i) {
        for (uint8_t j = 0; j < dataSize; ++j)
            tmp[j] = *(data++); // *(data++); work like data[i * dataSize + j]; here
        tmp += blockSize;
    }
    vertexUpdate = true;
}

void VertexArray::registerInstanceBuffer(const BufferAccess& ba, VkFormat format)
{
    VkVertexInputAttributeDescription desc;
    desc.binding = 1;
    desc.location = attributeDesc2.size() + 6; // in order not to overlap vertex locations
    desc.format = format;
    desc.offset = bindingDesc2.stride;
    blockSize2 += getFormatSize(format);
    bindingDesc2.stride = blockSize2 * sizeof(float);
    attributeDesc2.push_back(desc);
    if (instanceAccess < ba) instanceAccess = ba;
}

void VertexArray::fillInstanceBuffer(const std::vector<float> &data)
{
    assert(data.size() / blockSize2 <= maxInstances);
    memcpy(pInstanceData, data.data(), data.size() * sizeof(float));
    instanceUpdate = true;
}

float *VertexArray::getInstanceBufferPtr()
{
    return (static_cast<float *>(instanceBuffer->data));
}

void VertexArray::registerIndexBuffer(const BufferAccess& ba, unsigned int size, size_t blockSize, VkIndexType _indexType)
{
    indexBuffer = std::make_shared<Buffer>(master, size * blockSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    indexType = _indexType;
    maxIndex = size;
    pIndexData = static_cast<unsigned int *>(indexBuffer->data);
    indexAccess = ba;
}

void VertexArray::fillIndexBuffer(const std::vector<unsigned int> &data)
{
    fillIndexBuffer(data.size(), data.data());
}

void VertexArray::fillIndexBuffer(unsigned int size, const unsigned int* data)
{
    int dec = pVertexData ? getVertexOffset() : 0;
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

unsigned int VertexArray::getIndiceCount() const
{
    return indexBufferSize;
}

void VertexArray::assumeVerticeChanged(bool needUpdate)
{
    vertexUpdate = needUpdate;
}

void VertexArray::assign(VertexArray *vertex, uint32_t maxVertices, uint32_t maxIndex)
{
    vertexBuffer = vertex->vertexBuffer;
    indexBuffer = vertex->indexBuffer;
    pVertexData = vertex->pVertexData;
    pIndexData = vertex->pIndexData;
    indexType = vertex->indexType;
    this->maxVertices = vertex->getVertexOffset() + maxVertices;
    assert(this->maxVertices <= vertex->maxVertices);
    vertex->pVertexData += maxVertices * blockSize;
    if (maxIndex > 0) {
        this->maxIndex = vertex->getIndexOffset() + maxIndex;
        assert(this->maxIndex <= vertex->maxIndex);
        vertex->pIndexData += maxIndex;
    }
}

void VertexArray::print()
{
    std::ostringstream oss;
    if (pVertexData) {
        oss << "VertexBuffer :\n";
        vertexBuffer->print(oss);
    }
    if (pIndexData) {
        oss << "IndexBuffer :\n";
        indexBuffer->print(oss);
    }
    if (instanceBuffer) {
        oss << "InstanceBuffer :\n";
        instanceBuffer->print(oss);
    }
    cLog::get()->write(oss.str(), LOG_TYPE::L_OTHER);
}

void VertexArray::setName(const std::string &name)
{
    if (vertexBuffer)
        vertexBuffer->setName("vertex buffer " + name);
    if (indexBuffer)
        indexBuffer->setName("index buffer " + name);
    if (instanceBuffer)
        instanceBuffer->setName("instance buffer " + name);
}

float *VertexArray::getStagingVertexBufferPtr()
{
    return pVertexData;
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
