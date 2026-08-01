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
		// New-path fov mirror (§11.15c residual, closed T7 §11.45): bookmark
		// restore reached only the old projection fov; the new path carries fov
		// on the Camera. Same surface + unit convention as coreLink::setFov
		// (mBackup.fov is degrees, saved from projection->getFov()). loadBackup
		// is a runtime bookmark restore, so Camera::instance is always live.
		if (Camera::instance)
			Camera::instance->setHalfFov(mBackup.fov * M_PI / 360);
		// THE WRITE HALF OF THE BOOKMARK (B34, F24). It used to be
		// `observatory->moveTo` - the old observer alone - so `position load`
		// did not move the path that draws, while the fov two lines up was
		// mirrored long ago (§11.45 T7). observerMoveTo is the dual seam that
		// every other place-restoring caller already uses (I2), so the camera
		// lands where the bookmark was taken and the old observer follows.
		// ASYMMETRY, stated because it is real and is NOT fixed here (§5.68):
		// the two setters are dual but not equivalent - old clamps latitude to
		// ±90°, maps exactly 0 to 1e-6 and floors altitude at 0.1 m; the camera
		// clamps nothing. A bookmark taken at a clamped value therefore restores
		// the CAMERA exactly (the read half now reports the camera's own place,
		// so the round trip closes on the drawn path) and the old observer to
		// its clamped image of it - the same asymmetry every `moveto` already
		// has, not one this seam introduces.
		coreLink->observerMoveTo(mBackup.latitude, mBackup.longitude, mBackup.altitude, 1/*, mBackup.pos_name*/);
	}
	core->setHomePlanet(mBackup.home_planet_name);
	core->setFlagIngalaxy(mBackup.current_module);
}

void CoreBackup::saveBackup()
{
	mBackup.jday=core->timeMgr->getJDay();
	// THE READ HALF (B33 §11.131(f) -> B34, F24): the bookmark records the place
	// the operator is LOOKING FROM, which is the drawn path's - these three
	// getters ask which path draws and answer for it. Folded WITH the restore
	// above and not before it: reading the drawn place while restoring into the
	// old observer alone would have been strictly worse than the coherent
	// old->old round trip it replaced, which is why F23 refused the read alone.
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