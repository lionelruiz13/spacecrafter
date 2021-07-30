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

#ifndef _SOLARSYSTEM_H_
#define _SOLARSYSTEM_H_

#include <vector>
#include <functional>
#include <map>

#include "bodyModule/body_tesselation.hpp"
#include "bodyModule/body_common.hpp"
#include "bodyModule/body_sun.hpp"
#include "bodyModule/body_moon.hpp"
#include "bodyModule/body_bigbody.hpp"
#include "bodyModule/body_smallbody.hpp"
#include "bodyModule/body_artificial.hpp"
#include "tools/object.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/translator.hpp"
#include "navModule/observer.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "navModule/anchor_manager.hpp"
#include "tools/no_copy.hpp"
#include "bodyModule/body_color.hpp"
#include "tools/ScModule.hpp"

class ThreadContext;

class OrbitCreator;
class SSystemIterator;

class SolarSystem: public NoCopy , public ModuleFont{
    friend class SSystemIterator;
public:
	SolarSystem(ThreadContext *_context, ObjLMgr *_objLMgr);
	virtual ~SolarSystem();

	void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr);

	//! Draw all the elements of the solar system
	void draw(Projector * prj, const Navigator * nav, const Observer* observatory,
	          const ToneReproductor* eye,
	          bool drawHomePlanet );

	//! Load the bodies data from a file
	void load(const std::string& planetfile);

	// load one object from a hash (returns error message if any)
	// this public method always adds bodies as deletable
	void addBody(stringHash_t & param) {
		addBody(param, true);
	}

	virtual void setFont(float font_size, const std::string& font_name) override;

	//removes a body that has no satelites
	bool removeBodyNoSatellite(const std::string &name);

	//removes a body and its satellites
	bool removeBody(const std::string &name);

	//removes all bodies that do not come from ssystem.ini
	bool removeSupplementalBodies(const std::string &name);

	//! @brief Update i18 names from english names according to passed translator
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	std::string getPlanetHashString();  // locale and ssystem.ini names, newline delimiter, for tui

	void toggleHideSatellites(bool val);

	//! Compute the position for every elements of the solar system.
	//! home_planet is needed for light travel time computation
	void computePositions(double date,const Observer *obs);

	//! Compute the transformation matrix for every elements of the solar system.
	//! home_planet is needed for light travel time computation
	void computeTransMatrices(double date,const Observer * obs);

	//! Search if any Planet is close to position given in earth equatorial position.
	Object search(Vec3d, const Navigator * nav, const Projector * prj) const;

	//! Return a stl vector containing the planets located inside the lim_fov circle around position v
	std::vector<Object> searchAround(Vec3d v,
	                                  double lim_fov,
	                                  const Navigator * nav,
	                                  const Observer* observatory,
	                                  const Projector * prj,
	                                  bool *default_last_item,
	                                  bool aboveHomePlanet ) const;

	//! Return the matching planet pointer if exists or nullptr
	Body* searchByEnglishName(const std::string &planetEnglishName) const;

	//! Return the matching planet pointer if exists or nullptr
	//! @param planetNameI18n The case sensistive translated planet name
	Object searchByNamesI18(const std::string &planetNameI18n) const;


	//! get the position Alt Az for Sun
	void bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const;

	void bodyTraceBodyChange(const std::string &bodyName);

	//!return Earth planet object
	Body* getEarth(void) const {
		return earth;
	}

	//! return Sun planet object
	Sun* getSun(void) const {
		return sun;
	}

	//! return Moon planet object
	Moon* getMoon(void) const {
		return moon;
	}

	// get flag for Activate/Deactivate Body composant
	bool getFlag(BODY_FLAG name);

	//! set flag for Activate/Deactivate planets axis
	void setFlagAxis(bool b);

	//! set flag for Activate/Deactivate planets display
	void setFlagPlanets(bool b) {
		flagShow = b;
	}

	//! get flag for Activate/Deactivate planets display
	bool getFlagShow(void) const {
		return flagShow;
	}

	void setFlagLightTravelTime(bool b) {
		flag_light_travel_time = b;
	}

	bool getFlagLightTravelTime(void) const {
		return flag_light_travel_time;
	}

	//! Start/stop accumulating new trail data (clear old data)
	void startTrails(bool b);

	//! Set if Moon display is scaled
	void setFlagMoonScale(bool b) {
		if (!b) getMoon()->setSphereScale(1);
		else getMoon()->setSphereScale(moonScale);
		flagMoonScale = b;
	}

	//! Get if Moon display is scaled
	bool getFlagMoonScale(void) const {
		return flagMoonScale;
	}

	//! Set if Sun display is scaled
	void setFlagSunScale(bool b) {
		if (!b) {
			getSun()->setSphereScale(1);
			getSun()->setHaloSize(200);
		} else {
			getSun()->setSphereScale(SunScale);
			getSun()->setHaloSize(200+SunScale*40);
		}

		flagSunScale = b;
	}

	//! Get if Sun display is scaled
	bool getFlagSunScale(void) const {
		return flagSunScale;
	}

	//! Set Moon display scaling factor
	void setMoonScale(float f, bool resident = false) {
		moonScale = f;
		if (flagMoonScale)
			getMoon()->setSphereScale(moonScale, resident);
	}

	//! Get Moon display scaling factor
	float getMoonScale(void) const {
		return moonScale;
	}

	//! Set Sun display scaling factor
	void setSunScale(float f, bool resident = false) {
		SunScale = f;
		if (flagSunScale) getSun()->setSphereScale(SunScale, resident);
	}

	//! Get Sun display scaling factor
	float getSunScale(void) const {
		return SunScale;
	}
	//! Set flag for displaying clouds (planet rendering feature)
	void setFlagClouds(bool b) {
		Body::setFlagClouds(b);
	}

	bool getHideSatellitesFlag(){
		return flagHideSatellites;
	}

	//! Get list of all the translated planets name
	std::vector<std::string> getNamesI18(void);

	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const;

	//modify Planet "name" is hidden or not.
	void setPlanetHidden(const std::string &name, bool planethidden);

	//get the state of the planet
	bool getPlanetHidden(const std::string &name);


	//reinitialise l'ensemble des planetes comme elles étaient au chargement initial du logiciel
	// réinitialise les paramètes de la tesselaiton
	// prend en compte la taille et le flag caché ou pas
	void initialSolarSystemBodies();

	void modelRingInit(int low, int medium, int high) {
		ringsInit=Vec3i(low, medium, high);
	}

	std::string getPlanetsPosition();
	
	void setAnchorManager(AnchorManager * _anchorManager){
		anchorManager = _anchorManager;
	}

	const OrbitCreator * getOrbitCreator()const{
		return orbitCreator;
	}

	struct BodyContainer {
		std::unique_ptr<Body> body=nullptr;
		std::string englishName;  // for convenience
		bool isDeleteable = false;
		bool isHidden = false;
		bool initialHidden = false;
	};

	void computePreDraw(const Projector * prj, const Navigator * nav);

	// return the Sun altitude
	double getSunAltitude(const Navigator * nav) const;

	// return the Sun azimuth 
	double getSunAzimuth(const Navigator * nav) const;

	std::map<std::string, std::shared_ptr<BodyContainer>>::iterator begin() {return systemBodies.begin();};
    std::map<std::string, std::shared_ptr<BodyContainer>>::iterator end() {return systemBodies.end();};	

	std::unique_ptr<SSystemIterator> createIterator();

