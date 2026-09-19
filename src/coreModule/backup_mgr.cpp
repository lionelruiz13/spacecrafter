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
#include "coreModule/coreLink.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "experimentalModule/Camera.hpp"

CoreBackup::CoreBackup(std::shared_ptr<Core> _core, std::shared_ptr<CoreLink> _coreLink)
{
	core = _core;
	coreLink = _coreLink;
}
	
CoreBackup::~CoreBackup()
{}

void CoreBackup::loadBackup()
{
	if (mBackup.jday !=0) {
		core->timeMgr->setJDay(mBackup.jday);
		core->projection->setFov(mBackup.fov); //setFov(mBackup.fov);
		if (Camera::instance)
			Camera::instance->setHalfFov(mBackup.fov * M_PI / 360);
		coreLink->observerMoveTo(mBackup.latitude, mBackup.longitude, mBackup.altitude, 1/*, mBackup.pos_name*/);
	}
	core->setHomePlanet(mBackup.home_planet_name);
	core->setFlagIngalaxy(mBackup.current_module);
}

void CoreBackup::saveBackup()
{
	mBackup.jday=core->timeMgr->getJDay();
	mBackup.latitude=coreLink->observatoryGetLatitude();
	mBackup.longitude=coreLink->observatoryGetLongitude();
	mBackup.altitude=coreLink->observatoryGetAltitude();
	// mBackup.pos_name=core->observatory->getName();
	mBackup.fov = core->projection->getFov(); //getFov();
	mBackup.home_planet_name=core->observatory->getHomePlanetEnglishName();
	mBackup.current_module=core->getFlagIngalaxy();
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