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



/*! \class IlluminateMgr
  * \brief IlluminateMgr handles all illumiante stars for better stars visualisation.
  */
class IlluminateMgr {
public:
	IlluminateMgr();
	virtual ~IlluminateMgr();
	IlluminateMgr(IlluminateMgr const &) = delete;
	IlluminateMgr& operator = (IlluminateMgr const &) = delete;

	//! search by name and return an Illuminate object
	Illuminate *search(const std::string& name);

	void setDefaultSize(double v) {
		defaultSize =v;
	}

	//! Load an individual Illuminate from a script
	bool loadIlluminate(/*const std::string& filename, */double ra, double de, double angular_size, const std::string& name, double r, double g, double b, float tex_rotation);

	//! remove user added Illuminate and optionally unhide the original of the same name
	void removeIlluminate(const std::string& name);

	//! remove all user added Illuminate
	void removeAllIlluminate();

	//! Draw all the Illuminate
	void draw(Projector *prj, const Navigator *nav);

private:
	std::vector<Illuminate*> illuminateArray; 		//!< The Illuminate list
	std::vector<Illuminate*>* illuminateZones;		//!< array of Illuminate vector with the grid id as array rank
	littleGrid illuminateGrid;					//!< Grid for opimisation
	double defaultSize;
};

#endif // _ILLUMINATE_MGR_H_
