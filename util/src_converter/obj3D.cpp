#include "obj3D.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iterator>

using namespace std;

// *****************************************************************************
//
// FONCTIONS UTILITAIRES
//
// *****************************************************************************


//removes leacing and trailing spaces
static std::string trim(const std::string& str, const std::string& whitespace = " \t")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return "";

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}


// compte le nombre de / d'une chaine de caractère
static short countSlash(const std::string &s)
{
	short n = 0;
	for(unsigned int i=0; i<s.size()-1; i++) {
		if(s[i]=='/') n++;
	}
	return n;
}


// compte le nombre d'espace d'une string
static short countSpace(const std::string& str)
{
	short nbr=0;
	for(std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
		if (*it==' ') nbr++;
	}
	return nbr;
}


// vérifie la présence d'un // dans la chaine
static bool hasDoubleSlash(const std::string &s)
{
	for(unsigned int i=0; i<s.size()-1; i++) {
		if(s[i]=='/' && s[i+1]=='/')
			return true;
	}
	return false;
}


static std::string removeExtraWhitespaces(const string &input)
{
	std::string output;
	unique_copy(input.begin(), input.end(), back_insert_iterator<string>(output), [](char a,char b) {
		return isspace(a) && isspace(b);
	});
	return output;
}


// cas indices négatifs: on les remet positif
static void changeSigneAvecIndice(int *vector, int indiceMax)
{
	for (int i=0; i<3; i++) {
		if (vector[i]<0)
			vector[i] = indiceMax + vector[i];
	}
}


// *****************************************************************************
//
// CLASSE OBJ3D
//
// *****************************************************************************


Obj3D::Obj3D(const string _fileName)
{
	fileName = _fileName;
	is_ok = readOBJ();
	if (is_ok)
		is_ok=testIndices();
	if (is_ok)
		std::cout << "File " << fileName << " read without errors" << std::endl;
	else {
		std::cout << " : Errors detected while reading file "<< fileName << "  Aborting..." << std::endl;
		exit(-1);
	}
}


Obj3D::~Obj3D()
{
	meshes.clear();
	materials.clear();
}

bool Obj3D::testIndices()
{
	for(unsigned int i=0; i<meshes.size(); i++) {
		if (meshes[i]->vertexIndices.size() != meshes[i]->normalIndices.size()) {
			cout << "OBJ3D : erreur d'indices entre vertex et normal" << endl;
			return false;
		}
		if  (meshes[i]->uvIndices.size() !=0) {
			if (meshes[i]->vertexIndices.size() != meshes[i]->uvIndices.size()) {
				cout << "OBJ3D : erreur d'indices entre vertex et textures" << endl;
				return false;
			}
		}
	}
	return true;
}

//reads material libray (.MTL) file
bool Obj3D::ReadMaterialLibrary(const std::string& filename)
{
	ifstream fp(filename.c_str(),ios::in);
	if(!fp) {
		cout << "OBJ3D : Fichier mtl " << filename<< " non trouvé"<< endl;
		return false;
	}
	string tmp(std::istreambuf_iterator<char>(fp), (std::istreambuf_iterator<char>()));
	istringstream buffer(tmp);
	fp.close();

	//now parse the file
	string line;
	Material* pMat = 0;
	unsigned int nbReadLine = 0;
	while(getline(buffer, line)) {
		nbReadLine ++;

		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		line = trim(line);
		line = removeExtraWhitespaces(line);

		if(line.find_first_of("#") != string::npos) //its a comment leave it
			continue;
		if(line.length()==0)
			continue;

		int space_index = line.find_first_of(" ");
		string prefix = trim(line.substr(0, space_index));

		if(prefix.compare("newmtl") ==0) { //if we have a newmtl block
			pMat = new Material();
			pMat->name = line.substr(space_index+1);
			//~ cout << "OBJ3D: read material name : " << pMat->name << endl;
			this->materials.push_back(pMat);
			continue;
		}

		if(prefix.compare("Kd")==0) {
			line = line.substr(space_index+1);
			istringstream s(line);
			s>>pMat->Kd[0]>>pMat->Kd[1]>>pMat->Kd[2];
			continue;
		}

		if(prefix.compare("map_Kd") == 0) {
			pMat->map_Kd = line.substr(space_index+1);
			continue;
		}

		if(prefix.compare("Ka")==0) {
			line = line.substr(space_index+1);
			istringstream s(line);
			s>>pMat->Ka[0]>>pMat->Ka[1]>>pMat->Ka[2];
			continue;
		}

		if(prefix.compare("map_Ka") == 0) {
			pMat->map_Ka = line.substr(space_index+1);
			continue;
		}
		if(prefix.compare("Ks")==0) {
			line = line.substr(space_index+1);
			istringstream s(line);
			s>>pMat->Ks[0]>>pMat->Ks[1]>>pMat->Ks[2];
			continue;
		}

		if(prefix.compare("map_Ks") == 0) {
			pMat->map_Ks = line.substr(space_index+1);
			continue;
		}

		if(prefix.compare("Ns") == 0) {
			line = line.substr(space_index+1);
			istringstream s(line);
			s>>pMat->Ns;
			continue;
		}

		if(prefix[0]=='d') {
			line = line.substr(space_index+1);
			istringstream s(line);
			s>>pMat->T;
			continue;
		}

		if(prefix.compare("Tr")==0) {
			line = line.substr(space_index+1);
			istringstream s(line);
			s>>pMat->T;
			pMat->T=1.0-pMat->T;
			continue;
		}

		//on ignore le reste...
		cout << "OBJ3D : MTL line[" <<nbReadLine << "] ignore " << line << endl;
	}
	return true;
}



