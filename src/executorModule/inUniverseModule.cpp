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
#include "inUniverseModule.hpp"
#include "inGalaxyModule/cloudNavigator.hpp"
#include "inGalaxyModule/starGalaxy.hpp"
#include "eventModule/event.hpp"
#include "eventModule/event_recorder.hpp"
#include "eventModule/EventScreenFader.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/time_mgr.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "coreModule/tully.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "coreModule/volumObj3D.hpp"

InUniverseModule::InUniverseModule(std::shared_ptr<Core> _core, Observer *_observer) : core(_core), observer(_observer)
{
	module = MODULE::IN_UNIVERSE;

    minAltToGoDown = 1.E9;
    maxAltToGoUp = 1.E15;
}

void InUniverseModule::onEnter()
{
    std::cout << "J'arrive dans InUniverse" << std::endl;
	//réglage de l'altitude dans CoreExecutorInUniverse la première fois
	//~ if (observer->getAltitude() <1.E9) {
		printf("je change la valeur de l'altitude dans InUniverse\n");
		observer->setAltitude(minAltToGoDown);
		Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 1.0);
		EventRecorder::getInstance()->queue(event);
}

void InUniverseModule::onExit()
{
	Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 1.0);
	EventRecorder::getInstance()->queue(event);
	std::cout << "Je quitte InUniverse" << std::endl;
}

void InUniverseModule::update(int delta_time)
{
	// Update the position of observation and time etc...
	observer->update(delta_time);
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
	core->skyDisplayMgr->update(delta_time);

	core->tully->update(delta_time);

	// Give the updated standard projection matrices to the projector
	// NEEDED before atmosphere compute color
	core->projection->setModelViewMatrices( core->navigation->getEarthEquToEyeMat(),
	                                    core->navigation->getEarthEquToEyeMatFixed(),
	                                    core->navigation->getHelioToEyeMat(),
	                                    core->navigation->getLocalToEyeMat(),
	                                    core->navigation->getJ2000ToEyeMat(),
	                                    core->navigation->geTdomeMat(),
	                                    core->navigation->getDomeFixedMat());
	Event* event = new ScreenFaderInterludeEvent(
		ScreenFaderInterludeEvent::DOWN, minAltToGoDown,1.1*minAltToGoDown, observer->getAltitude());
	EventRecorder::getInstance()->queue(event);
}

void InUniverseModule::draw(int delta_time)
{
	core->applyClippingPlanes(0.01, 10);
	Context::instance->helper->beginDraw(PASS_BACKGROUND, *Context::instance->frame[Context::instance->frameIdx]);
	// core->universeCloudNav->computePosition(core->navigation->getObserverHelioPos(), core->projection);
	//for VR360 drawing
	core->media->drawVR360(core->projection, core->navigation);
	if (core->volumGalaxy->loaded()) {
		if (core->tully->mustBuild())
			core->tully->build(core->volumGalaxy.get());
	} else {
		if (core->tully->mustBuild())
			core->tully->build();
	}
	core->ojmMgr->draw(core->projection, core->navigation, OjmMgr::STATE_POSITION::IN_UNIVERSE);
	core->tully->draw(observer->getAltitude(), core->navigation, core->projection);
	core->skyDisplayMgr->drawPerson(core->projection, core->navigation);
	//core->postDraw();
}

bool InUniverseModule::testValidAltitude(double altitude)
{
	if (altitude<minAltToGoDown) {
		nextMode = downMode;
		return true;
	}
	if(altitude>maxAltToGoUp)
		observer->setAltitude(maxAltToGoUp);
	return false;
}
