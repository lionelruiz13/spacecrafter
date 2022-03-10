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

#include "executor.hpp"
#include "navModule/observer.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"

Executor::Executor(std::shared_ptr<Core> _core, Observer *_observer)
{
    core = _core;
    observer = _observer;

    ssystemModule = std::make_unique<SolarSystemModule>(core, observer);
    inGalaxyModule = std::make_unique<InGalaxyModule>(core, observer);
    inUniverseModule = std::make_unique<InUniverseModule>(core, observer);
    //inPauseModule = std::make_unique<InPauseModule>(core, observer);
    currentMode = ssystemModule.get();

    ssystemModule->defineUpMode(inGalaxyModule.get());
    inGalaxyModule->defineDownMode(ssystemModule.get());
    inGalaxyModule->defineUpMode(inUniverseModule.get());
    inUniverseModule->defineDownMode(inGalaxyModule.get());
    currentMode->onEnter();
}

void Executor::draw(int delta_time)
{
    currentMode->draw(delta_time);
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
    core->media->imageDraw(core->navigation, core->projection);
}

void Executor::update(int delta_time)
{
    currentMode->update(delta_time);
}

void Executor::updateMode(double altitude)
{
    if (currentMode->testValidAltitude(altitude)) {
        currentMode->onExit();
        currentMode = currentMode->getNextMode();
        // Don't select an object from a different mode
        core->selected_object = Object();
        currentMode->onEnter();
    }
}

void Executor::switchMode(const std::string &mode)
{
	if (mode.empty())
		return;

	std::string modeValue = mode;
	std::transform(modeValue.begin(), modeValue.end(),modeValue.begin(), ::tolower);

	currentMode->onExit();
	if (modeValue =="ingalaxy" || modeValue =="in_galaxy" ) {
		currentMode = inGalaxyModule.get();
	} else
	if (modeValue =="inuniverse" || modeValue =="in_universe" ) {
		currentMode = inUniverseModule.get();
	} else
	if (modeValue =="insolarsystem" || modeValue =="in_solarsystem" ) {
		currentMode = ssystemModule.get();
	}
    // Don't select an object from a different mode
    core->selected_object = Object();
	currentMode->onEnter();
}
