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

#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/Buffer.hpp"

Moon::Moon(Body *parent,
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
		   std::shared_ptr<BodyTexture> _bodyTexture,
		   ThreadContext *context):
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
		 _bodyTexture,
         context)
{
	if (_bodyTexture->tex_night != "") {
		tex_night = new s_texture(FilePath(_bodyTexture->tex_night,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
	}
	//more adding could be placed here for the constructor of Moon
    drawData = std::make_unique<Buffer>(context->surface, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	selectShader();
	orbitPlot = new Orbit3D(this);
}

Moon::~Moon()
{
	if (orbitPlot) delete orbitPlot;
}

void Moon::selectShader()
{
	//bool useShaderMoonNormal = true;
	if (tex_heightmap!=nullptr) { //altimetry Shader
		myShader = SHADER_MOON_NORMAL_TES;
		drawState = BodyShader::getShaderMoonNormalTes();

        set = std::make_unique<Set>(context->surface, context->setMgr, drawState->layout);
        uGlobalVertProj = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalVertProj));
        pGlobalVertProj = static_cast<typeof(pGlobalVertProj)>(uGlobalVertProj->data);
        set->bindUniform(uGlobalVertProj.get(), 0);
        uMoonFrag = std::make_unique<Uniform>(context->surface, sizeof(*pMoonFrag));
        pMoonFrag = static_cast<typeof(pMoonFrag)>(uMoonFrag->data);
        set->bindUniform(uMoonFrag.get(), 1);
        uGlobalTescGeom = std::make_unique<Uniform>(context->surface, sizeof(*pGlobalTescGeom));
        pGlobalTescGeom = static_cast<typeof(pGlobalTescGeom)>(uGlobalTescGeom->data);
        set->bindUniform(uGlobalTescGeom.get(), 2);
        set->bindTexture(tex_current->getTexture(), 5);
        set->bindTexture(tex_norm->getTexture(), 6);
        set->bindTexture(tex_eclipse_map->getTexture(), 7);
        set->bindTexture(tex_heightmap->getTexture(), 8);
		return;
	}

	if (tex_night!=nullptr) { //altimetry Shader
		myShader = SHADER_MOON_NIGHT;
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
		return;
	}

	if (tex_norm!=nullptr) { //bump Shader
		myShader = SHADER_MOON_BUMP;
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
	//if (useShaderMoonNormal) { // normal shaders
	myShader = SHADER_MOON_NORMAL;
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
	//}
}

void Moon::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	// StateGL::enable(GL_CULL_FACE);
	// StateGL::disable(GL_BLEND);

	//glBindTexture(GL_TEXTURE_2D, tex_current->getID());

	//myShaderProg->use();
    switch (commandIndex) {
        case -2: // Command not builded
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

    Vec3f tmp= v3fNull;
	Vec3f tmp2(0.4, 0.12, 0.0);
    Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(M_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
    //load specific values for shader
	switch (myShader) {
        case SHADER_MOON_NORMAL_TES: // myMoon
            pMoonFrag->MoonPosition1 = nav->getHelioToEyeMat() * parent->get_heliocentric_ecliptic_pos();
            pMoonFrag->MoonRadius1 = parent->getRadius();
            pMoonFrag->UmbraColor = (getEnglishName() == "Moon") ? tmp2 : tmp;
            pMoonFrag->SunHalfAngle = sun_half_angle;
            pGlobalTescGeom->TesParam = Vec3i(bodyTesselation->getMinTesLevel(),bodyTesselation->getMaxTesLevel(), bodyTesselation->getMoonAltimetryFactor());
            break;
		case SHADER_MOON_BUMP:
            *pUmbraColor = (getEnglishName() == "Moon") ? tmp2 : tmp;
            [[fallthrough]];
		case SHADER_MOON_NIGHT:
		case SHADER_MOON_NORMAL:
		default: // Common uniform affectation
            pGlobalFrag->MoonPosition1 = nav->getHelioToEyeMat() * parent->get_heliocentric_ecliptic_pos();
            pGlobalFrag->MoonRadius1 = parent->getRadius();
            pGlobalFrag->SunHalfAngle = sun_half_angle;
			break;
	}
    pGlobalVertProj->ModelViewMatrix = matrix;
    pGlobalVertProj->NormalMatrix = inv_matrix.transpose();
    pGlobalVertProj->clipping_fov = prj->getClippingFov();
    pGlobalVertProj->planetRadius = initialRadius;
    pGlobalVertProj->LightPosition = eye_sun;
    pGlobalVertProj->planetScaledRadius = radius;
    pGlobalVertProj->planetOneMinusOblateness = one_minus_oblateness;
    /*
	//paramétrage des matrices pour opengl4
	myShaderProg->setUniform("ModelViewProjectionMatrix",proj*matrix);
	myShaderProg->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("clipping_fov",prj->getClippingFov());
	myShaderProg->setUniform("planetScaledRadius",radius);

	//paramètres commun aux shaders sauf Sun
	myShaderProg->setUniform("planetRadius",initialRadius);
	myShaderProg->setUniform("planetOneMinusOblateness",one_minus_oblateness);
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("NormalMatrix", inv_matrix.transpose());

	//int index=1;
	myShaderProg->setUniform("LightPosition",eye_sun);
	myShaderProg->setUniform("SunHalfAngle",sun_half_angle);


	if (myShader == SHADER_MOON_BUMP || myShader == SHADER_MOON_NORMAL_TES) {
		if(getEnglishName() == "Moon")
			myShaderProg->setUniform("UmbraColor",tmp2);
		else
			myShaderProg->setUniform("UmbraColor",tmp);
	}

	Vec3d planet_helio = get_heliocentric_ecliptic_pos();
	Vec3d light = -planet_helio;
	light.normalize();

	// Parent may shadow this satellite
	tmp = nav->getHelioToEyeMat() * parent->get_heliocentric_ecliptic_pos();
	myShaderProg->setUniform("MoonPosition1",tmp);
	myShaderProg->setUniform("MoonRadius1",parent->getRadius());

	//tesselation
	if ( myShader == SHADER_MOON_NORMAL_TES) {
		myShaderProg->setUniform("TesParam",
				Vec3i(bodyTesselation->getMinTesLevel(),bodyTesselation->getMaxTesLevel(), bodyTesselation->getMoonAltimetryFactor() ));
	}
    */

    currentObj->draw(screen_sz, drawData->data);
    drawData->update();
	//myShaderProg->unuse();

	//glActiveTexture(GL_TEXTURE0);
	//StateGL::disable(GL_CULL_FACE);
}

void Moon::handleVisibilityFader(const Observer* observatory, const Projector* prj, const Navigator * nav)
{
	if (prj->getFov()>30) {
		// If not in the parent Body system OR one of the other sister moons do not draw
		if (observatory->isOnBody() && !observatory->isOnBody(parent) && parent != observatory->getHomeBody()->get_parent() && getEarthEquPos(nav).length() > 1) {
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
