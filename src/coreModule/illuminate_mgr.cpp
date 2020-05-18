/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <fstream>
#include <algorithm>
#include "coreModule/illuminate_mgr.hpp"
#include "coreModule/illuminate.hpp"
#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include "tools/fmath.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "starModule/hip_star_mgr.hpp"

//a copy of zone_array.hpp
#define NR_OF_HIP 120416

IlluminateMgr::IlluminateMgr(HipStarMgr * _hip_stars, Navigator* _navigator, ConstellationMgr* _asterism)
{
	hip_stars = _hip_stars;
	navigator = _navigator;
	asterism = _asterism;

	illuminateZones = new std::vector<Illuminate*>[illuminateGrid.getNbPoints()];

	defaultTex = new s_texture("star_illuminate.png", TEX_LOAD_TYPE_PNG_BLEND3 );
	if (defaultTex ==nullptr)
		cLog::get()->write("Error loading texture illuminateTex", LOG_TYPE::L_ERROR);

	currentTex = defaultTex;
	createShader();
}

IlluminateMgr::~IlluminateMgr()
{
	std::vector<Illuminate *>::iterator iter;
	for (iter=illuminateArray.begin(); iter!=illuminateArray.end(); iter++) {
		delete (*iter);
	}

	if (defaultTex) delete defaultTex;
	defaultTex = nullptr;

	delete[] illuminateZones;

	deleteShader();
}

// Load individual Illuminate for script
void IlluminateMgr::load(int num, double size, double rotation)
{
	if (num>NR_OF_HIP)
		return;
	Object selected_object = hip_stars->searchHP(num).get();
	Vec3f color = selected_object.getRGB();
	//std::cout << num << " ra/de " << ra << " " << de << " mag " << mag << " color " << color[0]<< ":"<< color[1]<< ":"<< color[2]<< std::endl;
	//std::cout <<num << " only" <<std::endl;
	load(num, color, size, rotation);	
}

void IlluminateMgr::loadConstellation(const std::string& abbreviation, double size, double rotation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAbbreviation(abbreviation, HPStars);
	for (auto i: HPStars)
		this->load(i,size,rotation);
}

void IlluminateMgr::loadConstellation(const std::string& abbreviation, const Vec3f& color, double size, double rotation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAbbreviation(abbreviation, HPStars);
	for (auto i: HPStars)
		this->load(i, color, size, rotation);
}

void IlluminateMgr::loadAllConstellation(double size, double rotation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAll(HPStars);
	for (auto i: HPStars)
		this->load(i,size,rotation);
}


void IlluminateMgr::removeConstellation(const std::string& abbreviation)
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAbbreviation(abbreviation, HPStars);
	for (auto i: HPStars)
		this->remove(i);
}


void IlluminateMgr::removeAllConstellation()
{
	std::vector<unsigned int> HPStars;
	asterism->getHPStarsFromAll(HPStars);
	for (auto i: HPStars)
		this->remove(i);
}

void IlluminateMgr::load(int num, const Vec3f& _color, double _size, double rotation)
{
	if (num>NR_OF_HIP)
		return;
	Object selected_object = hip_stars->searchHP(num).get();
	//Vec3f color = selected_object.getRGB();
	double ra, de;
	selected_object.getRaDeValue(navigator,&ra,&de);
	double size = _size;
	//setup size
	if (size<1.0) {
		float mag = selected_object.getMag(navigator);
		size = defaultSize + 5.0 * (6.5-mag);
	}
	//std::cout << num << " ra/de " << ra << " " << de << " mag " << mag << " color " << color[0]<< ":"<< color[1]<< ":"<< color[2]<< std::endl;
	//std::cout << num << " with color" << std::endl;
	loadIlluminate(num, ra, de, size, _color[0], _color[1], _color[2], rotation );
}


// Load individual Illuminate 
void IlluminateMgr::loadIlluminate(unsigned int name, double ra, double de,  double angular_size, double r, double g, double b, double tex_rotation)
{
	if (angular_size<1.0)
		angular_size=defaultSize;

	Illuminate *e = search(name);
	if(e)
		remove(name);

	e = new Illuminate;

	if(e->createIlluminate(name, ra, de, angular_size, r, b, g, tex_rotation)) {
		illuminateArray.push_back(e);
		illuminateZones[illuminateGrid.GetNearest(e->getXYZ())].push_back(e);
	} else {
		cLog::get()->write("Illuminate_mgr: Error while creating Illuminate " + e->getName(), LOG_TYPE::L_ERROR);
		delete e;
	}		
}

// Clear user added Illuminate
void IlluminateMgr::remove(unsigned int name)
{
	std::vector <Illuminate*>::iterator iter;
	std::vector <Illuminate*>::iterator iter2;

	for (iter = illuminateArray.begin(); iter != illuminateArray.end(); ++iter) {

		if ((*iter)->getName() == name) {
			// erase from locator grid
			int zone = illuminateGrid.GetNearest((*iter)->getXYZ());

			for (iter2 = illuminateZones[zone].begin(); iter2!=illuminateZones[zone].end(); ++iter2) {
				if(*iter2 == *iter) {
					illuminateZones[zone].erase(iter2);
					break;
				}
			}
			// Delete Illuminate
			delete *iter;
			illuminateArray.erase(iter);
			//cLog::get()->write("Illuminate_mgr: Erased Illuminate " + uname, LOG_TYPE::L_INFO);
			return;
		}
	}
	cLog::get()->write("Requested Illuminate to delete not found by name " + name, LOG_TYPE::L_INFO);
}

