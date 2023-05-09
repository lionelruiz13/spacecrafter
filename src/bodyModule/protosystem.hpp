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



#include "tools/no_copy.hpp"
#include "tools/ScModule.hpp"
#include "bodyModule/body.hpp"
#include <set>

class OrbitCreator;
class SSystemIterator;
class SSystemIteratorVector;
class ObjLMgr;
class AnchorManager;
class Object;
class Observer;
class TimeMgr;
class Body;
class Translator;

class ProtoSystem: public NoCopy, public ModuleFont {
    friend class SSystemIterator;
    friend class SSystemIteratorVector;
public:
    ProtoSystem(ObjLMgr *_objLMgr, Observer *observatory, Navigator *navigation, TimeMgr *timeMgr, const Vec3d &centerPos = {});
    ~ProtoSystem();

    struct BodyContainer {
		std::shared_ptr<Body> body;
		bool isDeleteable = true;
		bool initialHidden = false;
	};

	void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr);

	//! Load the bodies data from a file
	void load(const std::string& planetfile);

	//! Load the bodies data from an object
	void load(Object &obj);

	// load one object from a hash (returns error message if any)
	// this public method always adds bodies as deletable
	inline void addBody(stringHash_t &param) {
		addBody(param, true);
	}

    virtual void preloadBody(stringHash_t & param);

	//! Return the matching planet pointer if exists or nullptr
	inline std::shared_ptr<Body> searchByEnglishName(const std::string &planetEnglishName) const {
        try {
            return systemBodies.at(planetEnglishName).body;
        } catch (...) {
            return nullptr;
        }
    }

	//removes a body and its satellites
	bool removeBody(const std::string &name);

	void removeBodyNoSatellite(std::map<std::string, BodyContainer>::iterator it);

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
	std::vector<std::string> listMatchingObjectsI18n(std::string objPrefix, unsigned int maxNbItem) const;

	//get the state of the planet
	bool getPlanetHidden(const std::string &name);

	void modelRingInit(int low, int medium, int high) {
		ringsInit=Vec3i(low, medium, high);
	}

	std::string getPlanetsPosition();

	std::shared_ptr<OrbitCreator> getOrbitCreator()const{
		return orbitCreator;
	}

	void setAnchorManager(std::shared_ptr<AnchorManager> _anchorManager){
		anchorManager = _anchorManager;
	}

	std::shared_ptr<AnchorManager> getAnchorManager() {
		return anchorManager;
	}

	std::shared_ptr<Body> getCenterObject() const {
		return centerObject;
	}

	//resets all planets as they were when the software was first loaded
	// reset the parameters of the tesselaiton
	// takes into account the size and the flag hidden or not
	void initialSolarSystemBodies();

	void bodyTraceBodyChange(const std::string &bodyName);

    //! Iterate over all bodies
	inline std::map<std::string, BodyContainer>::const_iterator begin() const {return systemBodies.begin();}
    //! Iterate over all bodies
    inline std::map<std::string, BodyContainer>::const_iterator end() const {return systemBodies.end();}

    //! Iterate over visible bodies, from the farthest to the closest
    inline std::vector<Body *>::const_iterator beginSorted() const {return sortedRenderedBodies.begin();}
    //! Iterate over visible bodies, from the farthest to the closest
    inline std::vector<Body *>::const_iterator endSorted() const {return sortedRenderedBodies.end();}

    void selectSystem();

    static Vec3d getCenterPos() {return currentCenterPos;}

    // Call computeDraw on all visible bodies and sort them
    void computeDraw(const Projector *prj, const Navigator *nav);

    inline Body *getCenterOfInterest() const {
        return mainBody;
    }
protected:
    inline void hideBody(Body *body) {
        if (renderedBodies.erase(body))
            sortedRenderedBodies.erase(std::find(sortedRenderedBodies.begin(), sortedRenderedBodies.end(), body));
    }
    inline void showBody(Body *body) {
        if (renderedBodies.insert(body).second)
            sortedRenderedBodies.push_back(body);
    }
    static Vec3d currentCenterPos;
    Body *mainBody = nullptr; // This is the dominant body on screen, higher quality is expected for this body.
	ObjLMgr *objLMgr=nullptr;					// represents the light objects of the ss
	std::shared_ptr<Body> bodyTrace; //returns the body that is selected by bodyTrace
	std::shared_ptr<OrbitCreator> orbitCreator;
	std::shared_ptr<AnchorManager> anchorManager;
    std::shared_ptr<Body> centerObject;
    Vec3d centerPos;
	Vec3i ringsInit;
    static bool initGuard;

	// Master settings
	bool flagAxis= false;
	bool flagHideSatellites = false;

	// load one object from a hash
	virtual void addBody(stringHash_t param, bool deletable);
    void showBodyRecursive(Body *body);
    void hideBodyRecursive(Body *body);

	// determine the planet type: Sun, planet, moon, dwarf, asteroid ...
	BODY_TYPE setPlanetType (const std::string &str);

	std::map<std::string, BodyContainer> systemBodies; //Map containing the bodies and related information. the key is their english name
	std::set<Body *> renderedBodies; //Contains bodies that are not hidden
    std::vector<Body *> sortedRenderedBodies;
};

#endif
