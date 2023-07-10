#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <array>
#include <filesystem>
#include "ojmModule/ojm.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "tools/log.hpp"

// *****************************************************************************
//
// CLASSE OJM
//
// *****************************************************************************

std::map<std::string, std::weak_ptr<Ojm>> Ojm::recycler;

std::shared_ptr<Ojm> Ojm::load(const std::string &_fileName, const std::string &_pathFile)
{
	auto &cache = recycler[_fileName];
	std::shared_ptr<Ojm> ret = cache.lock();
	if (!ret)
		cache = ret = std::make_shared<Ojm>(_fileName, _pathFile);
	return ret;
}

Ojm::Ojm(const std::string & _fileName, const std::string & _pathFile)
{
	fileName = _fileName;
	pathFile = _pathFile;
	is_ok = init();
}

Ojm::~Ojm()
{
    for (auto &s : cshapes) {
		if (s.index.buffer)
			Context::instance->indexBufferMgr->releaseBuffer(s.index);
    }
	cshapes.clear();
}

bool Ojm::init()
{
	if (readCache())
		return true;
	cLog::get()->write("Generating cache for ojm '" + fileName + "'");
	if (readOJM(fileName)) {
		if (testIndices()) {
			compileCache();
			return readCache();
		} else {
			cLog::get()->write("Mismatching data sizes in ojm " + fileName, LOG_TYPE::L_ERROR);
			for (unsigned int i = 0; i < shapes.size(); ++i) {
				std::ostringstream oss;
				oss << "Shape " << i << " | Vertices : " << shapes[i].vertices.size()/3 << " | Uvs : " << shapes[i].uvs.size()/2 << " | Normals : " << shapes[i].normals.size()/3 << " | Indices : " << shapes[i].indices.size();
				cLog::get()->write(oss, LOG_TYPE::L_DEBUG);
			}
		}
	}
	return false;
}

bool Ojm::testIndices()
{
	for(unsigned int i=0;i<shapes.size();i++){
		if ((shapes[i].vertices.size()/3) != (shapes[i].normals.size()/3)) {
			std::cout << "vertices.size != normals.size : abord"<<std::endl;
			return false;
		}
		if (shapes[i].uvs.empty()) {
			Vec2f data{0.0,0.0};
			for(unsigned int k = shapes[i].vertices.size()/3; k; --k)
				insert_vec2(shapes[i].uvs, data);
		}
		if ((shapes[i].vertices.size()/3) != (shapes[i].uvs.size()/2)) {
			std::cout << "vertices.size != uvs.size : abord"<<std::endl;
			return false;
		}
	}
	return true;
}

int Ojm::record(VkCommandBuffer cmd, Pipeline *pipelines, PipelineLayout *layout, int selectedPipeline, bool firstRecorded)
{
	Texture *boundTex = nullptr;

	if (firstRecorded && selectedPipeline != -1) {
		VertexArray::bindGlobal(cmd, cshapes[0].vertex->get());
		vkCmdBindIndexBuffer(cmd, cshapes[0].index.buffer, 0, VK_INDEX_TYPE_UINT32);
		firstRecorded = false;
	}
	for (auto &s : cshapes) {
		if (s.map_Ka != nullptr) { // There is a texture
			if (selectedPipeline != 0) {
				pipelines[0].bind(cmd);
				selectedPipeline = 0;
			}
			if (&s.map_Ka->getTexture() != boundTex)
				s.map_Ka->bindTexture(cmd, layout);
		} else { // There is no texture
			if (selectedPipeline != 1) {
				pipelines[1].bind(cmd);
				selectedPipeline = 1;
			}
		}
		if (firstRecorded) {
			VertexArray::bindGlobal(cmd, s.vertex->get());
			vkCmdBindIndexBuffer(cmd, s.index.buffer, 0, VK_INDEX_TYPE_UINT32);
			firstRecorded = false;
		}
		if (s.pushAttr)
			layout->pushConstant(cmd, 0, &s.attr);
		vkCmdDrawIndexed(cmd, s.index.size / sizeof(int), 1, s.index.offset / sizeof(int), s.vertex->getOffset(), 0);
	}
	return selectedPipeline;
}

void Ojm::drawShadow(VkCommandBuffer cmd)
{
	VertexArray::bindGlobal(cmd, cshapes[0].vertex->get());
	vkCmdBindIndexBuffer(cmd, cshapes[0].index.buffer, 0, VK_INDEX_TYPE_UINT32);
	for (auto &s : cshapes)
		vkCmdDrawIndexed(cmd, s.index.size / sizeof(int), 1, s.index.offset / sizeof(int), s.vertex->getOffset(), 0);
}

