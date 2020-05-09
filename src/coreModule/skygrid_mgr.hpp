/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2016 of the LSS Team & Association Sirius
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


#ifndef SKYGRID_MGR_HPP
#define SKYGRID_MGR_HPP

#include <map>
#include <string>

#include "skygrid.hpp"
#include "tools/s_font.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/fader.hpp"
#include "coreModule/core_common.hpp"

// enum class GRID_TYPE : char {
// 	GRID_EQUATORIAL,
// 	GRID_ECLIPTIC,
// 	GRID_GALACTIC,
// 	GRID_ALTAZIMUTAL,
// 	GRID_UNKNOWN
// };


class SkyGridMgr {
public:
	SkyGridMgr();
	~SkyGridMgr();
	SkyGridMgr(SkyGridMgr const &) = delete;
	SkyGridMgr& operator = (SkyGridMgr const &) = delete;

	int size() {
		return m_map.size();
	};
	//Celle qui va créer les objets
	void Create(SKYGRID_TYPE type_obj);
	void draw(const Projector* prj);
	void update(int delta_time);

	void setFont(float font_size, const std::string& font_name);

	void setInternalNav(bool a);

	void setColor(SKYGRID_TYPE typeObj, const Vec3f& c);
	const Vec3f& getColor(SKYGRID_TYPE typeObj);

	//! change FlagShow: inverse la valeur du flag
	void setFlagShow(SKYGRID_TYPE typeObj, bool b);
	bool getFlagShow(SKYGRID_TYPE typeObj);
	void flipFlagShow(SKYGRID_TYPE typeObj);

	// fonctions de sauvegarde de l'état des grilles 
	void saveState(SkyGridSave &obj);
	void loadState(SkyGridSave &obj);
private:
	std::string typeToString(SKYGRID_TYPE typeObj);
	SKYGRID_TYPE stringToType(const std::string& typeObj);
	std::map<SKYGRID_TYPE ,SkyGrid*> m_map;
	Vec3f baseColor;
};
#endif
