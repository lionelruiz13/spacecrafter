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

#include "bodyModule/solarsystem.hpp"
#include "bodyModule/orbit_creator_cor.hpp"
#include "bodyModule/ssystem_iterator.hpp"
#include "appModule/space_date.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "navModule/anchor_manager.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"
#include "tools/translator.hpp"
#include "bodyModule/body_smallbody.hpp"
#include "bodyModule/body_artificial.hpp"
#include "bodyModule/body_center.hpp"
#include "tools/object.hpp"
#include "interfaceModule/base_command_interface.hpp"

#define EARTH_MASS 5.976e24
#define LUNAR_MASS 7.354e22

bool ProtoSystem::initGuard = false;
Vec3d ProtoSystem::currentCenterPos = {};

ProtoSystem::ProtoSystem(ObjLMgr *_objLMgr, Observer *observatory, Navigator *navigation, TimeMgr *timeMgr, const Vec3d &centerPos)
	: objLMgr(_objLMgr), centerPos(centerPos)
{
	bodyTrace = nullptr;

	if (!initGuard) {
		Body::createShader();
		BodyShader::createShader();
		Body::createDefaultAtmosphereParams();
		initGuard = true;
	}

	std::shared_ptr<OrbitCreator> special = std::make_shared<OrbitCreatorSpecial>(nullptr);
	std::shared_ptr<OrbitCreator> comet = std::make_shared<OrbitCreatorComet>(special, this);
	std::shared_ptr<OrbitCreator> elip = std::make_shared<OrbitCreatorEliptic>(comet, this);
	orbitCreator = std::make_shared<OrbitCreatorBary>(elip, this);

	anchorManager = std::make_shared<AnchorManager>(observatory,navigation, this, timeMgr, getOrbitCreator());
	setAnchorManager(anchorManager);
}

ProtoSystem::~ProtoSystem()
{
	systemBodies.clear();
	renderedBodies.clear();
	//delete anchorManager;
}


static ATMOSPHERE_MODEL setAtmosphere(const std::string& atmModel)
{
	if (atmModel=="earth_model") return ATMOSPHERE_MODEL::EARTH_MODEL;
	if (atmModel=="venus_model") return ATMOSPHERE_MODEL::VENUS_MODEL;
	if (atmModel=="mars_model") return ATMOSPHERE_MODEL::MARS_MODEL;
	return ATMOSPHERE_MODEL::NONE_MODEL;
}

static std::string setAtmosphereTable(ATMOSPHERE_MODEL atmModel)
{
	switch (atmModel) {
		case ATMOSPHERE_MODEL::EARTH_MODEL:
			return "bodies/AtmosphereGradient3.jpg";
		case ATMOSPHERE_MODEL::VENUS_MODEL:
			return "bodies/AtmosphereGradient2.jpg";
		case ATMOSPHERE_MODEL::MARS_MODEL:
			return "bodies/AtmosphereGradient4.jpg";
		default:
			return {};
	}
}

void ProtoSystem::load(Object &obj)
{
	stringHash_t bodyParams;
	bodyParams["name"] = obj.getEnglishName();
	bodyParams["parent"] = "Center" + bodyParams["name"];
	bodyParams["type"] = "Sun";
	bodyParams["radius"] = "1190.856";
	bodyParams["halo"] = "false";
	Vec3f color = obj.getRGB();
	bodyParams["color"] = std::to_string(color[0]) + "," + std::to_string(color[1]) + "," + std::to_string(color[2]);
	bodyParams["label_color"] = bodyParams["color"];
	bodyParams["orbit_color"] = bodyParams["color"];
	bodyParams["tex_map"] = "bodies/sirius.png";
	bodyParams["tex_halo"] = "empty";
	bodyParams["tex_big_halo"] = "big_halo.png";
	bodyParams["big_halo_size"] = "10";
	bodyParams["lighting"] = "false";
	bodyParams["albedo"] = "-1.";
	bodyParams["coord_func"] = "stellar_special";
	if (bodyParams["parent"] != "none") {
		stringHash_t parentParams = bodyParams;
		parentParams["name"] = bodyParams["parent"];
		parentParams["parent"] = "none";
		parentParams["type"] = "Center";
		addBody(parentParams, false);
	}
	addBody(bodyParams, false);
	bodyParams.clear();
}

