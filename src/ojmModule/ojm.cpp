#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include "ojmModule/ojm.hpp"
#include "renderGL/OpenGL.hpp"
#include "renderGL/shader.hpp"
#include "renderGL/Renderer.hpp"

// *****************************************************************************
//
// CLASSE OJM
//
// *****************************************************************************

Ojm::Ojm( const std::string & _fileName, const std::string & _pathFile, float multiplier)
{
	fileName = _fileName;
	is_ok = false;
	pathFile = _pathFile;
	is_ok = init(multiplier);
}

Ojm::~Ojm()
{
    for(unsigned int i=0;i<shapes.size();i++){
		// glDeleteBuffers(1,&shapes[i].dGL.pos);
		// glDeleteBuffers(1,&shapes[i].dGL.tex);
		// glDeleteBuffers(1,&shapes[i].dGL.norm);
		// glDeleteBuffers(1,&shapes[i].dGL.elementBuffer);
		// glDeleteVertexArrays(1,&shapes[i].dGL.vao);

    	if (shapes[i].map_Ka!=nullptr) {
            delete shapes[i].map_Ka;
            shapes[i].map_Ka=nullptr;
        }
    	if (shapes[i].map_Kd!=nullptr) {
            delete shapes[i].map_Kd;
            shapes[i].map_Kd=nullptr;
        }
    	if (shapes[i].map_Ks!=nullptr) {
            delete shapes[i].map_Ks;
            shapes[i].map_Ks=nullptr;
        }
    }
	shapes.clear();
}

Ojm::Ojm(const std::string& _fileName)
{
	Ojm(_fileName, "", 1.0);
}

bool Ojm::init(float multiplier)
{
	is_ok = readOJM(fileName, multiplier);
	if (is_ok) {
		is_ok=testIndices();
		initGLparam();
	}
	return is_ok;
}

bool Ojm::testIndices()
{
	for(unsigned int i=0;i<shapes.size();i++){
		if ((shapes[i].vertices.size()/3) != (shapes[i].normals.size()/3)) {
			std::cout << "vertices.size != normals.size : abord"<<std::endl;
			return false;
		}
		if (shapes[i].uvs.size()==0) {
			Vec2f data{0.0,0.0};
			for(unsigned int k=0; k< shapes[i].vertices.size(); k++)
				insert_vec2(shapes[i].uvs, data);
		}
		if ((shapes[i].vertices.size()/3) != (shapes[i].uvs.size()/2)) {
			std::cout << "vertices.size != uvs.size : abord"<<std::endl;
			return false;
		}
	}
	return true;
}

