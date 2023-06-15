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
#ifndef BODY_ARTIFICIAL_HPP_
#define BODY_ARTIFICIAL_HPP_

#include "bodyModule/orbit_2d.hpp"
#include "tools/app_settings.hpp"
#include "bodyModule/body_artificial.hpp"
#include "bodyModule/body_color.hpp"
#include "bodyModule/orbit_3d.hpp"
#include "coreModule/coreLink.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/trail.hpp"
#include "bodyModule/hints.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "ojmModule/ojm.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

class ShadowExtension {
public:
	ShadowExtension(VulkanMgr &master, SetMgr &mgr, BufferMgr &umgr, PipelineLayout *shadowLayout, PipelineLayout *shapeLayout) :
		shadow(umgr, sizeof(OjmShadowFrag) + sizeof(UShadowingBody) * Context::instance->maxShadowCast), shadowSet(master, mgr, shadowLayout, 2, true, true), traceSet(master, mgr, shapeLayout, -1, true, true)
	{
		traceSet.bindUniform(shadow, 0);
		shadowSet.bindUniform(shadow, 2);
		shadowSet.bindTexture(*Context::instance->shadowBuffer, 3);
		shadowSet.bindTexture(*Context::instance->shadow, 4);
	}
	SharedBuffer<OjmShadowFrag> shadow;
	Set shadowSet;
	Set traceSet;
};

Artificial::Artificial(std::shared_ptr<Body> parent,
                       const std::string& englishName,
                       bool flagHalo,
                       double radius,
                       std::unique_ptr<BodyColor> _myColor,
                       float _sol_local_day,
                       float albedo,
                       std::unique_ptr<Orbit> orbit,
                       bool close_orbit,
                       const std::string& model_name,
                       bool _deleteable,
                       double orbit_bounding_radius,
					   const BodyTexture &_bodyTexture
                      ):
	Body(parent,
	     englishName,
	     BODY_TYPE::ARTIFICIAL,
	     flagHalo,
	     radius,
	     0.0,
	     std::move(_myColor),
	     _sol_local_day,
	     albedo,
	     std::move(orbit),
	     close_orbit,
	     nullptr,
	     orbit_bounding_radius,
	     _bodyTexture), uProj(*Context::instance->uniformMgr), uLight(*Context::instance->uniformMgr), uVert(*Context::instance->uniformMgr)
{
	selectShader();
	obj3D = Ojm::load(AppSettings::Instance()->getModel3DDir() + model_name+"/" + model_name+".ojm", AppSettings::Instance()->getModel3DDir() + model_name+"/");
    initialRadius *= obj3D->getRadius();
    this->radius = initialRadius;
	if (!obj3D->getOk()) {
		std::cout << "Error with " << englishName << " " << model_name << std::endl;
        this->radius = initialRadius = 0; // Prevent it from being drawn
    }
    orbitPlot = std::make_unique<Orbit3D>(this);
    auto &vkmgr = *VulkanMgr::instance;
    auto &context = *Context::instance;
    set = std::make_unique<Set>(vkmgr, *context.setMgr, drawState->layout, 2, false, true);
    set->bindUniform(uVert, 0);
    set->bindUniform(uProj, 1);
    set->bindUniform(uLight, 2);
    // if (orbit_bounding_radius <= 0) {
    //     orbitPlot->computeOrbit(CoreLink::instance->getJDay(), true);
    //     orbit_bounding_radius = orbitPlot->computeOrbitBoundingRadius();
    // }
}

Artificial::~Artificial()
{
	//if (obj3D) delete obj3D;
}

void Artificial::selectShader ()
{
    if (isCenterOfInterest) {
        myShader = SHADER_ARTIFICIAL_SHADOW;
        drawState = BodyShader::getShaderArtificialShadowed();
    } else {
        myShader = SHADER_ARTIFICIAL;
        drawState = BodyShader::getShaderArtificial();
    }
}

