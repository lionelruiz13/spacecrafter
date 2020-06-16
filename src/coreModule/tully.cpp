/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/tully.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include <iomanip>
#include <math.h>
//#include "tools/fmath.hpp"
#include "tools/s_texture.hpp"
#include "tools/OpenGL.hpp"


Tully::Tully()
{
	texGalaxy = nullptr;
	fader = true;
	createShaderPoints();
	createShaderSquare();
	nbGalaxy=0;
	nbTextures = 0;
}

Tully::~Tully()
{
	if (texGalaxy!=nullptr)
		delete texGalaxy;

	posTully.clear();
	colorTully.clear();
	texTully.clear();
	scaleTully.clear();

	posTmpTully.clear();
	texTmpTully.clear();
	radiusTmpTully.clear();

	// deleteShaderSquare();
	// deleteShaderPoints();
}

void Tully::createShaderPoints()
{
	shaderPoints = new shaderProgram();
	shaderPoints->init("tully.vert","tully.geom","tully.frag");
	shaderPoints->setUniformLocation({"Mat", "fader", "camPos", "nbTextures"});

	shaderPoints->setSubroutineLocation(GL_FRAGMENT_SHADER, "useCustomColor");
	shaderPoints->setSubroutineLocation(GL_FRAGMENT_SHADER, "useWhiteColor");
	
	m_pointsGL = std::make_unique<VertexArray>();
	m_pointsGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_pointsGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
	m_pointsGL->registerVertexBuffer(BufferType::COLOR, BufferAccess::STATIC);
	m_pointsGL->registerVertexBuffer(BufferType::SCALE, BufferAccess::STATIC);
	// glGenVertexArrays(1,&m_pointsGL.vao);
	// glBindVertexArray(m_pointsGL.vao);
	// glGenBuffers(1,&m_pointsGL.pos);
	// glGenBuffers(1,&m_pointsGL.color);
	// glGenBuffers(1,&m_pointsGL.tex);
	// glGenBuffers(1,&m_pointsGL.scale);
	// glEnableVertexAttribArray(0);
	// glEnableVertexAttribArray(1);
	// glEnableVertexAttribArray(2);
	// glEnableVertexAttribArray(3);
}

// void Tully::deleteShaderPoints()
// {
// 	if(shaderPoints) shaderPoints=nullptr;

// 	glDeleteBuffers(1,&m_pointsGL.scale);
// 	glDeleteBuffers(1,&m_pointsGL.tex);
// 	glDeleteBuffers(1,&m_pointsGL.color);
// 	glDeleteBuffers(1,&m_pointsGL.pos);
// 	glDeleteVertexArrays(1,&m_pointsGL.vao);
// }

void Tully::createShaderSquare()
{
	shaderSquare = new shaderProgram();
	shaderSquare->init("tullyH.vert","tullyH.geom","tullyH.frag");
	shaderSquare->setUniformLocation("Mat");
	shaderSquare->setUniformLocation("fader");
	shaderSquare->setUniformLocation("nbTextures");

	m_squareGL =  std::make_unique<VertexArray>();
	m_squareGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
	m_squareGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::DYNAMIC);
	m_squareGL->registerVertexBuffer(BufferType::SCALE, BufferAccess::DYNAMIC);

	// glGenVertexArrays(1,&m_squareGL.vao);
	// glBindVertexArray(m_squareGL.vao);
	// glGenBuffers(1,&m_squareGL.pos);
	// glGenBuffers(1,&m_squareGL.scale);
	// glGenBuffers(1,&m_squareGL.tex);
	// glEnableVertexAttribArray(0);
	// glEnableVertexAttribArray(1);
	// glEnableVertexAttribArray(2);
}

// void Tully::deleteShaderSquare()
// {
// 	if(shaderSquare) shaderSquare=nullptr;

// 	glDeleteBuffers(1,&m_squareGL.tex);
// 	glDeleteBuffers(1,&m_squareGL.scale);
// 	glDeleteBuffers(1,&m_squareGL.pos);
// 	glDeleteVertexArrays(1,&m_squareGL.vao);
// }

