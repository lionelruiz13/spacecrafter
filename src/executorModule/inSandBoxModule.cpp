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

InSandBoxModule::~InSandBoxModule()
{
    if (thread.joinable()) {
        threadQueue.close();
        thread.join();
    }
}

void InSandBoxModule::onEnter()
{
	core->setFlagIngalaxy(MODULE::IN_SANDBOX);
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
	cLog::get()->write("-> ENTREE EN MODE SANDBOX (BAC A SABLE)", LOG_TYPE::L_INFO);
	cLog::get()->write("   Module actuel: " + std::to_string((int)core->getFlagIngalaxy()), LOG_TYPE::L_INFO);
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
    thread = std::thread(&InSandBoxModule::asyncUpdateLoop, this);
	// We enter sandbox mode don't keep any selected object from previous mode
	core->unSelect();
	// No special altitude management in sandbox mode
	// The user can set the altitude he wants via script
}

void InSandBoxModule::onExit()
{
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
	cLog::get()->write("SORTIE DU MODE SANDBOX", LOG_TYPE::L_INFO);
	cLog::get()->write("====================================", LOG_TYPE::L_INFO);
	// We leave sandbox mode, doon't keep any selected object for next mode
	core->unSelect();
    threadQueue.close();
    thread.join();
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

	core->currentTully->update(delta_time);




	//! InGalaxy
	// Position of sun and all the satellites (ie planets)
	core->currentSsystemFactory->computePositions(core->timeMgr->getJDay(), observer);
	core->currentSsystemFactory->updateAnchorManager();

	// Update faders
	core->currentStarLines->update(delta_time);
	core->currentMilkyWay->update(delta_time);
	core->currentDso3d->update(delta_time);



	//! solarSystem
	// update faders and Planet trails (call after nav is updated)
	core->currentSsystemFactory->update(delta_time, core->navigation, core->timeMgr.get());

	// Compute the sun position in local coordinate (use default if no sun in sandbox)
	Vec3d sunPos;
	auto sun = core->currentSsystemFactory->getSun();
	if (sun != nullptr) {
		Vec3d temp(0.,0.,0.);
		sunPos = core->navigation->helioToLocal(temp);
	} else {
		// Fictive position under the horizon to avoid illumination
		sunPos = Vec3d(0., 0., -1.);
	}

	// Compute the Earth position in heliocentric coordinate (use default if no earth in sandbox)
	Vec3d earthPos_helio;
	auto earth = core->currentSsystemFactory->getEarth();
	if (earth != nullptr) {
		earthPos_helio = earth->get_heliocentric_ecliptic_pos();
	} else {
		// Fictive position to avoid lunar eclipse calculation
		earthPos_helio = Vec3d(1., 0., 0.);
	}

	// Compute the moon position in local coordinate (use default if no moon in sandbox)
	Vec3d moonPos;
	Vec3d moonPos_helio;
	auto moon = core->currentSsystemFactory->getMoon();
	if (moon != nullptr) {
		moonPos_helio = moon->get_heliocentric_ecliptic_pos();
		moonPos = core->navigation->helioToLocal(moonPos_helio);
	} else {
		// Fictive position under the horizon
		moonPos = Vec3d(0., 0., -1.);
		moonPos_helio = Vec3d(0., 0., -1.);
	}

	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	core->projection->setModelViewMatrices( core->navigation->getEarthEquToEyeMat(),
											core->navigation->getEarthEquToEyeMatFixed(),
											core->navigation->getHelioToEyeMat(),
											core->navigation->getLocalToEyeMat(),
											core->navigation->getJ2000ToEyeMat(),
											core->navigation->geTdomeMat(),
											core->navigation->getDomeFixedMat());

    asyncUpdateBegin({sunPos, moonPos, earthPos_helio, moonPos_helio});

    // Update faders
	core->currentSkyGridMgr->update(delta_time);
	core->currentSkyLineMgr->update(delta_time);
	core->currentAsterisms->update(delta_time);
	core->currentOort->update(delta_time);

	core->currentToneConverter->setWorldAdaptationLuminance(core->currentAtmosphere->getWorldAdaptationLuminance());

	// Normalize sun and moon position vectors only if they are valid
	if (sunPos.length() > 0.)
		sunPos.normalize();
	if (moonPos.length() > 0.)
		moonPos.normalize();

	// compute global sky brightness TODO : make this more "scientifically"
	// TODO: also add moonlight illumination
	// In sandbox mode without sun, use ambient illumination (unlit)
	if (sun == nullptr) {
		// Ambient illumination in sandbox mode without sun (unlit)
		core->sky_brightness = 0.5;
	} else if (sunPos[2] < -0.1/1.5 ) {
		core->sky_brightness = 0.01;
	} else {
		core->sky_brightness = (0.01 + 1.5*(sunPos[2]+0.1/1.5));
	}
	// TODO make this more generic for non-atmosphere planets
	if (core->currentAtmosphere->getFadeIntensity() == 1) {
		// If the atmosphere is on, a solar eclipse might darken the sky otherwise we just use the sun position calculation above
		core->sky_brightness *= (core->currentAtmosphere->getIntensity()+0.1);
	}
	// TODO: should calculate dimming with solar eclipse even without atmosphere on
	core->currentLandscape->setSkyBrightness(core->sky_brightness+0.05);
}

