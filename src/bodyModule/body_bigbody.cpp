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

#include "bodyModule/orbit_2d.hpp"
#include "bodyModule/body_bigbody.hpp"
#include "coreModule/coreLink.hpp"
#include "tools/file_path.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/orbit_3d.hpp"
#include "bodyModule/halo.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "tools/log.hpp"
#include "bodyModule/ring.hpp"
#include "bodyModule/body_color.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

BigBody::BigBody(std::shared_ptr<Body> parent,
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
				 const BodyTexture &_bodyTexture
                ) :
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
        ),
	rings(nullptr), tex_night(nullptr), tex_specular(nullptr), tex_cloud(nullptr), tex_shadow_cloud(nullptr), tex_norm_cloud(nullptr)
{
	if (_bodyTexture.tex_night != "") {  // prépare au night_shader
		tex_night = std::make_shared<s_texture>(FilePath(_bodyTexture.tex_night,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true, true);
		tex_specular = std::make_shared<s_texture>(FilePath(_bodyTexture.tex_specular,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true);
	}
    trail = std::make_unique<Trail>(this,1460);
    orbitPlot = std::make_unique<Orbit3D>(this);
    if (orbit_bounding_radius <= 0) {
        orbitPlot->computeOrbit(CoreLink::instance->getJDay(), true);
        orbit_bounding_radius = orbitPlot->computeOrbitBoundingRadius();
    }
}

BigBody::~BigBody()
{
	//if (rings) delete rings;
	//rings = nullptr;

	//if (tex_night) delete tex_night;
	tex_night = nullptr;
	//if (tex_specular) delete tex_specular;
	tex_specular = nullptr;
	//if (tex_cloud) delete tex_cloud;
	tex_cloud = nullptr;
	//if (tex_norm_cloud) delete tex_norm_cloud;
	tex_norm_cloud = nullptr;

	//if (trail) delete trail;
	trail = nullptr;
	//if (orbitPlot) delete orbitPlot;
	orbitPlot = nullptr;
}

void BigBody::setRings(std::unique_ptr<Ring> r) {
	rings = std::move(r);
    if (myShader != SHADER_RINGED) {
        changed = true;
    }
}

void BigBody::selectShader ()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    changed = false;
    bigSet.reset();
    if (tex_night && tex_heightmap) { //night Shader with tessellation
		myShader = SHADER_NIGHT_TES; // myEarth
		drawState = BodyShader::getShaderNightTes();
        set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
        if (!uGlobalVertProj)
            uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        set->bindUniform(uGlobalVertProj, 0);
        if (!uGlobalFrag)
            uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
        set->bindUniform(uGlobalFrag, 1);
        if (!uGlobalTescGeom)
            uGlobalTescGeom = std::make_unique<SharedBuffer<globalTescGeom>>(*context.uniformMgr);
        set->bindUniform(uGlobalTescGeom, 2);
        set->bindTexture(tex_current->getTexture(), 5);
        set->bindTexture(tex_eclipse_map->getTexture(), 6);
        set->bindTexture(tex_night->getTexture(), 7);
        set->bindTexture(tex_specular->getTexture(), 8);
        if ((Body::flagClouds && tex_norm_cloud)==1) {
            set->bindTexture(tex_cloud->getTexture(), 9);
            set->bindTexture(tex_norm_cloud->getTexture(), 10);
            pipelineOffset = 1;
        }
        set->bindTexture(tex_heightmap->getTexture(), 11);
		return;
	}

	if (tex_night) { //night Shader
		myShader = SHADER_NIGHT;
		drawState = BodyShader::getShaderNight();
        set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
        if (!uGlobalVertProj)
            uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        set->bindUniform(uGlobalVertProj, 0);
        if (!uGlobalFrag)
            uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
        set->bindUniform(uGlobalFrag, 1);
        set->bindTexture(tex_current->getTexture(), 2);
        set->bindTexture(tex_eclipse_map->getTexture(), 3);
        set->bindTexture(tex_night->getTexture(), 4);
		return;
	}

	if (tex_norm) { //bump Shader
		myShader = SHADER_BUMP;
		drawState = BodyShader::getShaderBump();
        set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
        if (!uGlobalVertProj)
            uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        set->bindUniform(uGlobalVertProj, 0);
        if (!uGlobalFrag)
            uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
        set->bindUniform(uGlobalFrag, 1);
        if (!uUmbraColor)
            uUmbraColor = std::make_unique<SharedBuffer<Vec3f>>(*context.uniformMgr);
        set->bindUniform(uUmbraColor, 2);
        set->bindTexture(tex_current->getTexture(), 3);
        set->bindTexture(tex_norm->getTexture(), 4);
        set->bindTexture(tex_eclipse_map->getTexture(), 5);
		return;
	}

	if (rings) { //ringed Shader
		myShader = SHADER_RINGED;
		drawState = BodyShader::getShaderRinged();
        set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
        if (!uGlobalVertProj)
            uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        set->bindUniform(uGlobalVertProj, 0);
        if (!uGlobalFrag)
            uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
        set->bindUniform(uGlobalFrag, 1);
        if (!uModelViewMatrixInverse)
            uModelViewMatrixInverse = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
        set->bindUniform(uModelViewMatrixInverse, 2);
        if (!uRingFrag)
            uRingFrag = std::make_unique<SharedBuffer<ringFrag>>(*context.uniformMgr);
        set->bindUniform(uRingFrag, 3);
        set->bindTexture(tex_current->getTexture(), 4);
        set->bindTexture(tex_eclipse_map->getTexture(), 5);
        set->bindTexture(rings->getTexTexture(), 6);
		return;
	}

	if (tex_heightmap) {
		myShader = SHADER_NORMAL_TES;
		drawState = BodyShader::getShaderNormalTes();
        set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
        if (!uGlobalVertProj)
            uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
        set->bindUniform(uGlobalVertProj, 0);
        if (!uGlobalFrag)
            uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
        set->bindUniform(uGlobalFrag, 1);
        if (!uGlobalTescGeom)
            uGlobalTescGeom = std::make_unique<SharedBuffer<globalTescGeom>>(*context.uniformMgr);
        set->bindUniform(uGlobalTescGeom, 2);
        set->bindTexture(tex_heightmap->getTexture(), 5);
        set->bindTexture(tex_current->getTexture(), 6);
        set->bindTexture(tex_eclipse_map->getTexture(), 7);
		return;
	}

	myShader = SHADER_NORMAL;
	drawState = BodyShader::getShaderNormal();
    set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, -1, false, true);
    if (!uGlobalVertProj)
        uGlobalVertProj = std::make_unique<SharedBuffer<globalVertProj>>(*context.uniformMgr);
    set->bindUniform(uGlobalVertProj, 0);
    if (!uGlobalFrag)
        uGlobalFrag = std::make_unique<SharedBuffer<globalFrag>>(*context.uniformMgr);
    set->bindUniform(uGlobalFrag, 1);
    set->bindTexture(tex_current->getTexture(), 2);
    set->bindTexture(tex_eclipse_map->getTexture(), 3);
}

