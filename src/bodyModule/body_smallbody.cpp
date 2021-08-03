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

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"

SmallBody::SmallBody(Body *parent,
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
					 std::shared_ptr<BodyTexture> _bodyTexture,
                     ThreadContext *context):
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
		 _bodyTexture,
         context
        )
{
	if (_typePlanet == COMET) {
		trail = new Trail(this,2920);
		orbitPlot = new Orbit2D(this, 4800);
	}
	else {
		trail = new Trail(this, 60);
		orbitPlot = new Orbit2D(this);
	}

    drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	selectShader();
}

SmallBody::~SmallBody()
{
	if (trail) delete trail;
	trail = nullptr;
	if (orbitPlot) delete orbitPlot;
	orbitPlot = nullptr;
}

void SmallBody::selectShader ()
{
	bool useShaderNormal = true;

	if (tex_norm) { //bump Shader
		myShader = SHADER_BUMP;
        drawState = BodyShader::getShaderBump();

        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout, -1, false); // Don't initialize, save time and memory if never drawn
        uUmbraColor = std::make_unique<Uniform>(context->surface, sizeof(*pUmbraColor));
        pUmbraColor = static_cast<typeof(pUmbraColor)>(uUmbraColor->data);
        set->bindUniform(uUmbraColor.get(), 2);
        set->bindTexture(tex_current->getTexture(), 3);
        set->bindTexture(tex_norm->getTexture(), 4);
        set->bindTexture(tex_eclipse_map->getTexture(), 5);
		useShaderNormal = false;
	}
    if (useShaderNormal) {
		myShader = SHADER_NORMAL;
		drawState = BodyShader::getShaderNormal();

        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout, -1, false); // Don't initialize, save time and memory if never drawn
        set->bindTexture(tex_current->getTexture(), 2);
        set->bindTexture(tex_eclipse_map->getTexture(), 3);
	}
}

void SmallBody::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	// StateGL::enable(GL_CULL_FACE);
	// StateGL::disable(GL_BLEND);

	//glBindTexture(GL_TEXTURE_2D, tex_current->getID());

	// myShaderProg->use();

    switch (commandIndex) {
        case -2: // Command not builded
            // Create general uniforms and bind them
            uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
            pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
            set->bindUniform(uGlobalVertProj.get(), 0);
            uGlobalFrag = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalFrag));
            pGlobalFrag = static_cast<typeof(pGlobalFrag)>(uGlobalFrag->data);
            set->bindUniform(uGlobalFrag.get(), 1);
            pGlobalFrag->MoonRadius1 = 0;
            pGlobalFrag->MoonRadius2 = 0;
            pGlobalFrag->MoonRadius3 = 0;
            pGlobalFrag->MoonRadius4 = 0;
            // Begin build command
            commandIndex = -1;
            if (!context->commandMgr->isRecording()) {
                commandIndex = context->commandMgr->getCommandIndex();
                context->commandMgr->init(commandIndex);
                context->commandMgr->beginRenderPass(renderPassType::CLEAR_DEPTH_BUFFER_DONT_SAVE);
            }
            context->commandMgr->bindPipeline(drawState->pipeline);
            currentObj->bind(context->commandMgr);
            context->commandMgr->bindSet(drawState->layout, set.get());
            context->commandMgr->bindSet(drawState->layout, context->global->globalSet, 1);
            context->commandMgr->indirectDrawIndexed(drawData.get());
            context->commandMgr->compile(); // There is no halo for a moon
            return;
        case -1: break;
        default:
            context->commandMgr->setSubmission(commandIndex);
    }
	//load specific values for shader
    Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));
	Mat4f inv_matrix = matrix.inverse();
    if (myShader == SHADER_BUMP) {
        *pUmbraColor = v3fNull;
    }
    pGlobalVertProj->ModelViewMatrix = matrix;
    pGlobalVertProj->NormalMatrix = inv_matrix.transpose();
    pGlobalVertProj->clipping_fov = prj->getClippingFov();
    pGlobalVertProj->planetRadius = initialRadius;
    pGlobalVertProj->LightPosition = eye_sun;
    pGlobalVertProj->planetScaledRadius = radius;
    pGlobalVertProj->planetOneMinusOblateness = one_minus_oblateness;
    pGlobalFrag->SunHalfAngle = sun_half_angle;

	// Vec3f tmp= v3fNull;
	// Vec3f tmp2(0.4, 0.12, 0.0);
    //int index=1;
	Vec3d planet_helio = get_heliocentric_ecliptic_pos();
	Vec3d light = -planet_helio;
	light.normalize();

	// clear any leftover values
	// for(; index<=4; index++) {
	// 	if (index==1) // No moon data
	// 		myShaderProg->setUniform("MoonRadius1",0.0);
	// 	if (index==2)
	// 		myShaderProg->setUniform("MoonRadius2",0.0);
	// 	if (index==3)
	// 		myShaderProg->setUniform("MoonRadius3",0.0);
	// 	if (index==4)
	// 		myShaderProg->setUniform("MoonRadius4",0.0);
	// }

	currentObj->draw(screen_sz, drawData->data);

	//myShaderProg->unuse();
	//glActiveTexture(GL_TEXTURE0);
	//StateGL::disable(GL_CULL_FACE);
}
