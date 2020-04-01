/*
#ifndef ATMOSPHERE_EXT_HPP_INCLUDED
#define ATMOSPHERE_EXT_HPP_INCLUDED

//~ #include "model3D.hpp"
#include "ojmModule/objl.hpp"
#include "tools/shader.hpp"
#include <string>

class s_texture;

class AtmosphereExt {
public:
	AtmosphereExt(ObjL* _currentObj, double _radiusRatio, const std::string &gradient);
	~AtmosphereExt();

	void draw(float screen_sz);
	void use() {
		atmProg.use();
	}
	void unuse() {
		atmProg.unuse();
	}

	void setPlanetRadius(float radius);
	void setAtmAlphaScale(float atmAlphaScale);
	void setSunPos(Vec3f position);
	void setPlanetPos(Vec3f pos);
	void setClippingFov(Vec3f _clippingFov);
	void setInverseModelViewProjectionMatrix(Mat4f inverseModelViewProjectionMatrix);
	void setCameraPositionBeforeLookAt(Vec3f viewBeforeLookAt);
	void setView(Mat4f view);
	void setVp(Mat4f vp);
	void setModelView(Mat4f view);
	void setModel(Mat4f model);
	void setModelViewProjectionMatrix(Mat4f vp);


private:

	shaderProgram atmProg;
	s_texture * atmTexture = nullptr;
	ObjL *currentObj = nullptr;
	double radiusRatio;

};

#endif // ATMOSPHERE_HPP_INCLUDED
*/