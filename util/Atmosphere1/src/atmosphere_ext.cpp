
#include "atmosphere_ext.hpp"
#include "s_texture.hpp"

AtmosphereExt::AtmosphereExt()
{
	atmTexture = new s_texture("../data/textures/AtmosphereGradient3.png");
	
	atmosphere.load_OBJ("../data/models/sphere64x32.obj",NULL);

	atmProg.init("../data/shaders/atm.vert","../data/shaders/atm.tesc","../data/shaders/atm.tese","../data/shaders/atm.geom","../data/shaders/atm.frag");

	atmProg.setUniformLocation("planetRadius");
	atmProg.setUniformLocation("atmRadius");
	atmProg.setUniformLocation("atmAlpha");
	atmProg.setUniformLocation("sunPos");

	atmProg.setUniformLocation("model");
	atmProg.setUniformLocation("viewBeforeLookAt");
	atmProg.setUniformLocation("view");
	atmProg.setUniformLocation("vp");
}

AtmosphereExt::~AtmosphereExt()
{
	//glDeleteTextures(1,&atmGradient);
}

void AtmosphereExt::draw()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);

	glBindTexture (GL_TEXTURE_2D, atmTexture->getID());

	// atmProg.use();
	glBindFragDataLocation(atmProg.getProgram(),0,"color");
	atmosphere.bindBuffersAndDraw(GL_PATCHES);
}


void AtmosphereExt::setPlanetRadius(float radius)
{
	// atmProg.use();
	atmProg.setUniform("planetRadius",radius);
}

void AtmosphereExt::setAtmRadius(float radius)
{
	// atmProg.use();
	atmProg.setUniform("atmRadius",radius);
}

void AtmosphereExt::setAtmAlphaScale(float atmAlphaScale)
{
	// atmProg.use();
	atmProg.setUniform("atmAlpha",atmAlphaScale);
}

void AtmosphereExt::setSunPos(Vec3f position)
{
	// atmProg.use();
	atmProg.setUniform("sunPos",position);
}

void AtmosphereExt::setModel(Mat4f model)
{
	// atmProg.use();
	atmProg.setUniform("model",model);
}

void AtmosphereExt::setViewBeforeLookAt(Mat4f viewBeforeLookAt)
{
	// atmProg.use();
	atmProg.setUniform("viewBeforeLookAt",viewBeforeLookAt);
}

void AtmosphereExt::setView(Mat4f view)
{
	// atmProg.use();
	atmProg.setUniform("view",view);
}

void AtmosphereExt::setVp(Mat4f vp)
{
	// atmProg.use();
	atmProg.setUniform("vp",vp);
}




















