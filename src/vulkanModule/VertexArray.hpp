#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <memory>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

//! insert all Ts ...ts in vector<T>
template <typename T, typename ... Ts>
void insert_all(std::vector<T> &vec, Ts ... ts)
{
    (vec.push_back(ts), ...);
}

//! Define buffer layout
enum class BufferType : char { POS2D = 0 , POS3D , TEXTURE, NORMAL, COLOR, COLOR4, MAG, SCALE };

//! Define Buffer access type
enum class BufferAccess : char { STATIC = 0, DYNAMIC, STREAM};

class VirtualSurface;
class CommandMgr;
class VertexBuffer;
class Buffer;

class VertexArray
{
public:
    //! constructor...
    VertexArray(VirtualSurface *_master, CommandMgr *_mgr);
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
    //! modify the VertexBuffer identified by  his BufferType and integrating size elements from data
    void fillVertexBuffer(const BufferType& bt, unsigned int size , const float* data);
    //! modify the VertexBuffer identified by  his BufferType and integrating all elements fron vector data
    void fillVertexBuffer(const BufferType& bt, const std::vector<float> data);

    //! register a new type of vertex buffer giving its function and access type
    void registerInstanceBuffer(const BufferAccess& ba, VkFormat format, unsigned int stride);
    //! get pointer to InstanceBuffer data
    float *getInstanceBufferPtr();
    //! export data contained in pointer returned by getInstanceBufferPtr at next update call
    void assumeInstanceBufferChange() {instanceUpdate = true;}

    //! register a index buffer giving its access type
    void registerIndexBuffer(const BufferAccess& ba, unsigned int size = 4096, size_t blockSize = sizeof(unsigned int));
    //! modify the IndexBuffer by integrating count elements from integer indices
    void fillIndexBuffer(const std::vector<unsigned int> data);
    //! modify the IndexBuffer by integrating count elements from integer indices
    void fillIndexBuffer(unsigned int count , const unsigned int* indices);

    //! build VertexArray
    void build(int maxVertices = 4096);
    //! build VertexArray per instance
    void buildInstanceBuffer(int maxInstances);
    //! bind this vao
    void bind();
    //! update changes
    void update();
    //! unbind this vao
    void unBind() const {}
    //! returns the number of elements contained in the index buffer
    unsigned int getIndiceCount() const;
    Vertice &operator[](int pos);
    //! Tell vertice value have changed with operator[]
    void assumeVerticeChanged();
    VertexBuffer &getVertexBuffer() const {return *vertexBuffer;}
    const VkVertexInputBindingDescription &getVertexBindingDesc() {return bindingDesc;}
    const std::vector<VkVertexInputAttributeDescription> &getVertexAttributeDesc() {return attributeDesc;}
private:
    VirtualSurface *master;
    CommandMgr *mgr;
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<VertexBuffer> instanceBuffer;
    std::unique_ptr<Buffer> indexBuffer;
    VkVertexInputBindingDescription bindingDesc{0, 0, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputBindingDescription bindingDesc2{0, 0, VK_VERTEX_INPUT_RATE_INSTANCE};
    std::vector<VkVertexInputAttributeDescription> attributeDesc;
    std::vector<VkVertexInputAttributeDescription> attributeDesc2;
    Vertice vertice;
    //! offset of corresponding BufferType inside a block
    std::array<uint8_t, 6> offset;
    //! Size of one block
    int blockSize = 0;
    unsigned int indexBufferSize;
    bool vertexUpdate = false;
    bool instanceUpdate = false;
    bool indexUpdate = false;
};

#endif /* end of include guard: VERTEX_ARRAY_HPP */