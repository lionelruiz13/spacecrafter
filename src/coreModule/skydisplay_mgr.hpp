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


#ifndef SKYDISPLAY_MGR_HPP
#define SKYDISPLAY_MGR_HPP

#include <map>
#include <string>

#include "skyDisplay.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/fader.hpp"
#include "coreModule/core_common.hpp"
#include "tools/no_copy.hpp"
#include "tools/vecmath.hpp"

class shaderProgram;
class s_font;

class SkyDisplayMgr: public NoCopy  {
public:
	SkyDisplayMgr();
	~SkyDisplayMgr();

	int size() {
		return m_map.size();
	};
	//Celle qui va créer les objets
	void Create(SKYDISPLAY_NAME nameObj);
	void draw(const Projector *prj,const Navigator *nav, Vec3d equPos, Vec3d oldEquPos);
	void drawPerson(const Projector *prj,const Navigator *nav);
	void update(int delta_time);
	void clear(SKYDISPLAY_NAME nameObj);
	void loadData(SKYDISPLAY_NAME nameObj, const std::string& filename);
	void loadString(SKYDISPLAY_NAME nameObj, const std::string& dataStr);

	void setFont(float font_size, const std::string& font_name);

	void setColor(SKYDISPLAY_NAME nameObj, const Vec3f& c);
	const Vec3f& getColor(SKYDISPLAY_NAME nameObj);

	//! change FlagShow: inverse la valeur du flag
	void setFlagShow(SKYDISPLAY_NAME nameObj, bool b);
	bool getFlagShow(SKYDISPLAY_NAME nameObj);
	void flipFlagShow(SKYDISPLAY_NAME nameObj);

	// fonctions de sauvegarde de l'état de SkyDisplay
	void saveState(SkyDisplaySave &obj);
	void loadState(SkyDisplaySave &obj);

private:
	std::string getSkyName(SKYDISPLAY_NAME nameObj);
	std::map<SKYDISPLAY_NAME,SkyDisplay*> m_map;
	SkyDisplay* personAL = nullptr;
	SkyDisplay* personEQ = nullptr;
	Vec3f baseColor=Vec3f(0.f, 0.f, 0.f);
	s_font* font = nullptr;
};
#endif //SKYDISPLAY_MGR_HPP
