#ifndef OBJ_TO_OJM_HPP_INCLUDED
#define OBJ_TO_OJM_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "obj_common.hpp"

#include <vector>

class Obj3D;

class ObjToOjm {

public:
	ObjToOjm() {}
	~ObjToOjm() {}

	//! charge un objet OBJ
	void importOBJ(Obj3D* _obj);

	//! mise en commun des shapes identiques cad ayant les mêmes paramètres
	bool fusionMaterials();

	//! le sauvegarde sur disque dur
	bool exportOJM(const std::string &filename);
	bool exportV3D(const std::string &filename);
	bool createUniqueIndexFromVertices();
private:
	Obj3D* obj;

	std::vector<Shape> shapes;
};

#endif // OBJ3D_HPP_INCLUDED
