
#include "bodyModule/atmosphere_ext.hpp"
#include "tools/s_texture.hpp"
#include "tools/app_settings.hpp"



AtmosphereExt::AtmosphereExt(ObjL* _currentObj, double _radiusRatio, const std::string &gradient)
{
	std::string file = AppSettings::Instance()->getTextureDir() + "bodies/" + gradient;
	radiusRatio = _radiusRatio;
	atmTexture = new s_texture(file, TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);

	currentObj = _currentObj;

	atmProg.init("atmosphere_ext.vert", "atmosphere_ext.frag");

	atmProg.setUniformLocation("planetRadius");
	atmProg.setUniformLocation("atmRadius");
	atmProg.setUniformLocation("atmAlpha");
	atmProg.setUniformLocation("sunPos");

	atmProg.setUniformLocation("planetPos");
	atmProg.setUniformLocation("camPos");

	//déformation fisheye
	atmProg.setUniformLocation("ModelMatrix");
	atmProg.setUniformLocation("ModelViewMatrix");
	atmProg.setUniformLocation("ModelViewProjectionMatrix");
	atmProg.setUniformLocation("clipping_fov");
	atmProg.setUniformLocation("inverseModelViewProjectionMatrix");
}

AtmosphereExt::~AtmosphereExt()
{
}

void AtmosphereExt::draw(float screen_sz)
{
	//~ glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//~ StateGL::BlendFunc(GL_ONE, GL_ONE);
	//~ StateGL::enable(GL_CULL_FACE);
	StateGL::enable(GL_BLEND);
	StateGL::disable(GL_DEPTH_TEST);
	StateGL::disable(GL_CULL_FACE);
	//~ StateGL::disable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	//~ glDisable(GL_CULL_FACE);

	glBindTexture (GL_TEXTURE_2D, atmTexture->getID());

	//~ atmProg.use();
	//glBindFragDataLocation(atmProg.getProgram(),0,"color");
	//~ atmosphere.bindBuffersAndDraw(GL_PATCHES);
	currentObj->draw(screen_sz);
}


void AtmosphereExt::setPlanetRadius(float radius)
{
//~	atmProg.use();
	atmProg.setUniform("planetRadius",radius);
	atmProg.setUniform("atmRadius",radius*radiusRatio);
}

void AtmosphereExt::setAtmAlphaScale(float atmAlphaScale)
{
//~	atmProg.use();
	atmProg.setUniform("atmAlpha",atmAlphaScale);
}

void AtmosphereExt::setSunPos(Vec3f position)
{
//~	atmProg.use();
	atmProg.setUniform("sunPos",position);
}

void AtmosphereExt::setPlanetPos(Vec3f planetPos)
{
//~	atmProg.use();
	atmProg.setUniform("planetPos",planetPos);
}

void AtmosphereExt::setCameraPositionBeforeLookAt(Vec3f camPos)
{
//~	atmProg.use();
	atmProg.setUniform("camPos",camPos);
}

void AtmosphereExt::setView(Mat4f view)
{
//~	atmProg.use();
	atmProg.setUniform("view",view);
}

void AtmosphereExt::setVp(Mat4f vp)
{
//~	atmProg.use();
	atmProg.setUniform("vp",vp);
}

// déformation

void AtmosphereExt::setModelView(Mat4f view)
{
//~	atmProg.use();
	atmProg.setUniform("ModelViewMatrix",view);
}

void AtmosphereExt::setModel(Mat4f model)
{
//~	atmProg.use();
	atmProg.setUniform("ModelMatrix",model);
}

void AtmosphereExt::setModelViewProjectionMatrix(Mat4f vp)
{
//~	atmProg.use();
	atmProg.setUniform("ModelViewProjectionMatrix",vp);
}


void AtmosphereExt::setClippingFov(Vec3f _clippingFov)
{
//~	atmProg.use();
	atmProg.setUniform("clipping_fov",_clippingFov);
}

void AtmosphereExt::setInverseModelViewProjectionMatrix(Mat4f inverseModelViewProjectionMatrix)
{
//~	atmProg.use();
	atmProg.setUniform("inverseModelViewProjectionMatrix",inverseModelViewProjectionMatrix);
}
