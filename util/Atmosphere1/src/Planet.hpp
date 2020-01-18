#ifndef PLANET_HPP_INCLUDED
#define PLANET_HPP_INCLUDED

#include "model3D.hpp"
#include <map>


//! @file Planet.cpp Planet.hpp
//! @author Jérôme Lartillot
//! @date 06/04/2016
//!
//! @section Description Description
//!
//! Planet is a class to draw a planet.
//! This is a test class to test the method.



//! @struct PlanetData
//! This structure contains everything you need to initialize a Planet.
struct PlanetData {
	std::string texturePath;
	std::string heightmapPath;
	std::string normalsPath;
	GLuint textureUnit;
	GLuint heightmapUnit;
	GLuint normalsUnit;
};

//! @class Planet
//! This class can draw a Planet with tessellation,
//! heightmap and normal mapping.

class s_texture;

class Planet: public GPU::Model3D {
public:
	Planet(const PlanetData& data);
	~Planet();

	void drawWithTessellation(const Mat4f&model,const Mat4f&view,const Mat4f&projection,const Vec2f&resolution);

	void useProgram();
	
	void setModel(Mat4f model);
	void setVp(Mat4f vp);

private:
	std::map<GLuint, s_texture*> textures; //This map contains all the needed textures to draw the planet
										   //They are identified by the units values below
	// texture units
	GLuint textureUnit;
	GLuint heightmapUnit;
	GLuint normalsUnit;

	// Program object
	shaderProgram program;
	Vec3f  sunPosition;
	GLuint textureID;
	GLuint normalsID;
	GLuint heightmapID;
};


#endif // PLANET_HPP_INCLUDED