// remove all user added Illuminate
void IlluminateMgr::removeAll()
{
	std::vector<Illuminate *>::iterator iter;
	std::vector<Illuminate *>::iterator iter2;

	for (iter=illuminateArray.begin(); iter!=illuminateArray.end();) {
		// erase from locator grid
		int zone = illuminateGrid.GetNearest((*iter)->getXYZ());

		for (iter2 = illuminateZones[zone].begin(); iter2!=illuminateZones[zone].end(); ++iter2) {
			if(*iter2 == *iter) {
				illuminateZones[zone].erase(iter2);
				break;
			}
		}
		// Delete Illuminate
		delete *iter;
		iter = illuminateArray.erase(iter);
	}
}

// Draw all the Illuminate
void IlluminateMgr::draw(Projector* prj, const Navigator * nav)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Vec3f pXYZ;

	illumPos.clear();
	illumTex.clear();
	illumColor.clear();

	// Find the star zones which are in the screen
	int nbZones=0;
	float max_fov = myMax( prj->getFov(), prj->getFov()*prj->getViewportWidth()/prj->getViewportHeight());
	nbZones = illuminateGrid.Intersect(nav->getPrecEquVision(), max_fov*C_PI/180.f*1.2f);

	static int * zoneList = illuminateGrid.getResult();

	// Print all the stars of all the selected zones
	static std::vector<Illuminate *>::iterator end;
	static std::vector<Illuminate *>::iterator iter;
	Illuminate* n;

	for (int i=0; i<nbZones; ++i) {
		end = illuminateZones[zoneList[i]].end();
		for (iter = illuminateZones[zoneList[i]].begin(); iter!=end; ++iter) {
			n = *iter;
			//prj->projectJ2000(n->XYZ,n->XY);
			n->draw(prj, illumPos, illumTex, illumColor );
		}
	}

	int nbrIllumToTrace = illumPos.size()/12;
	// std::cout << "Illuminate à tracer: il y a " << nbrIllumToTrace << std::endl;
	// std::cout << "illumPos   size : " << illumPos.size() << std::endl;
	// std::cout << "illumTex   size : " << illumTex.size() << std::endl;
	// std::cout << "illumColor size : " << illumColor.size() << std::endl;
	shaderIllum->use();

	// if (specialTex)
	// 	glBindTexture(GL_TEXTURE_2D, illuminateSpecialTex->getID());
	// else
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentTex->getID());

	shaderIllum->setUniform("ModelViewMatrix", prj->getMatJ2000ToEye());

	glBindVertexArray(Illum.vao);
	glBindBuffer(GL_ARRAY_BUFFER,Illum.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*illumPos.size(), illumPos.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,Illum.tex);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*illumTex.size(), illumTex.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,Illum.color);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*illumColor.size(), illumColor.data() ,GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,NULL);

	for(int i=0;i<nbrIllumToTrace; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, 4*i, 4);
	shaderIllum->unuse();
}

// search by name
Illuminate *IlluminateMgr::search(unsigned int name)
{
	for (auto iter = illuminateArray.begin(); iter != illuminateArray.end(); ++iter) {
		if ((*iter)->getName()== name)
			return *iter;
	}
	return nullptr;
}

void IlluminateMgr::createShader()
{
	//======raw========
	shaderIllum = new shaderProgram();
	shaderIllum->init( "illuminate.vert", "illuminate.frag");
	shaderIllum->setUniformLocation("ModelViewMatrix");

	glGenVertexArrays(1,&Illum.vao);
	glBindVertexArray(Illum.vao);

	glGenBuffers(1,&Illum.pos);
	glGenBuffers(1,&Illum.tex);
	glGenBuffers(1,&Illum.color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}


void IlluminateMgr::deleteShader()
{
	if (shaderIllum) delete shaderIllum;

	glDeleteBuffers(1,&Illum.pos);
	glDeleteBuffers(1,&Illum.tex);
	glDeleteBuffers(1,&Illum.color);
	glDeleteVertexArrays(1,&Illum.vao);
}



void IlluminateMgr::changeTex(const std::string& fileName)
{
	this->removeTex();
	userTex = new s_texture(fileName, TEX_LOAD_TYPE_PNG_BLEND3 );
	if (userTex==nullptr) {
		cLog::get()->write("illuminate: error when loading user texture "+ fileName, LOG_TYPE::L_ERROR, LOG_FILE::SCRIPT);
	}
	currentTex = userTex;
}

		
void IlluminateMgr::removeTex()
{
	if (currentTex == defaultTex) //nothing to do
		return;
	// here, userTex is used
	currentTex = defaultTex;
	delete userTex;
	userTex = nullptr;
}
