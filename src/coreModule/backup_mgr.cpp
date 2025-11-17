/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

//!  @brief File for backup application core processing.
//!
//! This file describe all backup option

#include "coreModule/backup_mgr.hpp"
#include "coreModule/core.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/skydisplay_mgr.hpp"

CoreBackup::CoreBackup(std::shared_ptr<Core> _core)
{
	core = _core;
}
	
CoreBackup::~CoreBackup()
{}

void CoreBackup::loadBackup()
{
	if (mBackup.jday !=0) {
		if (core->getFlagIngalaxy() != mBackup.current_module) {
			// Use the callback (set by App) to switch mode
			if (switchModeCallback) {
				std::string modeString;
				switch (mBackup.current_module) {
					case MODULE::SOLAR_SYSTEM:
						modeString = "in_solarsystem";
						break;
					case MODULE::IN_GALAXY:
						modeString = "in_galaxy";
						break;
					case MODULE::IN_UNIVERSE:
						modeString = "in_universe";
						break;
					case MODULE::STELLAR_SYSTEM:
						modeString = "in_stellarsystem";
						break;
					default:
						modeString = "in_solarsystem"; // default fallback
						break;
				}
				std::cout << "CoreBackup::loadBackup: switching mode to " << modeString << std::endl;
				switchModeCallback(modeString);
			} else {
				// Should never happen but just in case
				std::cout << "CoreBackup::loadBackup: switchModeCallback not set!" << std::endl;
			}
		}
		core->timeMgr->setJDay(mBackup.jday);
		core->projection->setFov(mBackup.fov); //setFov(mBackup.fov);
		// Always move instantly to avoid issues with mode switching, landscape state, etc...
		// (everything based on altitude)
		core->observatory->moveTo(mBackup.latitude, mBackup.longitude, mBackup.altitude, 0);
		// Set the local vision direction
		core->navigation->setLocalVision(mBackup.observer_vision);
	}
	core->setHomePlanet(mBackup.home_planet_name);
}

void CoreBackup::saveBackup()
{
	mBackup.jday=core->timeMgr->getJDay();
	mBackup.latitude=core->observatory->getLatitude();
	mBackup.longitude=core->observatory->getLongitude();
	mBackup.altitude=core->observatory->getAltitude();
	// mBackup.pos_name=core->observatory->getName();
	mBackup.fov = core->projection->getFov(); //getFov();
	mBackup.observer_vision = core->navigation->getLocalVision();
	mBackup.home_planet_name=core->observatory->getHomePlanetEnglishName();
	mBackup.current_module=core->getFlagIngalaxy();

	std::string modulestr = "";
	switch (mBackup.current_module) {
		case MODULE::SOLAR_SYSTEM:
			modulestr = "Solar System";
			break;
		case MODULE::IN_GALAXY:
			modulestr = "In Galaxy";
			break;
		case MODULE::IN_UNIVERSE:
			modulestr = "In Universe";
			break;
		case MODULE::STELLAR_SYSTEM:
			modulestr = "Stellar System";
			break;
		case MODULE::IN_SANDBOX:
			modulestr = "In Sandbox";
			break;
		default:
			modulestr = "Unknown Module";
			break;
	}

	std::cout << "Backup saved: jday=" << std::to_string(mBackup.jday) <<
	" lat=" << std::to_string(mBackup.latitude) <<
	" lon=" << std::to_string(mBackup.longitude) <<
	" alt=" << std::to_string(mBackup.altitude) <<
	" fov=" << std::to_string(mBackup.fov) <<
	" vision=[" << std::to_string(mBackup.observer_vision[0]) << ", "
		<< std::to_string(mBackup.observer_vision[1]) << ", "
		<< std::to_string(mBackup.observer_vision[2]) << "]" <<
	" home_planet=" << mBackup.home_planet_name <<
	" module=" << modulestr << std::endl;
}

void CoreBackup::saveGridState()
{
	core->skyGridMgr->saveState(skyGridSave);
}

void CoreBackup::loadGridState()
{
	core->skyGridMgr->loadState(skyGridSave);
}

void CoreBackup::saveDisplayState()
{
	core->skyDisplayMgr->saveState(skyDisplaySave);
}

void CoreBackup::loadDisplayState()
{
	core->skyDisplayMgr->loadState(skyDisplaySave);
}

void CoreBackup::saveLineState()
{
	core->skyLineMgr->saveState(skyLineSave);
}

void CoreBackup::loadLineState()
{
	core->skyLineMgr->loadState(skyLineSave);
}