void Artificial::drawBody(VkCommandBuffer cmd, const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz, bool depthTest)
{
    if (changed) {
        selectShader();
        changed = false;
    }

    auto &context = *Context::instance;

    Mat4f matrix = mat.convert() * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));
    matrix.setMat3(uVert->normal);
    uVert->radius = radius;
    uProj->ModelViewMatrix = matrix * Mat4f::scaling(radius);
    uProj->clipping_fov = prj->getClippingFov();
    if (isCenterOfInterest) {
        ext->shadow->ModelMatrix = model.convert();
        ext->shadow->lightIntensity =Vec3f(1.0, 1.0, 1.0);
        ext->shadow->lightDirection = -lightDirection;
        drawState->layout->bindSet(cmd, *context.uboSet);
        drawState->layout->bindSet(cmd, ext->shadowSet, 2);
        obj3D->record(cmd, drawState->pipeline, drawState->layout);
        Context::instance->helper->selfShadow(this);
    } else {
        uLight->Intensity =Vec3f(1.0, 1.0, 1.0);
        uLight->Position = eye_sun;
        drawState->layout->bindSet(cmd, *context.uboSet);
        drawState->layout->bindSet(cmd, *set, 2);
        obj3D->record(cmd, depthTest ? drawState->pipeline : drawState->pipelineNoDepth, drawState->layout);
    }
}

void Artificial::drawShadow(VkCommandBuffer drawCmd)
{
    BodyShader::getShaderShadowTrace()->pipeline->bind(drawCmd);
    BodyShader::getShaderShadowTrace()->layout->bindSet(drawCmd, ext->traceSet);
    obj3D->drawShadow(drawCmd);
}

void Artificial::drawShadow(VkCommandBuffer drawCmd, int idx)
{
	BodyShader::getShaderShadowShape()->pipeline->bind(drawCmd);
	BodyShader::getShaderShadowShape()->layout->bindSet(drawCmd, *Context::instance->shadowData[idx].traceSet);
    obj3D->drawShadow(drawCmd);
}

void Artificial::bindShadows(const ShadowRenderData &renderData)
{
	if (!ext) {
		ext = std::make_unique<ShadowExtension>(*VulkanMgr::instance, *Context::instance->setMgr, *Context::instance->uniformMgr, BodyShader::getShaderArtificialShadowed()->layout, BodyShader::getShaderShadowTrace()->layout);
		ext->shadowSet.bindUniform(uVert, 0);
		ext->shadowSet.bindUniform(uProj, 1);
	}
    auto &frag = *ext->shadow;
    auto m = renderData.lookAt * (model * Mat4d::zrotation(M_PI/180*(axis_rotation + 90)));
    m.setMat3(frag.ShadowMatrix);
    // frag.sinSunAngle = 2 * renderData.sinSunHalfAngle;
    frag.nbShadowingBodies = renderData.shadowingBodies.size();
    for (uint8_t i = 0; i < renderData.shadowingBodies.size(); ++i) {
        frag.shadowingBodies[i].posRadius = renderData.shadowingBodies[i].posRadius / radius;
        frag.shadowingBodies[i].idx = renderData.shadowingBodies[i].idx;
    }
}

void Artificial::drawOrbit(VkCommandBuffer cmdBodyDepth, VkCommandBuffer cmdOrbit, const Observer* observatory, const Navigator* nav, const Projector* prj)
{
    if (isVisibleOnScreen()) {
        depthTraceInfo pdata {mat.convert(), prj->getClippingFov(), (float) radius, (float) one_minus_oblateness};
        BodyShader::getShaderDepthTrace()->layout->pushConstant(cmdBodyDepth, 0, &pdata);
        obj3D->drawShadow(cmdBodyDepth);
    }
    if (orbitPlot)
        orbitPlot->drawOrbit(cmdOrbit, nav, prj, parent_mat);
}


#endif /* end of include guard: BODY_ARTIFICIAL_HPP_ */
