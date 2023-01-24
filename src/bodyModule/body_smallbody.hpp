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
#include "bodyModule/tail.hpp"
#include "tools/sc_const.hpp"

class Ring;
class Set;
class Tail;

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

	// For the tail
	void setAbsoluteMagnitudeAndSlope(float magnitude, float slope);

	// Bind a tail to this body
	void bindTail(Tail &&newTail) {
	   tails.push_back(std::move(newTail));
	}

	// Formula found at http://www.projectpluto.com/update7b.htm#comet_tail_formula
	Vec2f getComaDiameterAndTailLengthAU(const float r) {
		if (abs(lastR / r - 1) > 0.0001) { // Avoid recomputing it if almost the same
			const float mhelio = absoluteMagnitude + slopeParameter * log10(r);
			float tmp = powf(10.f, -r);
			const float Do = powf(10.0f, ((-0.0033f*mhelio - 0.07f) * mhelio + 3.25f)) * (1.f - tmp);
			tmp *= tmp;
			const float common = (1.f - tmp);
			tmp *= tmp;
			const float Lo = powf(10.0f, ((-0.0075f*mhelio - 0.19f) * mhelio + 2.1f)) * (1.f - tmp) * 1000.f;
			cachedComaDiameterAndTailLengthAU.set(Do * common, Lo * common);
			lastR = r;
		}
		return cachedComaDiameterAndTailLengthAU;
	}

	void overrideHalo(float alpha, float scale) {
		overridedHalo = true;
		haloAlpha = alpha;
		haloScale = scale;
	}
protected :
	void defineSet();
	//! Return set to bind, may change at every frame
	virtual Set &getSet(float screen_sz) override;
	virtual void drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest);
	virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye) override;

	std::unique_ptr<Set> set;
	std::unique_ptr<SharedBuffer<globalVertProj>> uGlobalVertProj; // night bump normal
	std::unique_ptr<SharedBuffer<globalFrag>> uGlobalFrag; // night bump normal
	std::unique_ptr<SharedBuffer<Vec3f>> uUmbraColor; // bump
	std::vector<Tail> tails;
	float absoluteMagnitude = -99.f;
	float slopeParameter = -10.f;
	float lastR = 0;
	float haloAlpha;
	float haloScale;
	Vec2f cachedComaDiameterAndTailLengthAU;
	bool initialized = false;
	bool overridedHalo = false;
};
