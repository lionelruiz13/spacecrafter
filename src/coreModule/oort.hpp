/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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

#ifndef ___OORT_HPP___
#define ___OORT_HPP___

#include <string>
#include <fstream>

#include "tools/fader.hpp"
#include "tools/vecmath.hpp"
#include <vector>
#include <memory>
#include "EntityCore/Resource/SharedBuffer.hpp"

//! Class which manages the Oort Cloud
class Navigator;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;

class Oort {
public:
	Oort();
	~Oort();

	//! displays the point cloud
	void draw(double distance,const Navigator *nav) noexcept;

	//! sets the color of the cloud
	void setColor(const Vec3f& c) {
		color = c;
		uFrag->get().color = color;
	}

	//! returns the color of the cloud
	const Vec3f& getColor() {
		return color;
	}

	//! update the fader
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! changes the duration of the fader
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	//! modify the fader
	void setFlagShow(bool b) {
		fader = b;
	}

	//! returns the value of the fader
	bool getFlagShow(void) const {
		return fader;
	}

	//! builds the cloud
	//! \param nbr the number of points in the cloud
	void populate(unsigned int nbr) noexcept;
	//! build draw command
	void build();
private:
	// initialize the shader and the vao-vbo
	void createSC_context();
	// uniform color of the cloud
	Vec3f color;
	// fader for display
	LinearFader fader;
	// coefficient on light intensity
	float intensity;
	unsigned int nbAsteroids;
	// Vulkan elements
	VkCommandBuffer cmds[3] {};
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Set> set;
	std::unique_ptr<VertexArray> m_dataGL;
	std::unique_ptr<VertexBuffer> vertex;
	std::unique_ptr<SharedBuffer<Mat4f>> uMat;
	struct frag {
		Vec3f color;
		float fader;
	};
	std::unique_ptr<SharedBuffer<frag>> uFrag;
};

#endif // ___OORT_HPP___
