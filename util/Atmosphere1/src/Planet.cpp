#include "Planet.hpp"
#include "s_texture.hpp"

using namespace std;

//! Planet constructor
//! creates a planet.
//! @param data gives texture informations (path and texture units)
Planet::Planet(const PlanetData& data):
	Model3D(),
	textureUnit(data.textureUnit),
	heightmapUnit(data.heightmapUnit),
	normalsUnit(data.normalsUnit){
	sunPosition.set(0,0,0);

	program.init("../data/shaders/planet.vert","../data/shaders/planet.tesc","../data/shaders/planet.tese","../data/shaders/planet.geom","../data/shaders/planet.frag");

	program.setUniformLocation("model");
	program.setUniformLocation("view");
	program.setUniformLocation("projection");
	program.setUniformLocation("resolution");

	//NEW UNIFORMS
	program.setUniformLocation("vp");

	//NEW
	glActiveTexture(GL_TEXTURE0+ textureUnit);
	textures.insert(pair<int, s_texture*>(textureUnit, new s_texture(data.texturePath.c_str())));
	glActiveTexture(GL_TEXTURE0+ normalsUnit);
	textures.insert(pair<int, s_texture*>(normalsUnit, new s_texture(data.normalsPath.c_str())));
	glActiveTexture(GL_TEXTURE0+ heightmapUnit);
	textures.insert(pair<int, s_texture*>(heightmapUnit, new s_texture(data.heightmapPath.c_str())));

	textureID   =glGetUniformLocation(program.getProgram(),"diffuseMap");
	normalsID   =glGetUniformLocation(program.getProgram(),"normalMap");
	heightmapID =glGetUniformLocation(program.getProgram(),"heightMap");

	load_OBJ("../data/models/sphere64x32.obj","../data/textures/earthmap1k.bmp");
}

//! Planet destructor.https://www.youtube.com/watch?v=AiMF74z_Q3s
Planet::~Planet()
{
	//glDeleteBuffers(1,&vbo);
	//glDeleteVertexArrays(1,&vao);
}

//! Draw a planet.
//! with Tessellation means it's different from the draw function inherited from model3D.cpp.
//! @param model the model matrix.
//! @param view the wiew matrix.
//! @param projection the projection matrix.
//! @param resolution the screen size.
void Planet::drawWithTessellation(const Mat4f&model,const Mat4f&view,const Mat4f&projection,const Vec2f&resolution)
{


	/*glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	program.use();
	glBindFragDataLocation(program.getProgram(),0,"color");

	glUniform1i(textureID,textureUnit);
	glUniform1i(heightmapID,heightmapUnit);
	glUniform1i(normalsID,normalsUnit);

	glActiveTexture(GL_TEXTURE0+ textureUnit  );
	glBindTexture(GL_TEXTURE_2D,textures[textureUnit]->getID());
	glActiveTexture(GL_TEXTURE0+ normalsUnit  );
	glBindTexture(GL_TEXTURE_2D, textures[normalsUnit]->getID());
	glActiveTexture(GL_TEXTURE0+ heightmapUnit);
	glBindTexture(GL_TEXTURE_2D,textures[heightmapUnit]->getID());

	glActiveTexture(GL_TEXTURE0);

	/////////////////////////////////////////////////////
	//                                                 //
	//        IMPORTANT: adjust this parameter!        //
	//                                                 //
	/////////////////////////////////////////////////////
	Planet::bindBuffersAndDraw(GL_PATCHES);
	//Planet::bindBuffersAndDraw(GL_TRIANGLES);
}


void Planet::useProgram()
{
	program.use();
}


void Planet::setModel(Mat4f model){
	program.use();
	program.setUniform("model",model);
}

void Planet::setVp(Mat4f vp){
	program.use();
	program.setUniform("vp",vp);
}



















































