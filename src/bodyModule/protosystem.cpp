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
#include <fstream>
#include <string>

#include "bodyModule/protosystem.hpp"
#include "bodyModule/orbit_creator_cor.hpp"
#include "tools/log.hpp"



ProtoSystem::ProtoSystem(ThreadContext *_context, ObjLMgr *_objLMgr)
	:context(_context), objLMgr(_objLMgr)
{
	bodyTrace = nullptr;

	Body::createShader(context);
	BodyShader::createShader(context);
	Body::createDefaultAtmosphereParams();

	OrbitCreator * special = new OrbitCreatorSpecial(nullptr);
	OrbitCreator * comet = new OrbitCreatorComet(special, this);
	OrbitCreator * elip = new OrbitCreatorEliptic(comet, this);
	orbitCreator = new OrbitCreatorBary(elip, this);
}

// Init and load the solar system data
void ProtoSystem::load(const std::string& planetfile)
{
	stringHash_t bodyParams;

	std::ifstream fileBody (planetfile.c_str() , std::ifstream::in);
	if(fileBody) {
		std::string ligne;
		while(getline(fileBody , ligne)) {
			if (ligne[0] != '[' ) {
				if (ligne[0]!='#' && ligne.size() != 0) {
					int pos = ligne.find('=',0);
					std::string p1=ligne.substr(0,pos-1);
					std::string p2=ligne.substr(pos+2,ligne.size());
					bodyParams[p1]=p2;
				}
			} else {
				if (bodyParams.size() !=0) {
					//TODO récupérer cette erreur s'il y en a !
					addBody(bodyParams, false);  // config file bodies are not deletable
					bodyParams.clear();
				}
			}
		}
		fileBody.close();
	} else
		cLog::get()->write("Unable to open file "+ planetfile, LOG_TYPE::L_ERROR);

	cLog::get()->write("(solar system loaded)", LOG_TYPE::L_INFO);
	cLog::get()->mark();
}

Body* ProtoSystem::searchByEnglishName(const std::string &planetEnglishName) const
{
	//printf("SolarSystem::searchByEnglishName(\"%s\"): start\n", planetEnglishName.c_str());
	// side effect - bad?
	//	transform(planetEnglishName.begin(), planetEnglishName.end(), planetEnglishName.begin(), ::tolower);
	if(systemBodies.count(planetEnglishName) != 0){
		return systemBodies.find(planetEnglishName)->second->body.get();
	}
	else{
		return nullptr;
	}

}