/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <future>
#include "bodyModule/solarsystem.hpp"
#include "tools/s_texture.hpp"
#include "bodyModule/orbit.hpp"
#include "navModule/navigator.hpp"
#include "coreModule/projector.hpp"
#include "tools/utility.hpp"
#include <fstream>
#include "tools/log.hpp"
//#include "tools/fmath.hpp"
#include "tools/sc_const.hpp"
#include "tools/app_settings.hpp"
#include "bodyModule/ring.hpp"
#include "bodyModule/halo.hpp"

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/orbit_creator_cor.hpp"
#include "appModule/space_date.hpp"
#include "vulkanModule/Context.hpp"
#include "bodyModule/ssystem_iterator.hpp"


#define SOLAR_MASS 1.989e30
#define EARTH_MASS 5.976e24
#define LUNAR_MASS 7.354e22
#define MARS_MASS  0.64185e24


SolarSystem::SolarSystem(ThreadContext *_context, ObjLMgr *_objLMgr)
	:ProtoSystem(_context, _objLMgr), sun(nullptr),moon(nullptr),earth(nullptr), moonScale(1.)
{
}


SolarSystem::~SolarSystem()
{
	systemBodies.clear();
	renderedBodies.clear();

	// BodyShader::deleteShader();
	Body::deleteDefaultatmosphereParams();

	sun = nullptr;
	moon = nullptr;
	earth = nullptr;

}

void SolarSystem::setFont(float font_size, const std::string& font_name)
{
	ModuleFont::setFont(font_size, font_name);
	Body::setFont(font.get());
}

BODY_TYPE SolarSystem::setPlanetType (const std::string &str)
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

Body* SolarSystem::findBody(const std::string &name)
{

	if(systemBodies.count(name) != 0){
		return systemBodies[name]->body.get();
	}
	else{
		return nullptr;
	}
}

std::shared_ptr<SolarSystem::BodyContainer> SolarSystem::findBodyContainer(const std::string &name)
{
	if(systemBodies.count(name) != 0){
		return systemBodies[name];
	}
	else{
		return nullptr;
	}
}

