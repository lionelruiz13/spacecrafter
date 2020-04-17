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



IlluminateMgr::IlluminateMgr()
{
	illuminateZones = new std::vector<Illuminate*>[illuminateGrid.getNbPoints()];

	if (!Illuminate::illuminateTex)
		Illuminate::illuminateTex = new s_texture("star_illuminate.png");
	if (!Illuminate::illuminateTex)
		cLog::get()->write("Error loading texture illuminateTex", LOG_TYPE::L_ERROR);

	//meme souci que pour skylineMgr
	Illuminate::createShader();
}

IlluminateMgr::~IlluminateMgr()
{
	std::vector<Illuminate *>::iterator iter;
	for (iter=illuminateArray.begin(); iter!=illuminateArray.end(); iter++) {
		delete (*iter);
	}

	if (Illuminate::illuminateTex) delete Illuminate::illuminateTex;
	Illuminate::illuminateTex = nullptr;

	delete[] illuminateZones;

	Illuminate::deleteShader();
}

// Load individual Illuminate for script
bool IlluminateMgr::loadIlluminate(const std::string& filename, double ra, double de,  double angular_size, const std::string& name, double r, double g, double b, float tex_rotation)
{
	if (angular_size<1.0)
		angular_size=defaultSize;

	Illuminate *e = searchIlluminate(name);
	if(e)
		removeIlluminate(name);

	e = new Illuminate;

	if(!e->createIlluminate(filename, ra, de, angular_size, name, r, b, g, tex_rotation)) {
		cLog::get()->write("Illuminate_mgr: Error while creating Illuminate " + e->Name, LOG_TYPE::L_ERROR);
		delete e;
		return false;
	} else {
		illuminateArray.push_back(e);
		illuminateZones[illuminateGrid.GetNearest(e->XYZ)].push_back(e);
		return true;
	}
}

// Clear user added Illuminate
void IlluminateMgr::removeIlluminate(const std::string& name)
{
	std::string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	std::vector <Illuminate*>::iterator iter;
	std::vector <Illuminate*>::iterator iter2;

	// iter = std::find_if(illuminateArray.begin(),illuminateArray.end(), [&uname](auto &ptr) { return ptr->getName() == uname; });
	// if (iter == illuminateArray.end())	{
	// 	cLog::get()->write("Requested Illuminate to delete not found by name " + uname, LOG_TYPE::L_INFO);
	// 	return;
	// }
	for (iter = illuminateArray.begin(); iter != illuminateArray.end(); ++iter) {
		std::string testName = (*iter)->getName();

		if (testName==uname) {
			// erase from locator grid
			int zone = illuminateGrid.GetNearest((*iter)->XYZ);

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
	cLog::get()->write("Requested Illuminate to delete not found by name " + uname, LOG_TYPE::L_INFO);
	// return "Requested Illuminate to delete not found by name.";
}

// remove all user added Illuminate
void IlluminateMgr::removeAllIlluminate()
{
	std::vector<Illuminate *>::iterator iter;
	std::vector<Illuminate *>::iterator iter2;

	for (iter=illuminateArray.begin(); iter!=illuminateArray.end(); /*iter++*/) {
		// erase from locator grid
		int zone = illuminateGrid.GetNearest((*iter)->XYZ);

		for (iter2 = illuminateZones[zone].begin(); iter2!=illuminateZones[zone].end(); ++iter2) {
			if(*iter2 == *iter) {
				illuminateZones[zone].erase(iter2);
				break;
			}
		}
		// Delete Illuminate
		delete *iter;
		iter = illuminateArray.erase(iter);
		//iter--;
	}
	// return "";
}

// Draw all the Illuminate
void IlluminateMgr::draw(Projector* prj, const Navigator * nav)
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);
	Vec3f pXYZ;

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
			prj->projectJ2000(n->XYZ,n->XY);
			n->drawTex(prj, nav);
		}
	}
}

// search by name
Illuminate *IlluminateMgr::searchIlluminate(const std::string& name)
{
	std::string uname = name;
	transform(uname.begin(), uname.end(), uname.begin(), ::toupper);
	std::vector <Illuminate*>::const_iterator iter;

	for (iter = illuminateArray.begin(); iter != illuminateArray.end(); ++iter) {
		std::string testName = (*iter)->getName();
		transform(testName.begin(), testName.end(), testName.begin(), ::toupper);
		if (testName==uname  ) return *iter;
	}
	return nullptr;
}
