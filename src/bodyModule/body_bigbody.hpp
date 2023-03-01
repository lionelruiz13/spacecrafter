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

#ifndef _BODY_BIGBODY_HPP_
#define _BODY_BIGBODY_HPP_

#include "bodyModule/body.hpp"
#include "bodyModule/ring.hpp"

#include "EntityCore/Resource/SharedBuffer.hpp"

class Ring;
class Set;

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
			const BodyTexture &_bodyTexture);

	virtual ~BigBody();

	void setRings(std::unique_ptr<Ring> r);

	virtual void selectShader ();

	// Return the radius of a circle containing the object on screen
	virtual float getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only = false);

	virtual double calculateBoundingRadius();

	virtual void setSphereScale(float s, bool initial_scale =  false);

	virtual void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr);

	virtual void preload(int keepFrames) override;

	virtual void drawOrbit(VkCommandBuffer cmdBodyDepth, VkCommandBuffer cmdOrbit, const Observer* observatory, const Navigator* nav, const Projector* prj) override;

	virtual void bindShadows(const ShadowRenderData &renderData) override;

	void drawCenterOfInterest(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav);
protected :

	virtual void drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest);

	virtual bool hasRings() {
		return rings != nullptr;
	}

	virtual void drawRings(VkCommandBuffer cmd, const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius) override;

	virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye) override;

	//! Return set to bind, may change at every frame
	virtual Set &getSet(float screen_sz) override;
	std::unique_ptr<Ring> rings=nullptr;
	int pipelineOffset = 0; // pipeline to select inside drawState
	std::unique_ptr<Set> set;
	std::unique_ptr<SharedBuffer<globalVertProj>> uGlobalVertProj; // night bump normal tes
	std::unique_ptr<SharedBuffer<globalFrag>> uGlobalFrag; // night bump normal
	std::unique_ptr<SharedBuffer<Vec3f>> uUmbraColor; // bump
	std::unique_ptr<SharedBuffer<globalTescGeom>> uGlobalTescGeom; // tes
	std::unique_ptr<SharedBuffer<Mat4f>> uModelViewMatrixInverse;
	std::unique_ptr<SharedBuffer<ringFrag>> uRingFrag;
	std::unique_ptr<SharedBuffer<ShadowVert>> uShadowVert;
	std::unique_ptr<SharedBuffer<ShadowFrag>> uShadowFrag;
	//useful for shader NIGHT
	std::shared_ptr<s_texture> tex_night;
	std::shared_ptr<s_texture> tex_specular;
	std::shared_ptr<s_texture> tex_cloud;
	std::shared_ptr<s_texture> tex_shadow_cloud;
	std::shared_ptr<s_texture> tex_norm_cloud;
};

#endif
