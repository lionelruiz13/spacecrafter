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


template <typename T, typename ... Ts>
void insert_all(std::vector<T> &vec, Ts ... ts)
{
    (vec.push_back(ts), ...);
}

enum class BufferType : char { POS2D = 0 , POS3D , TEXTURE, NORMAL, COLOR, COLOR4, MAG, SCALE };
enum class BufferAccess : char { STATIC = 0, DYNAMIC, STREAM};

//unsigned int convert(BufferType& bt);


class Buffer {
public:
    Buffer(const BufferAccess& ba);
    virtual ~Buffer();
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

    void fillVertexBuffer(const BufferType& bt, unsigned int size , const float* data);
    void fillVertexBuffer(const BufferType& bt, const std::vector<float> data);

    void registerIndexBuffer(const BufferAccess& ba);

    void fillIndexBuffer(const std::vector<unsigned int> data);
    void fillIndexBuffer(unsigned int count , const unsigned int* indices);

    void bind() const;
    void unBind() const;
    unsigned int getIndiceCount() const;

private:
    unsigned int m_RendererID;
    IndexBuffer* m_indexBuffer = nullptr;
    std::map<const BufferType, std::unique_ptr<Buffer>> m_buffer;
};


#endif // OPENGL_HPP