/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the LSS Team & Association Sirius
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

#ifndef _ILLUMINATE_MGR_H_
#define _ILLUMINATE_MGR_H_

#include <vector>
#include "tools/object.hpp"
#include "tools/fader.hpp"
#include "coreModule/grid.hpp"
#include "illuminate.hpp"

class HipStarMgr;
class Navigator;

/*! \class IlluminateMgr
  * \brief IlluminateMgr handles all illumiante stars for better stars visualisation.
  */
class IlluminateMgr {
public:
	IlluminateMgr(HipStarMgr *_hip_stars, Navigator *_navigator);
	virtual ~IlluminateMgr();
	IlluminateMgr(IlluminateMgr const &) = delete;
	IlluminateMgr& operator = (IlluminateMgr const &) = delete;

	//! search by name and return an Illuminate object
	Illuminate *search(unsigned int name);

	void setDefaultSize(double v) {
		defaultSize =v;
	}

	void load(int num, double size, double rotation);
	void load(int num, const Vec3f& _color, double size, double rotation);

	//! remove user added Illuminate and optionally unhide the original of the same name
	void remove(unsigned int name);

	//! remove all user added Illuminate
	void removeAll();

	//! Draw all the Illuminate
	void draw(Projector *prj, const Navigator *nav);

	//! change la texture des illuminate par le fichier proposé en paramètre
	void changeTex(const std::string& fileName);
	//!	supprime la texture définie par l'utilisateur
	void removeTex();

private:
	//! Load an individual Illuminate with all data
	void loadIlluminate(unsigned int name, double ra, double de, double angular_size, double r, double g, double b, double tex_rotation);

	std::vector<Illuminate*> illuminateArray; 		//!< The Illuminate list
	std::vector<Illuminate*>* illuminateZones;		//!< array of Illuminate vector with the grid id as array rank
	LittleGrid illuminateGrid;					//!< Grid for opimisation
	double defaultSize;

	shaderProgram* shaderIllum;
	HipStarMgr* hip_stars = nullptr;
	Navigator* navigator = nullptr;

	DataGL Illum;
	std::vector<float> illumPos;
	std::vector<float> illumTex;
	std::vector<float> illumColor;

	s_texture * currentTex = nullptr;			//!< Pointer of texture used to draw
	s_texture * defaultTex = nullptr;		//!< Common texture if no other texture defined
	s_texture * userTex = nullptr;				//!< Texture define by user 

	void createShader();
	void deleteShader();
};

#endif // _ILLUMINATE_MGR_H_
