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
#include "vulkanModule/Context.hpp"

class Ojm;
class Set;
class Uniform;
class Buffer;

class Artificial: public Body {


public:
	Artificial(Body *parent,
	           const std::string& englishName,
	           bool flagHalo,
	           double radius,
	           BodyColor* _myColor,
	           float _sol_local_day,
	           float albedo,
	           std::shared_ptr<Orbit> orbit,
	           bool close_orbit,
	           const std::string& model_name,
	           bool _deleteable,
	           double orbit_bounding_radius,
			   BodyTexture* _bodyTexture,
			   ThreadContext *context
	          );

	virtual ~Artificial();

	virtual void selectShader ();

protected :
	void createSC_context(ThreadContext *context);

	virtual void drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz);

	virtual void drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)override {
		return;
	}

	Ojm *obj3D;
	std::unique_ptr<Set> set;
	Set *pushSet;
	std::unique_ptr<Uniform> uProj, uLight, uNormalMatrix;
	Mat4f *pNormalMatrix;
	artGeom *pProj;
	LightInfo *pLight;
	int commandIndex = -2;
};
