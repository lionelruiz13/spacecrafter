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
#include "bodyModule/body_star.hpp"
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

void ProtoSystem::load(Object &obj)
{
	stringHash_t bodyParams;
	bodyParams["name"] = obj.getEnglishName();
	bodyParams["parent"] = "none";
	bodyParams["type"] = "Star";
	bodyParams["radius"] = "1190.856";
	bodyParams["halo"] = "false";
	Vec3f color = obj.getRGB();
	bodyParams["color"] = std::to_string(color[0]) + "," + std::to_string(color[1]) + "," + std::to_string(color[2]);
	bodyParams["label_color"] = bodyParams["color"];
	bodyParams["orbit_color"] = bodyParams["color"];
	bodyParams["tex_halo"] = "empty";
	bodyParams["tex_big_halo"] = "big_halo.png";
	bodyParams["big_halo_size"] = "10";
	bodyParams["lighting"] = "false";
	bodyParams["albedo"] = "-1.";
	bodyParams["coord_func"] = "stellar_special";
	addBody(std::move(bodyParams), false);
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
					addBody(std::move(bodyParams), false);  // config file bodies are not deletable
				}
			}
		}
		if (!bodyParams.empty())
			addBody(std::move(bodyParams), false);  // config file bodies are not deletable
		fileBody.close();
	} else
		cLog::get()->write("Unable to open file "+ planetfile, LOG_TYPE::L_ERROR);

	cLog::get()->write("(solar system loaded)", LOG_TYPE::L_INFO);
	cLog::get()->mark();
}

void ProtoSystem::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	for (auto &v : systemBodies) {
		v.second.body->update(delta_time, nav, timeMgr);
	}
}

bool ProtoSystem::removeBody(const std::string &name)
{
	// std::cout << "removeBody " << name << std::endl;
	auto it = systemBodies.find(name);

	if (it == systemBodies.end()) {
		cLog::get()->write("SolarSystem::removeBody : Could not find a body named : " + name );
		return false;
	}

	if (it->second.body->hasSatellite()) {
		// std::cout << "removeBody " << name << " but have satellite" << std::endl;
		std::vector<std::string> names;

		for (auto sat : it->second.body->getSatellites()) {
			names.push_back(sat->getEnglishName());
		}

		for (const std::string &satName : names) {
			if (!removeBody(satName)) {
				cLog::get()->write("SolarSystem::removeBody : Could not remove satelite : " + satName );
			}
		}
	}
	// std::cout << "removeBody " << name << " is oki" << std::endl;
	removeBodyNoSatellite(it);
	return true;
}

void ProtoSystem::removeBodyNoSatellite(std::map<std::string, BodyContainer>::iterator it)
{
	// fix crash when delete body used from body_trace
	if (it->second.body == bodyTrace)
		bodyTrace = getCenterObject();

	hideBody(it->second.body.get());
	anchorManager->removeAnchor(it->second.body);
	systemBodies.erase(it);
}

bool ProtoSystem::removeSupplementalBodies(const std::string &name)
{
	auto bc = systemBodies.find(name);

	if (bc == systemBodies.end()) {
		cLog::get()->write("SolarSystem::removeSupplementalBodies : Could not find a body named : " + name );
		return false;
	}

	if (bc->second.isDeleteable) {
		cLog::get()->write("SolarSystem::removeSupplementalBodies : Can't destroy suplementary bodies if attached to one");
		return false;
	}

	std::vector<std::map<std::string, BodyContainer>::iterator> targets;
	for (auto it = systemBodies.begin(); it != systemBodies.end(); it++) {
		if (it->second.isDeleteable)
			targets.push_back(it);
	}

	for (auto &it : targets)
		removeBodyNoSatellite(it);
	return true;
}

//! @brief Update i18 names from english names according to passed translator
//! The translation is done using gettext with translated strings defined in translations.h
void ProtoSystem::translateNames(Translator& trans)
{
	for (auto &v : systemBodies)
		v.second.body->translateName(trans);

	if(font)
		font->clearCache();
}

