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

void SolarSystem::bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const
{
	bodyTrace->getAltAz(nav,alt,az);
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