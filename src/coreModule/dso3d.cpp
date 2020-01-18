/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/

#include "coreModule/dso3d.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include <iomanip>
#include <math.h>
#include "tools/fmath.hpp"
#include "tools/s_texture.hpp"


Dso3d::Dso3d()
{
	texNebulae = nullptr;
	fader = true;
	createShader();
	nbNebulae=0;
}

Dso3d::~Dso3d()
{
	if (texNebulae!=nullptr)
		delete texNebulae;

	posDso3d.clear();
	scaleDso3d.clear();
	texDso3d.clear();
	deleteShader();
}

void Dso3d::createShader()
{
	shaderDso3d = new shaderProgram();
	shaderDso3d->init("dso3d.vert", "dso3d.geom","dso3d.frag");
	shaderDso3d->setUniformLocation("Mat");
	shaderDso3d->setUniformLocation("fader");
	shaderDso3d->setUniformLocation("camPos");
	shaderDso3d->setUniformLocation("nbTextures");
	
	glGenVertexArrays(1,&sData.vao);
	glBindVertexArray(sData.vao);
	glGenBuffers(1,&sData.pos);
	glGenBuffers(1,&sData.tex);
	glGenBuffers(1,&sData.scale);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

}

void Dso3d::deleteShader()
{
	if(shaderDso3d) delete shaderDso3d;

	glDeleteBuffers(1,&sData.scale);
	glDeleteBuffers(1,&sData.tex);
	glDeleteBuffers(1,&sData.pos);
	glDeleteVertexArrays(1,&sData.vao);
}

bool Dso3d::loadCatalog(const std::string &cat) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERREUR: Impossible d'ouvrir le fichier " << cat << std::endl;
		cLog::get()->write("DSO3D: Impossible d'ouvrir le fichier "+ cat ,LOG_TYPE::L_ERROR);
		return false;
	}

	std::string index, line; // variable which will contain each line of the file
	//~ cout << "Lecture du catalogue DSO"  << cat << std::endl;
	int typeNebula, xyz;
	float size,x,y,z,xr,yr,zr,alpha,delta,r;

	while (getline(file, line)) {
		if (line[0]=='#')
			continue;
		std::istringstream aNebulae(line);
		aNebulae >> index >> xyz >> alpha >> delta >> r >> size >> typeNebula;
		nbNebulae++;
		//x y z position
		if (xyz==1) {
			posDso3d.push_back(alpha);
			posDso3d.push_back(delta);
			posDso3d.push_back(r);
		} else {
		//x y z calculation
			x=r*cos(alpha*C_PI/180.0)*sin((90.0-delta)*C_PI/180.0);
			y=r*sin(alpha*C_PI/180.0)*sin((90.0-delta)*C_PI/180.0);
			z=r*cos((90.0-delta)*C_PI/180.0);
		// ecliptic coordinates change
			xr=x;
			yr=y*cos(-23.43928*C_PI/180.0)-z*sin(-23.43928*C_PI/180.0);
			zr=y*sin(-23.43928*C_PI/180.0)+z*cos(-23.43928*C_PI/180.0);

			posDso3d.push_back(xr/2.0);
			posDso3d.push_back(yr/2.0);
			posDso3d.push_back(zr/2.0);
		}
		//dso texture
		texDso3d.push_back(typeNebula);
		scaleDso3d.push_back(size);
	}
	file.close();

	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,sData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*posDso3d.size(),posDso3d.data(),GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,sData.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*texDso3d.size(),texDso3d.data(),GL_STATIC_DRAW);
	glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,sData.scale);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*scaleDso3d.size(),scaleDso3d.data(),GL_STATIC_DRAW);
	glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,0,NULL);

	return true;
}

void Dso3d::setTexture(const std::string& tex_file)
{
	if (texNebulae != nullptr) {
		delete texNebulae;
		texNebulae = nullptr;
	}
	texNebulae =  new s_texture(/*1,*/tex_file,true);

	int width, height;
	texNebulae->getDimensions(width, height);
	if (width ==0 || height ==0)
		nbTextures = 0;
	else
		nbTextures = width / height;
}


void Dso3d::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate()) return;
	if (nbNebulae==0) return;

	//~ StateGL::enable(GL_DEPTH_TEST);
	StateGL::enable(GL_BLEND);
	//~ StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	Mat4f matrix= nav->getHelioToEyeMat().convert();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texNebulae->getID());

	camPos = nav->getObserverHelioPos();

	shaderDso3d->use();
	shaderDso3d->setUniform("Mat",matrix);
	shaderDso3d->setUniform("fader", fader.getInterstate());
	shaderDso3d->setUniform("camPos", camPos);
	shaderDso3d->setUniform("nbTextures", nbTextures);

	glBindVertexArray(sData.vao);

	glDrawArrays(GL_POINTS, 0, nbNebulae);
	//~ StateGL::disable(GL_DEPTH_TEST);
	shaderDso3d->unuse();
}
