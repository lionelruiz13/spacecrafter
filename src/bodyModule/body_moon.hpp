/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
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
#ifndef BODY_MOON_HPP_
#define BODY_MOON_HPP_

#include "bodyModule/body.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

class Set;

class Moon : public Body {

public:

	Moon(std::shared_ptr<Body> parent,
	     const std::string& englishName,
	     bool flagHalo,
	     double radius,
	     double oblateness,
	     std::unique_ptr<BodyColor> _myColor,
	     float _sol_local_day,
	     float albedo,
	     std::unique_ptr<Orbit> orbit,
	     bool close_orbit,
	     ObjL* _currentObj,
	     double orbit_bounding_radius,
		 const BodyTexture &_bodyTexture
		 );

	virtual ~Moon();

	virtual void selectShader();

protected :
	void defineSet();
	//! Return set to bind, may change at every frame
	virtual Set &getSet(float screen_sz) override;

	virtual void handleVisibilityFader(const Observer* observatory, const Projector* prj, const Navigator * nav) override;

	virtual void drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest);

	virtual void drawTrail(VkCommandBuffer cmd, const Navigator* nav, const Projector* prj) override {
		return;
	}

	std::unique_ptr<Set> set;
	std::unique_ptr<SharedBuffer<globalVertProj>> uGlobalVertProj; // night bump normal tes
	std::unique_ptr<SharedBuffer<globalFrag>> uGlobalFrag; // night bump normal
	std::unique_ptr<SharedBuffer<Vec3f>> uUmbraColor; // bump
	std::unique_ptr<SharedBuffer<moonFrag>> uMoonFrag;
	std::unique_ptr<SharedBuffer<globalTescGeom>> uGlobalTescGeom; // moon
	std::unique_ptr<s_texture> tex_night=nullptr;			// for moon with night event to see
};

#endif /* end of include guard: BODY_MOON_HPP_ */