private:
	struct depthBucket {
		double znear;
		double zfar;
	};

	Body* findBody(const std::string &name);
	std::shared_ptr<SolarSystem::BodyContainer> findBodyContainer(const std::string &name);

	// determine the planet type: Sun, planet, moon, dwarf, asteroid ...
	BODY_TYPE setPlanetType (const std::string &str);

	// load one object from a hash
	void addBody(stringHash_t & param, bool deletable);

	ThreadContext *context;

	Sun* sun=nullptr; //return the Sun
	Moon* moon=nullptr;	//return the Moon
	BigBody* earth=nullptr;	//return the earth
	Body* bodyTrace=nullptr; //retourne le body qui est sélectionné par bodyTrace

	// solar system related settings
	float object_scale;  // should be kept synchronized with star scale...

	bool flagMoonScale;	// say if float moonScale is used or not
	bool flagSunScale;	// say if float SunScale is used or not
	float moonScale;	// Moon scale value
	float SunScale;	// Sun scale value

	Vec3i ringsInit;

	std::map< std::string, std::shared_ptr<BodyContainer>> systemBodies; //Map containing the bodies and related information. the key is their english name
	std::vector<std::shared_ptr<BodyContainer>> renderedBodies; //Contains bodies that are not hidden
	std::list<depthBucket>listBuckets;

	bool nearLunarEclipse(const Navigator * nav, Projector * prj);

	// Master settings
	bool flag_light_travel_time;
	bool flagAxis= false;
	bool flagShow= true;
	bool flagHideSatellites = false;

	ObjLMgr* objLMgr=nullptr;					// représente  les objets légers du ss

	AnchorManager * anchorManager = nullptr;

	OrbitCreator * orbitCreator = nullptr;

};

#endif // _SOLARSYSTEM_H_
