#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <memory>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <cstring>

//! insert all Ts ...ts in vector<T>
template <typename T, typename ... Ts>
void insert_all(std::vector<T> &vec, Ts ... ts)
{
    (vec.push_back(ts), ...);
}

//! Define buffer layout
enum class BufferType : char { POS2D = 0 , POS3D , TEXTURE, NORMAL, COLOR, COLOR4, MAG, SCALE };

//! Define Buffer access type, in increasing priority order (STATIC < DYNAMIC < STREAM_LOCAL < STREAM)
enum class BufferAccess : char { STATIC = 0, DYNAMIC, STREAM_LOCAL, STREAM};

class VirtualSurface;
class CommandMgr;
class VertexBuffer;
class Buffer;

/**
*   \brief Handle arrays used in local scope in draws.
*   Include vertex, index and instance arrays
*   Must be bound to pipeline when all vertex and instance entry were registered
*   Always prefer using STREAM access for small sizes (<= 8 vertices)
*/
class VertexArray
{
public:
    //! constructor...
    VertexArray(VirtualSurface *_master, CommandMgr *_mgr = nullptr, bool updateOnBind = true);
    VertexArray(VertexArray &model);
    ~VertexArray();

    class Vertice {
    public:
        Vertice(std::array<uint8_t, 6> &_offset);
        float *operator[](const BufferType &bt);
        void setData(float *_data) {data = _data;}
    private:
        float *data;
        std::array<uint8_t, 6> &offset;
    };

    //! register a new type of vertex buffer giving its function and access type
    void registerVertexBuffer(const BufferType& bt, const BufferAccess& ba);
    //! copy data inside the VertexBuffer (e.g : if register POS2D and TEXTURE, datas were packed {posX, posY, texX, texY})
    void fillVertexBuffer(const std::vector<float> &data);
    //! modify the VertexBuffer identified by  his BufferType and integrating size elements from data
    void fillVertexBuffer(const BufferType& bt, unsigned int size , const float* data);
    //! modify the VertexBuffer identified by  his BufferType and integrating all elements from vector data
    void fillVertexBuffer(const BufferType& bt, const std::vector<float> &data);

    //! register a new type of vertex buffer giving its function and access type
    void registerInstanceBuffer(const BufferAccess& ba, VkFormat format);
    //! copy data inside the InstanceBuffer
    void fillInstanceBuffer(const std::vector<float> &data);
    //! get pointer to InstanceBuffer data
    float *getInstanceBufferPtr();
    //! export data contained in pointer returned by getInstanceBufferPtr at next update call
    void assumeInstanceBufferChange() {instanceUpdate = true;}

    //! register a index buffer giving its access type
    void registerIndexBuffer(const BufferAccess& ba, unsigned int size = 4096, size_t blockSize = sizeof(unsigned int), VkIndexType _indexType = VK_INDEX_TYPE_UINT32);
    //! modify the IndexBuffer by integrating count elements from integer indices
    void fillIndexBuffer(const std::vector<unsigned int> &data);
    //! modify the IndexBuffer by integrating count elements from integer indices
    void fillIndexBuffer(unsigned int count , const unsigned int* indices);

    //! build VertexArray
    void build(int maxVertices = 4096);
    //! build VertexArray per instance
    void buildInstanceBuffer(int maxInstances);
    //! assign maxVertices
    void assign(VertexArray *vertex, uint32_t maxVertices, uint32_t maxIndex = 0);
    //! bind this vao
    void bind(CommandMgr *cmdMgr = nullptr);
    //! update changes
    void update();
    //! update changes
    void update(VkCommandBuffer cmdBuffer);
    //! update vertexBuffer, even if unchanged
    //! @param size hint on number of vertices to update
    void updateVertex(int size = -1);
    //! returns the number of elements updated by the last call of fillIndexBuffer
    unsigned int getIndiceCount() const;
    //! Get offset (in index) of IndexBuffer for fillIndexBuffer
    int getIndexOffset() const;
    //! Get offset (in vertice) of VertexBuffer for fillVertexBuffer
    int getVertexOffset() const;
    //! Set offset (in vertice) of VertexBuffer for fillVertexBuffer
    void setVertexOffset(int offset);
    //! Return corresponding vertice of VertexBuffer
    Vertice &operator[](int pos);
    //! Tell vertice value has changed (implicit when using fillVertexBuffer)
    void assumeVerticeChanged(bool needUpdate = true);
    //! Internally used
    VertexBuffer &getVertexBuffer() const {return *vertexBuffer;}
    //! Internally used
    const VkVertexInputBindingDescription &getVertexBindingDesc() {return bindingDesc;}
    //! Internally used
    const std::vector<VkVertexInputAttributeDescription> &getVertexAttributeDesc() {return attributeDesc;}
    //! Return true if at least one instanceBuffer component is registered
    bool hasInstanceBuffer() {return bindingDesc2.stride > 0;}
    //! Internally used
    const VkVertexInputBindingDescription &getInstanceBindingDesc() {return bindingDesc2;}
    //! Internally used
    const std::vector<VkVertexInputAttributeDescription> &getInstanceAttributeDesc() {return attributeDesc2;}
    //! Display debug informations
    void print();
    //! set custom name visible on debug layers (must be called when vertex, index and instance buffer were fully registered and build)
    void setName(const std::string &name);
private:
    VirtualSurface *master;
    CommandMgr *mgr;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    float *pVertexData = nullptr;
    std::unique_ptr<VertexBuffer> instanceBuffer;
    float *pInstanceData = nullptr;
    std::shared_ptr<Buffer> indexBuffer;
    unsigned int *pIndexData = nullptr;
    BufferAccess vertexAccess = BufferAccess::STATIC;
    BufferAccess instanceAccess = BufferAccess::STATIC;
    BufferAccess indexAccess;
    VkVertexInputBindingDescription bindingDesc{0, 0, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputBindingDescription bindingDesc2{1, 0, VK_VERTEX_INPUT_RATE_INSTANCE};
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc2;
    Vertice vertice;
    //! offset of corresponding BufferType inside a block
    std::array<uint8_t, 6> offset;
    //! Size of one block
    int blockSize = 0;
    int blockSize2 = 0;
    unsigned int maxVertices, maxInstances, maxIndex;
    unsigned int indexBufferSize = 0; // Last size updated with fillIndexBuffer
    VkIndexType indexType;
    //! Tell if local vertexBuffer content has changed
    bool vertexUpdate = false;
    //! Tell if local instanceBuffer content has changed
    bool instanceUpdate = false;
    //! Tell if local indexBuffer content has changed
    bool indexUpdate = false;
    //! Tell if update must be called when bind is called
    bool updateOnBind;
};

#endif /* end of include guard: VERTEX_ARRAY_HPP */