float BigBody::getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only)
{
	double rad;
	if (rings && !orb_only) rad = rings->getOuterRadius();
	else
		rad = radius;

    double temp = getEarthEquPos(nav).lengthSquared()-rad*rad;
    if (temp < 0.) temp = 0.000001; // In case we're closer than the radius of the rings
	return atanf(rad/sqrt(temp))*2.f*180./M_PI/prj->getFov()*prj->getViewportHeight();
}

double BigBody::calculateBoundingRadius()
{
    double d = radius;

	if (hasAtmosphere)
		d *= atmosphereParams->atmosphereRadiusFactor; // Atmosphere shouldn't be bigger than that

	if (rings && d < rings->getOuterRadius())
        d = rings->getOuterRadius();

    if (d > Body::calculateBoundingRadius())
	   boundingRadius = d;
	return boundingRadius;
}

void BigBody::drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest)
{
    if (changed)
        selectShader();
    if (depthTest)
        drawState->pipeline[pipelineOffset].bind(cmd);
    else
        drawState->pipelineNoDepth[pipelineOffset].bind(cmd);
    currentObj->bind(cmd);
    drawState->layout->bindSets(cmd, {getSet(screen_sz), *Context::instance->uboSet});

    Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
	//load specific values for shader
	switch (myShader) {
        case SHADER_NIGHT_TES:
        case SHADER_NORMAL_TES:
            uGlobalTescGeom->get().TesParam = Vec3i(bodyTesselation->getMinTesLevel(),bodyTesselation->getMaxTesLevel(), bodyTesselation->getPlanetAltimetryFactor());
            break;
        case SHADER_RINGED:
            uRingFrag->get().RingInnerRadius = rings->getInnerRadius();
            uRingFrag->get().RingOuterRadius = rings->getOuterRadius();
            *uModelViewMatrixInverse = inv_matrix;
            break;
        case SHADER_BUMP:
            *uUmbraColor = v3fNull; // deduced from body_moon
            break;
        case SHADER_NIGHT:
        case SHADER_NORMAL:
		default: //shader normal
			break;
	}
	int index=1;
	double length;
	double moonDotLight;
	Vec3f tmp= v3fNull;
	Vec3f tmp2(0.4, 0.12, 0.0);

	Vec3d planet_helio = get_heliocentric_ecliptic_pos();
	Vec3d light = -planet_helio;
	light.normalize();

    uGlobalVertProj->get().ModelViewMatrix = matrix;
    uGlobalVertProj->get().NormalMatrix = inv_matrix.transpose();
    uGlobalVertProj->get().clipping_fov = prj->getClippingFov();
    uGlobalVertProj->get().planetRadius = initialRadius;
    uGlobalVertProj->get().LightPosition = eye_sun;
    uGlobalVertProj->get().planetScaledRadius = radius;
    uGlobalVertProj->get().planetOneMinusOblateness = one_minus_oblateness;
    uGlobalFrag->get().SunHalfAngle = sun_half_angle;
	for (auto iter=satellites.begin(); iter!=satellites.end() && index <= 4; iter++) {
		tmp2 = (*iter)->get_heliocentric_ecliptic_pos() - planet_helio;
		length = tmp2.length();
		tmp2.normalize();
		moonDotLight = tmp2.dot(light);
		if(moonDotLight > 0 && length*sin(acos(moonDotLight)) <= radius + 2*(*iter)->getRadius()) {
			tmp = nav->getHelioToEyeMat() * (*iter)->get_heliocentric_ecliptic_pos();

			if (index==1) {
				uGlobalFrag->get().MoonPosition1 = tmp;
				uGlobalFrag->get().MoonRadius1 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}
			else if (index==2) {
				uGlobalFrag->get().MoonPosition2 = tmp;
				uGlobalFrag->get().MoonRadius2 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}
			else if (index==3) {
				uGlobalFrag->get().MoonPosition3 = tmp;
				uGlobalFrag->get().MoonRadius3 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}
			else if (index==4) {
				uGlobalFrag->get().MoonPosition4 = tmp;
				uGlobalFrag->get().MoonRadius4 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}

			index++;
		}
	}

	// clear any leftover values
	for(; index<=4; index++) {
		if (index==1) // No moon data
			uGlobalFrag->get().MoonRadius1 = 0.0;
		if (index==2)
			uGlobalFrag->get().MoonRadius2 = 0.0;
		if (index==3)
			uGlobalFrag->get().MoonRadius3 = 0.0;
		if (index==4)
			uGlobalFrag->get().MoonRadius4 = 0.0;
	}
	currentObj->draw(cmd, screen_sz);
    drawAtmExt(cmd, prj, nav, matrix, screen_sz, depthTest);
}