bool Tully::loadCatalog(const std::string &cat) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERREUR: Impossible d'ouvrir le fichier " << cat << std::endl;
		cLog::get()->write("TULLY: Impossible d'ouvrir le fichier " + cat ,LOG_TYPE::L_ERROR);
		cLog::get()->write("TULLY: classe désactivée "+ cat ,LOG_TYPE::L_WARNING);
		return false;
	}

	std::string line; // variable which will contain each line of the file
	int index,typeGalaxy;
	float r,g,b,x,y,z,xr,yr,zr;

	/*
	*
	* Format de ligne : index , composantes (r, g ,b) entre [0;1]
	*					(x,y,z) coordonnées dans le repère et typeGalaxy: le type de l'objet
	*
	*	int, 3 floats, 3 floats, un int
	*/

	while (getline(file, line)) {
		if (line[0]=='#')
			continue;
		std::istringstream aGalaxie(line);
		aGalaxie >> index >> r >> g >> b >> x >> y >> z >> typeGalaxy;
		nbGalaxy++;

		xr=x;
		yr=y*cos(90*M_PI/180.0)-z*sin(90*M_PI/180.0);
		zr=y*sin(90*M_PI/180.0)+z*cos(90*M_PI/180.0);
		posTully.push_back(200*xr);
		posTully.push_back(200*yr);
		posTully.push_back(200*zr);

		colorTully.push_back(r);
		colorTully.push_back(g);
		colorTully.push_back(b);

		texTully.push_back(typeGalaxy);

		switch (typeGalaxy) {
			case 0  : scaleTully.push_back(2.0); break;  //Dwarf
			case 13 : scaleTully.push_back(4.0); break;  // LMC
			case 14 : scaleTully.push_back(4.0); break;  // SMC
			case 9  : scaleTully.push_back(75.0); break; // AG
			case 10 : scaleTully.push_back(128.0); break; // Dark NEB
			case 12 : scaleTully.push_back(128.0); break; // Bright NEB
			default : scaleTully.push_back(0.25); break; // GALAXY 
		}
	}

	file.close();

	// glBindVertexArray(m_pointsGL.vao);

	// glBindBuffer(GL_ARRAY_BUFFER,m_pointsGL.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*posTully.size(),posTully.data(),GL_STATIC_DRAW);
	// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_pointsGL.color);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*colorTully.size(),colorTully.data(),GL_STATIC_DRAW);
	// glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_pointsGL.tex);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*texTully.size(),texTully.data(),GL_STATIC_DRAW);
	// glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_pointsGL.scale);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*scaleTully.size(),scaleTully.data(),GL_STATIC_DRAW);
	// glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,0,NULL);

	m_pointsGL->fillVertexBuffer(BufferType::POS3D,posTully );
	m_pointsGL->fillVertexBuffer(BufferType::COLOR,colorTully );
	m_pointsGL->fillVertexBuffer(BufferType::TEXTURE,texTully );
	m_pointsGL->fillVertexBuffer(BufferType::SCALE,scaleTully );

	cLog::get()->write("Tully chargement réussi du catalogue : nombre d'items " + Utility::intToString(nbGalaxy) );

	isAlive = true;
	return true;
}


void Tully::setTexture(const std::string& tex_file)
{
	if (texGalaxy != nullptr) {
		delete texGalaxy;
		texGalaxy = nullptr;
	}
	texGalaxy =  new s_texture(/*1,*/tex_file,true);

	int width, height;
	texGalaxy->getDimensions(width, height);
	if (width ==0 || height ==0)
		nbTextures = 0;
	else
		nbTextures = width / height;
}

bool Tully::compTmpTully(const tmpTully &a,const tmpTully &b)
{
	if (a.distance>b.distance)
		return true;
	else
		return false;
}

