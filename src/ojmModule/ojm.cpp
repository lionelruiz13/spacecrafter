#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <array>

#include "ojmModule/ojm.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"

// *****************************************************************************
//
// CLASSE OJM
//
// *****************************************************************************

Ojm::Ojm(const std::string & _fileName, const std::string & _pathFile, float multiplier)
{
	fileName = _fileName;
	is_ok = false;
	pathFile = _pathFile;
	is_ok = init(multiplier);
}

Ojm::~Ojm()
{
    for(unsigned int i=0;i<shapes.size();i++) {
		if (shapes[i].index.buffer)
			Context::instance->indexBufferMgr->releaseBuffer(shapes[i].index);
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

int Ojm::record(VkCommandBuffer &cmd, Pipeline *pipelines, PipelineLayout *layout, Set *set, int selectedPipeline, bool firstRecorded)
{
	float tmp[11] {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	Texture *boundTex = nullptr;

	if (firstRecorded && selectedPipeline != -1) {
		VertexArray::bindGlobal(cmd, shapes[0].vertex->get());
		vkCmdBindIndexBuffer(cmd, shapes[0].index.buffer, 0, VK_INDEX_TYPE_UINT32);
		firstRecorded = false;
	}
	for(unsigned int i=0;i<shapes.size();i++) {
		if (shapes[i].map_Ka != nullptr) { // There is a texture
			if (selectedPipeline != 0) {
				pipelines[0].bind(cmd);
				selectedPipeline = 0;
			}
			if (&shapes[i].map_Ka->getTexture() != boundTex) {
				set->clear();
				boundTex = &shapes[i].map_Ka->getTexture();
				set->bindTexture(*boundTex, 0);
				set->push(cmd, *layout, 1);
			}
		} else { // There is no texture
			if (selectedPipeline != 1) {
				pipelines[1].bind(cmd);
				selectedPipeline = 1;
			}
		}
		if (firstRecorded) {
			VertexArray::bindGlobal(cmd, shapes[i].vertex->get());
			vkCmdBindIndexBuffer(cmd, shapes[i].index.buffer, 0, VK_INDEX_TYPE_UINT32);
			firstRecorded = false;
		}
		if (*reinterpret_cast<Vec3f *>(tmp) != shapes[i].Ka ||
			*reinterpret_cast<Vec3f *>(tmp + 4) != shapes[i].Kd ||
			*reinterpret_cast<Vec3f *>(tmp + 8) != shapes[i].Ks) {
			// Put data according to offset given in shader
			*reinterpret_cast<Vec3f *>(tmp) = shapes[i].Ka;
			tmp[3] = shapes[i].Ns;
			*reinterpret_cast<Vec3f *>(tmp + 4) = shapes[i].Kd;
			tmp[7] = shapes[i].T;
			*reinterpret_cast<Vec3f *>(tmp + 8) = shapes[i].Ks;
			layout->pushConstant(cmd, 0, tmp, 0, 44);
		}
		vkCmdDrawIndexed(cmd, shapes[i].index.size / sizeof(int), 1, shapes[i].index.offset / sizeof(int), shapes[0].vertex->getOffset(), 0);
	}
	return selectedPipeline;
}

void Ojm::initGLparam()
{
	Context &context = *Context::instance;

	for(unsigned int i=0;i<shapes.size();i++){
		const int vertexCount = shapes[i].vertices.size() / 3;
        shapes[i].vertex = context.ojmVertexArray->createBuffer(0, vertexCount, context.ojmBufferMgr.get());
		shapes[i].index = context.indexBufferMgr->acquireBuffer(shapes[i].indices.size() * sizeof(int));
		memcpy(context.transfer->planCopy(shapes[i].index), shapes[i].indices.data(), shapes[i].indices.size() * sizeof(int));
		float *data = (float *) context.transfer->planCopy(shapes[i].vertex->get());
		shapes[i].vertex->fillEntry(3, vertexCount, shapes[i].vertices.data(), data);
		shapes[i].vertex->fillEntry(2, vertexCount, shapes[i].uvs.data(), data + 3);
		shapes[i].vertex->fillEntry(3, vertexCount, shapes[i].normals.data(), data + 5);
		// Release unused ressources
		shapes[i].vertices.clear();
		shapes[i].vertices.shrink_to_fit();
		shapes[i].uvs.clear();
		shapes[i].uvs.shrink_to_fit();
		shapes[i].normals.clear();
		shapes[i].normals.shrink_to_fit();
		shapes[i].indices.clear();
		shapes[i].indices.shrink_to_fit();
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
									shapes[shapeIter].map_Ka = std::make_unique<s_texture>(pathFile+map_ka_filename, /*true*/ TEX_LOAD_TYPE_PNG_SOLID);
                            }
                        break;
                        case 'd':
                            {
                                std::string map_kd_filename(line+7);
                                //~ map_kd_filename = map_kd_filename.substr(0,map_kd_filename.find('\n'));
                                //~ cout << "map_kd : |"<<map_kd_filename << "|" << endl;
                                if ( ! map_kd_filename.empty() && map_kd_filename!="0")
									shapes[shapeIter].map_Kd = std::make_unique<s_texture>(pathFile+map_kd_filename, /*true*/ TEX_LOAD_TYPE_PNG_SOLID);
                            }
                        break;
                        case 's':
                            {
                                std::string map_ks_filename(line+7);
                                //~ map_ks_filename = map_ks_filename.substr(0,map_ks_filename.find('\n'));
                                //~ cout << "map_ks : |"<<map_ks_filename << "|" << endl;
                                if ( ! map_ks_filename.empty() && map_ks_filename!="0")
									shapes[shapeIter].map_Ks = std::make_unique<s_texture>(pathFile+map_ks_filename, /*true*/ TEX_LOAD_TYPE_PNG_SOLID);
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
