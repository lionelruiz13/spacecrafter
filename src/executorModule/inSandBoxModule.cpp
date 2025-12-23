/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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

#include <iostream>
#include "inSandBoxModule.hpp"
#include "eventModule/event.hpp"
#include "eventModule/event_recorder.hpp"
#include "eventModule/EventScreenFader.hpp"
#include "tools/log.hpp"

#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/meteor_mgr.hpp"
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/illuminate_mgr.hpp"
#include "coreModule/cardinals.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "inGalaxyModule/dso3d.hpp"
#include "inGalaxyModule/cloudNavigator.hpp"
#include "inGalaxyModule/dsoNavigator.hpp"
#include "coreModule/starLines.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "inGalaxyModule/starNavigator.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "inGalaxyModule/starGalaxy.hpp"
#include "coreModule/tully.hpp"
#include "coreModule/volumObj3D.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "coreModule/oort.hpp"
#include "coreModule/landscape.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"

InSandBoxModule::InSandBoxModule(std::shared_ptr<Core> _core, Observer *_observer) : core(_core), observer(_observer)
{
	module = MODULE::IN_SANDBOX;

    minAltToGoDown = 1.E10;
    maxAltToGoUp = 1.E14;
}

void InSandBoxModule::onEnter()
{
	core->setFlagIngalaxy(MODULE::IN_SANDBOX);
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
	cLog::get()->write("-> ENTREE EN MODE SANDBOX (BAC A SABLE)", LOG_TYPE::L_INFO);
	cLog::get()->write("   Module actuel: " + std::to_string((int)core->getFlagIngalaxy()), LOG_TYPE::L_INFO);
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
	// Pas de gestion d'altitude spéciale en mode sandbox
	// L'utilisateur peut définir l'altitude qu'il souhaite via script
}

void InSandBoxModule::onExit()
{
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
	cLog::get()->write("SORTIE DU MODE SANDBOX", LOG_TYPE::L_INFO);
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
}

void InSandBoxModule::update(int delta_time)
{
	//! InUniverse
	// Update the position of observation and time etc...
	observer->update(delta_time);
	core->timeMgr->update(delta_time);
	core->navigation->update(delta_time);
	// Transform matrices between coordinates systems
	core->navigation->updateTransformMatrices(observer, core->timeMgr->getJDay());
	// Direction of vision
	core->navigation->updateVisionVector(delta_time, core->selected_object);
	// Field of view
	core->projection->updateAutoZoom(delta_time, core->FlagManualZoom);
	// Move the view direction and/or fov
	core->updateMove(delta_time);
	// Update faders
	core->update(delta_time);

	core->sandboxTully->update(delta_time);




	//! InGalaxy
	// Position of sun and all the satellites (ie planets)
	core->sandboxSsystemFactory->computePositions(core->timeMgr->getJDay(), observer);
	core->sandboxSsystemFactory->updateAnchorManager();

	// Update faders
	core->sandboxStarLines->update(delta_time);
	core->sandboxMilkyWay->update(delta_time);
	core->sandboxDso3d->update(delta_time);



	//! solarSystem
	// update faders and Planet trails (call after nav is updated)
	core->sandboxSsystemFactory->update(delta_time, core->navigation, core->timeMgr.get());

	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	core->projection->setModelViewMatrices( core->navigation->getEarthEquToEyeMat(),
											core->navigation->getEarthEquToEyeMatFixed(),
											core->navigation->getHelioToEyeMat(),
											core->navigation->getLocalToEyeMat(),
											core->navigation->getJ2000ToEyeMat(),
											core->navigation->geTdomeMat(),
											core->navigation->getDomeFixedMat());

    // Update faders
	core->sandboxSkyGridMgr->update(delta_time);
	core->sandboxSkyLineMgr->update(delta_time);
	core->sandboxAsterisms->update(delta_time);
	core->oort->update(delta_time);

	core->tone_converter->setWorldAdaptationLuminance(core->atmosphere->getWorldAdaptationLuminance());

	// TODO make this more generic for non-atmosphere planets
	if (core->atmosphere->getFadeIntensity() == 1) {
		// If the atmosphere is on, a solar eclipse might darken the sky otherwise we just use the sun position calculation above
		core->sky_brightness *= (core->atmosphere->getIntensity()+0.1);
	}
	// TODO: should calculate dimming with solar eclipse even without atmosphere on
	core->landscape->setSkyBrightness(core->sky_brightness+0.05);
}

