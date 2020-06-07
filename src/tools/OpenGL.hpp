

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



enum class BufferType : char { POSITION = 0 , TEXTURE, NORMAL, COLOR, MAG, SCALE };

unsigned int convert(BufferType& bt);

class Buffer {
public:
    Buffer(){};
    ~Buffer(){};
    void update(const void* data, unsigned int size);
    virtual void bind() const;
    virtual void unBind() const;
    bool isStatic() const {return m_isStatic;}

protected:
    unsigned int m_RendererID;
    bool m_isStatic = true;
};


class VertexBuffer : public Buffer {
public:
    VertexBuffer();
    VertexBuffer(const void* data, unsigned int size);
    void update(const void* data, unsigned int size);
    ~VertexBuffer();
private:
};


class IndexBuffer : public Buffer {
public:
    IndexBuffer();
    ~IndexBuffer();
    
    void set(const unsigned int* indices, unsigned int count, bool isStatic=true);
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

    void setVertexBuffer(const BufferType& bt, unsigned int elementSize);
    void setVertexBuffer(const BufferType& bt, unsigned int elementSize,const void* data, unsigned int size);
    void updateBuffer(const BufferType& bt, const void* data, unsigned int size);
    void setIndexBuffer(const unsigned int* indices, unsigned int count);
    void updateIndexBuffer(const unsigned int* indices, unsigned int count);

    void bind() const;
    void unBind() const;

    IndexBuffer getIndexBuffer() const {
        return m_index;
    }

private:
    unsigned int m_RendererID;
    IndexBuffer m_index;
    std::map<const BufferType, Buffer*> m_buffer;
};


#endif // OPENGL_HPP