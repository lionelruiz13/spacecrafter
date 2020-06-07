#include <iostream>

#include "tools/OpenGL.hpp"

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
        case BufferType::MAG      : return 1; break;
        case BufferType::SCALE    : return 1; break;
        default: assert(false); return 0;
    }
}

//----------------------------------------------------------------------------
// Buffer
//----------------------------------------------------------------------------

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


VertexBuffer::VertexBuffer()
{
    GLCall( glGenBuffers(1, &m_RendererID) );
    //GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
    //GLCall( glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW) );
    m_isStatic = false;
}

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
    GLCall( glGenBuffers(1, &m_RendererID) );
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
    GLCall( glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW) );
}


void VertexBuffer::update(const void* data, unsigned int size)
{
    assert(m_isStatic==false);
    this->bind();
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
    GLCall( glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW) );
    this->unBind();
}


VertexBuffer::~VertexBuffer()
{
    GLCall( glDeleteBuffers(1, &m_RendererID) );
}

//----------------------------------------------------------------------------
// IndexBuffer
//----------------------------------------------------------------------------

IndexBuffer::IndexBuffer()
{
    ASSERT(sizeof(unsigned int) == sizeof(GLuint));
    m_Count = 0;
    GLCall( glGenBuffers(1, &m_RendererID) );
}


void IndexBuffer::set(const unsigned int* indices, unsigned int count, bool isStatic)
{
    this->bind();
    ASSERT(sizeof(unsigned int) == sizeof(GLuint));
    m_Count= count;
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID) );
    if (isStatic)
        GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW) );
    else
        GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW) );
    this->unBind();
}


IndexBuffer::~IndexBuffer()
{
    GLCall( glDeleteBuffers(1, &m_RendererID) );
}

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


void VertexArray::setVertexBuffer(const BufferType& bt)
{
    auto it= m_buffer.find(bt);
    if( it != m_buffer.end() ){
        std::cout << "Buffer already exist." << std::endl;
        assert(false);
    }
    this->bind();
    VertexBuffer* vb = new VertexBuffer();
    GLCall( glEnableVertexAttribArray( getLayout(bt)) );
    GLCall( glVertexAttribPointer(getLayout(bt), getDataSize(bt), GL_FLOAT,GL_FALSE,0,NULL) );
    m_buffer[bt] = vb;
    //this->unBind();
}


void VertexArray::setVertexBuffer(const BufferType& bt, const void* data, unsigned int size)
{
    auto it= m_buffer.find(bt);
    if( it != m_buffer.end() ){
        std::cout << "Buffer already exist." << std::endl;
        assert(false);
    }
    this->bind();
    VertexBuffer* vb = new VertexBuffer(data,size);
    GLCall( glEnableVertexAttribArray( getLayout(bt)) );
    std::cout << "Attrib " << getLayout(bt) << std::endl;
    GLCall( glBufferData(GL_ARRAY_BUFFER,sizeof(float)*size, data, GL_STATIC_DRAW) );
    GLCall( glVertexAttribPointer(getLayout(bt), getDataSize(bt), GL_FLOAT,GL_FALSE,0,NULL) );

    m_buffer[bt] = vb;
    this->unBind();
}


void VertexArray::updateBuffer(const BufferType& bt, const void* data, unsigned int size)
{
    auto it= m_buffer.find(bt);
    if( it == m_buffer.end() ){
        std::cout << "Buffer doen't exist." << std::endl;
        assert(false);
    }
    this->bind();
    VertexBuffer* vb= (VertexBuffer *)m_buffer[bt];
    vb->bind();
    assert(vb->isStatic()==false);
    GLCall( glBufferData(GL_ARRAY_BUFFER,sizeof(float)*size, data, GL_DYNAMIC_DRAW) );
    GLCall( glVertexAttribPointer(getLayout(bt), getDataSize(bt), GL_FLOAT,GL_FALSE,0,NULL) );
    this->unBind();
}

