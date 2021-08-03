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

#include "bodyModule/solarsystem_selected.hpp"
#include "bodyModule/ssystem_iterator.hpp"
#include "bodyModule/solarsystem.hpp"

SolarSystemSelected::SolarSystemSelected(SolarSystem * _ssystem)
{
    ssystem = _ssystem;
}

SolarSystemSelected::~SolarSystemSelected()
{
	// release selected:
	selected = Object();
}

void SolarSystemSelected::setSelected(const Object &obj)
{
	if (obj.getType() == OBJECT_BODY){
		selected = obj;
	}
	else{
		selected = Object();
	}
	// Undraw other objects hints, orbit, trails etc..
	//setFlagOrbits(flagTrails);
	setFlagPlanetsOrbits(flagPlanetsOrbits);
	setFlagSatellitesOrbits(flagSatellitesOrbits);
	setFlagTrails(flagTrails);  // TODO should just hide trail display and not affect data collection
}

void SolarSystemSelected::setFlagTrails(bool b)
{
	flagTrails = b;

	if (!b || !selected || selected == Object(ssystem->getSun())) {
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			it->current()->second->body->setFlagTrail(b);
		}
	} else {
		// if a Body is selected and trails are on, fade out non-selected ones
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			if (selected == it->current()->second->body.get() || (it->current()->second->body->get_parent() && it->current()->second->body->get_parent()->getEnglishName() == selected.getEnglishName()) )
				it->current()->second->body->setFlagTrail(b);
			else it->current()->second->body->setFlagTrail(false);
		}
	}
}

void SolarSystemSelected::setFlagHints(bool b)
{
	flagHints = b;
	for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
		it->current()->second->body->setFlagHints(b);
	}
}

void SolarSystemSelected::setFlagPlanetsOrbits(const std::string &_name, bool b)
{
	for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
		if (it->current()->second->englishName == _name) {
			it->current()->second->body->setFlagOrbit(b);
			return;
		}
	}
}

void SolarSystemSelected::setFlagPlanetsOrbits(bool b)
{
	flagPlanetsOrbits = b;

	if (!b || !selected || selected == Object(ssystem->getSun())) {
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			//if (it->current()->second->body->get_parent() && it->current()->second->body->getParent()->getEnglishName() =="Sun")
			if (it->current()->second->body->getTurnAround() == tACenter)
				it->current()->second->body->setFlagOrbit(b);
		}
	} else {
		// if a Body is selected and orbits are on,
		// fade out non-selected ones
		// unless they are orbiting the selected Body 20080612 DIGITALIS
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			if (!it->current()->second->body->isSatellite()) {
				//if ((selected == it->current()->second->body.get()) && (it->current()->second->body->getParent()->getEnglishName() =="Sun")){
				if ((selected == it->current()->second->body.get()) && (it->current()->second->body->getTurnAround() == tACenter)) {
					it->current()->second->body->setFlagOrbit(true);
				}
				else {
					it->current()->second->body->setFlagOrbit(false);
				}
			}
			else {
				if (selected == it->current()->second->body->getParent()) {
					it->current()->second->body->setFlagOrbit(true);
				}
				else{
					it->current()->second->body->setFlagOrbit(false);
				}
			}
		}
	}
}

void SolarSystemSelected::setFlagSatellitesOrbits(bool b)
{
	flagSatellitesOrbits = b;

	if (!b || !selected || selected == Object(ssystem->getSun())) {
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			//if (it->current()->second->body->get_parent() && it->current()->second->body->getParent()->getEnglishName() !="Sun"){
			if (it->current()->second->body->getTurnAround() == tABody) {
				it->current()->second->body->setFlagOrbit(b);
			}
		}
	}
	else {
		// if the mother Body is selected orbits are on, else orbits are off
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			if (it->current()->second->body->isSatellite()) {
				if (it->current()->second->body->get_parent()->getEnglishName() == selected.getEnglishName() || it->current()->second->englishName == selected.getEnglishName()) {
					it->current()->second->body->setFlagOrbit(true);
				}
				else{
					it->current()->second->body->setFlagOrbit(false);
				}
			}
		}
	}
}

bool SolarSystemSelected::getFlag(BODY_FLAG name)
{
	switch (name) {
		case BODY_FLAG::F_TRAIL: return flagTrails; break;
		case BODY_FLAG::F_HINTS: return flagHints; break;
		case BODY_FLAG::F_ORBIT : return (flagPlanetsOrbits||flagSatellitesOrbits); break;
		default: break;
	}
	return false;
}