bool Ojm::readCache()
{
	Context &context = *Context::instance;
	std::ifstream file(fileName + ".bin", std::ifstream::binary);

	if (file) {
		OjmHeader mainHead;
		file.read((char*) &mainHead, sizeof(mainHead));
		if (mainHead.sourceTimestamp != -1) {
			try {
				if (mainHead.sourceTimestamp != std::filesystem::last_write_time(fileName).time_since_epoch().count())
					return false; // out of date
			} catch (...) {
				cLog::get()->write("Failed to check source for '" + fileName + "', assume cache is up to date.", LOG_TYPE::L_WARNING);
			}
		}
		radius = mainHead.radius;
		if (mainHead.poorlyCentered)
			cLog::get()->write("OJM '" + fileName + "' is poorly centered, this may reduce rendering quality", LOG_TYPE::L_WARNING);
		cshapes.resize(mainHead.nbShapes);
		ShapeHeader header;
		char tmp[255];
		for (auto &s : cshapes) {
			file.read((char*) &header, sizeof(header));
			if ((s.pushAttr = header.pushAttr))
				file.read((char*) &s.attr, sizeof(s.attr));
			if (header.len_map_Ka) {
				file.read(tmp, header.len_map_Ka);
				s.map_Ka = std::make_unique<s_texture>(pathFile+std::string(tmp, header.len_map_Ka), TEX_LOAD_TYPE_PNG_SOLID);
			}
			s.vertex = context.ojmVertexArray->createBuffer(0, header.vertexCount, context.ojmBufferMgr.get());
			s.index = context.indexBufferMgr->acquireBuffer(header.indexCount * sizeof(int));
			file.read((char*) context.transfer->planCopy(s.vertex->get()), header.vertexCount * (8 * sizeof(float)));
			file.read((char*) context.transfer->planCopy(s.index), header.indexCount * sizeof(int));
		}
		return true;
	}
	return false;
}

void Ojm::compileCache()
{
	std::ofstream file(fileName + ".bin", std::ofstream::binary | std::ofstream::trunc);
	if (file) {
		OjmHeader mainHead;
		mainHead.sourceTimestamp = std::filesystem::last_write_time(fileName).time_since_epoch().count();
		mainHead.radius = radius;
		mainHead.nbShapes = shapes.size();
		mainHead.poorlyCentered = poorlyCentered;
		ShapeHeader header;
		ShapeAttributes attr{};
		header.pushAttr = true;
		file.write((const char*) &mainHead, sizeof(mainHead));
		for (auto &s : shapes) {
			header.vertexCount = s.vertices.size() / 3;
			header.indexCount = s.indices.size();
			header.len_map_Ka = s.map_Ka.size();
			if (attr.Ka != s.Ka || attr.Ns != s.Ns || attr.Kd != s.Kd || attr.T != s.T || attr.Ks != s.Ks) {
				attr.Ka = s.Ka;
				attr.Ns = s.Ns;
				attr.Kd = s.Kd;
				attr.T = s.T;
				attr.Ks = s.Ks;
				header.pushAttr = true;
			}
			file.write((const char*) &header, sizeof(header));
			if (header.pushAttr)
				file.write((const char*) &attr, sizeof(attr));
			if (header.len_map_Ka)
				file.write(s.map_Ka.data(), header.len_map_Ka);
			float *data = new float[header.vertexCount * 8];
			{
				float *dst = data;
				float *v = s.vertices.data();
				float *u = s.uvs.data();
				float *n = s.normals.data();
				auto i = header.vertexCount;
				while (i--) {
					*(dst++) = *(v++);
					*(dst++) = *(v++);
					*(dst++) = *(v++);
					*(dst++) = *(u++);
					*(dst++) = *(u++);
					*(dst++) = *(n++);
					*(dst++) = *(n++);
					*(dst++) = *(n++);
				}
			}
			file.write((const char*) data, header.vertexCount * (8 * sizeof(float)));
			delete data;
			file.write((const char*) s.indices.data(), s.indices.size() * sizeof(int));
			header.pushAttr = false;
		}
		shapes.clear();
		shapes.shrink_to_fit();
	}
}

