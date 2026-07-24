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
#include <random>

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

//! Frozen-seed PRNG factory for the oort point cloud - THE single seed authority
//! (I2) that makes the two paths' clouds POINT-identical. Returns a FRESH,
//! DEDICATED std::mt19937 seeded with the frozen constant (defined once in
//! oort.cpp). Because every caller seeds its own generator from the same
//! constant and consumes it in the same order, both paths draw the IDENTICAL
//! point sequence - cross-path AND cross-launch deterministic - regardless of
//! any other rand()/PRNG activity in the process (a dedicated instance is immune
//! to global-rand interleaving, which is exactly what broke cross-path identity
//! under the historical shared rand() stream). Seed a generator ONCE before the
//! populate loop and pass it to every oortSamplePoint() call.
std::mt19937 oortRng() noexcept;

//! Spatial law of the oort point cloud - THE single authority (I2) for the
//! cloud's geometry, shared between the old-path Oort (below) and the new-path
//! OortModule (B5 §6.9 content-migration pilot). One call draws one point in
//! heliocentric-ecliptic AU (spheToRect of the historical theta/phi/radius
//! distribution, verbatim from the old populate loop) from the caller-supplied
//! frozen-seed generator (oortRng above): three draws, azimuthally uniform in
//! theta so the cloud is invariant under any z-rotation (why the new path's
//! near-regime surface fold is visually inert). Both paths materialize their own
//! buffer from this law, each seeding a dedicated generator from the SAME frozen
//! constant, so the clouds are POINT-identical (B5-oort-2 [vixy 2026-07-24]:
//! turns cross-path pixel A/B into a valid instrument). NB the point positions
//! differ from the pre-2026-07-24 global-rand() cloud (the source of randomness
//! changed rand()->mt19937; the distribution SHAPE is unchanged) - authorized by
//! the directive; the OLD render path's gates/intensity/draw-order are untouched.
Vec3f oortSamplePoint(std::mt19937 &rng) noexcept;

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