void Ojm::draw(shaderProgram * shader)
{
	for(unsigned int i=0;i<shapes.size();i++){
		//~ cout << "shape " << i << " name " << shapes[i].name <<  endl;
		shader->setUniform("Material.Ka", shapes[i].Ka);
		shader->setUniform("Material.Kd", shapes[i].Kd);
		shader->setUniform("Material.Ks", shapes[i].Ks);
		shader->setUniform("Material.Ns", shapes[i].Ns);
		if (shapes[i].T < 1.0)
			shader->setUniform("T", shapes[i].T);

		if (shapes[i].map_Ka != nullptr) {
            glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shapes[i].map_Ka->getID());
			shader->setUniform("useTexture", true);
			//~ cout << "avec texture" << endl;
		} else {
			shader->setUniform("useTexture", false);
			//~ cout << "sans texture" << endl;
		}

		// glBindVertexArray(shapes[i].dGL.vao);
        // shapes[i].dGL->bind();
		// glDrawElements(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, shapes[i].indices.size(), GL_UNSIGNED_INT, (void*)0 );
        // shapes[i].dGL->unBind();
        Renderer::drawElementsWithoutShader(shapes[i].dGL.get(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	}
}

void Ojm::initGLparam()
{
	for(unsigned int i=0;i<shapes.size();i++){

        shapes[i].dGL = std::make_unique<VertexArray>();
		// glGenVertexArrays(1,&shapes[i].dGL.vao);
		// glBindVertexArray(shapes[i].dGL.vao);
        shapes[i].dGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
        shapes[i].dGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
        shapes[i].dGL->registerVertexBuffer(BufferType::NORMAL, BufferAccess::STATIC);
        shapes[i].dGL->registerIndexBuffer(BufferAccess::STATIC);

        // glGenBuffers(1,&shapes[i].dGL.pos);
		// glGenBuffers(1,&shapes[i].dGL.tex);
		// glGenBuffers(1,&shapes[i].dGL.norm);
		// glGenBuffers(1,&shapes[i].dGL.elementBuffer);

		// glBindBuffer(GL_ARRAY_BUFFER,shapes[i].dGL.pos);
		// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3*shapes[i].vertices.size(), shapes[i].vertices.data(),GL_STATIC_DRAW);
		// glBindBuffer(GL_ARRAY_BUFFER,shapes[i].dGL.tex);
		// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*2*shapes[i].uvs.size(), shapes[i].uvs.data(),GL_STATIC_DRAW);
		// glBindBuffer(GL_ARRAY_BUFFER,shapes[i].dGL.norm);
		// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*3*shapes[i].normals.size(), shapes[i].normals.data(),GL_STATIC_DRAW);
		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,shapes[i].dGL.elementBuffer);
		// glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(unsigned int)*shapes[i].indices.size(), shapes[i].indices.data(),GL_STATIC_DRAW);

        shapes[i].dGL->fillVertexBuffer(BufferType::POS3D,   shapes[i].vertices);
        shapes[i].dGL->fillVertexBuffer(BufferType::TEXTURE, shapes[i].uvs);
        shapes[i].dGL->fillVertexBuffer(BufferType::NORMAL,  shapes[i].normals);
        shapes[i].dGL->fillIndexBuffer(shapes[i].indices);

		// glBindBuffer(GL_ARRAY_BUFFER, shapes[i].dGL.pos);
		// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
		// glBindBuffer(GL_ARRAY_BUFFER, shapes[i].dGL.tex);
		// glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);
		// glBindBuffer(GL_ARRAY_BUFFER, shapes[i].dGL.norm);
		// glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,NULL);

		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes[i].dGL.elementBuffer);
		// glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,0,NULL);

		// glEnableVertexAttribArray(0);
		// glEnableVertexAttribArray(1);
		// glEnableVertexAttribArray(2);
		// glEnableVertexAttribArray(3);
	}
}

