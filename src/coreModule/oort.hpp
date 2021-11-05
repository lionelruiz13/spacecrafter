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

	//! affiche le nuage de points
	void draw(double distance,const Navigator *nav) noexcept;

	//! fixe la couleur du nuage
	void setColor(const Vec3f& c) {
		color = c;
		uFrag->get().color = color;
	}

	//! renvoie la couleur du nuage
	const Vec3f& getColor() {
		return color;
	}

	//! mise à jour du fader
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! modifie la durée du fader
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	//! modifie le fader
	void setFlagShow(bool b) {
		fader = b;
	}

	//! renvoie la valeur du fader
	bool getFlagShow(void) const {
		return fader;
	}

	//! construit le nuage
	//! \param nbr le nombre de points dans le nuage
	void populate(unsigned int nbr) noexcept;
	//! build draw command
	void build();
private:
	// initialise le shader et les vao-vbo
	void createSC_context();
	// couleur uniforme du nuage
	Vec3f color;
	// fader pour affichage
	LinearFader fader;
	// coefficient sur l'intensité lumineuse
	float intensity;
	unsigned int nbAsteroids;
	// Vulkan elements
	VkCommandBuffer cmds[3];
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
