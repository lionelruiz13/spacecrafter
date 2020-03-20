/*
 * Copyright (C) 2018 Immersive Adventure
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/core.hpp"
#include "coreModule/core_executor.hpp"
#include "navModule/observer.hpp"
#include "eventModule/event_manager.hpp"
#include "eventModule/ScreenFaderEvent.hpp"
/*
 *
 * Classe virtuelle CoreExecutor
 *
 *
 */

CoreExecutor::CoreExecutor(Core* _core, Observer* _observer)
{
	name = "NoDefined";
	core = _core;
	observer = _observer;
}

CoreExecutor::~CoreExecutor()
{}

CoreExecutor* CoreExecutor::getNextMode()
{
	if (nextMode!=nullptr) {
		std::cout << "Swapping to mode " << nextMode->getName() << std::endl;
		return nextMode;
	}
	else {
		std::cout << "Staying in the mode " << this->getName() << std::endl;
		return this;
	}
}

/*
 *
 *
 * Classe CoreExecutorInSolarSystem
 *
 *
 */
CoreExecutorInSolarSystem::CoreExecutorInSolarSystem(Core* _core, Observer* _observer) 
							: CoreExecutor(_core, _observer)
{
	name = "InSolarSystem";
	maxAltToGoUp = 1.E16;
}


CoreExecutorInSolarSystem::~CoreExecutorInSolarSystem() 
{}


void CoreExecutorInSolarSystem::onEnter()
{
	std::cout << "J'arrive dans InSolarSystem" << std::endl;
	nextMode = nullptr;
	observer->loadBodyInSolarSystem();
	Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 0.0);
	EventManager::getInstance()->queue(event);
	//réglage de l'altitude dans CoreExecutorInSolarSystem la première fois 
	if (observer->getAltitude() < maxAltToGoUp)
		observer->setAltitude(observer->getAltitude() *1.E6);
}


void CoreExecutorInSolarSystem::onExit()
{
	observer->saveBodyInSolarSystem();
	observer->fixBodyToSun();
	std::cout << "Je quitte InSolarSystem" << std::endl;
}

void CoreExecutorInSolarSystem::update(int delta_time)
{
	core->updateInSolarSystem(delta_time);
}


void CoreExecutorInSolarSystem::draw(int delta_time)
{
	core->applyClippingPlanes(0.000001 ,200);
	core->drawInSolarSystem(delta_time);
	core->postDraw();
}


bool CoreExecutorInSolarSystem::testValidAltitude(double altitude)
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


/*
 *
 *
 * Classe CoreExecutorInGalaxy
 *
 *
 */

CoreExecutorInGalaxy::CoreExecutorInGalaxy(Core* _core, Observer* _observer)
						: CoreExecutor(_core, _observer)
{
	name ="InGalaxy";
	minAltToGoDown = 1.E10;
	maxAltToGoUp = 1.E15;
}


CoreExecutorInGalaxy::~CoreExecutorInGalaxy() 
{}


void CoreExecutorInGalaxy::onEnter()
{
	std::cout << "J'arrive dans InGalaxy" << std::endl;
	nextMode = nullptr;
	//réglage de l'altitude dans CoreExecutorInGalaxy la première fois 
	if (observer->getAltitude() < minAltToGoDown) {
		std::cout << "On est dans inGalaxy mais trop bas: modification de l'altitude pour min" << std::endl;
		observer->setAltitude((maxAltToGoUp+minAltToGoDown)/2.0);
	} else
	if (observer->getAltitude() > maxAltToGoUp) {
		std::cout << "On est dans inGalaxy mais trop haut: modification de l'altitude pour max" << std::endl;
		observer->setAltitude(minAltToGoDown);
	}
	else {
		std::cout << "On est dans inGalaxy mais ni trop haut ni trop bas." << std::endl;
		observer->setAltitude((minAltToGoDown+maxAltToGoUp/2.0));
	}
}


void CoreExecutorInGalaxy::onExit()
{
	std::cout << "Je quitte InGalaxy" << std::endl;
}

void CoreExecutorInGalaxy::update(int delta_time)
{
	core->updateInGalaxy(delta_time);
	Event* event = new ScreenFaderInterludeEvent(
		ScreenFaderInterludeEvent::UP, maxAltToGoUp/10.0,maxAltToGoUp, observer->getAltitude());
	EventManager::getInstance()->queue(event);
}

void CoreExecutorInGalaxy::draw(int delta_time)
{
	core->applyClippingPlanes(0.01, 2000.01);
	core->drawInGalaxy(delta_time);
	core->postDraw();
}


bool CoreExecutorInGalaxy::testValidAltitude(double altitude)
{
	if (altitude>maxAltToGoUp) {
		nextMode = upMode;
		Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 1.0);
		EventManager::getInstance()->queue(event);
		return true;
	}
	if (altitude<minAltToGoDown) {
		nextMode = downMode;
		return true;
	}
	return false;
}


/*
 *
 *
 * Classe CoreExecutorInUniverse
 *
 *
 */

CoreExecutorInUniverse::CoreExecutorInUniverse(Core* _core, Observer* _observer)
					:CoreExecutor(_core, _observer)
{
	name ="InUniverse";
	minAltToGoDown = 1.E9;
	maxAltToGoUp = 1.E13;
}

CoreExecutorInUniverse::~CoreExecutorInUniverse() 
{}

void CoreExecutorInUniverse::onEnter()
{
	std::cout << "J'arrive dans InUniverse" << std::endl;
	nextMode = nullptr;
	//réglage de l'altitude dans CoreExecutorInUniverse la première fois 
	//~ if (observer->getAltitude() <1.E9) {
		printf("je change la valeur de l'altitude dans InUniverse\n");
		observer->setAltitude(minAltToGoDown);
		Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 1.0);
		EventManager::getInstance()->queue(event);
}


void CoreExecutorInUniverse::onExit()
{
	Event* event = new ScreenFaderEvent(ScreenFaderEvent::FIX, 1.0);
	EventManager::getInstance()->queue(event);
	std::cout << "Je quitte InUniverse" << std::endl;
}

void CoreExecutorInUniverse::update(int delta_time)
{
	core->updateInUniverse(delta_time);
	Event* event = new ScreenFaderInterludeEvent(
		ScreenFaderInterludeEvent::DOWN, minAltToGoDown,1.1*minAltToGoDown, observer->getAltitude());
	EventManager::getInstance()->queue(event);
}

void CoreExecutorInUniverse::draw(int delta_time)
{
	core->applyClippingPlanes(0.0001 ,2000.1);
	core->drawInUniverse(delta_time);
	core->postDraw();
}


bool CoreExecutorInUniverse::testValidAltitude(double altitude)
{
	if (altitude<minAltToGoDown) {
		nextMode = downMode;
		return true;
	}
	if(altitude>maxAltToGoUp)
		observer->setAltitude(maxAltToGoUp);
	return false;
}
