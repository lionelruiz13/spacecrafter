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
#include "bodyModule/ssystem_iterator.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"



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

void ProtoSystem::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->update(delta_time, nav, timeMgr);
	}
}


std::shared_ptr<ProtoSystem::BodyContainer> ProtoSystem::findBodyContainer(const std::string &name)
{
	if(systemBodies.count(name) != 0){
		return systemBodies[name];
	}
	else{
		return nullptr;
	}
}

bool ProtoSystem::removeBody(const std::string &name){
	// std::cout << "removeBody " << name << std::endl;
	std::shared_ptr<BodyContainer> bc = findBodyContainer(name);

	if(bc == nullptr){
		cLog::get()->write("SolarSystem::removeBody : Could not find a body named : " + name );
		return false;
	}

	if(bc->body->hasSatellite()){
		// std::cout << "removeBody " << name << " but have satellite" << std::endl;
		std::vector<std::string> names;
		std::list<Body *> satellites = bc->body->getSatellites();

		for(auto it = satellites.begin(); it != satellites.end(); it++){
			names.push_back((*it)->getEnglishName());
		}

		for(std::string satName : names){
			if(!removeBody(satName)){
				cLog::get()->write("SolarSystem::removeBody : Could not remove satelite : " + satName );
			}
		}
	}
	// std::cout << "removeBody " << name << " is oki" << std::endl;
	return removeBodyNoSatellite(name);
}

bool ProtoSystem::removeSupplementalBodies(const std::string &name)
{
	// std::cout << "removeSupplementalBodies " << name << std::endl;
	std::shared_ptr<BodyContainer> bc = findBodyContainer(name);

	if(bc == nullptr){
		cLog::get()->write("SolarSystem::removeSupplementalBodies : Could not find a body named : " + name );
		return false;
	}

	if(bc->isDeleteable){
		cLog::get()->write("SolarSystem::removeSupplementalBodies : Can't distroy suplementary bodies if attached to one");
		return false;
	}

	std::vector<std::string> names;

	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){

		if(it->second->isDeleteable){

			if(!it->second->body->getParent()){
				names.push_back(it->first);
			}
			else{
				std::vector<std::string>::iterator ite;

				ite = find(names.begin(), names.end(),it->first);

				if(ite == names.end()){
					names.push_back(it->first);
				}
			}
		}
	}

	for(std::string name : names){
		removeBody(name);
	}
	// std::cout << "removeSupplementalBodies " << name << " is oki" << std::endl;
	return true;
}

void ProtoSystem::setFont(float font_size, const std::string& font_name)
{
	ModuleFont::setFont(font_size, font_name);
	Body::setFont(font.get());
}

//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void ProtoSystem::translateNames(Translator& trans)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->translateName(trans);
	}

	if(font)
		font->clearCache();
}

// returns a newline delimited hash of localized:standard Body names for tui
// Body translated name is PARENT : NAME
std::string ProtoSystem::getPlanetHashString(void)
{
	std::ostringstream oss;
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if (!it->second->isDeleteable ) { // no supplemental bodies in list
			//if (it->second->body->get_parent() != nullptr && it->second->body->get_parent()->getEnglishName() != "Sun") {
			if (it->second->body->getTurnAround() == tABody) {
				oss << Translator::globalTranslator.translateUTF8(it->second->body->get_parent()->getEnglishName())
				    << " : ";
			}

			oss << Translator::globalTranslator.translateUTF8(it->second->englishName) << "\n";
			oss << it->second->englishName << "\n";
		}
	}
	return oss.str();
}

void ProtoSystem::toggleHideSatellites(bool val){

	val = !val;

	if(flagHideSatellites == val){
		return;
	}
	else{
		flagHideSatellites = val;
	}

	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){

		//If we are a planet with satellites
		if(it->second->body->getParent() &&
		   //it->second->body->getParent()->getEnglishName() == "Sun" &&
		   it->second->body->getTurnAround() == tACenter &&
		   it->second->body->hasSatellite()){

		   for(Body * satellite : it->second->body->getSatellites()){
			   std::shared_ptr<BodyContainer> sat = findBodyContainer(satellite->getEnglishName());
				setPlanetHidden(sat->englishName, val);
			}
		}
	}
}

void ProtoSystem::setPlanetHidden(const std::string &name, bool planethidden)
{

	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
		Body * body = it->second->body.get();
		if (
			body->getEnglishName() == name ||
			(body->get_parent() && body->get_parent()->getEnglishName() == name) ){

			it->second->isHidden = planethidden;

			if(planethidden){
				std::remove_if(renderedBodies.begin(), renderedBodies.end(), [it](std::shared_ptr<BodyContainer> const obj) {
					return it->second->englishName == obj->englishName;
				});
			}
			else{
				// std::cout << "Je cherche un doublon de " << name << std::endl;
				if (std::find(renderedBodies.begin(), renderedBodies.end(), it->second) == renderedBodies.end() ) {
					renderedBodies.push_back(it->second);
					// std::cout << "je crée vraiment "<< name << " dans renderedBodies" << std::endl;
				}
				// else {
					// std::cout << "j'ai évité un doublon" << std::endl;
				// }
			}

		}
	}
}

// Search if any Body is close to position given in earth equatorial position and return the distance
Object ProtoSystem::search(Vec3d pos, const Navigator * nav, const Projector * prj) const
{
	pos.normalize();
	Body * closest = nullptr;
	double cos_angle_closest = 0.;
	static Vec3d equPos;

	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		equPos = it->second->body->getEarthEquPos(nav);
		equPos.normalize();
		double cos_ang_dist = equPos[0]*pos[0] + equPos[1]*pos[1] + equPos[2]*pos[2];
		if (cos_ang_dist>cos_angle_closest) {
			closest = it->second->body.get();
			cos_angle_closest = cos_ang_dist;
		}
	}

	if (cos_angle_closest>0.999) {
		return closest;
	}
	else
		return nullptr;
}