void BigBody::setSphereScale(float s, bool initial_scale)
{
	radius = initialRadius * s;
	if (initial_scale)
		initialScale = s;
	if (rings!=nullptr)
		rings->multiplyRadius(s);
}

void BigBody::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	Body::update(delta_time, nav, timeMgr);
	if (radius.isScaling() && rings) {
        rings->multiplyRadius(radius/initialRadius);
        if (boundingRadius < rings->getOuterRadius())
            boundingRadius = rings->getOuterRadius();
    }
}

void BigBody::drawRings(VkCommandBuffer cmd, const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius)
{
    const double observerDistanceToBody = (get_heliocentric_ecliptic_pos() - obs->getHeliocentricPosition(CoreLink::instance->getJDay())).length();
	rings->draw(cmd, prj, observerDistanceToBody, mat,screen_sz,_lightDirection,_planetPosition,planetRadius);
}

void BigBody::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{

	if (isVisible && flags.flag_halo && this->getOnScreenSize(prj, nav) < 10 && this->getSphereScale()<10.0) {
		// Workaround for depth buffer precision and near planets
		halo->drawHalo(nav, prj, eye);
        //StateGL::disable(GL_DEPTH_TEST);
	}

}

Set &BigBody::getSet(float screen_sz)
{
    if (screen_sz < 180)
        return *set;
    switch (myShader) {
        case SHADER_NIGHT_TES: {
            auto tex0 = tex_current->getBigTexture(); // 5
            auto tex1 = tex_eclipse_map->getBigTexture(); // 6
            auto tex2 = tex_night->getBigTexture(); // 7
            auto tex3 = tex_specular->getBigTexture(); // 8
            auto tex4 = tex_heightmap->getBigTexture(); // 11
            if (bigSet) {
                if (!(tex0 && tex1 && tex2 && tex3 && tex4))
                    bigSet.reset();
            } else {
                if (tex0 && tex1 && tex2 && tex3 && tex4) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uGlobalFrag, 1);
                    bigSet->bindUniform(uGlobalTescGeom, 2);
                    bigSet->bindTexture(*tex0, 5);
                    bigSet->bindTexture(*tex1, 6);
                    bigSet->bindTexture(*tex2, 7);
                    bigSet->bindTexture(*tex3, 8);
                    bigSet->bindTexture(*tex4, 11);
                }
            }
            break;
        }
        case SHADER_NIGHT: {
            auto tex0 = tex_current->getBigTexture(); // 2
            auto tex1 = tex_eclipse_map->getBigTexture(); // 3
            auto tex2 = tex_night->getBigTexture(); // 4
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
        case SHADER_RINGED: {
            auto tex0 = tex_current->getBigTexture(); // 4
            auto tex1 = tex_eclipse_map->getBigTexture(); // 5
            if (bigSet) {
                if (!(tex0 && tex1))
                    bigSet.reset();
            } else {
                if (tex0 && tex1) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uGlobalFrag, 1);
                    bigSet->bindUniform(uModelViewMatrixInverse, 2);
                    bigSet->bindUniform(uRingFrag, 3);
                    bigSet->bindTexture(*tex0, 4);
                    bigSet->bindTexture(*tex1, 5);
                    bigSet->bindTexture(rings->getTexTexture(), 6);
                }
            }
            break;
        }
        case SHADER_NORMAL_TES: {
            auto tex0 = tex_heightmap->getBigTexture(); // 5
            auto tex1 = tex_current->getBigTexture(); // 6
            auto tex2 = tex_eclipse_map->getBigTexture(); // 7
            if (bigSet) {
                if (!(tex0 && tex1 && tex2))
                    bigSet.reset();
            } else {
                if (tex0 && tex1 && tex2) {
                    bigSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, drawState->layout, -1, true, true);
                    bigSet->bindUniform(uGlobalVertProj, 0);
                    bigSet->bindUniform(uGlobalFrag, 1);
                    bigSet->bindUniform(uGlobalTescGeom, 2);
                    bigSet->bindTexture(*tex0, 5);
                    bigSet->bindTexture(*tex1, 6);
                    bigSet->bindTexture(*tex2, 7);
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
    return (bigSet && screen_sz > 512) ? *bigSet : *set;
}

void BigBody::preload(int keepFrames)
{
    int tmp = s_texture::setBigTextureLifetime(keepFrames);
    if (changed)
        selectShader();
    if (rings)
        rings->preload();
    getSet(2048); // Assume the big texture is used for such screen_sz
    s_texture::setBigTextureLifetime(tmp);
}

void BigBody::drawOrbit(VkCommandBuffer cmdBodyDepth, VkCommandBuffer cmdOrbit, const Observer* observatory, const Navigator* nav, const Projector* prj)
{
    Body::drawOrbit(cmdBodyDepth, cmdOrbit, observatory, nav, prj);
    if (rings && isVisibleOnScreen())
        rings->drawDepthTrace(cmdBodyDepth, *BodyShader::getShaderDepthTrace()->layout);
}