void Tully::computeSquareGalaxies(Vec3f camPosition)
{
	float x,y,z,a,b,c,distance, radius;

	a = camPosition[0];
	b = camPosition[1];
	c = camPosition[2];
	for(unsigned int i=0; i< nbGalaxy;i++) {
		x=posTully[3*i];
		y=posTully[3*i+1];
		z=posTully[3*i+2];

		//on ne sélectionne que les galaxies assez grandes pour être affichées
        distance=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
		radius = 3.0/(distance*scaleTully[i]);
		if (radius<2)
			continue;

		/* OPTIMISATION : radius < 2 signifie que d²< (3/2scale)² */

		tmpTully tmp;
		tmp.position = Vec3f(x,y,z);
		tmp.distance = distance;
		tmp.radius = radius;
		tmp.texture = texTully[i];
		lTmpTully.push_back(tmp);
	}
	// printf("taille de la liste: %i\n", lTmpTully.size());
	lTmpTully.sort(compTmpTully);

	posTmpTully.clear();
	radiusTmpTully.clear();
	texTmpTully.clear();

	for (std::list<tmpTully>::iterator it=lTmpTully.begin(); it!=lTmpTully.end(); ++it) {
		posTmpTully.push_back((*it).position[0]);
		posTmpTully.push_back((*it).position[1]);
		posTmpTully.push_back((*it).position[2]);
		radiusTmpTully.push_back((*it).radius);
		texTmpTully.push_back((*it).texture);
	}
	
	lTmpTully.clear();	//données devenues inutiles

	// glBindBuffer(GL_ARRAY_BUFFER,m_squareGL.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*posTmpTully.size(),posTmpTully.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_squareGL.scale);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*radiusTmpTully.size(),radiusTmpTully.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_squareGL.tex);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*texTmpTully.size(),texTmpTully.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,0,NULL);

	m_squareGL->fillVertexBuffer(BufferType::POS3D,posTmpTully );
	m_squareGL->fillVertexBuffer(BufferType::TEXTURE,texTmpTully );
	m_squareGL->fillVertexBuffer(BufferType::SCALE,radiusTmpTully );
}


void Tully::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate()) return;
	if (!isAlive) return;

	Mat4f matrix= nav->getHelioToEyeMat().convert();
	camPos = nav->getObserverHelioPos();

	computeSquareGalaxies(camPos);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texGalaxy->getID());

	//tracé des galaxies de taille <1 px
	StateGL::disable(GL_DEPTH_TEST);
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderPoints->use();
	shaderPoints->setUniform("Mat",matrix);
	shaderPoints->setUniform("fader", fader.getInterstate());
	shaderPoints->setUniform("camPos", camPos);
	shaderPoints->setUniform("nbTextures", nbTextures);

	if (useWhiteColor)
		shaderPoints->setSubroutine(GL_FRAGMENT_SHADER, "useWhiteColor");
	else
		shaderPoints->setSubroutine(GL_FRAGMENT_SHADER, "useCustomColor");

	// glBindVertexArray(m_pointsGL.vao);
	m_pointsGL->bind();
	glDrawArrays(GL_POINTS, 0, nbGalaxy);
	m_pointsGL->unBind();
	shaderPoints->unuse();

	//tracé des galaxies de taille >1 px;
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
	glBlendEquation(GL_MAX);
	
	shaderSquare->use();
	shaderSquare->setUniform("Mat",matrix);
	shaderSquare->setUniform("fader", fader.getInterstate());
	shaderSquare->setUniform("nbTextures", nbTextures);

	// glBindVertexArray(m_squareGL.vao);

	// glBindBuffer(GL_ARRAY_BUFFER,m_squareGL.pos);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*posTmpTully.size(),posTmpTully.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_squareGL.scale);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*radiusTmpTully.size(),radiusTmpTully.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,NULL);

	// glBindBuffer(GL_ARRAY_BUFFER,m_squareGL.tex);
	// glBufferData(GL_ARRAY_BUFFER,sizeof(float)*texTmpTully.size(),texTmpTully.data(),GL_DYNAMIC_DRAW);
	// glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,0,NULL);

	m_squareGL->bind();
	glDrawArrays(GL_POINTS, 0, radiusTmpTully.size());
	m_squareGL->unBind();
	shaderSquare->unuse();

	glBlendEquation(GL_FUNC_ADD);
	StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE); // Normal transparency mode
}
