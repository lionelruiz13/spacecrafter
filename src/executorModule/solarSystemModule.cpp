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
#include <future>

#include "solarSystemModule.hpp"
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


void SolarSystemModule::onEnter()
{    
    std::cout << "J'arrive dans InSolarSystem" << std::endl;
	Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 0.0);
	EventRecorder::getInstance()->queue(event);
	// TODO
	//réglage de l'altitude dans CoreExecutorInSolarSystem la première fois
	if (observer->getAltitude() < maxAltToGoUp)
		observer->setAltitude(observer->getAltitude() *1.E6);
}

void SolarSystemModule::onExit()
{
	std::cout << "Je quitte InSolarSystem" << std::endl;
	core->timeMgr->setTimeSpeed(1);
}


//! Update all the objects in function of the time
void SolarSystemModule::update(int delta_time)
{
	if( core->firstTime ) // Do not update prior to Init. Causes intermittent problems at startup
		return;

	// Update the position of observation and time etc...
	observer->update(delta_time);
	core->timeMgr->update(delta_time);
	core->navigation->update(delta_time);

	// Position of sun and all the satellites (ie planets)
	core->ssystemFactory->computePositions(core->timeMgr->getJDay(), observer);

	core->ssystemFactory->updateAnchorManager();

	// Transform matrices between coordinates systems
	core->navigation->updateTransformMatrices(observer, core->timeMgr->getJDay());
	// Direction of vision
	core->navigation->updateVisionVector(delta_time, core->selected_object);
	// Field of view
	core->projection->updateAutoZoom(delta_time, core->FlagManualZoom);
	// update faders and Planet trails (call after nav is updated)
	core->ssystemFactory->update(delta_time, core->navigation, core->timeMgr.get());
			
	// Move the view direction and/or fov
	core->updateMove(delta_time);
	// Update info about selected object
	// selected_object.update();
	// Update faders
	core->skyGridMgr->update(delta_time);
	core->skyLineMgr->update(delta_time);
	core->skyDisplayMgr->update(delta_time);
	core->asterisms->update(delta_time);
	core->atmosphere->update(delta_time);
	core->landscape->update(delta_time);
	core->hip_stars->update(delta_time);
	core->nebulas->update(delta_time);
	core->cardinals_points->update(delta_time);
	core->milky_way->update(delta_time);
	//text_usr->update(delta_time);

	core->starLines->update(delta_time);

	core->oort->update(delta_time);

	// Compute the sun position in local coordinate
	Vec3d temp(0.,0.,0.);
	Vec3d sunPos = core->navigation->helioToLocal(temp);

	// Compute the moon position in local coordinate
	Vec3d moon = core->ssystemFactory->getMoon()->get_heliocentric_ecliptic_pos();
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

	std::future<void> a = std::async(std::launch::async, &SolarSystemModule::ssystemComputePreDraw, this);
	std::future<void> b = std::async(std::launch::async, &SolarSystemModule::atmosphereComputeColor, this, sunPos, moonPos);
	std::future<void> c = std::async(std::launch::async, &SolarSystemModule::hipStarMgrPreDraw, this);

	a.get();
	b.get();
	c.get();
	core->tone_converter->setWorldAdaptationLuminance(core->atmosphere->getWorldAdaptationLuminance());

	sunPos.normalize();
	moonPos.normalize();

	core->ssystemFactory->bodyTrace(core->navigation);


	// compute global sky brightness TODO : make this more "scientifically"
	// TODO: also add moonlight illumination
	if (sunPos[2] < -0.1/1.5 ) core->sky_brightness = 0.01;
	else core->sky_brightness = (0.01 + 1.5*(sunPos[2]+0.1/1.5));
	// TODO make this more generic for non-atmosphere planets
	if (core->atmosphere->getFadeIntensity() == 1) {
		// If the atmosphere is on, a solar eclipse might darken the sky otherwise we just use the sun position calculation above
		core->sky_brightness *= (core->atmosphere->getIntensity()+0.1);
	}
	// TODO: should calculate dimming with solar eclipse even without atmosphere on
	core->landscape->setSkyBrightness(core->sky_brightness+0.05);


	core->uboCamUpdate();
}

