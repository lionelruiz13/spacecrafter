#ifndef OBJ3D_HPP_INCLUDED
#define OBJ3D_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "obj_common.hpp"
#include <vector>


class Obj3D {
	friend class ObjToOjm;

public:
	Obj3D(const std::string _fileName);
	~Obj3D();

	//! renvoie l'état de l'objet: chargé et opérationnel, négatif sinon
	bool getOk() {
		return is_ok;
	}

	// //! charge et initialise un objet OBJ pour OpenGL
	// bool init();

	//! affiche sur console le contenu de l'OBJ
	void print();

private:
	bool is_ok; //say if the model is correctly initialised and operationnal

	//! lecture du fichier MTL
	bool ReadMaterialLibrary(const std::string& filename);
	//! vérifie si les indices coincident dans l'objet
	bool testIndices();
	//! charge un objet OBJ du disque dur
	bool readOBJ();

	RawData positionData;
	//! indices des différents morceaux de l'objet
	std::vector<Mesh*> meshes;
	//! vector contenant les materials de l'objet
	std::vector<Material*> materials;

	std::string fileName;
};

#endif // OBJ3D_HPP_INCLUDED
