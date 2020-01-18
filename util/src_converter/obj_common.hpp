#ifndef OBJ_COMMON_HPP_INCLUDED
#define OBJ_COMMON_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include <vector>

struct Shape {
	std::string name;

	std::vector<Vec3f> vertices;
	std::vector<Vec2f> uvs;
	std::vector<Vec3f> normals;
	std::vector<unsigned int> indices;

	Vec3f Ka;
	Vec3f Kd;
	Vec3f Ks;

	std::string map_Ka;
	std::string map_Kd;
	std::string map_Ks;

	float Ns;
	float T; //transparency
};


//utilisé par OBJ3D
struct Material {
	std::string name="";
	Vec3f Ka=Vec3f(1.0, 1.0, 1.0);
	Vec3f Kd=Vec3f(0.64, 0.64, 0.64);
	Vec3f Ks=Vec3f(0.5, 0.5, 0.5);
	std::string map_Ka="";
	std::string map_Kd="";
	std::string map_Ks="";
	float Ns = 100.0;
	float T = 1.0;
};

struct RawData {
	std::vector<Vec3f> vertex;
	std::vector<Vec2f> uvs;
	std::vector<Vec3f> normals;
};

struct Mesh {
	std::vector<unsigned int> vertexIndices;
	std::vector<unsigned int> uvIndices;
	std::vector<unsigned int> normalIndices;
	Material * material;
};

#endif // OBJ_COMMON_HPP_INCLUDED
