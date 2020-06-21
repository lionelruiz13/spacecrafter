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
* (c) 2017 - 2020 all rights reserved
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
#include "tools/s_texture.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"


Dso3d::Dso3d()
{
	texNebulae = nullptr;
	fader = true;
	createSC_context();
	nbNebulae=0;
}

Dso3d::~Dso3d()
{
	if (texNebulae!=nullptr)
		delete texNebulae;

	posDso3d.clear();
	scaleDso3d.clear();
	texDso3d.clear();
}

void Dso3d::createSC_context()
{
	shaderDso3d = std::make_unique<shaderProgram>();
	shaderDso3d->init("dso3d.vert", "dso3d.geom","dso3d.frag");
	shaderDso3d->setUniformLocation({"Mat", "fader", "camPos", "nbTextures"});
	
	sData = std::make_unique<VertexArray>();
	sData->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	sData->registerVertexBuffer(BufferType::MAG, BufferAccess::STATIC);
	sData->registerVertexBuffer(BufferType::SCALE, BufferAccess::STATIC);
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
			x=r*cos(alpha*M_PI/180.0)*sin((90.0-delta)*M_PI/180.0);
			y=r*sin(alpha*M_PI/180.0)*sin((90.0-delta)*M_PI/180.0);
			z=r*cos((90.0-delta)*M_PI/180.0);
		// ecliptic coordinates change
			xr=x;
			yr=y*cos(-23.43928*M_PI/180.0)-z*sin(-23.43928*M_PI/180.0);
			zr=y*sin(-23.43928*M_PI/180.0)+z*cos(-23.43928*M_PI/180.0);

			posDso3d.push_back(xr/2.0);
			posDso3d.push_back(yr/2.0);
			posDso3d.push_back(zr/2.0);
		}
		//dso texture
		texDso3d.push_back(typeNebula);
		scaleDso3d.push_back(size);
	}
	file.close();

	sData->fillVertexBuffer(BufferType::POS3D, posDso3d);
	sData->fillVertexBuffer(BufferType::SCALE, scaleDso3d);
	sData->fillVertexBuffer(BufferType::MAG, texDso3d);

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

	StateGL::enable(GL_BLEND);
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

	sData->bind();
	glDrawArrays(GL_POINTS, 0, nbNebulae);
	sData->unBind();
	shaderDso3d->unuse();
}
