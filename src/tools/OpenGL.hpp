/*
 * Copyright (C) 2020 of the Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */


#ifndef OPENGL_HPP
#define OPENGL_HPP

#include <map>
#include <vector>
#include <memory>
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

/**
* \file OpenGL.hpp
* \brief Define and use any GL buffer 
* \author Olivier NIVOIX
* \version 1
*/

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


/**
* \class Buffer
*
* \brief Commun class that manage un single GL Buffer
*
* This object should be wrapped in a VertexArray and not used alone.
*/
class Buffer {
public:
    //! constructor. Need to know his access type
    Buffer(const BufferAccess& ba);
    virtual ~Buffer();
    //! bind this buffer
    virtual void bind() const;
    //! unbind this buffer
    virtual void unBind() const;
protected:
    // its unique GL identifier
    unsigned int m_RendererID;
    // its type of access.
    GLenum m_bufferAcces;
};

/**
* \class VertexBuffer
*
* \brief Commun class that manage un single GL Vertex Buffer
*
* This object should be wrapped in a VertexArray and not used alone.
*/
class VertexBuffer : public Buffer {
public:
    //! constructor. Need to know his access type
    VertexBuffer(const BufferAccess& ba);
    ~VertexBuffer();
    //! modify the buffer by integrating size elements from data
    void fill(unsigned int size , const void* data);
private:
};

/**
* \class IndexBuffer
*
* \brief Commun class that manage un single GL Index Buffer
*
* This object should be wrapped in a VertexArray and not used alone.
*/
class IndexBuffer : public Buffer {
public:
    //! constructor. Need to know his access type
    IndexBuffer(const BufferAccess& ba);
    ~IndexBuffer();
    //! modify the buffer by integrating count elements from integer indices
    void fill(unsigned int count , const unsigned int* indices);
    //! return the number of element stored in this IndexBuffer
    inline unsigned int GetCount() const { return m_Count; }
    //! bind this buffer
    void bind() const override;
    //! unbind this buffer
    void unBind() const override;
private:
    // its unique GL identifier
    unsigned int m_Count;
};

/** 
* \class VertexArray
*
* \brief Represents a vao in software
*
* offers all the basic functionality to use a vao
* 
* @section Working
* 
* - Define a VertexArray before use it :)
* - Register --before use it-- all buffers needed by the VAO identified by 
* his function (represented by BufferType) 
* his access type (represented by BufferAccess)
* - Fill buffer as you need
* - Bind/unbind to use.
*/
class VertexArray
{
public:
    //! constructor...
    VertexArray();
    ~VertexArray();
    //! register a new type of vertex buffer giving its function and access type
    void registerVertexBuffer(const BufferType& bt, const BufferAccess& ba);
    //! modify the VertexBuffer identified by  his BufferType and integrating size elements from data
    void fillVertexBuffer(const BufferType& bt, unsigned int size , const float* data);
    //! modify the VertexBuffer identified by  his BufferType and integrating all elements fron vector data 
    void fillVertexBuffer(const BufferType& bt, const std::vector<float> data);
    //! register a index buffer giving its access type
    void registerIndexBuffer(const BufferAccess& ba);
    //! modify the IndexBuffer by integrating count elements from integer indices
    void fillIndexBuffer(const std::vector<unsigned int> data);
    //! modify the IndexBuffer by integrating count elements from integer indices
    void fillIndexBuffer(unsigned int count , const unsigned int* indices);
    //! bind this vao
    void bind() const;
    //! unbind this vao
    void unBind() const;
    //! returns the number of elements contained in the index buffer
    unsigned int getIndiceCount() const;
private:
    // its unique GL identifier
    unsigned int m_RendererID;
    //! his IndexBuffer if needed
    std::unique_ptr<IndexBuffer> m_indexBuffer;
    //! map that store all VertexBuffer: BufferType is the key
    std::map<const BufferType, std::unique_ptr<Buffer>> m_buffer;
};


#endif // OPENGL_HPP