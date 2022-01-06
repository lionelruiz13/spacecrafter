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

#include "bodyModule/body_smallbody.hpp"

#include "bodyModule/trail.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/orbit_2d.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/halo.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "bodyModule/body_color.hpp"

#include "bodyModule/ring.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

SmallBody::SmallBody(std::shared_ptr<Body> parent,
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
					 std::shared_ptr<BodyTexture> _bodyTexture):
	Body(parent,
	     englishName,
	     _typePlanet,
	     flagHalo,
	     radius,
	     oblateness,
	     std::move(_myColor),
	     _sol_local_day,
	     albedo,
	     std::move(orbit),
	     close_orbit,
	     _currentObj,
	     orbit_bounding_radius,
		 _bodyTexture
        )
{
	if (_typePlanet == COMET) {
		trail = std::make_unique<Trail>(this,2920);
		orbitPlot = std::make_unique<Orbit2D>(this, 4800);
	}
	else {
		trail = std::make_unique<Trail>(this, 60);
		orbitPlot = std::make_unique<Orbit2D>(this);
	}
}

SmallBody::~SmallBody()
{
	//if (trail) delete trail;
	trail = nullptr;
	//if (orbitPlot) delete orbitPlot;
	orbitPlot = nullptr;
}

void SmallBody::defineSet()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
    switch (myShader) {
        case SHADER_BUMP:
            set->bindUniform(uUmbraColor, 2);
            set->bindTexture(tex_current->getTexture(), 3);
            set->bindTexture(tex_norm->getTexture(), 4);
            set->bindTexture(tex_eclipse_map->getTexture(), 5);
            break;
        case SHADER_NORMAL:
            set->bindTexture(tex_current->getTexture(), 2);
            set->bindTexture(tex_eclipse_map->getTexture(), 3);
            break;
        default:;
    }
    set->bindUniform(uGlobalVertProj, 0);
    set->bindUniform(uGlobalFrag, 1);
    bigSet.reset();
    changed = false;
}

Set &SmallBody::getSet(float screen_sz)
{
    if (screen_sz < 180)
        return *set;
    switch (myShader) {
        case SHADER_BUMP: {
            auto tex0 = tex_current->getBigTexture(); // 3
            auto tex1 = tex_norm->getBigTexture(); // 4
            auto tex2 = tex_eclipse_map->getBigTexture(); // 5
            if (bigSet) {
                if (!(tex0 && tex1 && tex2))
                    bigSet.reset();
            } else {
                if (tex0 && tex1 && tex2) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uGlobalFrag, 1);
                    bigSet->bindUniform(uUmbraColor, 2);
                    bigSet->bindTexture(*tex0, 3);
                    bigSet->bindTexture(*tex1, 4);
                    bigSet->bindTexture(*tex2, 5);
                }
            }
            break;
        }
        case SHADER_NORMAL: {
            auto tex0 = tex_current->getBigTexture(); // 2
            auto tex1 = tex_eclipse_map->getBigTexture(); // 3
            if (bigSet) {
                if (!(tex0 && tex1))
                    bigSet.reset();
            } else {
                if (tex0 && tex1) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uGlobalFrag, 1);
                    bigSet->bindTexture(*tex0, 2);
                    bigSet->bindTexture(*tex1, 3);
                }
            }
            break;
        }
        default:
            // Not handled !
            return *set;
    }
    return bigSet ? *bigSet : *set;
}

void SmallBody::selectShader ()
{
    Context &context = *Context::instance;
	if (tex_norm) { //bump Shader
		myShader = SHADER_BUMP;
        drawState = BodyShader::getShaderBump();

        uUmbraColor = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
        *uUmbraColor = v3fNull;
	} else {
		myShader = SHADER_NORMAL;
		drawState = BodyShader::getShaderNormal();
	}
    // Create general uniforms and bind them
    uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
    uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
    uGlobalFrag->get().MoonRadius1 = 0;
    uGlobalFrag->get().MoonRadius2 = 0;
    uGlobalFrag->get().MoonRadius3 = 0;
    uGlobalFrag->get().MoonRadius4 = 0;
    defineSet();
    initialized = true;
}

void SmallBody::drawBody(VkCommandBuffer &cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
    if (initialized) {
        if (changed)
            defineSet();
    } else
        selectShader();

    drawState->pipeline->bind(cmd);
    currentObj->bind(cmd);
    drawState->layout->bindSets(cmd, {getSet(screen_sz), *Context::instance->uboSet});

	//load specific values for shader
    Mat4f matrix = mat.convert() * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));
    uGlobalVertProj->get().ModelViewMatrix = matrix;
    uGlobalVertProj->get().NormalMatrix = matrix.inverse().transpose();
    uGlobalVertProj->get().clipping_fov = prj->getClippingFov();
    uGlobalVertProj->get().planetRadius = initialRadius;
    uGlobalVertProj->get().LightPosition = eye_sun;
    uGlobalVertProj->get().planetScaledRadius = radius;
    uGlobalVertProj->get().planetOneMinusOblateness = one_minus_oblateness;
    uGlobalFrag->get().SunHalfAngle = sun_half_angle;

	currentObj->draw(cmd, screen_sz);
}
