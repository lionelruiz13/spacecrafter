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

#ifndef _BACKUP_MGR_H_
#define _BACKUP_MGR_H_

#include <string>
#include "coreModule/core_common.hpp"

struct InitialValue {
	std::string initial_skyCulture;
	std::string initial_skyLocale;
	std::string initial_landscapeName;
};

struct BackupWorkspace {
	double jday= 0.0;
	double latitude = 0.0;
	double longitude = 0.0;
	double altitude = 0.f;
	float fov = 0.f;
	std::string home_planet_name;
	std::string pos_name;
};

class Core;

class CoreBackup {

public:
	CoreBackup(Core* _core);
	~CoreBackup();
	void loadBackup();
	void saveBackup();

	//gestion des états des grilles
	void saveGridState();
	void loadGridState();
private:
	BackupWorkspace mBackup;
	SkyGridSave	skyGridSave;
	Core* core= nullptr;
};

#endif // _BACKUP_MGR_H_