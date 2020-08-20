#include "obj_to_ojm.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

#include "obj3D.hpp"

// *****************************************************************************
//
// FONCTIONS UTILITAIRES
//
// fonctions copiées de opengl-tutorial.org
//
// *****************************************************************************

// Returns true if v1 can be considered equal to v2
static bool is_near(float v1, float v2)
{
	return fabs( v1-v2 ) < 0.005f;
}

// Searches through all already-exported vertices for a similar one.
// Similar = same position + same UVs + same normal
static bool getSimilarVertexIndex(
    const Vec3f & in_vertex,
    const Vec2f & in_uv,
    const Vec3f & in_normal,
    const std::vector<Vec3f> & out_vertices,
    const std::vector<Vec2f> & out_uvs,
    const std::vector<Vec3f> & out_normals,
    unsigned int & result)
{
	// Lame linear search
	for ( unsigned int i=0; i<out_vertices.size(); i++ ) {
		if (
		    is_near( in_vertex[0], out_vertices[i][0] ) &&
		    is_near( in_vertex[1], out_vertices[i][1] ) &&
		    is_near( in_vertex[2], out_vertices[i][2] ) &&
		    is_near( in_uv[0],     out_uvs     [i][0] ) &&
		    is_near( in_uv[1],     out_uvs     [i][1] ) &&
		    is_near( in_normal[0], out_normals [i][0] ) &&
		    is_near( in_normal[1], out_normals [i][1] ) &&
		    is_near( in_normal[2], out_normals [i][2] )
		) {
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

// Searches through all already-exported vertices for a similar one.
// Similar = same position +  same normal
static  bool getSimilarVertexNoUvIndex(
    const Vec3f & in_vertex,
    const Vec3f & in_normal,
    const std::vector<Vec3f> & out_vertices,
    const std::vector<Vec3f> & out_normals,
    unsigned int & result)
{
	// Lame linear search
	for ( unsigned int i=0; i<out_vertices.size(); i++ ) {
		if (
		    is_near( in_vertex[0], out_vertices[i][0] ) &&
		    is_near( in_vertex[1], out_vertices[i][1] ) &&
		    is_near( in_vertex[2], out_vertices[i][2] ) &&
		    is_near( in_normal[0], out_normals [i][0] ) &&
		    is_near( in_normal[1], out_normals [i][1] ) &&
		    is_near( in_normal[2], out_normals [i][2] )
		) {
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}


static std::string extractFileName(const std::string& str)
{
	if (str.empty())
		return "";

	std::size_t found = str.find_last_of("/\\");
	if (found>str.length())
		return "tex/"+str;
	else
		return "tex/"+str.substr(found+1);
}

void ObjToOjm::importOBJ(Obj3D* _obj)
{
	obj= _obj;
}

bool ObjToOjm::createUniqueIndexFromVertices()
{
	//cout << "Nombre total de shape "<< obj->meshes.size() << endl;
	for(unsigned int  i=0; i < obj->meshes.size(); i++) {
		//~ cout << "meshe "<< i << " sur "<< obj->meshes.size() << endl;
		Mesh* mesh = obj->meshes[i];
		Shape* pShape = new Shape();

		pShape->name = mesh->material->name;
		pShape->map_Ka = extractFileName(mesh->material->map_Ka);
		pShape->map_Kd = extractFileName(mesh->material->map_Kd);
		pShape->map_Ks = extractFileName(mesh->material->map_Ks);
		pShape->T= mesh->material->T;

		if (!pShape->map_Kd.empty() && pShape->map_Ka.empty())
			pShape->map_Ka = pShape->map_Kd;

		pShape->Ka = mesh->material->Ka;
		pShape->Kd = mesh->material->Kd;
		pShape->Ks = mesh->material->Ks;

		pShape->Ns = mesh->material->Ns;
		//~ cout << "test Ns = "<< pShape->Ns << endl;
		//~ cout << "  " << endl;
		
		//cout << "Shape ["<< i << "] nombre de vertex " << obj->meshes[i]->vertexIndices.size() << endl;
		for (unsigned int j=0; j< obj->meshes[i]->vertexIndices.size(); j++) {
			// Try to find a similar vertex in out_XXXX
			//cout << "etape " << j << "/" << obj->meshes[i]->vertexIndices.size() << " name " << mesh->material->name << endl;
			unsigned int index;
			bool hasUV;
			mesh->uvIndices.size()!=0 ? hasUV=true: hasUV = false;
			bool found;

			if (hasUV)
				found = getSimilarVertexIndex(obj->positionData.vertex[obj->meshes[i]->vertexIndices[j]-1],
				                              obj->positionData.uvs[obj->meshes[i]->uvIndices[j]-1],
				                              obj->positionData.normals[obj->meshes[i]->normalIndices[j]-1],
				                              pShape->vertices,
				                              pShape->uvs,
				                              pShape->normals,
				                              index);
			else
				found = getSimilarVertexNoUvIndex(obj->positionData.vertex[obj->meshes[i]->vertexIndices[j]-1],
				                              obj->positionData.normals[obj->meshes[i]->normalIndices[j]-1],
				                              pShape->vertices,
				                              pShape->normals,
				                              index);

			if ( found ) { // A similar vertex is already in the VBO, use it instead !
				pShape->indices.push_back( index );
			}
			else { // If not, it needs to be added in the output data.

				pShape->vertices.push_back( obj->positionData.vertex[obj->meshes[i]->vertexIndices[j]-1]);
				if (hasUV) {
					pShape->uvs.push_back( obj->positionData.uvs[obj->meshes[i]->uvIndices[j]-1]);
				}
				pShape->normals.push_back( obj->positionData.normals[obj->meshes[i]->normalIndices[j]-1]);
				pShape->indices.push_back( pShape->vertices.size() - 1 );
			}
		}
		shapes.push_back(*pShape);
	}
	//std::cout << std::endl;
	return true;
}

bool ObjToOjm::fusionMaterials()
{
	//std::cout << "ObjToOjm fusion des matériaux similaires" << std::endl;
	bool isDone = false;
	while (!isDone) {
		for(unsigned short i=0; i< obj->meshes.size(); i++) {
			for(unsigned short j=i+1; j< obj->meshes.size(); j++) {
				if ( obj->meshes[i]->material->name == obj->meshes[j]->material->name ) {

					//std::cout << "etape " << i << " " << j << std::endl;

					int nbUv1 = obj->meshes[i]->uvIndices.size();
					int nbUv2 = obj->meshes[j]->uvIndices.size();

					if (((nbUv1 ==0) && (nbUv2 == 0)) || ((nbUv1 >0) && (nbUv2 > 0)) ) {
						obj->meshes[i]->vertexIndices.insert( obj->meshes[i]->vertexIndices.end(), obj->meshes[j]->vertexIndices.begin(), obj->meshes[j]->vertexIndices.end() );
						obj->meshes[i]->normalIndices.insert( obj->meshes[i]->normalIndices.end(), obj->meshes[j]->normalIndices.begin(), obj->meshes[j]->normalIndices.end() );

						if ((nbUv1 >0) && (nbUv2 > 0))
							obj->meshes[i]->uvIndices.insert( obj->meshes[i]->uvIndices.end(), obj->meshes[j]->uvIndices.begin(), obj->meshes[j]->uvIndices.end() );

						Mesh* tmpToDelete = obj->meshes[j];
						obj->meshes.erase(obj->meshes.begin()+j);
						delete tmpToDelete;
						//std::cout << " fusion " << i << " " << j << std::endl;
						goto fin_de_la_boucle;
					}
				}
			}
		}
		return true;

	fin_de_la_boucle:
		isDone = false;
	}
	return true;
}


bool ObjToOjm::exportOJM(const std::string &filename)
{
	std::ofstream stream;
	stream.open(filename.c_str(),std::ios_base::out);
	if(!stream.is_open())
		return -1;

	//std::cout << "Début export OBJ" << std::endl;

	stream<<"# Spacecrafter personal file format"<<std::endl;
	stream<<"# By Olivier Nivoix and Jérôme Lartillot"<< std::endl;
	stream<<std::endl<<std::endl;
	
	for(unsigned int i=0; i<shapes.size(); i++) {
		stream<<"o "<<shapes[i].name<<std::endl;

		stream<<"ka "<<shapes[i].Ka.v[0] << " " <<
		      shapes[i].Ka.v[1] << " "<<
		      shapes[i].Ka.v[2] << std::endl;

		stream<<"kd "<<shapes[i].Kd.v[0] << " " <<
		      shapes[i].Kd.v[1] << " "<<
		      shapes[i].Kd.v[2] << std::endl;

		stream<<"ks "<<shapes[i].Ks.v[0] << " " <<
		      shapes[i].Ks.v[1] << " "<<
		      shapes[i].Ks.v[2] << std::endl;

		stream<<"Ns " << shapes[i].Ns << std::endl;
		stream<<"t " << shapes[i].T << std::endl;

		stream<<"map_ka "<<shapes[i].map_Ka<<std::endl;
		stream<<"map_kd "<<shapes[i].map_Kd<<std::endl;
		stream<<"map_ks "<<shapes[i].map_Ks<<std::endl;

		for(Vec3f vert:shapes[i].vertices)
			stream<<"v "<<vert.v[0]<<" "<<vert.v[1]<<" "<<vert.v[2]<<std::endl;

		for(Vec2f uv:shapes[i].uvs)
			stream<<"u "<<uv.v[0]<<" "<<uv.v[1]<<std::endl;

		for(Vec3f norm:shapes[i].normals)
			stream<<"n "<<norm.v[0]<<" "<<norm.v[1]<<" "<<norm.v[2]<<std::endl;

		unsigned int nbrIndices = shapes[i].indices.size();

		unsigned int groupesTri = nbrIndices/9;
		for(unsigned int j=0; j<groupesTri; j++) {
			stream<<"j "<<
			      shapes[i].indices[9*j+0]<<" "<< shapes[i].indices[9*j+1]<<" "<< shapes[i].indices[9*j+2]<<" "<<
			      shapes[i].indices[9*j+3]<<" "<< shapes[i].indices[9*j+4]<<" "<< shapes[i].indices[9*j+5]<<" "<<
			      shapes[i].indices[9*j+6]<<" "<< shapes[i].indices[9*j+7]<<" "<< shapes[i].indices[9*j+8]<<std::endl;
		}

		for(unsigned int j=groupesTri*9; j<nbrIndices; j=j+3) {
			stream<<"i "<<
			      shapes[i].indices[j+0]<<" "<< shapes[i].indices[j+1]<<" "<< shapes[i].indices[j+2]<<std::endl;
		}

		stream<<std::endl;
	}

	//std::cout << "Fin export OBJ" << std::endl;
	return true;
}

bool ObjToOjm::exportV3D(const std::string &filename)
{
	std::ofstream stream;
	stream.open(filename.c_str(),std::ios_base::out);
	if(!stream.is_open())
		return -1;

	//std::cout << "Début export V3D" << std::endl;

	stream<<"# Vertex 3D file format"<<std::endl;
	stream<<"# By Association Sirius && Andromede"<< std::endl;

	time_t tTime = time(NULL);
	tm * tmTime = localtime (&tTime);
	char timestr[20];
	strftime(timestr, 20, "%y-%m-%d_%H:%M:%S", tmTime);

	stream<<"# Created at "<< std::string(timestr) <<std::endl;
	stream<<std::endl<<std::endl;

	for(unsigned int i=0; i<shapes.size(); i++) {
		stream<<"o "<<shapes[i].name<<std::endl;

		stream<<"ka "<<shapes[i].Ka.v[0] << " " <<
		      shapes[i].Ka.v[1] << " "<<
		      shapes[i].Ka.v[2] << std::endl;

		stream<<"kd "<<shapes[i].Kd.v[0] << " " <<
		      shapes[i].Kd.v[1] << " "<<
		      shapes[i].Kd.v[2] << std::endl;

		stream<<"ks "<<shapes[i].Ks.v[0] << " " <<
		      shapes[i].Ks.v[1] << " "<<
		      shapes[i].Ks.v[2] << std::endl;

		stream<<"Ns " << shapes[i].Ns << std::endl;
		stream<<"t " << shapes[i].T << std::endl;

		stream<<"map_ka "<<shapes[i].map_Ka<<std::endl;
		stream<<"map_kd "<<shapes[i].map_Kd<<std::endl;
		stream<<"map_ks "<<shapes[i].map_Ks<<std::endl;

		for (long unsigned int j=0; j< shapes[i].vertices.size(); j++)
			stream<<"v "<< shapes[i].vertices[j].v[0]<< " " << shapes[i].vertices[j].v[1]<< " " << shapes[i].vertices[j].v[2] << " "
			<< shapes[i].uvs[j].v[0]<< " " << shapes[i].uvs[j].v[1]<< " " 
			<< shapes[i].normals[j].v[0]<< " " << shapes[i].normals[j].v[1]<< " " << shapes[i].normals[j].v[2]
			<<std::endl;	 

		unsigned int nbrIndices = shapes[i].indices.size();
		for(unsigned int j=0; j<nbrIndices; j=j+3) {
			stream<<"i "<<
			      shapes[i].indices[j+0]<<" "<< shapes[i].indices[j+1]<<" "<< shapes[i].indices[j+2]<<std::endl;
		}

		stream<<std::endl;
	}

	//std::cout << "Fin export V3D" << std::endl;
	return true;
}
