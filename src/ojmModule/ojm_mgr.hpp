/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 of the LSS Team && Immersive Adventure
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

#ifndef OJM_MGR_HPP
#define OJM_MGR_HPP

#include <vector>
#include "ojmModule/ojm.hpp"
#include "tools/shader.hpp"

class Projector;
class Navigator;

class OjmMgr {
public:
	enum STATE_POSITION {
		IN_UNIVERSE,
		IN_GALAXY,
		OTHER
	};

	OjmMgr();
	virtual ~OjmMgr();
	OjmMgr(OjmMgr const &) = delete;
	OjmMgr& operator = (OjmMgr const &) = delete;

	bool load(const std::string &mode, const std::string &name, const std::string &fileName, const std::string &pathFile, Vec3f Position, float multiplier = 1.0f);

	bool remove(const std::string &mode, const std::string& name);

	void removeAll(const std::string &mode);

	void update(int delta_time);

	void draw(Projector *prj, const Navigator *nav, STATE_POSITION state);

	void init()
	{
		createShader();
	}

private:
	OjmMgr::STATE_POSITION convert(const std::string & value);

	void createShader();
	void deleteShader();

	bool remove(STATE_POSITION state, const std::string& name);

	void removeAll(STATE_POSITION state);

	struct OjmContainer
	{
		Ojm *Obj3D;
		std::string name;
		Mat4f model;
		STATE_POSITION myState;
	};
	Mat4f view, normal, proj;
	std::vector<OjmContainer*> OjmVector;
	shaderProgram* shaderOJM;
};

#endif