// void Ojm::initGLparam()
// {
// 	Context &context = *Context::instance;
//
// 	for(unsigned int i=0;i<shapes.size();i++){
// 		const int vertexCount = shapes[i].vertices.size() / 3;
//         shapes[i].vertex = context.ojmVertexArray->createBuffer(0, vertexCount, context.ojmBufferMgr.get());
// 		shapes[i].index = context.indexBufferMgr->acquireBuffer(shapes[i].indices.size() * sizeof(int));
// 		memcpy(context.transfer->planCopy(shapes[i].index), shapes[i].indices.data(), shapes[i].indices.size() * sizeof(int));
// 		float *data = (float *) context.transfer->planCopy(shapes[i].vertex->get());
// 		shapes[i].vertex->fillEntry(3, vertexCount, shapes[i].vertices.data(), data);
// 		shapes[i].vertex->fillEntry(2, vertexCount, shapes[i].uvs.data(), data + 3);
// 		shapes[i].vertex->fillEntry(3, vertexCount, shapes[i].normals.data(), data + 5);
// 		// Release unused ressources
// 		shapes[i].vertices.clear();
// 		shapes[i].vertices.shrink_to_fit();
// 		shapes[i].uvs.clear();
// 		shapes[i].uvs.shrink_to_fit();
// 		shapes[i].normals.clear();
// 		shapes[i].normals.shrink_to_fit();
// 		shapes[i].indices.clear();
// 		shapes[i].indices.shrink_to_fit();
// 	}
// }

bool Ojm::readOJM(const std::string& filename)
{
   // std::cout<<"OJM: Reading file "<<filename << std::endl;
    std::ifstream stream;
    char line[265];

    stream.open(filename.c_str(),std::ios_base::in);

    if(stream.is_open())
    {
        int shapeIter=-1;
		Vec3f posBorder{};
		Vec3f negBorder{};

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
						for (int i = 0; i < 3; ++i) {
							ss >> vertex.v[i];
							if (posBorder.v[i] < vertex.v[i])
								posBorder.v[i] = vertex.v[i];
							if (negBorder.v[i] > vertex.v[i])
								negBorder.v[i] = vertex.v[i];
						}
						float tmp = vertex.lengthSquared();
						if (radius < tmp)
							radius = tmp;
                        insert_vec3(shapes[shapeIter].vertices, vertex);
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
									shapes[shapeIter].map_Ka = map_ka_filename;
                            }
                        break;
                        case 'd':
                            {
                                std::string map_kd_filename(line+7);
                                //~ map_kd_filename = map_kd_filename.substr(0,map_kd_filename.find('\n'));
                                //~ cout << "map_kd : |"<<map_kd_filename << "|" << endl;
                                if ( ! map_kd_filename.empty() && map_kd_filename!="0")
									shapes[shapeIter].map_Kd = map_kd_filename;
                            }
                        break;
                        case 's':
                            {
                                std::string map_ks_filename(line+7);
                                //~ map_ks_filename = map_ks_filename.substr(0,map_ks_filename.find('\n'));
                                //~ cout << "map_ks : |"<<map_ks_filename << "|" << endl;
                                if ( ! map_ks_filename.empty() && map_ks_filename!="0")
									shapes[shapeIter].map_Ks = map_ks_filename;
                            }
                        break;

						default: break;
                    }
                break;

                default: break;
            }
        } while(!stream.eof());
		radius = sqrt(radius);
		posBorder += negBorder;
		if (posBorder.length() / radius > 0.4) {
			poorlyCentered = true;
			std::ostringstream oss;
			oss << "OJM '" << fileName << "' center is detected around " << posBorder << ", is it expected ?";
			cLog::get()->write(oss.str(), LOG_TYPE::L_WARNING);
		}
		for (auto &s : shapes) {
			for (auto &v : s.vertices) {
				v /= radius;
			}
		}
        return true;
    }
    std::cout<<"OJM: error reading file"<<std::endl;
	return false;
}


void Ojm::print()
{
	std::cout<< "Number of shape " << shapes.size() << std::endl;
	for(unsigned int i=0; i< shapes.size();i++) {
		std::cout<< "***** shapes n°"<< i<< std::endl;
		std::cout<< "Number of vectrices " << shapes[i].vertices.size() << std::endl;
		std::cout<< "Number of uvs        " << shapes[i].uvs.size() << std::endl;
		std::cout<< "Number of normales  " << shapes[i].normals.size() << std::endl;
		std::cout<< "Number of indices  " << shapes[i].indices.size() << std::endl;
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