// Init and load one solar system object
// This is a the private method
void SolarSystem::addBody(stringHash_t & param, bool deletable)
{
	//~ AutoPerfDebug apd(&pd, "SolarSystem::addBody$"); //Debug
	BODY_TYPE typePlanet= UNKNOWN;
	const std::string englishName = param["name"];
	std::string str_parent = param["parent"];
	const std::string type_Body = param["type"];
	Body *parent = nullptr;

	cLog::get()->write("Loading new Solar System object... " + englishName, LOG_TYPE::L_INFO);
	//~ cout << "Loading new Solar System object... " << englishName << endl;
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

	// no parent ? so it's Sun
	if (str_parent.empty())
		str_parent = "Sun";

	// no type ? it's an asteroid
	if (typePlanet == UNKNOWN)
		typePlanet = ASTEROID;

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
	// determination de l'orbite de l'astre
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
	// fin détermination de l'orbite
	//

	std::unique_ptr<BodyColor> bodyColor = nullptr;
	bodyColor = std::make_unique<BodyColor>(param["color"], param["label_color"], param["orbit_color"], param["trail_color"]);

	float solLocalDay= Utility::strToDouble(param["sol_local_day"],1.0);

	// Create the Body and add it to the list
	// p est un pointeur utilisé pour l'objet qui sera au final intégré dans la liste des astres que gère body_mgr
	std::unique_ptr<Body> p = nullptr;
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
		case SUN : {
			std::unique_ptr<Sun> p_sun = std::make_unique<Sun>(parent,
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
			  				bodyTexture,
							context);
			//update of sun's big_halo texture
			std::string bighalotexfile = param["tex_big_halo"];
			if (!bighalotexfile.empty()) {
				p_sun->setBigHalo(bighalotexfile, param["path"]);
				p_sun->setHaloSize(Utility::strToDouble(param["big_halo_size"], 50.f));
			}

			if (englishName == "Sun") {
				sun = p_sun.get();
				bodyTrace = p_sun.get();
			}
			p = std::move(p_sun);
		}
		break;

		case ARTIFICIAL: {
			std::unique_ptr<Artificial> p_artificial = std::make_unique<Artificial>(parent,
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
							  bodyTexture,
						  	  context);
			p=std::move(p_artificial);
			}
			break;

		case MOON: {
			std::unique_ptr<Moon> p_moon = std::make_unique<Moon>(parent,
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
							  bodyTexture,
							  context
			                 );
			if (englishName == "Moon") {
				moon = p_moon.get();
				if(earth) {
					BinaryOrbit *earthOrbit = dynamic_cast<BinaryOrbit *>(earth->getOrbit());
					if(earthOrbit) {
						cLog::get()->write("Adding Moon to Earth binary orbit.", LOG_TYPE::L_INFO);
						earthOrbit->setSecondaryOrbit(orb.get());
					} else
						cLog::get()->write(englishName + " body could not be added to Earth orbit.", LOG_TYPE::L_WARNING);

				} else
					cLog::get()->write(englishName + " body could not be added to Earth orbit calculation, position may be inacurate", LOG_TYPE::L_WARNING);
			}
			p=std::move(p_moon);
		}
		break;

		case DWARF:
		case PLANET: {
			std::unique_ptr<BigBody> p_big = std::make_unique<BigBody>(parent,
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
								bodyTexture,
								context
								);
			if (Utility::strToBool(param["rings"], 0)) {
				const double r_min = Utility::strToDouble(param["ring_inner_size"])/AU;
				const double r_max = Utility::strToDouble(param["ring_outer_size"])/AU;
				std::unique_ptr<Ring> r = std::make_unique<Ring>(r_min,r_max,param["tex_ring"],ringsInit,context);
				p_big->setRings(std::move(r));
				p_big->updateBoundingRadii();
			}

			if(englishName=="Earth") {
				earth = p_big.get();
			}

			p = std::move(p_big);
		}
		break;

		case ASTEROID:
		case KBO:
		case COMET: {
			std::unique_ptr<SmallBody> p_small = std::make_unique<SmallBody>(parent,
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
									bodyTexture,
									context
			                       );
			p = std::move(p_small);
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

	anchorManager->addAnchor(englishName, p.get());
	p->updateBoundingRadii();

	std::shared_ptr<BodyContainer> container = std::make_shared<BodyContainer>();
	container->body = std::move(p);
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

bool SolarSystem::removeBodyNoSatellite(const std::string &name)
{
	// std::cout << "removeBodyNoSatellite " << name << std::endl;
	// std::cout << "removing : " << name << std::endl;

	std::shared_ptr<BodyContainer> bc = findBodyContainer(name);

	if(bc == nullptr){
		cLog::get()->write("SolarSystem::removeBodyNoSatellite : Could not find a body named : " + name );
		return false;
	}

	//check if the body was a satellite
	if(bc->body->getParent() != nullptr){
		bc->body->getParent()->removeSatellite(bc->body.get());
	}
	// fix crash when delete body used from body_trace
	if (bc->body.get() == bodyTrace )
		bodyTrace = sun;

	//remove from containers :
	systemBodies.erase(bc->englishName);
	if(!bc->isHidden){
		// std::cout << "removeBodyNoSatellite from renderedBodies " << name << std::endl;
		std::remove_if(renderedBodies.begin(), renderedBodies.end(), [bc](std::shared_ptr<BodyContainer> const obj) {
			return bc->englishName == obj->englishName;
		});
	}

	anchorManager->removeAnchor(bc->body.get());
	//delete bc->body;

	// std::cout << "removeBodyNoSatellite " << name << " is oki" << std::endl;

	// std::cout << "début contenu de systemBodies--------------------" << std::endl;
	// for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
	// 	std::cout << "name " << it->first << std::endl;
	// }
	// std::cout << "fin contenu de systemBodies--------------------" << std::endl;

	// std::cout << "début contenu de renderedBodies--------------------" << std::endl;
	// for(auto it = renderedBodies.begin(); it != renderedBodies.end(); it++){
	// 	std::cout << "name " << (*it)->englishName << std::endl;
	// }
	// std::cout << "fin contenu de renderedBodies--------------------" << std::endl;
	return true;
}

bool SolarSystem::removeBody(const std::string &name){
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

bool SolarSystem::removeSupplementalBodies(const std::string &name)
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

void SolarSystem::initialSolarSystemBodies(){
	for(auto it = systemBodies.begin(); it != systemBodies.end();it++){
		it->second->body->reinitParam();
		if (it->second->isHidden != it->second->initialHidden) {
			if(it->second->initialHidden){
				std::remove_if(renderedBodies.begin(), renderedBodies.end(), [it](std::shared_ptr<BodyContainer> const obj) {
					return it->second->englishName == obj->englishName;
				});
			}
			else{

				renderedBodies.push_back(it->second);
			}
			it->second->isHidden = it->second->initialHidden;
		}
	}
}

void SolarSystem::toggleHideSatellites(bool val){

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


void SolarSystem::setPlanetHidden(const std::string &name, bool planethidden)
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

bool SolarSystem::getPlanetHidden(const std::string &name)
{
	if(systemBodies.count(name) != 0){
		return systemBodies[name]->isHidden;
	}
	else{
		return false;
	}
}

Object SolarSystem::searchByNamesI18(const std::string &planetNameI18) const
{
	// side effect - bad?
	//	transform(planetNameI18.begin(), planetNameI18.end(), planetNameI18.begin(), ::tolower);
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if ( it->second->body->getNameI18n() == planetNameI18 )
			return it->second->body.get(); // also check standard ini file names
	}
	return nullptr;
}

// Search if any Body is close to position given in earth equatorial position and return the distance
Object SolarSystem::search(Vec3d pos, const Navigator * nav, const Projector * prj) const
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
std::vector<Object> SolarSystem::searchAround(Vec3d v,
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

//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void SolarSystem::translateNames(Translator& trans)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->translateName(trans);
	}

	if(font)
		font->clearCache();
}

std::vector<std::string> SolarSystem::getNamesI18(void)
{
	std::vector<std::string> names;
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		names.push_back(it->second->body->getNameI18n());
	}
	return names;
}

std::string SolarSystem::getPlanetsPosition()
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


// returns a newline delimited hash of localized:standard Body names for tui
// Body translated name is PARENT : NAME
std::string SolarSystem::getPlanetHashString(void)
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

void SolarSystem::startTrails(bool b)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->startTrail(b);
	}
}

