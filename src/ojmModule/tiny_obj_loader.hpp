//
// Copyright 2012-2013, Syoyo Fujita.
//
// Licensed under 2-clause BSD liecense.
//

// Copyright 2016, Lartillot Jérôme.


#ifndef _TINY_OBJ_LOADER_H
#define _TINY_OBJ_LOADER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace tinyobj {

// may help:
//https://en.wikipedia.org/wiki/Wavefront_.obj_file
//http://web.cse.ohio-state.edu/~hwshen/581/Site/Lab3_files/Labhelp_Obj_parser.htm
typedef struct {
	std::string name;

	float ambient[3]; // Ka
	float diffuse[3];  // Kd
	float specular[3]; // Ks
	float transmittance[3];
	float emission[3]; // Ke
	float shininess; // Ns
	float ior;                // index of refraction
	float dissolve;       // d or Tr (Tr = 1-d)
	// 1 == opaque; 0 == fully transparent
	// illumination model (see http://www.fileformat.info/format/material/)
	int illum; // 1: no specular highlight 2: use specular highlight

	/*

	0. Color on and Ambient off
	1. Color on and Ambient on
	2. Highlight on
	3. Reflection on and Ray trace on
	4. Transparency: Glass on, Reflection: Ray trace on
	5. Reflection: Fresnel on and Ray trace on
	6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
	7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
	8. Reflection on and Ray trace off
	9. Transparency: Glass on, Reflection: Ray trace off
	10. Casts shadows onto invisible surfaces
	*/

	std::string ambient_texname; // map_Ka
	std::string diffuse_texname; // map_Kd
	std::string specular_texname; // map_Ks
	std::string normal_texname; // map_normal or must be generated
	std::map<std::string, std::string> unknown_parameter;
	// map_bump or bump
	// disp   for displacement map
	// decal    for stencil decal texture
	// refl    spherical reflection map


	//texture options missing here
} material_t;

typedef struct {
	std::vector<float>          positions;
	std::vector<float>          normals;
	std::vector<float>          texcoords;
	std::vector<unsigned int>   indices;
	std::vector<int>            material_ids; // per-mesh material ID
} mesh_t;

typedef struct {
	std::string  name;
	mesh_t       mesh;
} shape_t;

class MaterialReader {
public:
	MaterialReader() {}
	virtual ~MaterialReader() {}

	virtual std::string operator() (
	    const std::string& matId,
	    std::vector<material_t>& materials,
	    std::map<std::string, int>& matMap) = 0;
};

class MaterialFileReader:
	public MaterialReader {
public:
	MaterialFileReader(const std::string& mtl_basepath): m_mtlBasePath(mtl_basepath) {}
	virtual ~MaterialFileReader() {}
	virtual std::string operator() (
	    const std::string& matId,
	    std::vector<material_t>& materials,
	    std::map<std::string, int>& matMap);

private:
	std::string m_mtlBasePath;
};

/// Loads .obj from a file.
/// 'shapes' will be filled with parsed shape data
/// The function returns error string.
/// Returns empty string when loading .obj success.
/// 'mtl_basepath' is optional, and used for base path for .mtl file.
std::string LoadObj(
    std::vector<shape_t>& shapes,   // [output]
    std::vector<material_t>& materials,   // [output]
    const char* filename,
    const char* mtl_basepath = NULL);

/// Loads object from a std::istream, uses GetMtlIStreamFn to retrieve
/// std::istream for materials.
/// Returns empty string when loading .obj success.
std::string LoadObj(
    std::vector<shape_t>& shapes,   // [output]
    std::vector<material_t>& materials,   // [output]
    std::istream& inStream,
    MaterialReader& readMatFn);

/// Loads materials into std::map
/// Returns an empty string if successful
std::string LoadMtl (
    std::map<std::string, int>& material_map,
    std::vector<material_t>& materials,
    std::istream& inStream);
}

#endif  // _TINY_OBJ_LOADER_H

