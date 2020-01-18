#ifndef ATMOSPHERE_HPP_INCLUDED
#define ATMOSPHERE_HPP_INCLUDED

#include "main.hpp"

#include "model3D.hpp"
#include "shader.hpp"

class s_texture;

class AtmosphereExt {
public:
	AtmosphereExt();
	~AtmosphereExt();

	void draw();

	void use() {
		atmProg.use();
	}

	void setPlanetRadius(float radius);
	void setAtmRadius(float radius);
	void setAtmAlphaScale(float atmAlphaScale);
	void setSunPos(Vec3f position);
	void setModel(Mat4f model);
	void setViewBeforeLookAt(Mat4f viewBeforeLookAt);
	void setView(Mat4f view);
	void setVp(Mat4f vp);


private:

	GPU::Model3D atmosphere;
	shaderProgram atmProg;
	
	s_texture * atmTexture = NULL;

};

#endif // ATMOSPHERE_HPP_INCLUDED