// Init and load the solar system data
void ProtoSystem::load(const std::string& planetfile)
{
	stringHash_t bodyParams;

	std::ifstream fileBody (planetfile.c_str() , std::ifstream::in);
	if(fileBody) {
		std::string ligne;
		while(getline(fileBody , ligne)) {
			if (ligne.size() < 2)
				continue;
			if (ligne[0] != '[' ) {
				if (ligne[ligne.length() - 1] == '\r')
					ligne.pop_back();
				if (ligne[0]!='#' && ligne.size() != 0) {
					int pos = ligne.find('=',0);
					std::string p1=ligne.substr(0,pos-1);
					std::string p2=ligne.substr(pos+2,ligne.size());
					bodyParams[p1]=p2;
				}
			} else {
				if (bodyParams.size() !=0) {
					//TODO recover this error if there is one!
					addBody(bodyParams, false);  // config file bodies are not deletable
					bodyParams.clear();
				}
			}
		}
		if (!bodyParams.empty())
			addBody(bodyParams, false);  // config file bodies are not deletable
		fileBody.close();
	} else
		cLog::get()->write("Unable to open file "+ planetfile, LOG_TYPE::L_ERROR);

	cLog::get()->write("(solar system loaded)", LOG_TYPE::L_INFO);
	cLog::get()->mark();
}

