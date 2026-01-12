/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
 * Copyright (C) 2022 Calvin Ruiz
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
#include <future>
#include "coreModule/landscape.hpp"
#include "stellarSystemModule.hpp"
#include "eventModule/event.hpp"
#include "eventModule/event_recorder.hpp"
#include "eventModule/EventScreenFader.hpp"
#include "coreModule/cardinals.hpp"
#include "inGalaxyModule/dso3d.hpp"
#include "coreModule/meteor_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/oort.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/starLines.hpp"
#include "coreModule/illuminate_mgr.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "coreModule/time_mgr.hpp"
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "inGalaxyModule/starNavigator.hpp"

StellarSystemModule::StellarSystemModule(std::shared_ptr<Core> _core, Observer *_observer) :
    core(_core), observer(_observer)
{
    maxAltToGoUp = 1.E10;
    module = MODULE::STELLAR_SYSTEM;
}

StellarSystemModule::~StellarSystemModule()
{
    if (thread.joinable()) {
        threadQueue.close();
        thread.join();
    }
}

void StellarSystemModule::onEnter()
{
	core->setFlagIngalaxy(MODULE::STELLAR_SYSTEM);
    std::cout << "->InStellarSystem" << std::endl;
	Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 0.0);
	EventRecorder::getInstance()->queue(event);
    thread = std::thread(&StellarSystemModule::asyncUpdateLoop, this);
    center = observer->getObserverCenterPoint();
    core->currentStarNav->computePosition(center);
    // We should inject the starNav stars into the hip_star_mgr
    core->currentSsystemFactory->enterSystem();
    core->setFlagTracking(false); // Just in case
    core->selectObject(core->currentSsystemFactory->getSelected());
}

void StellarSystemModule::onExit()
{
	std::cout << "InStellarSystem->" << std::endl;
    threadQueue.close();
    thread.join();
    core->currentSsystemFactory->leaveSystem();
}


//! Update all the objects in function of the time
void StellarSystemModule::update(int delta_time)
{
	if( core->firstTime ) // Do not update prior to Init. Causes intermittent problems at startup
		return;

    core->update(delta_time);

	// Update the position of observation and time etc...
	observer->update(delta_time);
	core->timeMgr->update(delta_time);
	core->navigation->update(delta_time);

    if (core->selected_object && core->observatory->getAltitude() <= 7.91706e+08){
        int hip = core->currentHipStars->getHPFromStarName(core->selected_object.getNameI18n());
        if (hip != -1)
            core->currentStarNav->hideStar(hip);
    }

    if (core->selected_object && core->observatory->getAltitude() >= 7.91706e+08){
        int hip = 0;
        hip = core->currentHipStars->getHPFromStarName(core->selected_object.getNameI18n());
        if (hip != -1){
            core->currentStarNav->showStar(hip);
        }
    }

    core->currentStarNav->computePosition(center);

	// Position of sun and all the satellites (ie planets)
	core->currentSsystemFactory->computePositions(core->timeMgr->getJDay(), observer);

	core->currentSsystemFactory->updateAnchorManager();
	// Transform matrices between coordinates systems
	core->navigation->updateTransformMatrices(observer, core->timeMgr->getJDay());
	// Direction of vision
	core->navigation->updateVisionVector(delta_time, core->selected_object);
	// Field of view
	core->projection->updateAutoZoom(delta_time, core->FlagManualZoom);
	// update faders and Planet trails (call after nav is updated)
	core->currentSsystemFactory->update(delta_time, core->navigation, core->timeMgr.get());

	// Move the view direction and/or fov
	core->updateMove(delta_time);

	// Compute the sun position in local coordinate
	Vec3d sunPos = core->navigation->helioToLocal(ProtoSystem::getCenterPos());

	// Compute the moon position in local coordinate
	Vec3d moon = core->currentSsystemFactory->getMoon()->get_heliocentric_ecliptic_pos();
	Vec3d moonPos = core->navigation->helioToLocal(moon);
	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	core->projection->setModelViewMatrices( core->navigation->getEarthEquToEyeMat(),
	                                    core->navigation->getEarthEquToEyeMatFixed(),
	                                    core->navigation->getHelioToEyeMat(),
	                                    core->navigation->getLocalToEyeMat(),
	                                    core->navigation->getJ2000ToEyeMat(),
	                                    core->navigation->geTdomeMat(),
	                                    core->navigation->getDomeFixedMat());

    asyncUpdateBegin({sunPos, moonPos});

    // Update faders
	core->currentSkyGridMgr->update(delta_time);
	core->currentSkyLineMgr->update(delta_time);
	core->currentAsterisms->update(delta_time);
	core->currentMilkyWay->update(delta_time);
	core->currentStarLines->update(delta_time);

	core->currentToneConverter->setWorldAdaptationLuminance(core->currentAtmosphere->getWorldAdaptationLuminance());

	sunPos.normalize();
	moonPos.normalize();

	// compute global sky brightness TODO : make this more "scientifically"
	// TODO: also add moonlight illumination
	if (sunPos[2] < -0.1/1.5 ) core->sky_brightness = 0.01;
	else core->sky_brightness = (0.01 + 1.5*(sunPos[2]+0.1/1.5));
	// TODO make this more generic for non-atmosphere planets
	if (core->currentAtmosphere->getFadeIntensity() == 1) {
		// If the atmosphere is on, a solar eclipse might darken the sky otherwise we just use the sun position calculation above
		core->sky_brightness *= (core->currentAtmosphere->getIntensity()+0.1);
	}
	// TODO: should calculate dimming with solar eclipse even without atmosphere on
	core->currentLandscape->setSkyBrightness(core->sky_brightness+0.05);
}