bool Ojm::readOJM(const std::string& filename, float multiplier)
{
   // std::cout<<"OJM: Reading file "<<filename << std::endl;
    std::ifstream stream;
    char line[265];

    stream.open(filename.c_str(),std::ios_base::in);

    if(stream.is_open())
    {
        int shapeIter=-1;

        do{
            stream.getline(line,256);

            switch(line[0])
            {
                case 'o':
                    // //////////////////////////////////////////////////////////////////////////////////////////////
                    shapeIter++;
                    shapes.push_back({});
                    shapes[shapeIter].name = line+2;
                    shapes[shapeIter].name = shapes[shapeIter].name.substr(0,shapes[shapeIter].name.find('\n'));
                    // //////////////////////////////////////////////////////////////////////////////////////////////
                break;

                case 'v':
                    {
                        Vec3f vertex;
                        std::stringstream ss(std::string(line+2));
                        ss>>vertex.v[0];
                        ss>>vertex.v[1];
                        ss>>vertex.v[2];
                        insert_vec3(shapes[shapeIter].vertices, vertex * multiplier);
                    }
                break;

                case 'u':
                    {
                        Vec2f uv;
                        std::stringstream ss(std::string(line+2));
                        ss>>uv.v[0];
                        ss>>uv.v[1];
                        insert_vec2(shapes[shapeIter].uvs, uv);
                    }
                break;

                case 'n':
                    {
                        Vec3f normal;
                        std::stringstream ss(std::string(line+2));
                        ss>>normal.v[0];
                        ss>>normal.v[1];
                        ss>>normal.v[2];
                        insert_vec3(shapes[shapeIter].normals, normal);
                    }
                break;

                case 'T':
                    {
                        float tmp;
                        std::stringstream ss(std::string(line+2));
						ss >> tmp;
						std::cout << "T " << tmp << std::endl;
                        shapes[shapeIter].T = tmp;
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
							shapes[shapeIter].indices.push_back(indice[k]);
                    }
                    break;

                case 'i':
                    {
                        unsigned int indice1, indice2, indice3;
                        std::stringstream ss(std::string(line+2));
                        ss>>indice1 >> indice2 >> indice3;
                        shapes[shapeIter].indices.push_back(indice1);
                        shapes[shapeIter].indices.push_back(indice2);
                        shapes[shapeIter].indices.push_back(indice3);
                    }
                break;

                case 'k':
                    switch(line[1])
                    {
                        case 'a':
                            {
                                std::stringstream ss(std::string(line+3));
                                ss>>shapes[shapeIter].Ka.v[0];
                                ss>>shapes[shapeIter].Ka.v[1];
                                ss>>shapes[shapeIter].Ka.v[2];
                            }
                        break;
                        case 'd':
                            {
                                std::stringstream ss(std::string(line+3));
                                ss>>shapes[shapeIter].Kd.v[0];
                                ss>>shapes[shapeIter].Kd.v[1];
                                ss>>shapes[shapeIter].Kd.v[2];
                            }
                        break;
                        case 's':
                            {
                                std::stringstream ss(std::string(line+3));
                                ss>>shapes[shapeIter].Ks.v[0];
                                ss>>shapes[shapeIter].Ks.v[1];
                                ss>>shapes[shapeIter].Ks.v[2];
                            }
                        break;
                    }
                break;

                case 'N':
					if (line[1]=='s') {
						std::stringstream ss(std::string(line+3));
						ss>> shapes[shapeIter].Ns;
					}
					break;

                case 'm':
                    if(line[1]=='a'&& line[2]=='p' && line[3]=='_' && line[4]=='k')
                    switch(line[5])
                    {
                        case 'a':
                            {
                                std::string map_ka_filename(line+7);
                                //~ cout << "line |" << line << "|" << map_ka_filename << "|" << endl;
                                //~ map_ka_filename = map_ka_filename.substr(0,map_ka_filename.find('\n'));
                                //~ cout << "map_ka : |"<<map_ka_filename << "|" << endl;
                                if ( ! map_ka_filename.empty() && map_ka_filename!="0")
									shapes[shapeIter].map_Ka = new s_texture(pathFile+map_ka_filename, true);
                            }
                        break;
                        case 'd':
                            {
                                std::string map_kd_filename(line+7);
                                //~ map_kd_filename = map_kd_filename.substr(0,map_kd_filename.find('\n'));
                                //~ cout << "map_kd : |"<<map_kd_filename << "|" << endl;
                                if ( ! map_kd_filename.empty() && map_kd_filename!="0")
									shapes[shapeIter].map_Kd = new s_texture(pathFile+map_kd_filename, true);
                            }
                        break;
                        case 's':
                            {
                                std::string map_ks_filename(line+7);
                                //~ map_ks_filename = map_ks_filename.substr(0,map_ks_filename.find('\n'));
                                //~ cout << "map_ks : |"<<map_ks_filename << "|" << endl;
                                if ( ! map_ks_filename.empty() && map_ks_filename!="0")
									shapes[shapeIter].map_Ks = new s_texture(pathFile+map_ks_filename, true);
                            }
                        break;

						default: break;
                    }
                break;

                default: break;
            }
        }while(!stream.eof());
        // std::cout<<"OJM: reached end of file"<<std::endl;
        // print();
        return true;
    }
    std::cout<<"OJM: error reading file"<<std::endl;
	return false;
}


void Ojm::print()
{
	std::cout<< "Nombre de shape " << shapes.size() << std::endl;
	for(unsigned int i=0; i< shapes.size();i++) {
		std::cout<< "***** shapes n°"<< i<< std::endl;
		std::cout<< "Nombre de vectrices " << shapes[i].vertices.size() << std::endl;
		std::cout<< "Nombre d'uvs        " << shapes[i].uvs.size() << std::endl;
		std::cout<< "Nombre de normales  " << shapes[i].normals.size() << std::endl;
		std::cout<< "Nombre d'indices  " << shapes[i].indices.size() << std::endl;
		// std::cout << "v ";
		// for(unsigned int j=0; j< shapes[i].vertices.size() ; j++)
		// 	std::cout << " " << shapes[i].vertices[j];
		// std::cout << std::endl;
		// std::cout << "uv ";
		// for(unsigned int j=0; j< shapes[i].uvs.size() ; j++)
		// 	std::cout << " " << shapes[i].uvs[j];
		// std::cout << std::endl;
		// std::cout << "n ";
		// for(unsigned int j=0; j< shapes[i].normals.size() ; j++)
		// 	std::cout << " " << shapes[i].normals[j];
		// std::cout << std::endl;
		// std::cout << "i ";
		// for(unsigned int j=0; j< shapes[i].indices.size() ; j++)
		// 	std::cout << " " << shapes[i].indices[j];
		// std::cout << std::endl;
	}
}
