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

class Set;
class Uniform;
class Buffer;

class Moon : public Body {

public:

	Moon(Body *parent,
	     const std::string& englishName,
	     bool flagHalo,
	     double radius,
	     double oblateness,
	     BodyColor* _myColor,
	     float _sol_local_day,
	     float albedo,
	     Orbit *orbit,
	     bool close_orbit,
	     ObjL* _currentObj,
	     double orbit_bounding_radius,
		 BodyTexture* _bodyTexture,
		 ThreadContext *context
		 );

	virtual ~Moon();

	virtual void selectShader();

protected :

	virtual void handleVisibilityFader(const Observer* observatory, const Projector* prj, const Navigator * nav) override;

	virtual void drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

	virtual void drawTrail(const Navigator* nav, const Projector* prj) override {
		return;
	}

	int commandIndex = -2;
	std::unique_ptr<Set> set;
	std::unique_ptr<Uniform> uGlobalVertProj; // night bump normal tes
	std::unique_ptr<Uniform> uGlobalFrag; // night bump normal
	std::unique_ptr<Uniform> uUmbraColor; // bump
	std::unique_ptr<Uniform> uMoonFrag, uGlobalTescGeom; // moon
	globalVertProj *pGlobalVertProj = nullptr;
	globalFrag *pGlobalFrag = nullptr;
	Vec3f *pUmbraColor = nullptr;
	moonFrag *pMoonFrag = nullptr;
	globalTescGeom *pGlobalTescGeom = nullptr;
	std::unique_ptr<Buffer> drawData;
	//shaderProgram *myShaderProg;	// Shader moderne
	s_texture * tex_night=nullptr;			// for moon with night event to see
};
