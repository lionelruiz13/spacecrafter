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

#ifndef _PROTOSYSTEM_
#define _PROTOSYSTEM_

#include "ojmModule/objl_mgr.hpp"
#include "bodyModule/body_tesselation.hpp"
#include "bodyModule/body_common.hpp"
#include "bodyModule/body_sun.hpp"
#include "bodyModule/body_center.hpp"
#include "bodyModule/body_moon.hpp"
#include "bodyModule/body_bigbody.hpp"
#include "bodyModule/body_smallbody.hpp"
#include "bodyModule/body_artificial.hpp"
#include "navModule/observer.hpp"
#include "navModule/anchor_manager.hpp"
#include "tools/translator.hpp"
#include "tools/object.hpp"
#include "tools/ScModule.hpp"

class ThreadContext;
class OrbitCreator;
class SSystemIterator;
class SSystemIteratorVector;

class ProtoSystem: public NoCopy, public ModuleFont {
friend class SSystemIterator;
friend class SSystemIteratorVector;
public:

    ProtoSystem(ThreadContext *_context, ObjLMgr *_objLMgr, Observer *observatory, Navigator *navigation, TimeMgr *timeMgr);
    ~ProtoSystem();

	void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr);
	
	//! Load the bodies data from a file
	void load(const std::string& planetfile);

	// load one object from a hash (returns error message if any)
	// this public method always adds bodies as deletable
	virtual void addBody(stringHash_t & param){
		addBody(param, true);
	}

	//! Return the matching planet pointer if exists or nullptr
	Body* searchByEnglishName(const std::string &planetEnglishName) const;

	//removes a body and its satellites
	bool removeBody(const std::string &name);
	
	virtual bool removeBodyNoSatellite(const std::string &name){return true;}
	
	//removes all bodies that do not come from ssystem.ini
	bool removeSupplementalBodies(const std::string &name);

	//! @brief Update i18 names from english names according to passed translator
	//! The translation is done using gettext with translated strings defined in translations.h
	void translateNames(Translator& trans);

	std::string getPlanetHashString();  // locale and ssystem.ini names, newline delimiter, for tui

	void toggleHideSatellites(bool val);

	//modify Planet "name" is hidden or not.
	void setPlanetHidden(const std::string &name, bool planethidden);

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
	//! @param planetNameI18n The case sensistive translated planet name
	Object searchByNamesI18(const std::string &planetNameI18n) const;

	// get flag for Activate/Deactivate Body composant
	bool getFlag(BODY_FLAG name);

	//! set flag for Activate/Deactivate planets axis
	void setFlagAxis(bool b);
	
	//! Start/stop accumulating new trail data (clear old data)
	void startTrails(bool b);
	
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

	//get the state of the planet
	bool getPlanetHidden(const std::string &name);

	void modelRingInit(int low, int medium, int high) {
		ringsInit=Vec3i(low, medium, high);
	}
	
	std::string getPlanetsPosition();
	
	const OrbitCreator * getOrbitCreator()const{
		return orbitCreator;
	}
		
	void setAnchorManager(AnchorManager * _anchorManager){
		anchorManager = _anchorManager;
	}

	AnchorManager *getAnchorManager() {
		return anchorManager;
	}

	Object * getCenterObject() {
		return centerObject;
	}
	
	//reinitialise l'ensemble des planetes comme elles étaient au chargement initial du logiciel
	// réinitialise les paramètes de la tesselaiton
	// prend en compte la taille et le flag caché ou pas
	void initialSolarSystemBodies();

	struct BodyContainer {
		std::unique_ptr<Body> body=nullptr;
		std::string englishName;  // for convenience
		bool isDeleteable = false;
		bool isHidden = false;
		bool initialHidden = false;
	};

	void bodyTraceBodyChange(const std::string &bodyName);

	std::vector<std::shared_ptr<BodyContainer>>::iterator begin() {return renderedBodies.begin();};
    std::vector<std::shared_ptr<BodyContainer>>::iterator end() {return renderedBodies.end();};	

	std::unique_ptr<SSystemIterator> createIterator();
	std::unique_ptr<SSystemIteratorVector> createIteratorVector();

protected:

	ThreadContext *context;
	ObjLMgr* objLMgr=nullptr;					// représente  les objets légers du ss
	Body* bodyTrace=nullptr; //retourne le body qui est sélectionné par bodyTrace
	OrbitCreator * orbitCreator = nullptr;
	AnchorManager * anchorManager = nullptr;
	Vec3i ringsInit;
	Object * centerObject = nullptr;

	// Master settings
	bool flagAxis= false;
	bool flagHideSatellites = false;
	
	// load one object from a hash
	virtual void addBody(stringHash_t & param, bool deletable);	

	std::shared_ptr<ProtoSystem::BodyContainer> findBodyContainer(const std::string &name);
	Body* findBody(const std::string &name);
	
	// determine the planet type: Sun, planet, moon, dwarf, asteroid ...
	BODY_TYPE setPlanetType (const std::string &str); 
	
	std::map< std::string, std::shared_ptr<BodyContainer>> systemBodies; //Map containing the bodies and related information. the key is their english name
	std::vector<std::shared_ptr<BodyContainer>> renderedBodies; //Contains bodies that are not hidden
};

#endif