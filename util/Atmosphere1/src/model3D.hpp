//! @file model3D.cpp model3D.hpp
//! @author Jérôme Lartillot
//! @date 28/12/2015
//!
//! @section Example2 Example with OpenGL 4.x
//!
//! \class Model3D
//!
//! Cette classe permet de charger et affiche un fichier .obj, et aussi un fichier .3ds dans le futur.
//!
//! Pour charger un .obj, utiliser load_OBJ suivi du nom du fichier OBJ et du nom du fichier BMP qui sert de texture.
//!
//! draw() permet ensuite de dessiner le modèle chargé.
//!
//!

#ifndef MODEL3D_HPP_INCLUDED
#define MODEL3D_HPP_INCLUDED

#include "tiny_obj_loader.hpp"
#include "shader.hpp"
#include "vecmath.hpp"

namespace GPU {

#define TBN_REPRESENTATION_THREE_VECTORS 0
#define TBN_REPRESENTATION_MATRIX 1

// Index of the buffers in the VBO array
#define VBO_POSITIONS  0
#define VBO_TEXCOORDS  1
#define VBO_TBN        2 // use this or the 3 other vectors
#define VBO_NORMALS    2
#define VBO_TANGENTS   3
#define VBO_BITANGENTS 4
#define VBO_INDICES    5

class Model3D {
public:
	Model3D(){};
	~Model3D();

	void load_OBJ(const char*filename,const char*texname); // used in planet

	void bindBuffersAndDraw(GLenum mode);


private:
	// Tiny obj loader
	std::string err;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;

	GLuint vao;
	GLuint vbo[5];

	float*positionsBuffer;
	float*texcoordsBuffer;
	float*normalsBuffer;
	float*tangentsBuffer;
	float*bitangentBuffer;
	float*TBNBuffer;
	int   bufferSize; // vertex number

public:
	int id;
	float sradius; // squared radius
};
}// namespace GPU

#endif // MODEL3D_HPP_INCLUDED
