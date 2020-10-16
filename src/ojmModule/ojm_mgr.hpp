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
#include <memory>
#include "ojmModule/ojm.hpp"
#include "renderGL/shader.hpp"
#include "tools/no_copy.hpp"

class Projector;
class Navigator;
class Pipeline;
class PipelineLayout;
class CommandMgr;
class VertexArray;
class Uniform;
class Set;
class VirtualSurface;
class ThreadContext;

class OjmMgr: public NoCopy {
public:
	enum STATE_POSITION {
		IN_UNIVERSE,
		IN_GALAXY,
		OTHER
	};

	OjmMgr(ThreadContext *context);
	virtual ~OjmMgr();

	bool load(const std::string &mode, const std::string &name, const std::string &fileName, const std::string &pathFile, Vec3f Position, float multiplier = 1.0f);

	bool remove(const std::string &mode, const std::string& name);

	void removeAll(const std::string &mode);

	void update(int delta_time);

	void draw(Projector *prj, const Navigator *nav, STATE_POSITION state);

	void init(ThreadContext *context)
	{
		createShader(context);
	}

private:
	OjmMgr::STATE_POSITION convert(const std::string & value);

	void createShader(ThreadContext *context);
	// void deleteShader();

	bool remove(STATE_POSITION state, const std::string& name);

	void removeAll(STATE_POSITION state);

	//! rebuild command buffer
	void rebuild();

	struct OjmContainer
	{
		Ojm *Obj3D;
		std::string name;
		Mat4f model;
		STATE_POSITION myState;
		std::unique_ptr<Uniform> uniform;
		struct {
			Mat4f ModelViewMatrix;
			Mat4f NormalMatrix;
		} *pUniform;
	};
	Mat4f view, proj;
	VirtualSurface *surface;
	CommandMgr *cmdMgr;
	CommandMgr *cmdMgrMaster;
	Set *globalSet;
	STATE_POSITION actualState = OTHER; // mustn't match any possible state
	std::vector<OjmContainer*> OjmVector;
	//std::unique_ptr<shaderProgram> shaderOJM;
	std::unique_ptr<PipelineLayout> layout;
	Pipeline *pipeline;
	std::unique_ptr<VertexArray> vertex;
	std::unique_ptr<Set> set, pushSet;
	bool needRebuild = false; // for future commandMgr threading
	int commandIndex, commandIndexSwitch;
	std::unique_ptr<Uniform> uniformModel;
	int virtualUniformID;
};

#endif
