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

#include "tools/file_path.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/orbit_2d.hpp"
#include "bodyModule/halo.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "tools/log.hpp"
#include "bodyModule/ring.hpp"

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"
#include "vulkanModule/Pipeline.hpp"

BigBody::BigBody(Body *parent,
                 const std::string& englishName,
                 BODY_TYPE _typePlanet,
                 bool flagHalo,
                 double radius,
                 double oblateness,
                 std::shared_ptr<BodyColor> _myColor,
                 float _sol_local_day,
                 float albedo,
                 std::unique_ptr<Orbit> orbit,
                 bool close_orbit,
                 ObjL* _currentObj,
                 double orbit_bounding_radius,
				 std::shared_ptr<BodyTexture> _bodyTexture,
				 ThreadContext *context
                ) :
	Body(parent,
	     englishName,
	     _typePlanet,
	     flagHalo,
	     radius,
	     oblateness,
	     _myColor,
	     _sol_local_day,
	     albedo,
	     std::move(orbit),
	     close_orbit,
	     _currentObj,
	     orbit_bounding_radius,
		 _bodyTexture,
		 context
        ),
	rings(NULL), tex_night(nullptr), tex_specular(nullptr), tex_cloud(nullptr), tex_shadow_cloud(nullptr), tex_norm_cloud(nullptr)
{
	if (_bodyTexture->tex_night != "") {  // prépare au night_shader
		tex_night = new s_texture(FilePath(_bodyTexture->tex_night,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
		tex_specular = new s_texture(FilePath(_bodyTexture->tex_specular,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
	}

	// if(_bodyTexture->tex_cloud != "") {
	// 	// Try to use cloud texture in any event, even if can not use shader
	// 	tex_cloud = new s_texture(FilePath(_bodyTexture->tex_cloud,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_ALPHA, true);

	// 	if(_bodyTexture->tex_cloud_normal=="") {
	// 		cLog::get()->write("No cloud normal texture defined for " + englishName, LOG_TYPE::L_ERROR);
	// 	}
	// 	else {
	// 		tex_norm_cloud = new s_texture(FilePath(_bodyTexture->tex_cloud_normal,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true);
	// 	}
	// }

	trail = new Trail(this,1460);
	orbitPlot = new Orbit2D(this);
    drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	selectShader();
}

BigBody::~BigBody()
{
	if (rings) delete rings;
	rings = nullptr;

	if (tex_night) delete tex_night;
	tex_night = nullptr;
	if (tex_specular) delete tex_specular;
	tex_specular = nullptr;
	if (tex_cloud) delete tex_cloud;
	tex_cloud = nullptr;
	if (tex_norm_cloud) delete tex_norm_cloud;
	tex_norm_cloud = nullptr;

	if (trail) delete trail;
	trail = nullptr;
	if (orbitPlot) delete orbitPlot;
	orbitPlot = nullptr;
}

void BigBody::setRings(Ring* r) {
	rings = r;
    if (myShader != SHADER_RINGED) {
        if (commandIndex == -1) {
            cLog::get()->write("Failed to enable rings.", LOG_TYPE::L_ERROR);
            return;
        }
        commandIndex = -2;
        selectShader();
    }
}

void BigBody::selectShader ()
{
	if (tex_night && tex_heightmap) { //night Shader with tessellation
		myShader = SHADER_NIGHT_TES; // myEarth
		drawState = BodyShader::getShaderNightTes();
        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
        uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
        pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
        set->bindUniform(uGlobalVertProj.get(), 0);
        uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
        pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
        set->bindUniform(uGlobalFrag.get(), 1);
        uGlobalTescGeom = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalTescGeom));
        pGlobalTescGeom = static_cast<typeof(pGlobalTescGeom)>(uGlobalTescGeom->data);
        set->bindUniform(uGlobalTescGeom.get(), 2);
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
        // if ((Body::flagClouds && tex_norm_cloud)==1)
        // 	drawState->setUniform("Clouds",1);
        // else
        // 	drawState->setUniform("Clouds",0);
		return;
	}

	if (tex_night) { //night Shader
		myShader = SHADER_NIGHT;
		drawState = BodyShader::getShaderNight();
        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
        uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
        pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
        set->bindUniform(uGlobalVertProj.get(), 0);
        uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
        pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
        set->bindUniform(uGlobalFrag.get(), 1);
        set->bindTexture(tex_current->getTexture(), 2);
        set->bindTexture(tex_eclipse_map->getTexture(), 3);
        set->bindTexture(tex_night->getTexture(), 4);
		return;
	}

	if (tex_norm) { //bump Shader
		myShader = SHADER_BUMP;
		drawState = BodyShader::getShaderBump();
        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
        uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
        pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
        set->bindUniform(uGlobalVertProj.get(), 0);
        uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
        pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
        set->bindUniform(uGlobalFrag.get(), 1);
        uUmbraColor = std::make_unique<Uniform>(context->surface, sizeof(*pUmbraColor));
        pUmbraColor = static_cast<typeof(pUmbraColor)>(uUmbraColor->data);
        set->bindUniform(uUmbraColor.get(), 2);
        set->bindTexture(tex_current->getTexture(), 3);
        set->bindTexture(tex_norm->getTexture(), 4);
        set->bindTexture(tex_eclipse_map->getTexture(), 5);
		return;
	}

	if (rings) { //ringed Shader
		myShader = SHADER_RINGED;
		drawState = BodyShader::getShaderRinged();
        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
        uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
        pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
        set->bindUniform(uGlobalVertProj.get(), 0);
        uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
        pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
        set->bindUniform(uGlobalFrag.get(), 1);
        uModelViewMatrixInverse = std::make_unique<Uniform>(context->surface, sizeof(*pModelViewMatrixInverse));
        pModelViewMatrixInverse = static_cast<typeof(pModelViewMatrixInverse)>(uModelViewMatrixInverse->data);
        set->bindUniform(uModelViewMatrixInverse.get(), 2);
        uRingFrag = std::make_unique<Uniform>(context->surface, sizeof(*pRingFrag));
        pRingFrag = static_cast<typeof(pRingFrag)>(uRingFrag->data);
        set->bindUniform(uRingFrag.get(), 3);
        set->bindTexture(tex_current->getTexture(), 4);
        set->bindTexture(tex_eclipse_map->getTexture(), 5);
        set->bindTexture(rings->getTexTexture(), 6);
		return;
	}

	if (tex_heightmap) {
		myShader = SHADER_NORMAL_TES;
		drawState = BodyShader::getShaderNormalTes();
        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
        uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
        pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
        set->bindUniform(uGlobalVertProj.get(), 0);
        uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
        pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
        set->bindUniform(uGlobalFrag.get(), 1);
        uGlobalTescGeom = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalTescGeom));
        pGlobalTescGeom = static_cast<typeof(pGlobalTescGeom)>(uGlobalTescGeom->data);
        set->bindUniform(uGlobalTescGeom.get(), 2);
        set->bindTexture(tex_heightmap->getTexture(), 5);
        set->bindTexture(tex_current->getTexture(), 6);
        set->bindTexture(tex_eclipse_map->getTexture(), 7);
		return;
	}

	myShader = SHADER_NORMAL;
	drawState = BodyShader::getShaderNormal();
    set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
    uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
    pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
    set->bindUniform(uGlobalVertProj.get(), 0);
    uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
    pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
    set->bindUniform(uGlobalFrag.get(), 1);
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

void BigBody::removeSatellite(Body *planet)
{
	std::list<Body *>::iterator iter;
	for (iter=satellites.begin(); iter != satellites.end(); iter++) {
		if ( (*iter) == planet ) {
			satellites.erase(iter);
			break;
		}
	}
}

double BigBody::calculateBoundingRadius()
{
	double d = radius.final();

	if (rings) d = rings->getOuterRadius();

	std::list<Body *>::const_iterator iter;
	std::list<Body *>::const_iterator end = satellites.end();

	double r;
	for ( iter=satellites.begin(); iter != end; iter++) {

		r = (*iter)->getBoundingRadius();
		if ( r > d ) d = r;
	}

	boundingRadius = d;
	return boundingRadius;
}

void BigBody::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	//glEnable(GL_TEXTURE_2D);
	// StateGL::enable(GL_CULL_FACE);
	// StateGL::disable(GL_BLEND);

//	glBindTexture(GL_TEXTURE_2D, tex_current->getID());

	//drawState->use();

    switch (commandIndex) {
        case -2: // Command not builded
            commandIndex = -1;
            if (!context->commandMgr->isRecording()) {
                commandIndex = context->commandMgr->getCommandIndex();
                context->commandMgr->init(commandIndex);
                context->commandMgr->beginRenderPass(renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
            }
            context->commandMgr->bindPipeline(drawState->pipeline + pipelineOffset);
            currentObj->bind(context->commandMgr);
            context->commandMgr->bindSet(drawState->layout, set.get());
            context->commandMgr->bindSet(drawState->layout, context->global->globalSet, 1);
            context->commandMgr->indirectDrawIndexed(drawData.get());
            if (!hasRings())
                context->commandMgr->compile();
            return;
        case -1: break;
        default:
            context->commandMgr->setSubmission(commandIndex);
    }

    Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
	//load specific values for shader
	switch (myShader) {
        case SHADER_NIGHT_TES:
        case SHADER_NORMAL_TES:
            pGlobalTescGeom->TesParam = Vec3i(bodyTesselation->getMinTesLevel(),bodyTesselation->getMaxTesLevel(), bodyTesselation->getPlanetAltimetryFactor());
            break;
        case SHADER_RINGED:
            pRingFrag->RingInnerRadius = rings->getInnerRadius();
            pRingFrag->RingOuterRadius = rings->getOuterRadius();
            *pModelViewMatrixInverse = inv_matrix;
            break;
        case SHADER_BUMP:
            *pUmbraColor = v3fNull; // deduced from body_moon
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

    pGlobalVertProj->ModelViewMatrix = matrix;
    pGlobalVertProj->NormalMatrix = inv_matrix.transpose();
    pGlobalVertProj->clipping_fov = prj->getClippingFov();
    pGlobalVertProj->planetRadius = initialRadius;
    pGlobalVertProj->LightPosition = eye_sun;
    pGlobalVertProj->planetScaledRadius = radius;
    pGlobalVertProj->planetOneMinusOblateness = one_minus_oblateness;
    pGlobalFrag->SunHalfAngle = sun_half_angle;
	std::list<Body*>::iterator iter;
	for(iter=satellites.begin(); iter!=satellites.end() && index <= 4; iter++) {
		tmp2 = (*iter)->get_heliocentric_ecliptic_pos() - planet_helio;
		length = tmp2.length();
		tmp2.normalize();
		moonDotLight = tmp2.dot(light);
		if(moonDotLight > 0 && length*sin(acos(moonDotLight)) <= radius + 2*(*iter)->getRadius()) {
			tmp = nav->getHelioToEyeMat() * (*iter)->get_heliocentric_ecliptic_pos();

			if (index==1) {
				pGlobalFrag->MoonPosition1 = tmp;
				pGlobalFrag->MoonRadius1 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}
			else if (index==2) {
				pGlobalFrag->MoonPosition2 = tmp;
				pGlobalFrag->MoonRadius2 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}
			else if (index==3) {
				pGlobalFrag->MoonPosition3 = tmp;
				pGlobalFrag->MoonRadius3 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}
			else if (index==4) {
				pGlobalFrag->MoonPosition4 = tmp;
				pGlobalFrag->MoonRadius4 = (*iter)->getRadius()/(*iter)->getSphereScale();
			}

			index++;
		}
	}

	// clear any leftover values
	for(; index<=4; index++) {
		if (index==1) // No moon data
			pGlobalFrag->MoonRadius1 = 0.0;
		if (index==2)
			pGlobalFrag->MoonRadius2 = 0.0;
		if (index==3)
			pGlobalFrag->MoonRadius3 = 0.0;
		if (index==4)
			pGlobalFrag->MoonRadius4 = 0.0;
	}
	currentObj->draw(screen_sz, drawData->data);
    drawData->update();
	// drawState->use();
	// glActiveTexture(GL_TEXTURE0);

	// StateGL::disable(GL_CULL_FACE);
}

void BigBody::setSphereScale(float s, bool initial_scale)
{
	radius = initialRadius * s;
	if (initial_scale)
		initialScale = s;
	if (rings!=nullptr)
		rings->multiplyRadius(s);

	calculateBoundingRadius();
	updateBoundingRadii();
}

void BigBody::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	Body::update(delta_time, nav, timeMgr);
	if (radius.isScaling() && rings!=nullptr)
		rings->multiplyRadius(radius/initialRadius);
}


void BigBody::drawRings(const Projector* prj, const Observer *obs,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius)
{

	rings->draw(prj,obs->isOnBody(this) ? obs : nullptr,mat,screen_sz,_lightDirection,_planetPosition,planetRadius);

}

void BigBody::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye)
{

	if (isVisible && flags.flag_halo && this->getOnScreenSize(prj, nav) < 10 && this->getSphereScale()<10.0) {
		// Workaround for depth buffer precision and near planets
		halo->drawHalo(nav, prj, eye);
        //StateGL::disable(GL_DEPTH_TEST);
	}

}