std::shared_ptr<Body> ProtoSystem::searchByEnglishName(const std::string &planetEnglishName) const
{
	//printf("SolarSystem::searchByEnglishName(\"%s\"): start\n", planetEnglishName.c_str());
	// side effect - bad?
	//	transform(planetEnglishName.begin(), planetEnglishName.end(), planetEnglishName.begin(), ::tolower);
	if(systemBodies.count(planetEnglishName) != 0){
		return systemBodies.find(planetEnglishName)->second->body;
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

		for (auto it : bc->body->getSatellites()) {
			names.push_back(it->getEnglishName());
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

bool ProtoSystem::removeBodyNoSatellite(const std::string &name)
{
	// std::cout << "removeBodyNoSatellite " << name << std::endl;
	// std::cout << "removing : " << name << std::endl;

	std::shared_ptr<BodyContainer> bc = findBodyContainer(name);

	if(bc == nullptr){
		cLog::get()->write("SolarSystem::removeBodyNoSatellite : Could not find a body named : " + name );
		return false;
	}

	// fix crash when delete body used from body_trace
	if (bc->body == bodyTrace )
		bodyTrace = getCenterObject();

	//remove from containers :
	systemBodies.erase(bc->englishName);
	if(!bc->isHidden){
		auto it2 = std::find_if(renderedBodies.begin(), renderedBodies.end(), [bc](std::shared_ptr<BodyContainer> const obj) {
			return bc->englishName == obj->englishName;
		});
		if (it2 != renderedBodies.end())
			renderedBodies.erase(it2);
	}

	anchorManager->removeAnchor(bc->body);
	//delete bc->body;

	// std::cout << "removeBodyNoSatellite " << name << " is oki" << std::endl;

	// std::cout << "Start of the content of systemBodies--------------------" << std::endl;
	// for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
	// 	std::cout << "name " << it->first << std::endl;
	// }
	// std::cout << "End of the content of systemBodies--------------------" << std::endl;

	// std::cout << "Start of the content of renderedBodies--------------------" << std::endl;
	// for(auto it = renderedBodies.begin(); it != renderedBodies.end(); it++){
	// 	std::cout << "name " << (*it)->englishName << std::endl;
	// }
	// std::cout << "End of the content of renderedBodies--------------------" << std::endl;
	return true;
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

		   for (auto satellite : it->second->body->getSatellites()){
			   std::shared_ptr<BodyContainer> sat = findBodyContainer(satellite->getEnglishName());
				setPlanetHidden(sat->englishName, val);
			}
		}
	}
}

void ProtoSystem::setPlanetHidden(const std::string &name, bool planethidden)
{

	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
		std::shared_ptr<Body> body = it->second->body;
		if (
			body->getEnglishName() == name ||
			(body->get_parent() && body->get_parent()->getEnglishName() == name) ){

			it->second->isHidden = planethidden;

			if(planethidden){
				auto it2 = std::find_if(renderedBodies.begin(), renderedBodies.end(), [it](std::shared_ptr<BodyContainer> const obj) {
					return it->second->englishName == obj->englishName;
				});
				if (it2 != renderedBodies.end())
					renderedBodies.erase(it2);
			}
			else{
				// std::cout << "I am looking for a duplicate of " << name << std::endl;
				if (std::find(renderedBodies.begin(), renderedBodies.end(), it->second) == renderedBodies.end() ) {
					renderedBodies.push_back(it->second);
					// std::cout << "I really create "<< name << " in renderedBodies" << std::endl;
				}
				// else {
					// std::cout << "I avoided a duplicate" << std::endl;
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
	std::shared_ptr<Body> home_Body = observatory->getHomeBody();

	*default_last_item = false;

	// Should still be sorted by distance from farthest to closest
	// So work backwards to go closest to furthest
	for(auto it = systemBodies.rbegin(); it != systemBodies.rend(); it++){//reverse order

		equPos = it->second->body->getEarthEquPos(nav);
		equPos.normalize();

		// First see if within a Body disk
		if (it->second->body != home_Body || aboveHomeBody) {
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
	std::shared_ptr<Body> body = searchByEnglishName(bodyName);

	if(body != nullptr){
		bodyTrace = body;
	}
	else{
		cLog::get()->write("Unknown planet_name in bodyTraceBodyChange", LOG_TYPE::L_ERROR);
	}
}

std::shared_ptr<Body> ProtoSystem::findBody(const std::string &name)
{

	if(systemBodies.count(name) != 0){
		return systemBodies[name]->body;
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
	else if (str == "Center") return CENTER;
	else
		return UNKNOWN;
}


// Init and load one solar system object
// This is a the private method
void ProtoSystem::addBody(stringHash_t & param, bool deletable)
{
	//~ AutoPerfDebug apd(&pd, "SolarSystem::addBody$"); //Debug
	BODY_TYPE typePlanet= UNKNOWN;
	const std::string englishName = param["name"];
	std::string str_parent = param["parent"];
	const std::string type_Body = param["type"];
	std::shared_ptr<Body> parent = nullptr;
	cLog::get()->write("Loading new Stellar System object... " + englishName, LOG_TYPE::L_INFO);
	std::cout << "Loading new Stellar System object... " << englishName << std::endl;
	//~ for ( stringHashIter_t iter = param.begin(); iter != param.end(); ++iter ) {
	//~ cout << iter->first << " : " << iter->second << endl;
	//~ }

	// set the Body type: ie what it is in universe
	typePlanet= setPlanetType(type_Body);

	// do not add if no name or no parent or no typePlanet
	if (englishName.empty()) {
		cLog::get()->write("SolarSystem: can not add body with no name", LOG_TYPE::L_WARNING);
		return;
	}

	// no parent ? so it's the center body
	if (str_parent.empty()) {
		str_parent = (centerObject) ? centerObject->getEnglishName() : "none";
		cLog::get()->write("No body parent specified, assume parent is " + str_parent + " (Specify none for no parent)", LOG_TYPE::L_WARNING);
	}

	// no type ? it's an asteroid
	if (typePlanet == UNKNOWN) {
		typePlanet = ASTEROID;
		cLog::get()->write("No valid body type specified for " + englishName + ", assume 'Asteroid'", LOG_TYPE::L_WARNING);
	}

	// Do not add if body already exists - name must be unique
	if ( findBody(englishName)!=nullptr ) {
		cLog::get()->write("SolarSystem: Can not add body named " + englishName + " because a body of that name already exist", LOG_TYPE::L_WARNING);
		return;
	//	return (std::string("Can not add body named \"") + englishName + std::string("\" because a body of that name already exists\n"));
	}

	if (str_parent!="none") {
		parent = findBody(str_parent);

		if (parent == nullptr) {
			//std::string error = std::string("WARNING : can't find parent for ") + englishName;
			cLog::get()->write("SolarSystem: can't find parent for " + englishName, LOG_TYPE::L_WARNING);
			return;
		}
	}

	const std::string funcname = param["coord_func"];

	//
	// determination of the orbit of the star
	//
	std::unique_ptr<Orbit> orb = nullptr;
	bool close_orbit = Utility::strToBool(param["close_orbit"], 1);

	// default value of -1 means unused
	double orbit_bounding_radius = Utility::strToDouble(param["orbit_bounding_radius"], -1);
	if (funcname=="earth_custom") {
		// Special case to take care of Earth-Moon Barycenter at a higher level than in ephemeris library

		//cout << "Creating Earth orbit...\n" << endl;
		cLog::get()->write("Creating Earth orbit...", LOG_TYPE::L_INFO);
		std::unique_ptr<SpecialOrbit> sorb = std::make_unique<SpecialOrbit>("emb_special");
		if (!sorb->isValid()) {
			std::string error = std::string("ERROR : can't find position function ") + funcname + std::string(" for ") + englishName + std::string("\n");
			cLog::get()->write(error, LOG_TYPE::L_ERROR);
			return;
		}
		// NB. moon has to be added later
		orb = std::make_unique<BinaryOrbit>(std::move(sorb), 0.0121505677733761);

	} else if(funcname == "lunar_custom") {
		// This allows chaotic Moon ephemeris to be removed once start leaving acurate and sane range

		std::unique_ptr<SpecialOrbit> sorb = std::make_unique<SpecialOrbit>("lunar_special");

		if (!sorb->isValid()) {
			std::string error = std::string("ERROR : can't find position function ") + funcname + std::string(" for ") + englishName + std::string("\n");
			cLog::get()->write(error, LOG_TYPE::L_ERROR);
			return ;
		}

		orb = std::make_unique<MixedOrbit>(std::move(sorb),
		                     Utility::strToDouble(param["orbit_period"]),
		                     SpaceDate::JulianDayFromDateTime(-10000, 1, 1, 1, 1, 1),
		                     SpaceDate::JulianDayFromDateTime(10000, 1, 1, 1, 1, 1),
		                     EARTH_MASS + LUNAR_MASS,
		                     0, 0, 0,
		                     false);

	} else if (funcname == "still_orbit") {
		orb = std::make_unique<stillOrbit>(Utility::strToDouble(param["orbit_x"]),
		                     Utility::strToDouble(param["orbit_y"]),
		                     Utility::strToDouble(param["orbit_z"]));
	} else {

		orb = std::move(orbitCreator->handle(param));

		if(orb == nullptr) {
			std::cout << "something went wrong when creating orbit from "<< englishName << std::endl;
			cLog::get()->write("Error when creating orbit from " + englishName, LOG_TYPE::L_ERROR);
		}
	}

	if(param["coord_func"] == "ell_orbit"){
		orbit_bounding_radius = orb->getBoundingRadius();
	}

	//
	// end orbit determination
	//

	std::unique_ptr<BodyColor> bodyColor = nullptr;
	bodyColor = std::make_unique<BodyColor>(param["color"], param["label_color"], param["orbit_color"], param["trail_color"]);

	float solLocalDay= Utility::strToDouble(param["sol_local_day"],1.0);

	// Create the Body and add it to the list
	// p is a pointer used for the object that will be finally integrated in the list of stars that body_mgr manages
	std::shared_ptr<Body> p = nullptr;
	ObjL* currentOBJ = nullptr;

	std::string modelName = param["model_name"];

	std::shared_ptr<BodyTexture> bodyTexture = std::make_shared<BodyTexture>();
	bodyTexture->tex_map = param["tex_map"];
	bodyTexture->tex_norm = param["tex_normal"];
	bodyTexture->tex_heightmap = param["tex_heightmap"];
	bodyTexture->tex_night = param["tex_night"];
	bodyTexture->tex_specular = param["tex_specular"];
	// bodyTexture->tex_cloud = param["tex_cloud"];
	// bodyTexture->tex_cloud_normal = param["tex_cloud_normal"];
	bodyTexture->tex_skin =  param["tex_skin"];


	if ( !modelName.empty()) {
		objLMgr->insertObj(modelName);
		currentOBJ = objLMgr->select(modelName);
	}
	else
		currentOBJ = objLMgr->selectDefault();

	switch (typePlanet) {
		case CENTER: {
			std::shared_ptr<Center> p_center = std::make_shared<Center>(parent,
			                englishName,
			                Utility::strToBool(param["halo"]),
			                Utility::strToDouble(param["radius"])/AU,
			                Utility::strToDouble(param["oblateness"], 0.0),
			                std::move(bodyColor),
			                solLocalDay,
			                Utility::strToDouble(param["albedo"]),
			                std::move(orb),
			                close_orbit,
			                currentOBJ,
			                orbit_bounding_radius,
			  				bodyTexture);
			//update of sun's big_halo texture
			std::string bighalotexfile = param["tex_big_halo"];
			if (!bighalotexfile.empty()) {
				p_center->setBigHalo(bighalotexfile, param["path"]);
				p_center->setHaloSize(Utility::strToDouble(param["big_halo_size"], 50.f));
			}

			bodyTrace = p_center;
			centerObject = p_center;
			p = p_center;
		}
		break;
		case SUN : {
			std::shared_ptr<Sun> p_sun = std::make_shared<Sun>(parent,
			                englishName,
			                Utility::strToBool(param["halo"]),
			                Utility::strToDouble(param["radius"])/AU,
			                Utility::strToDouble(param["oblateness"], 0.0),
			                std::move(bodyColor),
			                solLocalDay,
			                Utility::strToDouble(param["albedo"]),
			                std::move(orb),
			                close_orbit,
			                currentOBJ,
			                orbit_bounding_radius,
			  				bodyTexture);
			//update of sun's big_halo texture
			std::string bighalotexfile = param["tex_big_halo"];
			if (!bighalotexfile.empty()) {
				p_sun->setBigHalo(bighalotexfile, param["path"]);
				p_sun->setHaloSize(Utility::strToDouble(param["big_halo_size"], 50.f));
			}

			if (!parent) {
				centerObject = p_sun;
				bodyTrace = p_sun;
			}
			p = p_sun;
		}
		break;

		case ARTIFICIAL: {
			std::shared_ptr<Artificial> p_artificial = std::make_shared<Artificial>(parent,
							  englishName,
							  Utility::strToBool(param["halo"]),
							  Utility::strToDouble(param["radius"])/AU,
			                  std::move(bodyColor),
			                  solLocalDay,
			                  Utility::strToDouble(param["albedo"]),
							  std::move(orb),
			                  close_orbit,
			                  param["model_name"],
			                  deletable,
			                  orbit_bounding_radius,
							  bodyTexture);
			p= p_artificial;
			}
			break;

		case MOON: {
			std::shared_ptr<Moon> p_moon = std::make_shared<Moon>(parent,
			                  englishName,
			                  Utility::strToBool(param["halo"]),
			                  Utility::strToDouble(param["radius"])/AU,
			                  Utility::strToDouble(param["oblateness"], 0.0),
			                  std::move(bodyColor),
			                  solLocalDay,
			                  Utility::strToDouble(param["albedo"]),
			                  std::move(orb),
			                  close_orbit,
			                  currentOBJ,
			                  orbit_bounding_radius,
							  bodyTexture
			                 );
			p= p_moon;
		}
		break;

		case DWARF:
		case PLANET: {
			std::shared_ptr<BigBody> p_big = std::make_shared<BigBody>(parent,
			                    englishName,
			                    typePlanet,
			                    Utility::strToBool(param["halo"]),
			                    Utility::strToDouble(param["radius"])/AU,
			                    Utility::strToDouble(param["oblateness"], 0.0),
			                    std::move(bodyColor),
			                    solLocalDay,
			                    Utility::strToDouble(param["albedo"]),
			                    std::move(orb),
			                    close_orbit,
			                    currentOBJ,
			                    orbit_bounding_radius,
								bodyTexture
								);


			if (Utility::strToBool(param["rings"], 0)) {
				const double r_min = Utility::strToDouble(param["ring_inner_size"])/AU;
				const double r_max = Utility::strToDouble(param["ring_outer_size"])/AU;
				std::unique_ptr<Ring> r = std::make_unique<Ring>(r_min,r_max,param["tex_ring"],ringsInit);
				p_big->setRings(std::move(r));
				p_big->updateBoundingRadii();
			}
			p = p_big;
		}
		break;

		case ASTEROID:
		case KBO:
		case COMET: {
			std::shared_ptr<SmallBody> p_small = std::make_shared<SmallBody>(parent,
			                        englishName,
			                        typePlanet,
			                        Utility::strToBool(param["halo"]),
			                        Utility::strToDouble(param["radius"])/AU,
			                        Utility::strToDouble(param["oblateness"], 0.0),
			                        std::move(bodyColor),
			                        solLocalDay,
			                        Utility::strToDouble(param["albedo"]),
			                        std::move(orb),
			                        close_orbit,
			                        currentOBJ,
			                        orbit_bounding_radius,
									bodyTexture
			                       );
			p = p_small;
		}
		break;

		default:
			cLog::get()->write("Undefined body", LOG_TYPE::L_ERROR);
	}
	if (p == nullptr) {
		cLog::get()->write("Failed to create body", LOG_TYPE::L_ERROR);
		return;
	}

	if (!param["has_atmosphere"].empty()) {
		AtmosphereParams* tmp = nullptr;
		tmp = new(AtmosphereParams);
		tmp->hasAtmosphere = Utility::strToBool(param["has_atmosphere"], false);
		tmp->modelAtmosphere = setAtmosphere(param["atmosphere_model"]);
		tmp->tableAtmosphere = param["atmosphere_ext_model"].empty() ? setAtmosphereTable(tmp->modelAtmosphere) : param["atmosphere_ext_model"];
		tmp->atmosphereRadiusFactor = param["atmosphere_radius_factor"].empty() ? 1.05 : Utility::strToDouble(param["atmosphere_radius_factor"]);
		tmp->limInf = Utility::strToFloat(param["atmosphere_lim_inf"], 40000.f);
		tmp->limSup = Utility::strToFloat(param["atmosphere_lim_sup"], 80000.f);
		tmp->limLandscape = Utility::strToFloat(param["atmosphere_lim_landscape"], 10000.f);
		p->setAtmosphereParams(tmp);
	}


	// Use J2000 N pole data if available
	double rot_obliquity = Utility::strToDouble(param["rot_obliquity"],0.)*M_PI/180.;
	double rot_asc_node  = Utility::strToDouble(param["rot_equator_ascending_node"],0.)*M_PI/180.;

	// In J2000 coordinates
	double J2000_npole_ra = Utility::strToDouble(param["rot_pole_ra"],0.)*M_PI/180.;
	double J2000_npole_de = Utility::strToDouble(param["rot_pole_de"],0.)*M_PI/180.;

	// NB: north pole needs to be defined by right hand rotation rule
	if (param["rot_pole_ra"] != "" || param["rot_pole_de"] != "") {
		// cout << "Using north pole data for " << englishName << endl;
		Vec3d J2000_npole;
		Utility::spheToRect(J2000_npole_ra,J2000_npole_de,J2000_npole);

		Vec3d vsop87_pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(J2000_npole));

		double ra, de;
		Utility::rectToSphe(&ra, &de, vsop87_pole);

		rot_obliquity = (M_PI_2 - de);
		rot_asc_node = (ra + M_PI_2);
		//cout << "\tCalculated rotational obliquity: " << rot_obliquity*180./M_PI << endl;
		//cout << "\tCalculated rotational ascending node: " << rot_asc_node*180./M_PI << endl;
	}

	p->set_rotation_elements(
	    Utility::strToDouble(param["rot_periode"], Utility::strToDouble(param["orbit_period"], 24.))/24.,
	    Utility::strToDouble(param["rot_rotation_offset"],0.),
	    Utility::strToDouble(param["rot_epoch"], J2000),
	    rot_obliquity,
	    rot_asc_node,
	    Utility::strToDouble(param["rot_precession_rate"],0.)*M_PI/(180*36525),
	    Utility::strToDouble(param["orbit_visualization_period"],0.),
	    Utility::strToDouble(param["axial_tilt"],0.) );

	// Clone current flags to new body unless one is currently selected
	// WARNING TODO
	//p->setFlagHints(flagHints);
	//p->setFlagTrail(flagTrails);
//
	//if (!selected || selected == Object(sun)) {
	//	p->setFlagOrbit(getFlag(BODY_FLAG::F_ORBIT));
	//}

	anchorManager->addAnchor(englishName, p);
	p->updateBoundingRadii();
	std::shared_ptr<BodyContainer> container = std::make_shared<BodyContainer>();
	container->body = p;
	container->englishName = englishName;
	container->isDeleteable = deletable;
	container->isHidden = Utility::strToBool(param["hidden"], 0);
	container->initialHidden = container->isHidden;

	systemBodies.insert(std::pair<std::string, std::shared_ptr<BodyContainer>>(englishName, container));

	if(!container->isHidden){
		// std::cout << "renderedBodies from addBody " << englishName << std::endl;
		renderedBodies.push_back(container);
	}
}

void ProtoSystem::initialSolarSystemBodies(){
	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
		it->second->body->reinitParam();
		if (it->second->isHidden != it->second->initialHidden) {
			if(it->second->initialHidden){
				auto it2 = std::find_if(renderedBodies.begin(), renderedBodies.end(), [it](std::shared_ptr<BodyContainer> const obj) {
					return it->second->englishName == obj->englishName;
				});
				if (it2 != renderedBodies.end())
					renderedBodies.erase(it2);
			}
			else{

				renderedBodies.push_back(it->second);
			}
			it->second->isHidden = it->second->initialHidden;
		}
	}
}

void ProtoSystem::preloadBody(stringHash_t & param)
{
	auto body = searchByEnglishName(param[W_NAME]);
	if (!param[W_PURGE].empty()) {
		if (param[W_PURGE] == W_ALL) {
			// Aggressive purge, release as many memory as possible, even used ones if possible
			s_texture::releaseAllMemory();
		}
		if (param[W_PURGE] == W_AUTO)
			s_texture::releaseUnusedMemory();
	}
	if (body) {
		int duration = 60 * 10;
		if (!param[W_KEEPTIME].empty())
			duration = Utility::strToInt(param[W_KEEPTIME]);
		body->preload(duration);
	}
}

void ProtoSystem::selectSystem()
{
	anchorManager->selectAnchor();
	currentCenterPos = centerPos;
}