void InSandBoxModule::draw(int delta_time)
{
	core->applyClippingPlanes(0.01, 2000.01);
	Context::instance->helper->beginDraw(PASS_BACKGROUND, *Context::instance->frame[Context::instance->frameIdx]);

	// Sandbox Mode: Empty environment by default
	// We only draw what has been explicitly added

	//! InUniverse
	core->currentDsoNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);
	// core->universeCloudNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);

	//for VR360 drawing
	core->media->drawVR360(core->projection, core->navigation);

	if (core->currentVolumGalaxy->loaded()) {
		if (core->currentTully->mustBuild())
			core->currentTully->build(core->currentVolumGalaxy.get());
		core->currentTully->draw(observer->getAltitude(), core->navigation, core->projection);
	} else {
		if (core->currentTully->mustBuild())
			core->currentTully->build();
		core->currentTully->draw(observer->getAltitude(), core->navigation, core->projection);
	}

	core->ojmMgr->draw(core->projection, core->navigation, OjmMgr::STATE_POSITION::IN_SANDBOX);

	// core->currentSkyDisplayMgr->drawPerson(core->projection, core->navigation); // See line core->currentSkyDisplayMgr->draw
	core->currentStarGalaxy->draw(core->navigation, core->projection);
	if (core->selected_object && core->object_pointer_visibility)
		core->selected_object.drawPointer(delta_time, core->projection, core->navigation);
	core->currentDsoNav->draw(core->navigation, core->projection);





	//! InGalaxy
	core->currentStarNav->computePosition(core->navigation->getObserverHelioPos());
	core->currentCloudNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);

	core->currentMilkyWay->draw(core->currentToneConverter.get(), core->projection, core->navigation, core->timeMgr->getJulian());

	core->currentStarLines->draw(core->navigation);

	// transparency.
	core->currentDso3d->draw(observer->getAltitude(), core->projection, core->navigation);
	core->currentStarNav->draw(core->navigation, core->projection, false);
	core->currentCloudNav->draw(core->navigation, core->projection);




	//! solarSystem
	core->currentNebulas->draw(core->projection, core->navigation, core->currentToneConverter.get(), core->currentAtmosphere->getFlagShow() ? core->sky_brightness : 0);
	core->currentOort->draw(observer->getAltitude(), core->navigation);
	core->currentIlluminates->draw(core->projection, core->navigation);
	core->currentAsterisms->draw(core->projection, core->navigation);
	core->currentHipStars->draw(core->geodesic_grid, core->currentToneConverter.get(), core->projection, core->timeMgr.get(), core->observatory->getAltitude());
	core->currentSkyGridMgr->draw(core->projection);
	core->currentSkyLineMgr->draw(core->projection, core->navigation, core->timeMgr.get(), core->observatory.get());
	// Draw everything related to sky display (the drawPerson content too, drawPerson is just a restricted draw)
	core->currentSkyDisplayMgr->draw(core->projection, core->navigation, core->selected_object.getEarthEquPos(core->navigation), core->old_selected_object.getEarthEquPos(core->navigation));
	core->currentSsystemFactory->draw(core->projection, core->navigation, observer, core->currentToneConverter.get(), core->currentBodyDecor->canDrawBody() /*aboveHomePlanet*/ );

	// Update meteors
	core->currentMeteors->update(core->projection, core->navigation, core->timeMgr.get(), core->currentToneConverter.get(), delta_time);

	// removed the condition && atmosphere->getFlagShow() so that you can have some by atmosphere
	// if (!aboveHomePlanet && (sky_brightness<0.1) && (observatory->getHomeBody()->getEnglishName() == "Earth" || observatory->getHomeBody()->getEnglishName() == "Mars")) {
	if (core->currentBodyDecor->canDrawMeteor() && (core->sky_brightness<0.1))
		core->currentMeteors->draw(core->projection, core->navigation);

    Context::instance->helper->nextDraw(PASS_FOREGROUND);
	core->currentAtmosphere->draw();

	// Draw the landscape
	if (core->currentBodyDecor->canDrawLandscape()) {
		core->currentLandscape->draw(core->projection, core->navigation);
	}

	core->cardinals_points->draw(core->projection, observer->getLatitude());
}

bool InSandBoxModule::testValidAltitude(double altitude)
{
	// Mode sandbox : on ne change JAMAIS de mode automatiquement par altitude
	// Le changement de mode doit se faire uniquement via script
	return false;
}

void InSandBoxModule::asyncUpdateBegin(AsyncUpdateData data)
{
    asyncWorkState = true;
    threadQueue.push(data);
}

void InSandBoxModule::asyncUpdateEnd()
{
    while (asyncWorkState)
        threadQueue.waitIdle();
}

void InSandBoxModule::asyncUpdateLoop()
{
    AsyncUpdateData data;
    threadQueue.acquire();
    while (threadQueue.pop(data)) {
        core->currentSsystemFactory->computePreDraw(core->projection, core->navigation);

		// Compute the moon phase only if it exists
        float moonPhase = 0.0f;
        auto moon = core->currentSsystemFactory->getMoon();
        auto earth = core->currentSsystemFactory->getEarth();
        if (moon != nullptr && earth != nullptr) {
            moonPhase = moon->get_phase(data.earthPos_helio);
        }

        core->currentAtmosphere->computeColor(core->timeMgr->getJDay(), data.sunPos, data.moonPos,
    	                          moonPhase,
    	                          core->currentToneConverter.get(), core->projection,
								  data.earthPos_helio, data.moonPos_helio,
								  observer->getLatitude(), observer->getAltitude(),
    	                          15.f, 40.f);	// Temperature = 15c, relative humidity = 40%
        core->currentHipStars->preDraw(core->geodesic_grid, core->currentToneConverter.get(), core->projection, core->navigation, core->timeMgr.get(),core->observatory->getAltitude(), core->currentAtmosphere->getFlagShow() && core->FlagAtmosphericRefraction);
        core->currentSsystemFactory->bodyTrace(core->navigation);
        asyncWorkState = false;
    }
    threadQueue.release();
}
