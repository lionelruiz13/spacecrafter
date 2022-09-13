/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the LSS Team & Association Sirius
 * Copyright (C) 2020 of the LSS Team & Association Sirius
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

// max amount of illuminate simultaneously displayed
#define MAX_ILLUMINATE 120416

#include <vector>
#include <memory>

#include "tools/object.hpp"
#include "tools/fader.hpp"
#include "coreModule/illuminate.hpp"
#include "tools/no_copy.hpp"
#include "tools/SphereGrid.hpp"

class HipStarMgr;
class Navigator;
class ConstellationMgr;
class VertexArray;
class VertexBuffer;
class PipelineLayout;
class Pipeline;
class Set;

/*! \class IlluminateMgr
  * \brief handles all illuminate stars from Hipparcos catalog for better stars visualisation.
  * \author Olivier NIVOIX
  * \date 13 mai 2020
  *
  * @section DESCRIPTION
  * This class stores in the illuminatingGrid structure an illuminate, object aiming at covering a star with a texture
  * to make it more visible under the dome.
  *
  * The stars are from the Hipparcos catalog.
  *
  */
class IlluminateMgr: public NoCopy {
public:
	IlluminateMgr(std::shared_ptr<HipStarMgr> _hip_stars, Navigator *_navigator, std::shared_ptr<ConstellationMgr> _asterism);
	virtual ~IlluminateMgr();

	// indicates the default size of the illuminates
	void setDefaultSize(double v) {
		defaultSize =v;
	}

	// builds the illuminate of a star
	void load(int num, double size, double rotation);
	// constructs the illuminate of a star by indicating the color used
	void load(int num, const Vec3f& _color, double _size, double rotation);

	//! remove user added Illuminate
	void remove(unsigned int name);

	//! remove all user added Illuminate
	void removeAll();

	//! Draw all the Illuminate
	void draw(Projector *prj, const Navigator *nav);

	//! change the default texture of the illuminates by the file proposed in parameter
	void changeTex(const std::string& fileName);
	//!	remove the texture defined by the user
	void removeTex();

	//! load all the stars of the asterism of a constellation
	void loadConstellation(const std::string& abbreviation, double size, double rotation);
	//! load all the stars of the asterism of a constellation by imposing the color of the grouping
	void loadConstellation(const std::string& abbreviation, const Vec3f& color, double size, double rotation);
	//! load all stars of all asterisms of a sky_culture
	void loadAllConstellation(double size, double rotation);

	//! delete all the stars of the asterism of a constellation
	void removeConstellation(const std::string& abbreviation);
	//! removes all stars from all asterisms of a sky_culture
	void removeAllConstellation();

private:
	//! Load an individual Illuminate with all data
	void loadIlluminate(unsigned int name, double ra, double de, double angular_size, double r, double g, double b, float tex_rotation);
	void buildSet();

	#ifdef _MSC_VER // MSVC is not C++11 compliant, using copy for moving in resize and reserve
	SphereGrid<std::shared_ptr<Illuminate>> illuminateGrid;
	#else
	SphereGrid<std::unique_ptr<Illuminate>> illuminateGrid;
	#endif

	double defaultSize;							//!< defautl Size from illuninate if not precised


	std::shared_ptr<HipStarMgr> hip_stars;			//!< provide acces point to HipStarMgr
	Navigator* navigator = nullptr;				//!< provide acces point to Navigator
	std::shared_ptr<ConstellationMgr> asterism;		//!< provide acces point to ConstellationMgr

	int cmds[3];
	std::unique_ptr<PipelineLayout> m_layoutIllum;
	std::unique_ptr<Pipeline> m_pipelineIllum;
	std::unique_ptr<VertexArray> m_illumGL;
	std::unique_ptr<VertexBuffer> vertex;
	SubBuffer index;
	std::unique_ptr<Set> m_setIllum;

	std::shared_ptr<s_texture> currentTex;		//!< Pointer of texture used to draw
	std::shared_ptr<s_texture> defaultTex;		//!< Common texture if no other texture defined

	void createSC_context();
};

#endif // _ILLUMINATE_MGR_H_