void SolarSystem::setFlagAxis(bool b)
{
	flagAxis=b;
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->setFlagAxis(b);
	}
}


void SolarSystem::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second->body->update(delta_time, nav, timeMgr);
	}
}

// is a lunar eclipse close at hand?
bool SolarSystem::nearLunarEclipse(const Navigator * nav, Projector *prj)
{
	// TODO: could replace with simpler test
	Vec3d e = getEarth()->get_ecliptic_pos();
	Vec3d m = getMoon()->get_ecliptic_pos();  // relative to earth
	Vec3d mh = getMoon()->get_heliocentric_ecliptic_pos();  // relative to sun

	// shadow location at earth + moon distance along earth vector from sun
	Vec3d en = e;
	en.normalize();
	Vec3d shadow = en * (e.length() + m.length());

	// find shadow radii in AU
	double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;

	// modify shadow location for scaled moon
	Vec3d mdist = shadow - mh;
	if (mdist.length() > r_penumbra + 2000/AU) return 0;  // not visible so don't bother drawing

	return 1;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
std::vector<std::string> SolarSystem::listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const
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


void SolarSystem::bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const
{
	bodyTrace->getAltAz(nav,alt,az);
}

void SolarSystem::bodyTraceBodyChange(const std::string &bodyName)
{
	Body * body = searchByEnglishName(bodyName);

	if(body != nullptr){
		bodyTrace = body;
	}
	else{
		cLog::get()->write("Unknown planet_name in bodyTraceBodyChange", LOG_TYPE::L_ERROR);
	}
}

bool SolarSystem::getFlag(BODY_FLAG name)
{
	switch (name) {
		case BODY_FLAG::F_AXIS : return flagAxis; break;
		case BODY_FLAG::F_CLOUDS: return Body::getFlagClouds(); break;
		default: break;
	}
	return false;
}

double SolarSystem::getSunAltitude(const Navigator * nav) const
{
	double alt, az;
	sun->getAltAz(nav, &alt, &az);
	return alt*180.0/M_PI;
}

double SolarSystem::getSunAzimuth(const Navigator * nav) const
{
	double alt, az;
	sun->getAltAz(nav, &alt, &az);
	return az*180.0/M_PI;
}

std::unique_ptr<SSystemIterator> SolarSystem::createIterator()
{
	return std::make_unique<SSystemIterator>(this);
}

std::unique_ptr<SSystemIteratorVector> SolarSystem::createIteratorVector()
{
	return std::make_unique<SSystemIteratorVector>(this);
}