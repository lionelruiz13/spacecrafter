/*
#include <iostream>
#include "renderGL/OpenGL.hpp"

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLCheckError()
{
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] ";
          switch(error) {
              case GL_INVALID_ENUM :
                  std::cout << "GL_INVALID_ENUM : An unacceptable value is specified for an enumerated argument.";
                  break;
              case GL_INVALID_VALUE :
                  std::cout << "GL_INVALID_OPERATION : A numeric argument is out of range.";
                  break;
              case GL_INVALID_OPERATION :
                  std::cout << "GL_INVALID_OPERATION : The specified operation is not allowed in the current state.";
                  break;
              case GL_INVALID_FRAMEBUFFER_OPERATION :
                  std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION : The framebuffer object is not complete.";
                  break;
              case GL_OUT_OF_MEMORY :
                  std::cout << "GL_OUT_OF_MEMORY : There is not enough memory left to execute the command.";
                  break;
              case GL_STACK_UNDERFLOW :
                  std::cout << "GL_STACK_UNDERFLOW : An attempt has been made to perform an operation that would cause an internal stack to underflow.";
                  break;
              case GL_STACK_OVERFLOW :
                  std::cout << "GL_STACK_OVERFLOW : An attempt has been made to perform an operation that would cause an internal stack to overflow.";
                  break;
              default :
                  std::cout << "Unrecognized error" << error;
          }
          std::cout << std::endl;
          return false;
    }
    return true;
}

static unsigned int getLayout(const BufferType& bt)
{
    switch (bt) {
        case BufferType::POS2D    : return 0; break;
        case BufferType::POS3D    : return 0; break;
        case BufferType::TEXTURE  : return 1; break;
        case BufferType::NORMAL   : return 2; break;
        case BufferType::COLOR    : return 3; break;
        case BufferType::COLOR4   : return 3; break;
        case BufferType::MAG      : return 4; break;
        case BufferType::SCALE    : return 5; break;
        default: assert(false); return 0;
    }
}

static unsigned int getDataSize(const BufferType& bt)
{
    switch (bt) {
        case BufferType::POS2D    : return 2; break;
        case BufferType::POS3D    : return 3; break;
        case BufferType::TEXTURE  : return 2; break;
        case BufferType::NORMAL   : return 3; break;
        case BufferType::COLOR    : return 3; break;
        case BufferType::COLOR4   : return 4; break;
        case BufferType::MAG      : return 1; break;
        case BufferType::SCALE    : return 1; break;
        default: assert(false); return 0;
    }
}


static GLenum getBufferAccess(const BufferAccess& ba)
{
    switch (ba){
        case BufferAccess::STREAM : return GL_STREAM_DRAW; break;
        case BufferAccess::STATIC : return GL_STATIC_DRAW; break;
        case BufferAccess::DYNAMIC: return GL_DYNAMIC_DRAW; break;
    default: assert(false); return GL_STATIC_DRAW;
    }
}

//----------------------------------------------------------------------------
// Buffer
//----------------------------------------------------------------------------

Buffer::Buffer(const BufferAccess& ba)
{
    GLCall( glGenBuffers(1, &m_RendererID) );
    m_bufferAcces = getBufferAccess(ba);
}

Buffer::~Buffer()
{
    GLCall( glDeleteBuffers(1, &m_RendererID) );
}

void Buffer::bind() const
{
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
}

void Buffer::unBind() const
{
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
}

//----------------------------------------------------------------------------
// VertexBuffer
//----------------------------------------------------------------------------

VertexBuffer::VertexBuffer(const BufferAccess& ba):Buffer(ba)
{}

VertexBuffer::~VertexBuffer()
{}


void VertexBuffer::fill(unsigned int size, const void* data)
{
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
    GLCall( glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * size, data, this->m_bufferAcces) );
}



//----------------------------------------------------------------------------
// IndexBuffer
//----------------------------------------------------------------------------

IndexBuffer::IndexBuffer(const BufferAccess& ba) : Buffer(ba)
{
    ASSERT(sizeof(unsigned int) == sizeof(GLuint));
    m_Count = 0;
}


void IndexBuffer::fill(unsigned int count, const unsigned int* indices)
{
    ASSERT(sizeof(unsigned int) == sizeof(GLuint));
    m_Count= count;
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID) );
    GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, this->m_bufferAcces) );
}


IndexBuffer::~IndexBuffer()
{}

void IndexBuffer::bind() const
{
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID) );
}

void IndexBuffer::unBind() const
{
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
}


//----------------------------------------------------------------------------
// VertexArray
//----------------------------------------------------------------------------

VertexArray::VertexArray()
{
    GLCall( glGenVertexArrays(1, &m_RendererID) );
}

VertexArray::~VertexArray()
{
    m_buffer.clear();
    GLCall( glDeleteVertexArrays(1, &m_RendererID) );
}

void VertexArray::bind() const
{
    GLCall( glBindVertexArray(m_RendererID) );
}

void VertexArray::unBind() const
{
    GLCall( glBindVertexArray(0) );
}


void VertexArray::registerVertexBuffer(const BufferType& bt, const BufferAccess& ba)
{
    auto it= m_buffer.find(bt);
    if( it != m_buffer.end() ){
        std::cout << "Buffer already exist" << std::endl;
        assert(false);
    }
    this->bind();
    m_buffer[bt] = std::make_unique<VertexBuffer>(ba);
    this->unBind();
}

void VertexArray::fillVertexBuffer(const BufferType& bt, const std::vector<float> data)
{
    this->fillVertexBuffer(bt,data.size(), data.data());
}


void VertexArray::fillVertexBuffer(const BufferType& bt, unsigned int size , const float* data)
{
    auto it= m_buffer.find(bt);
    if( it == m_buffer.end() ){
        std::cout << "Buffer does not exist" << std::endl;
        assert(false);
    }
    this->bind();
    VertexBuffer* vb= (VertexBuffer *) m_buffer[bt].get();
    GLCall( glEnableVertexAttribArray( getLayout(bt)) );
    vb->fill(size, data);
    GLCall( glVertexAttribPointer(getLayout(bt), getDataSize(bt), GL_FLOAT,GL_FALSE,0,NULL) );
    this->unBind();
}


void VertexArray::registerIndexBuffer(const BufferAccess& ba)
{
    ASSERT(!m_indexBuffer);
    this->bind();
    m_indexBuffer = std::make_unique<IndexBuffer>(ba);
    this->unBind();
}

void VertexArray::fillIndexBuffer(const std::vector<unsigned int> data)
{
    this->fillIndexBuffer(data.size(), data.data());
}

void VertexArray::fillIndexBuffer(unsigned int count , const unsigned int* indices)
{
    ASSERT(m_indexBuffer);

    this->bind();
    GLCall( glEnableVertexAttribArray(3) );
    m_indexBuffer->fill(count, indices);
    this->unBind();
}

unsigned int VertexArray::getIndiceCount() const
{
    ASSERT(m_indexBuffer);
    return m_indexBuffer->GetCount();
}
//*/
