
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include "ojmModule/ojml.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/Renderer.hpp"
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"

// *****************************************************************************
//
// CLASSE OJML: un dérivé de OJM mais avec un seul shape
//
// *****************************************************************************

OjmL::OjmL(const std::string & _fileName, ThreadContext *context, bool mergeVertexArray, int *maxVertex, int *maxIndex)
{
	is_ok = false;
	is_ok = init(_fileName, context, mergeVertexArray, maxVertex, maxIndex);
}

OjmL::~OjmL()
{
	vertices.clear();
	uvs.clear();
	normals.clear();
	indices.clear();

	// glDeleteBuffers(1,&dGL->pos);
	// glDeleteBuffers(1,&dGL->tex);
	// glDeleteBuffers(1,&dGL->norm);
	// glDeleteBuffers(1,&dGL->elementBuffer);
    // glDeleteVertexArrays(1,&dGL->vao);
}

bool OjmL::init(const std::string & _fileName, ThreadContext *context, bool mergeVertexArray, int *maxVertex, int *maxIndex)
{
	is_ok = readOJML(_fileName);
	if (is_ok) {
		initGLparam(context, mergeVertexArray, maxVertex, maxIndex);
	}
	return is_ok;
}

void OjmL::bind(CommandMgr *cmdMgr)
{
	if (is_ok)
		cmdMgr->bindVertex(dGL.get());
}

void OjmL::bind(Pipeline *pipeline)
{
	if (is_ok)
		pipeline->bindVertex(dGL.get());
}

void OjmL::draw(void *pDrawData)
{
	if (is_ok) {
		// glBindVertexArray(dGL->vao);
        // dGL->bind();
		// GLCall( glDrawElements(mode, dGL->getIndiceCount(), GL_UNSIGNED_INT, (void*)0 ) );
        // dGL->unBind();
        //Renderer::drawElementsWithoutShader(dGL.get(), mode);
		*static_cast<typeof(&drawData)>(pDrawData) = drawData;
	}
}

void OjmL::initGLparam(ThreadContext *context, bool mergeVertexArray, int *maxVertex, int *maxIndex)
{
    dGL = std::make_unique<VertexArray>(context->surface, context->commandMgr);
    dGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    dGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
    dGL->registerVertexBuffer(BufferType::NORMAL, BufferAccess::STATIC);
	// glGenVertexArrays(1,&dGL->vao);
	// glBindVertexArray(dGL->vao);

	// glGenBuffers(1,&dGL->pos);
	// glGenBuffers(1,&dGL->tex);
	// glGenBuffers(1,&dGL->norm);
	// glGenBuffers(1,&dGL->elementBuffer);
	drawData.indexCount = indices.size();
	drawData.instanceCount = 1;
	drawData.vertexOffset = 0;
	drawData.firstInstance = 0;
	if (mergeVertexArray) {
		*maxVertex += vertices.size() / 3;
		*maxIndex += indices.size();
	} else {
		drawData.firstIndex = 0;
		dGL->registerIndexBuffer(BufferAccess::STATIC, indices.size());
		dGL->build(vertices.size() / 3);

	    dGL->fillVertexBuffer(BufferType::POS3D,vertices);
	    dGL->fillVertexBuffer(BufferType::TEXTURE,uvs);
	    dGL->fillVertexBuffer(BufferType::NORMAL,normals);
	    dGL->fillIndexBuffer(indices);
	}
	// glBindBuffer(GL_ARRAY_BUFFER,dGL->pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3*vertices.size(), vertices.data(),GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER,dGL->tex);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*2*uvs.size(), uvs.data(),GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER,dGL->norm);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3*normals.size(), normals.data(),GL_STATIC_DRAW);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,dGL->elementBuffer);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(unsigned int)*indices.size(), indices.data(),GL_STATIC_DRAW);

	// glBindBuffer(GL_ARRAY_BUFFER, dGL->pos);
	// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	// glBindBuffer(GL_ARRAY_BUFFER, dGL->tex);
	// glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);
	// glBindBuffer(GL_ARRAY_BUFFER, dGL->norm);
	// glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dGL->elementBuffer);
	// glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,0,NULL);

	// glEnableVertexAttribArray(0);
	// glEnableVertexAttribArray(1);
	// glEnableVertexAttribArray(2);
	// glEnableVertexAttribArray(3);
}

void OjmL::initFrom(VertexArray *vertex)
{
	if (is_ok) {
		// assign one part of the vertex
		dGL->assign(vertex, vertices.size() / 3, indices.size());
		dGL->fillVertexBuffer(BufferType::POS3D,vertices);
		dGL->fillVertexBuffer(BufferType::TEXTURE,uvs);
		dGL->fillVertexBuffer(BufferType::NORMAL,normals);
		dGL->fillIndexBuffer(indices);
		drawData.firstIndex = dGL->getIndexOffset();
	}
}

bool OjmL::readOJML(const std::string & _fileName)
{
    std::ifstream stream;
    char line[265];

    stream.open(_fileName.c_str(),std::ios_base::in);

    if(stream.is_open())
    {
        while(!stream.eof()) {
            stream.getline(line,256);

            switch(line[0])
            {
                case 'v':
                    {
                        Vec3f vertex;
                        std::stringstream ss(std::string(line+2));
                        ss>>vertex.v[0];
                        ss>>vertex.v[1];
                        ss>>vertex.v[2];
                        insert_vec3(vertices, vertex);
                    }
                break;
                case 'u':
                    {
                        Vec2f uv;
                        std::stringstream ss(std::string(line+2));
                        ss>>uv.v[0];
                        ss>>uv.v[1];
                        insert_vec2(uvs, uv);
                    }
                break;

                case 'n':
                    {
                        Vec3f normal;
                        std::stringstream ss(std::string(line+2));
                        ss>>normal.v[0];
                        ss>>normal.v[1];
                        ss>>normal.v[2];
                        insert_vec3(normals, normal);
                    }
                break;

                case 'j':
                    {
                        unsigned int indice[9];
                        std::stringstream ss(std::string(line+2));
                        ss>>indice[0] >> indice[1] >> indice[2] >> indice[3] >>
                            indice[4] >> indice[5] >> indice[6] >> indice[7] >>
                            indice[8];
                        for(unsigned int k=0; k<9; k++)
							indices.push_back(indice[k]);
                    }
                    break;

                case 'i':
                    {
                        unsigned int indice1, indice2, indice3;
                        std::stringstream ss(std::string(line+2));
                        ss>>indice1 >> indice2 >> indice3;
                        indices.push_back(indice1);
                        indices.push_back(indice2);
                        indices.push_back(indice3);
                    }
                break;
            }
        };
        return true;
    }
	return false;
}
