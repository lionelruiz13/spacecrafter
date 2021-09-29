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
#include "bodyModule/ring.hpp"

class Ring;
class Set;
class Uniform;
class Buffer;

class BigBody : public Body {


public:
	BigBody(std::shared_ptr<Body> parent,
	        const std::string& englishName,
	        BODY_TYPE _typePlanet,
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
			std::shared_ptr<BodyTexture> _bodyTexture,
			ThreadContext *context);

	virtual ~BigBody();

	void setRings(std::unique_ptr<Ring> r);

	virtual void selectShader ();

	// Return the radius of a circle containing the object on screen
	virtual float getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only = false);

	virtual double calculateBoundingRadius();

	virtual void setSphereScale(float s, bool initial_scale =  false);

	virtual void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr);

protected :

	virtual void drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

	virtual bool hasRings() {
		return rings != nullptr;
	}

	virtual void drawRings(const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius) override;

	virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye) override;

	// remove from parent satellite list
	virtual void removeSatellite(std::shared_ptr<Body> planet);
	std::unique_ptr<Ring> rings=nullptr;
	int commandIndex = -2;
	int pipelineOffset = 0; // pipeline to select inside drawState
	std::unique_ptr<Set> set;
	std::unique_ptr<Uniform> uGlobalVertProj; // night bump normal tes
	std::unique_ptr<Uniform> uGlobalFrag; // night bump normal
	std::unique_ptr<Uniform> uUmbraColor; // bump
	std::unique_ptr<Uniform> uGlobalTescGeom; // tes
	std::unique_ptr<Uniform> uModelViewMatrixInverse, uRingFrag;
	globalVertProj *pGlobalVertProj = nullptr;
	globalFrag *pGlobalFrag = nullptr;
	Vec3f *pUmbraColor = nullptr;
	globalTescGeom *pGlobalTescGeom = nullptr;
	Mat4f *pModelViewMatrixInverse = nullptr;
	ringFrag *pRingFrag = nullptr;
	std::unique_ptr<Buffer> drawData;
	//utile pour le shader NIGHT
	s_texture * tex_night;
	s_texture * tex_specular;
	s_texture * tex_cloud;
	s_texture * tex_shadow_cloud;
	s_texture * tex_norm_cloud;
};
