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


#include "bodyModule/body_moon.hpp"


#include "tools/file_path.hpp"
#include "bodyModule/body_color.hpp"
#include "coreModule/coreLink.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/orbit_3d.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/halo.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/axis.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

Moon::Moon(std::shared_ptr<Body> parent,
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
		   const BodyTexture &_bodyTexture):
	Body(parent,
	     englishName,
	     MOON,
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
		 _bodyTexture)
{
	if (_bodyTexture.tex_night != "") {
		tex_night = std::make_unique<s_texture>(FilePath(_bodyTexture.tex_night,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
	}
	//more adding could be placed here for the constructor of Moon
	selectShader();
	orbitPlot = std::make_unique<Orbit3D>(this);
    // if (orbit_bounding_radius <= 0) {
    //     orbitPlot->computeOrbit(CoreLink::instance->getJDay(), true);
    //     orbit_bounding_radius = orbitPlot->computeOrbitBoundingRadius();
    // }
}

Moon::~Moon()
{
	//if (orbitPlot) delete orbitPlot;
}

void Moon::defineSet()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
    set->bindUniform(uGlobalVertProj, 0);
    switch (myShader) {
        case SHADER_TES_SHADOW:
            set->bindUniform(uShadowVert, 0);
            set->bindUniform(uShadowFrag, 1);
            set->bindTexture(tex_heightmap->getTexture(), 2);
            set->bindTexture(tex_norm->getTexture(), 3);
            set->bindTexture(tex_current->getTexture(), 4);
            break;
        case SHADER_MOON_NORMAL_TES:
            set->bindUniform(uMoonFrag, 1);
            set->bindUniform(uGlobalTescGeom, 2);
            set->bindTexture(tex_current->getTexture(), 5);
            set->bindTexture(tex_norm->getTexture(), 6);
            set->bindTexture(tex_eclipse_map->getTexture(), 7);
            set->bindTexture(tex_heightmap->getTexture(), 8);
            break;
        case SHADER_MOON_NIGHT:
            set->bindUniform(uGlobalFrag, 1);
            set->bindTexture(tex_current->getTexture(), 2);
            set->bindTexture(tex_eclipse_map->getTexture(), 3);
            set->bindTexture(tex_night->getTexture(), 4);
            break;
        case SHADER_MOON_BUMP:
            set->bindUniform(uGlobalFrag, 1);
            set->bindUniform(uUmbraColor, 2);
            set->bindTexture(tex_current->getTexture(), 3);
            set->bindTexture(tex_norm->getTexture(), 4);
            set->bindTexture(tex_eclipse_map->getTexture(), 5);
            break;
        case SHADER_MOON_NORMAL:
            set->bindUniform(uGlobalFrag, 1);
            set->bindTexture(tex_current->getTexture(), 2);
            set->bindTexture(tex_eclipse_map->getTexture(), 3);
            break;
        default:;
    }
    bigSet.reset();
    changed = false;
}

Set &Moon::getSet(float screen_sz)
{
    if (screen_sz < 180)
        return *set;
    switch (myShader) {
        case SHADER_TES_SHADOW: {
            auto tex0 = tex_heightmap->getBigTexture();
            auto tex1 = tex_norm->getBigTexture();
            auto tex2 = tex_current->getBigTexture();
            if (bigSet) {
                if (!(tex0 && tex1 && tex2))
                    bigSet.reset();
            } else {
                if (tex0 && tex1 && tex2) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uShadowVert, 0);
                    bigSet->bindUniform(uShadowFrag, 1);
                    bigSet->bindTexture(*tex0, 2);
                    bigSet->bindTexture(*tex1, 3);
                    bigSet->bindTexture(*tex2, 4);
                }
            }
            break;
        }
        case SHADER_MOON_NORMAL_TES: {
            auto tex0 = tex_current->getBigTexture();
            auto tex1 = tex_norm->getBigTexture();
            auto tex2 = tex_eclipse_map->getBigTexture();
            auto tex3 = tex_heightmap->getBigTexture();
            if (bigSet) {
                if (!(tex0 && tex1 && tex2 && tex3))
                    bigSet.reset();
            } else {
                if (tex0 && tex1 && tex2 && tex3) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uMoonFrag, 1);
                    bigSet->bindUniform(uGlobalTescGeom, 2);
                    bigSet->bindTexture(*tex0, 5);
                    bigSet->bindTexture(*tex1, 6);
                    bigSet->bindTexture(*tex2, 7);
                    bigSet->bindTexture(*tex3, 8);
                }
            }
            break;
        }
        case SHADER_MOON_NIGHT: {
            auto tex0 = tex_current->getBigTexture();
            auto tex1 = tex_eclipse_map->getBigTexture();
            auto tex2 = tex_night->getBigTexture();
            if (bigSet) {
                if (!(tex0 && tex1 && tex2))
                    bigSet.reset();
            } else {
                if (tex0 && tex1 && tex2) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uGlobalFrag, 1);
                    bigSet->bindTexture(*tex0, 2);
                    bigSet->bindTexture(*tex1, 3);
                    bigSet->bindTexture(*tex2, 4);
                }
            }
            break;
        }
        case SHADER_MOON_NORMAL: {
            auto tex0 = tex_current->getBigTexture();
            auto tex1 = tex_eclipse_map->getBigTexture();
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
        case SHADER_MOON_BUMP: {
            auto tex0 = tex_current->getBigTexture();
            auto tex1 = tex_norm->getBigTexture();
            auto tex2 = tex_eclipse_map->getBigTexture();
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
        default:
            // Not handled !
            return *set;
    }
    return bigSet ? *bigSet : *set;
}

