

#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <map>
#include <GL/glew.h>

#include <assert.h>
#include <stdint.h>

void GLClearError();
bool GLCheckError();

#define ASSERT(x) if (!(x)) assert(false)

#define DEBUG 1

#ifdef DEBUG
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLCheckError())
#else
#define GLCall(x) x
#endif



enum class BufferType : char { POS2D = 0 , POS3D , TEXTURE, NORMAL, COLOR, MAG, SCALE };
enum class BufferAccess : char { STATIC = 0, DYNAMIC, STREAM};

//unsigned int convert(BufferType& bt);


class Buffer {
public:
    Buffer(const BufferAccess& ba);
    ~Buffer(){};
    virtual void bind() const;
    virtual void unBind() const;
    bool isDefine() const {return m_isDefine;}
    GLenum getAccess() const {return m_bufferAcces;}
protected:
    unsigned int m_RendererID;
    bool m_isDefine;
    GLenum m_bufferAcces;
};


class VertexBuffer : public Buffer {
public:
    VertexBuffer(const BufferAccess& ba);
    //VertexBuffer(const void* data, unsigned int size);
    void fill(unsigned int size , const void* data);
    ~VertexBuffer();
private:
};


class IndexBuffer : public Buffer {
public:
    IndexBuffer(const BufferAccess& ba);
    ~IndexBuffer();
    
    void fill(unsigned int count , const unsigned int* indices);
    inline unsigned int GetCount() const { return m_Count; }

    void bind() const override;
    void unBind() const override;
private:
    unsigned int m_Count;
};


class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void registerVertexBuffer(const BufferType& bt, const BufferAccess& ba);

    void fillVertexBuffer(const BufferType& bt, unsigned int size , const void* data);

    void registerIndexBuffer(const BufferAccess& ba);

    void fillIndexBuffer(unsigned int count , const unsigned int* indices);

    void bind() const;
    void unBind() const;

    IndexBuffer* getIndexBuffer() const {
        return m_indexBuffer;
    }

private:
    unsigned int m_RendererID;
    IndexBuffer* m_indexBuffer = nullptr;
    std::map<const BufferType, Buffer*> m_buffer;
};


#endif // OPENGL_HPP