void SolarSystemModule::draw(int delta_time)
{
	core->applyClippingPlanes(0.000001 ,200);
	s_font::beginPrint(true); // multisample print
	core->milky_way->draw(core->tone_converter, core->projection, core->navigation, core->timeMgr->getJulian());
	//for VR360 drawing
	core->media->drawVR360(core->projection, core->navigation);
	core->nebulas->draw(core->projection, core->navigation, core->tone_converter, core->atmosphere->getFlagShow() ? core->sky_brightness : 0);
	core->oort->draw(observer->getAltitude(), core->projection, core->navigation);
	core->illuminates->draw(core->projection, core->navigation);
	core->asterisms->draw(core->projection, core->navigation);
	core->starLines->draw(core->projection);
	core->hip_stars->draw(core->geodesic_grid, core->tone_converter, core->projection, core->timeMgr.get(), core->observatory->getAltitude());
	core->skyGridMgr->draw(core->projection);
	core->skyLineMgr->draw(core->projection, core->navigation, core->timeMgr.get(), core->observatory.get());
	core->skyDisplayMgr->draw(core->projection, core->navigation, core->selected_object.getEarthEquPos(core->navigation), core->old_selected_object.getEarthEquPos(core->navigation));
	core->ssystemFactory->draw(core->projection, core->navigation, observer, core->tone_converter, core->bodyDecor->canDrawBody() /*aboveHomePlanet*/ );

	// Draw the pointer on the currently selected object
	// TODO: this would be improved if pointer was drawn at same time as object for correct depth in scene
	if (core->selected_object && core->object_pointer_visibility) core->selected_object.drawPointer(delta_time, core->projection, core->navigation);

	// Update meteors
	core->meteors->update(core->projection, core->navigation, core->timeMgr.get(), core->tone_converter, delta_time);

	// retiré la condition && atmosphere->getFlagShow() de sorte à pouvoir en avoir par atmosphère ténue
	// if (!aboveHomePlanet && (sky_brightness<0.1) && (observatory->getHomeBody()->getEnglishName() == "Earth" || observatory->getHomeBody()->getEnglishName() == "Mars")) {
	if (core->bodyDecor->canDrawMeteor() && (core->sky_brightness<0.1))
		core->meteors->draw(core->projection, core->navigation);

	s_font::nextPrint(false);
	core->atmosphere->draw(core->projection, observer->getHomePlanetEnglishName());

	// Draw the landscape
	if (core->bodyDecor->canDrawLandscape()) {
		core->landscape->draw(core->projection, core->navigation);
	}

	core->cardinals_points->draw(core->projection, observer->getLatitude());
}

bool SolarSystemModule::testValidAltitude(double altitude)
{
	if (altitude>maxAltToGoUp) {
		std::cout << "Swapping to mode Solar System" << std::endl;
		nextMode = upMode;
		if (upMode == nullptr)
			std::cout << "upMode not defined" << std::endl;
		else
			std::cout << "upMode is defined" << std::endl;
		return true;
	}
	return false;
}

void SolarSystemModule::atmosphereComputeColor(Vec3d sunPos, Vec3d moonPos )
{
	core->atmosphere->computeColor(core->timeMgr->getJDay(), sunPos, moonPos,
	                          core->ssystemFactory->getMoon()->get_phase(core->ssystemFactory->getEarth()->get_heliocentric_ecliptic_pos()),
	                          core->tone_converter, core->projection, observer->getLatitude(), observer->getAltitude(),
	                          15.f, 40.f);	// Temperature = 15c, relative humidity = 40%
}

void SolarSystemModule::hipStarMgrPreDraw()
{
	core->hip_stars->preDraw(core->geodesic_grid, core->tone_converter, core->projection, core->navigation, core->timeMgr.get(),core->observatory->getAltitude(), core->atmosphere->getFlagShow() && core->FlagAtmosphericRefraction);
}

void SolarSystemModule::ssystemComputePreDraw()
{
	core->ssystemFactory->computePreDraw(core->projection, core->navigation);
}