bool Obj3D::readOBJ()
{
	//read file fileName et convert it to readable char
	ifstream fp(fileName,ios::in);
	if(!fp) {
		cout << "OBJ3D : Unable to read file " << fileName << endl;
		return false;
	}
	string tmp(std::istreambuf_iterator<char>(fp), (std::istreambuf_iterator<char>()));
	istringstream buffer(tmp);
	fp.close();

	//initialisation
	bool hasNormals = false;
	string line;
	Mesh* tmpMesh= nullptr;
	unsigned nbReadLine = 0;

	while(getline(buffer, line)) {
		nbReadLine++;
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		line = trim(line);
		line = removeExtraWhitespaces(line);

		if(line[0]=='#') //its a comment leave it
			continue;

		int space_index = line.find_first_of(" ");
		string prefix = trim(line.substr(0, space_index));
		if(prefix.length()==0)
			continue;

		if(prefix.compare("v")==0) { //if we have a vertex
			Vec3f vertex;
			sscanf(line.c_str(), "v %f %f %f\n", &vertex[0], &vertex[1], &vertex[2] );
			positionData.vertex.push_back(vertex);
			continue;
		}

		if(prefix.compare("vt")==0) { //if we have a texture coord
			Vec2f uv;
			sscanf(line.c_str(), "vt %f %f\n", &uv[0], &uv[1] );
			positionData.uvs.push_back(uv);
			continue;
		}

		if(prefix.compare("vn")==0) { //if we have a normal
			Vec3f normal;
			sscanf(line.c_str(), "vn %f %f %f\n", &normal[0], &normal[1], &normal[2] );
			positionData.normals.push_back(normal);
			hasNormals = true;
			continue;
		}

		if(prefix.compare("f")==0) {
			if (tmpMesh == nullptr) {
				std::cout << "OBJ3D : No mtl initialised" << std::endl;
				return false;
			}

			if (hasNormals == false) {
				std::cout << "OBJ3D : The normals are not defined"<< std::endl;
				return false;
			}

			if ((countSlash(line) !=6) && (countSlash(line) !=8)) {
				std::cout << "OBJ3D : line[" <<nbReadLine << "] face not correctly defined " << line << endl;
				return false;
			}

			short nbIndices=countSpace(line);
			bool  hasUVs = !hasDoubleSlash(line);

			switch (nbIndices)	{
			case 3: {		//if (nbIndices ==3) { //on traite un triangle
				int vertexIndex[3], uvIndex[3], normalIndex[3];
				if (hasUVs)
					sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					       &vertexIndex[0], &uvIndex[0], &normalIndex[0],
					       &vertexIndex[1], &uvIndex[1], &normalIndex[1],
					       &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				else
					sscanf(line.c_str(), "f %d//%d %d//%d %d//%d\n",
					       &vertexIndex[0], &normalIndex[0],
					       &vertexIndex[1], &normalIndex[1],
					       &vertexIndex[2], &normalIndex[2]);

				changeSigneAvecIndice(vertexIndex, positionData.vertex.size());
				tmpMesh->vertexIndices.push_back(vertexIndex[0]);
				tmpMesh->vertexIndices.push_back(vertexIndex[1]);
				tmpMesh->vertexIndices.push_back(vertexIndex[2]);

				if (hasUVs) {
					changeSigneAvecIndice(uvIndex, positionData.uvs.size());
					tmpMesh->uvIndices.push_back(uvIndex[0]);
					tmpMesh->uvIndices.push_back(uvIndex[1]);
					tmpMesh->uvIndices.push_back(uvIndex[2]);
				}

				changeSigneAvecIndice(normalIndex, positionData.normals.size());
				tmpMesh->normalIndices.push_back(normalIndex[0]);
				tmpMesh->normalIndices.push_back(normalIndex[1]);
				tmpMesh->normalIndices.push_back(normalIndex[2]);
			} break;

			case 4 : {//if (nbIndices ==4) { //on traite un quadrilatère
				int vertexIndex[4], uvIndex[4], normalIndex[4];

				if (hasUVs)
					sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
					       &vertexIndex[0], &uvIndex[0], &normalIndex[0],
					       &vertexIndex[1], &uvIndex[1], &normalIndex[1],
					       &vertexIndex[2], &uvIndex[2], &normalIndex[2],
					       &vertexIndex[3], &uvIndex[3], &normalIndex[3]);
				else
					sscanf(line.c_str(), "f %d//%d %d//%d %d//%d %d//%d\n",
					       &vertexIndex[0], &normalIndex[0],
					       &vertexIndex[1], &normalIndex[1],
					       &vertexIndex[2], &normalIndex[2],
					       &vertexIndex[3], &normalIndex[3]);

				changeSigneAvecIndice(vertexIndex, positionData.vertex.size());
				tmpMesh->vertexIndices.push_back(vertexIndex[0]);
				tmpMesh->vertexIndices.push_back(vertexIndex[1]);
				tmpMesh->vertexIndices.push_back(vertexIndex[2]);
				tmpMesh->vertexIndices.push_back(vertexIndex[2]);
				tmpMesh->vertexIndices.push_back(vertexIndex[3]);
				tmpMesh->vertexIndices.push_back(vertexIndex[0]);

				if (hasUVs) {
					changeSigneAvecIndice(uvIndex, positionData.uvs.size());
					tmpMesh->uvIndices.push_back(uvIndex[0]);
					tmpMesh->uvIndices.push_back(uvIndex[1]);
					tmpMesh->uvIndices.push_back(uvIndex[2]);
					tmpMesh->uvIndices.push_back(uvIndex[2]);
					tmpMesh->uvIndices.push_back(uvIndex[3]);
					tmpMesh->uvIndices.push_back(uvIndex[0]);
				}

				changeSigneAvecIndice(normalIndex, positionData.normals.size());
				tmpMesh->normalIndices.push_back(normalIndex[0]);
				tmpMesh->normalIndices.push_back(normalIndex[1]);
				tmpMesh->normalIndices.push_back(normalIndex[2]);
				tmpMesh->normalIndices.push_back(normalIndex[2]);
				tmpMesh->normalIndices.push_back(normalIndex[3]);
				tmpMesh->normalIndices.push_back(normalIndex[0]);
			} break;
			default:
				std::cout << "OBJ3D : line[" <<nbReadLine << "] Unknown face definition" << std::endl;
				return false;
				break;
			}
			continue;
		}

		if(prefix.compare("mtllib")==0) {
			//we have a material library
			char materials_name[80];
			sscanf(line.c_str(), "mtllib %s\n", materials_name);
			//~ cout << "OBJ3D: lecture fichier mtl " << materials_name << endl;
			string fullpath = string(materials_name);
			bool tmp = ReadMaterialLibrary(fullpath);
			if (tmp==false) {
				cout << "OBJ3D : Problème ReadMaterialLibrary " << fullpath << " n'existe pas !" << endl;
				return false;
			}
			continue;
		}

		if(prefix.compare("usemtl")==0) {
			char usemtl_name[80];
			sscanf(line.c_str(), "usemtl %s\n", usemtl_name);
			//~ cout << "OBJ3D : lecture usemtl |" << usemtl_name << "|"<< endl;

			tmpMesh = new Mesh();
			meshes.push_back(tmpMesh);
			bool tmp=false;

			for(size_t i=0; i<this->materials.size(); i++) {
				if(this->materials[i]->name.compare(usemtl_name) == 0) {
					tmpMesh->material = this->materials[i];
					tmp= true;
					break;
				}
			}

			if (tmp==false) {
				cout << "OBJ3D : usemtl line[" <<nbReadLine << "] "<< usemtl_name << " not found" << endl;
				return false;
			}
			continue;
		}

		cout << "OBJ3D : ignore : line[" <<nbReadLine << "] "<< line << endl;
	}
	return true;
}