void Moon::selectShader()
{
    Context &context = *Context::instance;
	//bool useShaderMoonNormal = true;
	if (tex_heightmap!=nullptr) { //altimetry Shader
		myShader = SHADER_MOON_NORMAL_TES;
		drawState = BodyShader::getShaderMoonNormalTes();

        uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        uMoonFrag = std::make_unique<SharedBuffer<moonFrag>>(*context.uniformMgr);
        uGlobalTescGeom = std::make_unique<SharedBuffer<globalTescGeom>>(*context.uniformMgr);
    	return;
	}

	if (tex_night!=nullptr) { //altimetry Shader
		myShader = SHADER_MOON_NIGHT;
		drawState = BodyShader::getShaderNight();

        uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
    	return;
	}

	if (tex_norm!=nullptr) { //bump Shader
		myShader = SHADER_MOON_BUMP;
		drawState = BodyShader::getShaderBump();

        uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
        uUmbraColor = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
    	return;
	}
	//if (useShaderMoonNormal) { // normal shaders
	myShader = SHADER_MOON_NORMAL;
	drawState = BodyShader::getShaderNormal();

    uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
    uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
	//}
}

void Moon::drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest)
{
    if (changed) {
        defineSet();
        updateBoundingRadii();
    }
    switch (myShader) {
        case SHADER_TES_SHADOW:
            return drawCenterOfInterest(cmd, prj, nav);
        default:;
    }

    if (depthTest)
        drawState->pipeline->bind(cmd);
    else
        drawState->pipelineNoDepth->bind(cmd);
    currentObj->bind(cmd);
    drawState->layout->bindSets(cmd, {getSet(screen_sz), *Context::instance->uboSet});

    Vec3f tmp= v3fNull;
	Vec3f tmp2(0.4, 0.12, 0.0);
    Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
    //load specific values for shader
	switch (myShader) {
        case SHADER_MOON_NORMAL_TES: // myMoon
            uMoonFrag->get().MoonPosition1 = nav->getHelioToEyeMat() * parent->get_heliocentric_ecliptic_pos();
            uMoonFrag->get().MoonRadius1 = parent->getRadius();
            uMoonFrag->get().UmbraColor = (getEnglishName() == "Moon") ? tmp2 : tmp;
            uMoonFrag->get().SunHalfAngle = sun_half_angle;
            uGlobalTescGeom->get().TesParam = Vec3i(bodyTesselation->getMinTesLevel(),bodyTesselation->getMaxTesLevel(), bodyTesselation->getMoonAltimetryFactor());
            break;
		case SHADER_MOON_BUMP:
            *uUmbraColor = (getEnglishName() == "Moon") ? tmp2 : tmp;
            [[fallthrough]];
		case SHADER_MOON_NIGHT:
		case SHADER_MOON_NORMAL:
		default: // Common uniform affectation
            uGlobalFrag->get().MoonPosition1 = nav->getHelioToEyeMat() * parent->get_heliocentric_ecliptic_pos();
            uGlobalFrag->get().MoonRadius1 = parent->getRadius();
            uGlobalFrag->get().SunHalfAngle = sun_half_angle;
			break;
	}
    uGlobalVertProj->get().ModelViewMatrix = matrix;
    uGlobalVertProj->get().NormalMatrix = inv_matrix.transpose();
    uGlobalVertProj->get().clipping_fov = prj->getClippingFov();
    uGlobalVertProj->get().planetRadius = initialRadius;
    uGlobalVertProj->get().LightPosition = eye_sun;
    uGlobalVertProj->get().planetScaledRadius = radius;
    uGlobalVertProj->get().planetOneMinusOblateness = one_minus_oblateness;

    currentObj->draw(cmd, screen_sz);
    drawAtmExt(cmd, prj, nav, matrix, screen_sz, depthTest);
}