// Return a stl vector containing the planets located inside the lim_fov circle around position v
std::vector<Object> ProtoSystem::searchAround(Vec3d v,
        double lim_fov,
        const Navigator * nav,
        const Observer* observatory,
        const Projector * prj,
        bool *default_last_item,
        bool aboveHomeBody ) const
{
	std::vector<Object> result;
	v.normalize();
	double cos_lim_fov = cos(lim_fov * M_PI/180.);
	static Vec3d equPos;
	const Body *home_Body = observatory->getHomeBody();

	*default_last_item = false;

	// Should still be sorted by distance from farthest to closest
	// So work backwards to go closest to furthest
	for(auto it = systemBodies.rbegin(); it != systemBodies.rend(); it++){//reverse order

		equPos = it->second->body->getEarthEquPos(nav);
		equPos.normalize();

		// First see if within a Body disk
		if (it->second->body.get() != home_Body || aboveHomeBody) {
			// Don't want home Body too easy to select unless can see it

			double angle = acos(v*equPos) * 180.f / M_PI;
			  //~ cout << "Big testing " << (*iter)->getEnglishName()
			  //~ << " angle: " << angle << " screen_angle: "
			  //~ << (*iter)->get_angular_size(prj, nav)/2.f
			  //~ << endl;
			if ( angle < it->second->body->get_angular_size(prj, nav)/2.f ) {

				// If near planet, may be huge but hard to select, so check size
				result.push_back(it->second->body.get());
				*default_last_item = true;

				break;  // do not want any planets behind this one!

			}
		}
		// See if within area of interest
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {
			result.push_back(it->second->body.get());
		}

	}
	return result;
}

Object ProtoSystem::searchByNamesI18(const std::string &planetNameI18) const
{
	// side effect - bad?
	//	transform(planetNameI18.begin(), planetNameI18.end(), planetNameI18.begin(), ::tolower);
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if ( it->second->body->getNameI18n() == planetNameI18 )
			return it->second->body.get(); // also check standard ini file names
	}
	return nullptr;
}

bool ProtoSystem::getFlag(BODY_FLAG name)
{
	switch (name) {
		case BODY_FLAG::F_AXIS : return flagAxis; break;
		case BODY_FLAG::F_CLOUDS: return Body::getFlagClouds(); break;
		default: break;
	}
	return false;
}


void ProtoSystem::setFlagAxis(bool b)
{
	flagAxis=b;
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->setFlagAxis(b);
	}
}

void ProtoSystem::startTrails(bool b)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->startTrail(b);
	}
}

std::vector<std::string> ProtoSystem::getNamesI18(void)
{
	std::vector<std::string> names;
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		names.push_back(it->second->body->getNameI18n());
	}
	return names;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
std::vector<std::string> ProtoSystem::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	if (maxNbItem==0) return result;

	std::string objw = objPrefix;
	transform(objw.begin(), objw.end(), objw.begin(), ::toupper);

	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){

		std::string constw = it->second->body->getNameI18n().substr(0, objw.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objw) {
			result.push_back(it->second->body->getNameI18n());
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}

bool ProtoSystem::getPlanetHidden(const std::string &name)
{
	if(systemBodies.count(name) != 0){
		return systemBodies[name]->isHidden;
	}
	else{
		return false;
	}
}

std::string ProtoSystem::getPlanetsPosition()
{
	std::string msg;
	Vec3d tmp;

	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if (it->second->body->isSatellite())
			continue;
		tmp = it->second->body->get_heliocentric_ecliptic_pos();
		tmp[0]= round(tmp[0]*100);
		tmp[1]= round(tmp[1]*100);
		tmp[2]= round(tmp[2]*100);
		msg= msg+it->second->body->getNameI18n()+":"+std::to_string(tmp[0])+":"+std::to_string(tmp[1])+":"+std::to_string(tmp[2])+";";
	}
	return msg;
}

std::unique_ptr<SSystemIterator> ProtoSystem::createIterator()
{
	return std::make_unique<SSystemIterator>(this);
}

std::unique_ptr<SSystemIteratorVector> ProtoSystem::createIteratorVector()
{
	return std::make_unique<SSystemIteratorVector>(this);
}

void ProtoSystem::bodyTraceBodyChange(const std::string &bodyName)
{
	Body * body = searchByEnglishName(bodyName);

	if(body != nullptr){
		bodyTrace = body;
	}
	else{
		cLog::get()->write("Unknown planet_name in bodyTraceBodyChange", LOG_TYPE::L_ERROR);
	}
}

Body* ProtoSystem::findBody(const std::string &name)
{

	if(systemBodies.count(name) != 0){
		return systemBodies[name]->body.get();
	}
	else{
		return nullptr;
	}
}

BODY_TYPE ProtoSystem::setPlanetType (const std::string &str)
{
	if (str =="Sun") return SUN;
	else if (str == "Planet") return PLANET;
	else if (str == "Moon") return MOON;
	else if (str == "Dwarf") return DWARF;
	else if (str == "Asteroid") return ASTEROID;
	else if (str == "KBO") return KBO;
	else if (str == "Comet") return COMET;
	else if (str == "Artificial") return ARTIFICIAL;
	else if (str == "Observer") return OBSERVER;
	else
		return UNKNOWN;
}