// returns a newline delimited hash of localized:standard Body names for tui
// Body translated name is PARENT : NAME
std::string ProtoSystem::getPlanetHashString(void)
{
	std::ostringstream oss;
	for (auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		if (!it->second.isDeleteable ) { // no supplemental bodies in list
			if (it->second.body->getTurnAround() == tABody) {
				oss << Translator::globalTranslator.translateUTF8(it->second.body->get_parent()->getEnglishName())
				    << " : ";
			}

			oss << Translator::globalTranslator.translateUTF8(it->first) << "\n";
			oss << it->first << "\n";
		}
	}
	return oss.str();
}

void ProtoSystem::toggleHideSatellites(bool val){

	val = !val;

	if (flagHideSatellites == val)
		return;

	flagHideSatellites = val;
	if (val) {
		for (auto &v : systemBodies) {
			if (v.second.body->isSatellite())
				hideBody(v.second.body.get());
		}
	} else {
		for (auto &v : systemBodies) {
			if (v.second.body->isSatellite())
				showBody(v.second.body.get());
		}
	}
}

void ProtoSystem::setPlanetHidden(const std::string &name, bool planethidden)
{
	Body *body;
	try {
		body = systemBodies.at(name).body.get();
	} catch (...) {
		return;
	}
	if (planethidden) {
		hideBodyRecursive(body);
	} else {
		showBody(body);
	}
}

void ProtoSystem::hideBodyRecursive(Body *body)
{
	for (auto &b : body->getSatellites())
		hideBodyRecursive(b);
	hideBody(body);
}

// Search if any Body is close to position given in earth equatorial position and return the distance
Object ProtoSystem::search(Vec3d pos, const Navigator * nav, const Projector * prj) const
{
	pos.normalize();
	Body * closest = nullptr;
	double cos_angle_closest = 0.;

	for (auto &v : systemBodies){
		Vec3d equPos = v.second.body->getEarthEquPos(nav);
		equPos.normalize();
		double cos_ang_dist = equPos[0]*pos[0] + equPos[1]*pos[1] + equPos[2]*pos[2];
		if (cos_ang_dist>cos_angle_closest) {
			closest = v.second.body.get();
			cos_angle_closest = cos_ang_dist;
		}
	}

	return (cos_angle_closest>0.999) ? closest : nullptr;
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
	auto home_Body = observatory->getHomeBody().get();

	*default_last_item = false;

	// Should still be sorted by distance from farthest to closest
	// So work backwards to go closest to furthest
	const auto _end = sortedRenderedBodies.rend();
	for (auto it = sortedRenderedBodies.rbegin(); it != _end; ++it) {
		Vec3d equPos = (*it)->getEarthEquPos(nav);
		equPos.normalize();

		// First see if within a Body disk
		if ((*it) != home_Body || aboveHomeBody) {
			// Don't want home Body too easy to select unless can see it

			double angle = acos(v*equPos) * 180.f / M_PI;
			  //~ cout << "Big testing " << (*iter)->getEnglishName()
			  //~ << " angle: " << angle << " screen_angle: "
			  //~ << (*iter)->get_angular_size(prj, nav)/2.f
			  //~ << endl;
			if ( angle < (*it)->get_angular_size(prj, nav)/2.f ) {
				// If near planet, may be huge but hard to select, so check size
				result.push_back((*it));
				*default_last_item = true;
				// TODO take distance into consideration
				break;  // do not want any planets behind this one!
			}
		}
		// See if within area of interest
		if (equPos[0]*v[0] + equPos[1]*v[1] + equPos[2]*v[2]>=cos_lim_fov) {
			result.push_back((*it));
		}
	}
	return result;
}

