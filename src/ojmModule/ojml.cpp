
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include "ojmModule/ojml.hpp"
#include "tools/OpenGL.hpp"

// *****************************************************************************
// 
// CLASSE OJML: un dérivé de OJM mais avec un seul shape
//
// *****************************************************************************

OjmL::OjmL(const std::string & _fileName)
{
	is_ok = false;
	is_ok = init(_fileName);
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

bool OjmL::init(const std::string & _fileName)
{
	is_ok = readOJML(_fileName);
	if (is_ok) {
		initGLparam();
	}
	return is_ok;
}

void OjmL::draw(GLenum mode)
{
	if (is_ok) {
		// glBindVertexArray(dGL->vao);
        dGL->bind();
		GLCall( glDrawElements(mode, dGL->getIndiceCount(), GL_UNSIGNED_INT, (void*)0 ) );
        dGL->unBind();
	}
}

void OjmL::initGLparam()
{
    dGL = std::make_unique<VertexArray>();
    dGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    dGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
    dGL->registerVertexBuffer(BufferType::NORMAL, BufferAccess::STATIC);
    dGL->registerIndexBuffer(BufferAccess::STATIC);
	// glGenVertexArrays(1,&dGL->vao);
	// glBindVertexArray(dGL->vao);

	// glGenBuffers(1,&dGL->pos);
	// glGenBuffers(1,&dGL->tex);
	// glGenBuffers(1,&dGL->norm);
	// glGenBuffers(1,&dGL->elementBuffer);

    dGL->fillVertexBuffer(BufferType::POS3D,vertices);
    dGL->fillVertexBuffer(BufferType::TEXTURE,uvs);
    dGL->fillVertexBuffer(BufferType::NORMAL,normals);
    dGL->fillIndexBuffer(indices);
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