void Moon::handleVisibilityFader(const Observer* observatory, const Projector* prj, const Navigator * nav)
{
	if (prj->getFov()>30) {
		// If not in the parent Body system OR one of the other sister moons do not draw
		if (observatory->isOnBody() && !observatory->isOnBody(parent.get()) && parent != observatory->getHomeBody()->get_parent() && getEarthEquPos(nav).length() > 1) {
			// If further than 1 AU of object if flying
			visibilityFader = false; // orbits will fade in and out now
		}
		else {
			if ( !observatory->isOnBody() ) {
				visibilityFader = false;
			}
			else
				visibilityFader = true;
		}
	}
	else
		visibilityFader = true;
}

void Moon::bindShadows(const ShadowRenderData &renderData)
{
    if (changed) {
        defineSet();
        updateBoundingRadii();
    }
    if (uShadowVert) {
        auto &frag = **uShadowFrag;
        auto m = renderData.lookAt * model;
        frag.ShadowMatrix[0] = m.r[0];
        frag.ShadowMatrix[1] = m.r[1];
        frag.ShadowMatrix[2] = m.r[2];
        frag.ShadowMatrix[4] = m.r[4];
        frag.ShadowMatrix[5] = m.r[5];
        frag.ShadowMatrix[6] = m.r[6];
        frag.ShadowMatrix[8] = m.r[8];
        frag.ShadowMatrix[9] = m.r[9];
        frag.ShadowMatrix[10] = m.r[10];
        frag.sinSunAngle = 2 * renderData.sinSunHalfAngle;
        frag.nbShadowingBodies = renderData.shadowingBodies.size();
        for (uint8_t i = 0; i < renderData.shadowingBodies.size(); ++i) {
            frag.shadowingBodies[i] = renderData.shadowingBodies[i];
        }
    }
}

void Moon::drawCenterOfInterest(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav)
{
    auto &vert = **uShadowVert;
    auto &frag = **uShadowFrag;

    const float altimetryFactor = 0.01 * bodyTesselation->getMoonAltimetryFactor();
    float finalRadius = std::min(radius * (1 + altimetryFactor), mat.getTranslation().length() - radius/64);
    auto m = mat * Mat4d::zrotation(M_PI/180*(axis_rotation + 90));
    vert.ModelViewMatrix = (m * Mat4d::scaling(Vec3d(1, 1, one_minus_oblateness))).convert();
    {
        auto m2 = m.transpose();
        vert.WorldToModelMatrix[0] = m2.r[0];
        vert.WorldToModelMatrix[1] = m2.r[1];
        vert.WorldToModelMatrix[2] = m2.r[2];
        vert.WorldToModelMatrix[4] = m2.r[4];
        vert.WorldToModelMatrix[5] = m2.r[5];
        vert.WorldToModelMatrix[6] = m2.r[6];
        vert.WorldToModelMatrix[8] = m2.r[8];
        vert.WorldToModelMatrix[9] = m2.r[9];
        vert.WorldToModelMatrix[10] = m2.r[10];
        vert.radius = finalRadius;

        Vec3f tmp = m2 * (eye_planet - eye_sun);
        tmp.normalize();
        frag.lightDirection = tmp;
    }
    auto clipping_fov = prj->getClippingFov();
    vert.zNear = clipping_fov[0];
    vert.zRange = clipping_fov[1] - clipping_fov[0];
    vert.fov = clipping_fov[2];

    const float altimetryCoef = radius / finalRadius;
    frag.heightMapDepthLevel = altimetryCoef;
    frag.heightMapDepth = altimetryFactor * altimetryCoef;
    frag.squaredHeightMapDepthLevel = altimetryCoef*altimetryCoef;
    frag.atmColor = atmosphereParams->atmColor;
    frag.sunDeviation = atmosphereParams->sunDeviation;
    frag.atmDeviation = atmosphereParams->atmDeviation;

    drawState->pipeline->bind(cmd);
    currentObj->bind(cmd);
    drawState->layout->bindSets(cmd, {getSet(screen_sz), *Context::instance->uboSet});
    currentObj->draw(cmd, screen_sz);
    drawAtmExt(cmd, prj, nav, m.convert(), screen_sz, true);
}

void Moon::gainInterest()
{
   isCenterOfInterest = true;
   if (tex_heightmap && tex_norm) {
       myShader = SHADER_TES_SHADOW;
       drawState = BodyShader::getShaderTesShadowed();
       if (!uShadowVert)
           uShadowVert = std::make_unique<SharedBuffer<ShadowVert>>(*Context::instance->uniformMgr);
       if (!uShadowFrag)
           uShadowFrag = std::make_unique<SharedBuffer<ShadowFrag>>(*Context::instance->uniformMgr);
       changed = true;
   }
}

void Moon::looseInterest()
{
    isCenterOfInterest = false;
    if (myShader == SHADER_TES_SHADOW) {
        myShader = SHADER_MOON_NORMAL_TES;
        drawState = BodyShader::getShaderMoonNormalTes();
        changed = true;
    }
}