Object ProtoSystem::searchByNamesI18(const std::string &planetNameI18) const
{
	// side effect - bad?
	//	transform(planetNameI18.begin(), planetNameI18.end(), planetNameI18.begin(), ::tolower);
	for (auto &v : systemBodies) {
		if (v.second.body->getNameI18n() == planetNameI18)
			return v.second.body.get();
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
	flagAxis = b;
	for (auto &v : systemBodies)
		v.second.body->setFlagAxis(b);
}

void ProtoSystem::startTrails(bool b)
{
	for(auto it = systemBodies.begin(); it != systemBodies.end(); it++){
		it->second.body->startTrail(b);
	}
}

std::vector<std::string> ProtoSystem::getNamesI18()
{
	std::vector<std::string> names;
	names.reserve(systemBodies.size());
	for (auto &v : systemBodies)
		names.push_back(v.second.body->getNameI18n());
	return names;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
std::vector<std::string> ProtoSystem::listMatchingObjectsI18n(std::string objPrefix, unsigned int maxNbItem) const
{
	std::vector<std::string> result;
	if (maxNbItem==0)
		return result;

	transform(objPrefix.begin(), objPrefix.end(), objPrefix.begin(), ::toupper);
	for (auto &v : systemBodies) {
		std::string constw = v.second.body->getNameI18n().substr(0, objPrefix.size());
		transform(constw.begin(), constw.end(), constw.begin(), ::toupper);
		if (constw==objPrefix) {
			result.push_back(v.second.body->getNameI18n());
			if (result.size()==maxNbItem)
				return result;
		}
	}
	return result;
}

bool ProtoSystem::getPlanetHidden(const std::string &name)
{
	try {
		return !renderedBodies.count(systemBodies.at(name).body.get());
	} catch (...) {
		return false;
	}
}

std::string ProtoSystem::getPlanetsPosition()
{
	std::string msg;
	Vec3d tmp;

	for (auto &value : systemBodies) {
		if (value.second.body->isSatellite())
			continue;
		tmp = value.second.body->get_heliocentric_ecliptic_pos();
		tmp[0]= round(tmp[0]*100);
		tmp[1]= round(tmp[1]*100);
		tmp[2]= round(tmp[2]*100);
		msg= msg+value.second.body->getNameI18n()+":"+std::to_string(tmp[0])+":"+std::to_string(tmp[1])+":"+std::to_string(tmp[2])+";";
	}
	return msg;
}

void ProtoSystem::bodyTraceBodyChange(const std::string &bodyName)
{
	try {
		bodyTrace = systemBodies.at(bodyName).body;
	} catch (...) {
		cLog::get()->write("Unknown planet_name in bodyTraceBodyChange", LOG_TYPE::L_ERROR);
	}
}

constexpr uint32_t casify(const char *data) {
	return ((uint32_t) data[0]) | (((uint32_t) data[1]) << 8) | (((uint32_t) data[2]) << 16) | (((uint32_t) data[3]) << 24);
}

#define CASE(name, type) case casify(name): return type

BODY_TYPE ProtoSystem::setPlanetType (const std::string &str)
{
	if (str.size() < 3)
		return UNKNOWN;
	switch (*(const uint32_t *) str.data()) {
		CASE("Sun", SUN);
		CASE("Star", STAR);
		CASE("Planet", PLANET);
		CASE("Moon", MOON);
		CASE("Dwarf", DWARF);
		CASE("Asteroid", ASTEROID);
		CASE("KBO", KBO);
		CASE("Comet", COMET);
		CASE("Artificial", ARTIFICIAL);
		CASE("Observer", OBSERVER);
		CASE("Center", CENTER);
		default:
			return UNKNOWN;
	}
}

#undef CASE

// Init and load one solar system object
// This is a the private method
void ProtoSystem::addBody(stringHash_t param, bool deletable)
{
	// Avoid string copy and map search
	const std::string &englishName = param["name"];
	const std::string &str_parent = param["parent"];
	const std::string &funcname = param["coord_func"];
	// p is a pointer used for the object that will be finally integrated in the list of stars that body_mgr manages
	std::shared_ptr<Body> p, parent;

	cLog::get()->write("Loading new Stellar System object... " + englishName, LOG_TYPE::L_INFO);
	// set the Body type: ie what it is in universe
	BODY_TYPE typePlanet = setPlanetType(param["type"]);

	// do not add if no name or no parent or no typePlanet
	if (englishName.empty()) {
		cLog::get()->write("SolarSystem: can not add body with no name", LOG_TYPE::L_WARNING);
		return;
	}

	// no parent ? so it's the center body
	if (str_parent.empty()) {
		parent = centerObject;
		cLog::get()->write("No parent specified for " + englishName + ", assume parent is the center body (Specify 'none' for no parent)", LOG_TYPE::L_WARNING);
	} else if (str_parent != "none") {
		parent = searchByEnglishName(str_parent);
		if (parent == nullptr) {
			cLog::get()->write("SolarSystem: can't find parent for " + englishName, LOG_TYPE::L_WARNING);
			return;
		}
	}

	// no type ? it's an asteroid
	if (typePlanet == UNKNOWN) {
		typePlanet = ASTEROID;
		cLog::get()->write("No valid body type specified for " + englishName + ", assume 'Asteroid'", LOG_TYPE::L_WARNING);
	}

	// Do not add if body already exists - name must be unique
	if (systemBodies.find(englishName) != systemBodies.end()) {
		cLog::get()->write("SolarSystem: Can not add body named " + englishName + " because a body of that name already exist", LOG_TYPE::L_WARNING);
		return;
	}

	//
	// determination of the orbit of the star
	//
	std::unique_ptr<Orbit> orb;
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

	auto bodyColor = std::make_unique<BodyColor>(param["color"], param["label_color"], param["orbit_color"], param["trail_color"]);

	float solLocalDay= Utility::strToDouble(param["sol_local_day"],1.0);

	// Create the Body and add it to the list
	BodyTexture bodyTexture {
		.tex_map = param["tex_map"],
		.tex_map_alternative = {},
		.tex_norm = param["tex_normal"],
		.tex_night = param["tex_night"],
		.tex_specular = param["tex_specular"],
		.tex_heightmap = param["tex_heightmap"],
		.tex_skin =  param["tex_skin"]
	};

	ObjL *currentOBJ;
	{
		const std::string &modelName = param["model_name"];
		if (modelName.empty()) {
			currentOBJ = objLMgr->selectDefault();
		} else {
			objLMgr->insertObj(modelName);
			currentOBJ = objLMgr->select(modelName);
		}
	}

	switch (typePlanet) {
		case CENTER: {
			auto p_center = std::make_shared<Center>(parent,
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
			p = std::move(p_center);
		}
		break;
		case SUN : {
			auto p_sun = std::make_shared<Sun>(parent,
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
			p = std::move(p_sun);
		}
		break;
		case STAR :  {
			auto p_sun = std::make_shared<BodyStar>(parent,
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
			p = std::move(p_sun);
		}
		break;
		case ARTIFICIAL:
			p = std::make_shared<Artificial>(std::move(parent),
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
			break;
		case MOON:
			p = std::make_shared<Moon>(std::move(parent),
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
			break;
		case DWARF:
		case PLANET: {
			std::shared_ptr<BigBody> p_big = std::make_shared<BigBody>(std::move(parent),
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
				p_big->setRings(std::make_unique<Ring>(r_min,r_max,param["tex_ring"],ringsInit));
			}
			p = std::move(p_big);
		}
		break;
		case ASTEROID:
		case KBO:
		case COMET:
			p = std::make_shared<SmallBody>(std::move(parent),
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
									bodyTexture);
			if (!param["apparent_magnitude"].empty() && !param["slope"].empty()) {
				auto &b = static_cast<SmallBody&>(*p);
				b.setAbsoluteMagnitudeAndSlope(Utility::strToFloat(param["apparent_magnitude"]), Utility::strToFloat(param["slope"]));
				b.bindTail({
					Utility::strToFloat(param["gaz_tail_trace_jd"], 1),
					Utility::strToFloat(param["gaz_tail_ejection_force"], 30),
					Vec3f{
						Utility::strToFloat(param["gaz_tail_radius_xx_coef"], -1),
						Utility::strToFloat(param["gaz_tail_radius_x_coef"], 0.5),
						Utility::strToFloat(param["gaz_tail_radius_base_coef"], 2),
					},
					Vec3f{
						Utility::strToFloat(param["gaz_tail_color_red"], 0.3),
						Utility::strToFloat(param["gaz_tail_color_green"], 0.3),
						Utility::strToFloat(param["gaz_tail_color_blue"], 0.7),
					}
				});
				b.bindTail({
					Utility::strToFloat(param["dust_tail_trace_jd"], 30),
					Utility::strToFloat(param["dust_tail_ejection_force"], 0.5),
					Vec3f{
						Utility::strToFloat(param["dust_tail_radius_xx_coef"], -2),
						Utility::strToFloat(param["dust_tail_radius_x_coef"], 1),
						Utility::strToFloat(param["dust_tail_radius_base_coef"], 2),
					},
					Vec3f{
						Utility::strToFloat(param["dust_tail_color_red"], 0.5),
						Utility::strToFloat(param["dust_tail_color_green"], 0.5),
						Utility::strToFloat(param["dust_tail_color_blue"], 0.5),
					}
				});
				if (!param["extra_tail_trace_jd"].empty()) {
					b.bindTail({
						Utility::strToFloat(param["extra_tail_trace_jd"], 30),
						Utility::strToFloat(param["extra_tail_ejection_force"], 0.5),
						Vec3f{
							Utility::strToFloat(param["extra_tail_radius_xx_coef"], -2),
							Utility::strToFloat(param["extra_tail_radius_x_coef"], 1),
							Utility::strToFloat(param["extra_tail_radius_base_coef"], 2),
						},
						Vec3f{
							Utility::strToFloat(param["extra_tail_color_red"], 0.5),
							Utility::strToFloat(param["extra_tail_color_green"], 0.5),
							Utility::strToFloat(param["extra_tail_color_blue"], 0.5),
						}
					});
				}
			}
			break;
		default:
			cLog::get()->write("Undefined body", LOG_TYPE::L_ERROR);
	}
	if (p == nullptr) {
		cLog::get()->write("Failed to create body", LOG_TYPE::L_ERROR);
		return;
	}

	if (!param["has_atmosphere"].empty() || !param["atmosphere_lim_landscape"].empty()) {
		AtmosphereParams* tmp = nullptr;
		tmp = new(AtmosphereParams);
		tmp->hasAtmosphere = Utility::strToBool(param["has_atmosphere"], false);
		tmp->modelAtmosphere = setAtmosphere(param["atmosphere_model"]);
		tmp->tableAtmosphere = param["atmosphere_ext_model"];
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

	bool isHidden = Utility::strToBool(param["hidden"], 0);
	if (!isHidden)
		showBody(p.get());

	systemBodies.insert(std::pair<std::string, BodyContainer>(englishName, {
		.body = std::move(p),
		.isDeleteable = deletable,
		.initialHidden = isHidden,
	}));
}

void ProtoSystem::initialSolarSystemBodies()
{
	for (auto &v : systemBodies) {
		v.second.body->reinitParam();
		if (v.second.initialHidden) {
			hideBody(v.second.body.get());
		} else
			showBody(v.second.body.get());
	}
}

void ProtoSystem::preloadBody(stringHash_t & param)
{
	if (!param[W_PURGE].empty()) {
		if (param[W_PURGE] == W_ALL) {
			// Aggressive purge, release as many memory as possible, even used ones if possible
			s_texture::releaseAllMemory();
		}
		if (param[W_PURGE] == W_AUTO)
			s_texture::releaseUnusedMemory();
	}
	if (auto body = searchByEnglishName(param[W_NAME]))
		body->preload(std::stoi(param[W_KEEPTIME]));
}

void ProtoSystem::selectSystem()
{
	anchorManager->selectAnchor();
	currentCenterPos = centerPos;
}

void ProtoSystem::computeDraw(const Projector *prj, const Navigator *nav)
{
	for (Body *body : renderedBodies)
		body->computeDraw(prj, nav);
	if (sortedRenderedBodies.size() < 2)
		return; // Nothing to sort

	// Use dual bubble sort algorithm - average complexity of O(N) as bodies stay mostly sorted between frames
	// This come with a higher complexity at the frame newly showing multiple bodies (due to addBody or setPlanetHidden)
	Body ** const begin = sortedRenderedBodies.data() - 1;
	Body ** const end = begin + sortedRenderedBodies.size();
	Body **swapPos;
	Body **pos = sortedRenderedBodies.data();
	Body *tmp;
	do {
		if (pos[0]->getDistance() < pos[1]->getDistance()) {
			swapPos = pos;
			tmp = pos[1];
			do { // Backward loop, bring up the body
				pos[1] = pos[0];
			} while (--pos != begin && pos[0]->getDistance() < pos[1]->getDistance());
			pos[1] = tmp;
			pos = swapPos;
		}
	} while (++pos < end);
}
