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

#include <memory>

#include "bodyModule/ssystem_factory.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"

SSystemFactory::SSystemFactory(ThreadContext *_context)
{
    // creation ds models 3D pour les planetes
    objLMgr = std::make_unique<ObjLMgr>(_context);
	objLMgr -> setDirectoryPath(AppSettings::Instance()->getModel3DDir() );
	objLMgr->insertDefault("Sphere");

	if (!objLMgr->checkDefaultObject()) {
		cLog::get()->write("SolarSystem: no default objMgr loaded, system aborded", LOG_TYPE::L_ERROR);
		exit(-7);
	}

    ssystem = std::make_unique<SolarSystem>(_context, objLMgr.get());
    ssystemColor = std::make_unique<SolarSystemColor>(ssystem.get());
    ssystemTex = std::make_unique<SolarSystemTex>(ssystem.get());
    ssystemSelected = std::make_unique<SolarSystemSelected>(ssystem.get());
    ssystemScale = std::make_unique<SolarSystemScale>(ssystem.get());
}
    
SSystemFactory::~SSystemFactory()
{
}