void Obj3D::print()
{
	cout<< "Nombre de mesh " << meshes.size() << endl;
	cout<< "Nombre de materials " << materials.size() << endl;
	cout<< "Nombre de vectrices " << positionData.vertex.size() << endl;
	cout<< "Nombre de uvs " << positionData.uvs.size() << endl;
	cout<< "Nombre de normales " << positionData.normals.size() << endl;

	for(unsigned int i=0; i< meshes.size(); i++) {
		cout<< "***** Mesh n°"<< i<< endl;
		cout<< "Nombre d'indices de vectrices " << meshes[i]->vertexIndices.size() << endl;
		//~ for(unsigned int j=0; j< meshes[i]->vertexIndices.size() ; j++)
		//~ cout << " " << meshes[i]->vertexIndices[j];
		//~ cout << endl;

		cout<< "Nombre d'indices de uvs " << meshes[i]->uvIndices.size() << endl;
		//~ for(unsigned int j=0; j< meshes[i]->uvIndices.size() ; j++)
		//~ cout << " " << meshes[i]->uvIndices[j];
		//~ cout << endl;

		cout<< "Nombre d'indices de normales " << meshes[i]->normalIndices.size() << endl;
		//~ for(unsigned int j=0; j< meshes[i]->normalIndices.size() ; j++)
		//~ cout << " " << meshes[i]->normalIndices[j];
		//~ cout << endl;
	}

	//Bilan des matériaux
	cout << "OBJ3D : Nombre de matériaux :" << this->materials.size()<< endl;
	for(unsigned int i=0; i<this->materials.size(); i++) {
		cout << "OBJ3D: -- Name : " << this->materials[i]->name<< endl;
	}
}