void StellarSystemModule::draw(int delta_time)
{
    Context::instance->helper->beginDraw(PASS_BACKGROUND, *Context::instance->frame[Context::instance->frameIdx]); // multisample print
    asyncUpdateEnd();
	core->applyClippingPlanes(0.000001 ,200);
	core->currentMilkyWay->draw(core->currentToneConverter.get(), core->projection, core->navigation, core->timeMgr->getJulian());
	//for VR360 drawing
	core->media->drawVR360(core->projection, core->navigation);
	core->currentNebulas->draw(core->projection, core->navigation, core->currentToneConverter.get(), core->currentAtmosphere->getFlagShow() ? core->sky_brightness : 0);
	core->currentIlluminates->draw(core->projection, core->navigation);
	core->currentAsterisms->draw(core->projection, core->navigation);
	core->currentStarLines->draw(core->navigation);
    core->currentStarNav->draw(core->navigation, core->projection, true);
	core->currentSkyGridMgr->draw(core->projection);
	core->currentSkyLineMgr->draw(core->projection, core->navigation, core->timeMgr.get(), core->observatory.get());
	core->currentSkyDisplayMgr->draw(core->projection, core->navigation, core->selected_object.getEarthEquPos(core->navigation), core->old_selected_object.getEarthEquPos(core->navigation));
	core->currentSsystemFactory->draw(core->projection, core->navigation, observer, core->currentToneConverter.get(), core->currentBodyDecor->canDrawBody() /*aboveHomePlanet*/ );

	// Draw the pointer on the currently selected object
	// TODO: this would be improved if pointer was drawn at same time as object for correct depth in scene
	if (core->selected_object && core->object_pointer_visibility) core->selected_object.drawPointer(delta_time, core->projection, core->navigation);

	// Update meteors
	core->currentMeteors->update(core->projection, core->navigation, core->timeMgr.get(), core->currentToneConverter.get(), delta_time);

	// retiré la condition && atmosphere->getFlagShow() de sorte à pouvoir en avoir par atmosphère ténue
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

bool StellarSystemModule::testValidAltitude(double altitude)
{
	if (altitude>maxAltToGoUp) {
		std::cout << "Swapping to mode Stellar System" << std::endl;
		nextMode = upMode;
		if (upMode == nullptr)
			std::cout << "upMode not defined" << std::endl;
		else
			std::cout << "upMode is defined" << std::endl;
		return true;
	}
	return false;
}

void StellarSystemModule::asyncUpdateBegin(std::pair<Vec3d, Vec3d> data)
{
    asyncWorkState = true;
    threadQueue.push(data);
}

void StellarSystemModule::asyncUpdateEnd()
{
    while (asyncWorkState)
        threadQueue.waitIdle();
}

void StellarSystemModule::asyncUpdateLoop()
{
    // std::pair<sunPos, moonMos>
    std::pair<Vec3d, Vec3d> data;
    threadQueue.acquire();
    while (threadQueue.pop(data)) {
        core->currentSsystemFactory->computePreDraw(core->projection, core->navigation);
        core->currentAtmosphere->computeColor(core->timeMgr->getJDay(), data.first, data.second,
    	                          core->currentSsystemFactory->getMoon()->get_phase(core->currentSsystemFactory->getEarth()->get_heliocentric_ecliptic_pos()),
    	                          core->currentToneConverter.get(), core->projection, observer->getLatitude(), observer->getAltitude(),
    	                          15.f, 40.f);	// Temperature = 15c, relative humidity = 40%
        core->currentHipStars->preDraw(core->geodesic_grid, core->currentToneConverter.get(), core->projection, core->navigation, core->timeMgr.get(),core->observatory->getAltitude(), core->currentAtmosphere->getFlagShow() && core->FlagAtmosphericRefraction);
        core->currentSsystemFactory->bodyTrace(core->navigation);
        asyncWorkState = false;
    }
    threadQueue.release();
}
