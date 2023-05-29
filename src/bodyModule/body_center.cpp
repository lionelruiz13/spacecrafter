/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
 * Copyright (C) 2017-2020 AssociationSirius
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

#include "bodyModule/body_center.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/halo.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body_color.hpp"
#include "navModule/observer.hpp"
#include "tools/sc_const.hpp"
#include "tools/s_font.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

Center::Center(std::shared_ptr<Body> parent,
         const std::string& englishName,
         bool flagHalo,
         double radius,
         double oblateness,
         std::unique_ptr<BodyColor> myColor,
         float _sol_local_day,
         float albedo,
         std::unique_ptr<Orbit> orbit,
         bool close_orbit,
         ObjL* _currentObj,
         double orbit_bounding_radius,
		 const BodyTexture &_bodyTexture):
	Sun(parent,
	     englishName,
	     flagHalo,
	     radius,
	     oblateness,
	     std::move(myColor),
	     _sol_local_day,
	     albedo,
	     std::move(orbit),
	     close_orbit,
	     _currentObj,
	     orbit_bounding_radius,
		_bodyTexture,
        CENTER)
{
}

Center::~Center()
{
}

// // Draw the Center and all the related infos : name, circle etc..
// void Center::computeDraw(const Projector* prj, const Navigator * nav)
// {
// 	eye_sun = nav->getHelioToEyeMat() * v3fNull;
//
// 	mat = mat_local_to_parent;
// 	parent_mat = Mat4d::identity();
//
// 	// This removed totally the Body shaking bug!!!
// 	mat = nav->getHelioToEyeMat() * mat;
// 	parent_mat = nav->getHelioToEyeMat() * parent_mat;
//
// 	eye_planet = mat * v3fNull;
//
// 	lightDirection = eye_sun - eye_planet;
// 	sun_half_angle = atan(696000.0/AU/lightDirection.length());  // hard coded Center radius!
// 	//	cout << sun_half_angle << " center angle on " << englishName << endl;
// 	lightDirection.normalize();
//
// 	// Compute the 2D position and check if in the screen
// 	screen_sz = getOnScreenSize(prj, nav);
//
// 	float screen_size_with_halo = screen_sz;
// 	if (big_halo_size > screen_sz)
// 		screen_size_with_halo = big_halo_size;
//
// 	isVisible = prj->projectCustomCheck(v3fNull, screenPos, mat, (int)(screen_size_with_halo/2));
//
// 	visibilityFader = isVisible;
//
// 	// Do not draw anything else if was not visible
// 	// Draw the name, and the circle if it's not too close from the body it's turning around
// 	// this prevents name overlaping (ie for jupiter satellites)
// 	ang_dist = 300.f*atan(get_ecliptic_pos().length()/getEarthEquPos(nav).length())/prj->getFov();
//
//     // Compute the distance to the observer
//     distance = eye_planet.length();
// }
