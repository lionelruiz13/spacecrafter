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

#include "bodyModule/body.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

class Ring;
class Set;

class SmallBody : public Body {

public:
	SmallBody(std::shared_ptr<Body> parent,
	          const std::string& englishName,
	          BODY_TYPE _typePlanet,
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
			  const BodyTexture &_bodyTexture);

	virtual ~SmallBody();

	virtual void selectShader ();

protected :
	void defineSet();
	//! Return set to bind, may change at every frame
	virtual Set &getSet(float screen_sz) override;
	virtual void drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest);

	std::unique_ptr<Set> set;
	std::unique_ptr<SharedBuffer<globalVertProj>> uGlobalVertProj; // night bump normal
	std::unique_ptr<SharedBuffer<globalFrag>> uGlobalFrag; // night bump normal
	std::unique_ptr<SharedBuffer<Vec3f>> uUmbraColor; // bump
	bool initialized = false;
};