void InSandBoxModule::draw(int delta_time)
{
	core->applyClippingPlanes(0.01, 2000.01);
	Context::instance->helper->beginDraw(PASS_BACKGROUND, *Context::instance->frame[Context::instance->frameIdx]);

	// Sandbox Mode: Empty environment by default
	// We only draw what has been explicitly added

	//! InUniverse
	core->sandboxDsoNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);
	// core->universeCloudNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);

	//for VR360 drawing
	core->media->drawVR360(core->projection, core->navigation);

	if (core->sandboxVolumGalaxy->loaded()) {
		if (core->sandboxTully->mustBuild())
			core->sandboxTully->build(core->sandboxVolumGalaxy.get());
		core->sandboxTully->draw(observer->getAltitude(), core->navigation, core->projection);
	} else {
		if (core->sandboxTully->mustBuild())
			core->sandboxTully->build();
		core->sandboxTully->draw(observer->getAltitude(), core->navigation, core->projection);
	}

	core->ojmMgr->draw(core->projection, core->navigation, OjmMgr::STATE_POSITION::IN_SANDBOX);

	core->sandboxSkyDisplayMgr->drawPerson(core->projection, core->navigation);
	core->sandboxStarGalaxy->draw(core->navigation, core->projection);
	if (core->selected_object && core->object_pointer_visibility)
		core->selected_object.drawPointer(delta_time, core->projection, core->navigation);
	core->sandboxDsoNav->draw(core->navigation, core->projection);





	//! InGalaxy
	core->sandboxStarNav->computePosition(core->navigation->getObserverHelioPos());
	core->sandboxCloudNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);

	core->sandboxMilkyWay->draw(core->tone_converter, core->projection, core->navigation, core->timeMgr->getJulian());

	core->sandboxStarLines->draw(core->navigation);

	// transparency.
	core->sandboxDso3d->draw(observer->getAltitude(), core->projection, core->navigation);
	core->sandboxStarNav->draw(core->navigation, core->projection, false);
	core->sandboxCloudNav->draw(core->navigation, core->projection);




	//! solarSystem
	core->sandboxNebulas->draw(core->projection, core->navigation, core->tone_converter, core->atmosphere->getFlagShow() ? core->sky_brightness : 0);
	core->oort->draw(observer->getAltitude(), core->navigation);
	core->sandboxIlluminates->draw(core->projection, core->navigation);
	core->sandboxAsterisms->draw(core->projection, core->navigation);
	// TODO: Use the sandboxHipStars (cause a crash for now (error with vulkan))
	core->hip_stars->draw(core->geodesic_grid, core->tone_converter, core->projection, core->timeMgr.get(), core->observatory->getAltitude());
	core->sandboxSkyGridMgr->draw(core->projection);
	core->sandboxSkyLineMgr->draw(core->projection, core->navigation, core->timeMgr.get(), core->observatory.get());
	core->sandboxSkyDisplayMgr->draw(core->projection, core->navigation, core->selected_object.getEarthEquPos(core->navigation), core->old_selected_object.getEarthEquPos(core->navigation));
	core->sandboxSsystemFactory->draw(core->projection, core->navigation, observer, core->tone_converter, core->bodyDecor->canDrawBody() /*aboveHomePlanet*/ );

	// Update meteors
	core->sandboxMeteors->update(core->projection, core->navigation, core->timeMgr.get(), core->tone_converter, delta_time);

	// removed the condition && atmosphere->getFlagShow() so that you can have some by atmosphere
	// if (!aboveHomePlanet && (sky_brightness<0.1) && (observatory->getHomeBody()->getEnglishName() == "Earth" || observatory->getHomeBody()->getEnglishName() == "Mars")) {
	if (core->bodyDecor->canDrawMeteor() && (core->sky_brightness<0.1))
		core->sandboxMeteors->draw(core->projection, core->navigation);

    Context::instance->helper->nextDraw(PASS_FOREGROUND);
	core->atmosphere->draw();

	// Draw the landscape
	if (core->bodyDecor->canDrawLandscape()) {
		core->landscape->draw(core->projection, core->navigation);
	}

	core->cardinals_points->draw(core->projection, observer->getLatitude());
}

bool InSandBoxModule::testValidAltitude(double altitude)
{
	// Mode sandbox : on ne change JAMAIS de mode automatiquement par altitude
	// Le changement de mode doit se faire uniquement via script
	return false;
}
