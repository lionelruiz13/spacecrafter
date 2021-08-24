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

#ifndef _SSYSTEMFACTORY_H_
#define _SSYSTEMFACTORY_H_

#include "tools/no_copy.hpp"
#include "bodyModule/solarsystem.hpp"

class SSystemFactory: public NoCopy {
public:
    SSystemFactory();
    ~SSystemFactory();

    SolarSystem * getSolarSystem(void) {
        return ssystem;
    }

    void registerFont(s_font* _font) {
        ssystem->registerFont(_font);
    }

	//! Set selected planet by english name or "" to select none
	void setSelected(const std::string& englishName) {
		ssystem->setSelected((englishName));
	}

    //! Set selected object from its pointer
	void setSelected(const Object &obj) {
        ssystem->setSelected(obj);
    }

    //! Get base planets display limit in pixels
	float getSizeLimit(void) const {
		return ssystem->getSizeLimit();
	}

	void bodyTraceBodyChange(const std::string &bodyName){
        ssystem->bodyTraceBodyChange(bodyName);
    }

	std::string getPlanetsPosition() {
        return ssystem->getPlanetsPosition();
    }

    Body* getEarth(void) const {
		return ssystem->getEarth();
	}

	Moon* getMoon(void) const {
        return ssystem->getMoon();
    }

    void setFlagLightTravelTime(bool b) {
		ssystem->setFlagLightTravelTime(b);
	}

    bool getFlagLightTravelTime(void) const {
        return ssystem->getFlagLightTravelTime();
    }

    void startTrails(bool b) {
        ssystem->startTrails(b);
    }

    void setFlagMoonScale(bool b) {
        ssystem->setFlagMoonScale(b);
    }

    bool getFlagMoonScale(void) const {
        return ssystem->getFlagMoonScale();
    }

	void setFlagSunScale(bool b) {
        ssystem->setFlagSunScale(b);
    }

	bool getFlagSunScale(void) const {
        return ssystem->getFlagSunScale();
    }

	void setMoonScale(float f, bool resident = false) {
        ssystem->setMoonScale(f, resident);
    }

	float getMoonScale(void) const {
        return ssystem->getMoonScale();
    }

	void setSunScale(float f, bool resident = false) {
        ssystem->setSunScale(f, resident);
    }

	float getSunScale(void) const {
        return ssystem->getSunScale();
    }

	void setFlagClouds(bool b) {
        ssystem->setFlagClouds(b);
    }

    bool getFlag(BODY_FLAG name) {
        return ssystem->getFlag(name);
    }

	void initialSolarSystemBodies() {
        ssystem->initialSolarSystemBodies();
    }

	void setPlanetHidden(const std::string &name, bool planethidden) {
        ssystem->setPlanetHidden(name, planethidden);
    }

	bool getPlanetHidden(const std::string &name) {
        return ssystem->getPlanetHidden(name);
    }

	void setFlagPlanets(bool b) {
        ssystem->setFlagPlanets(b);
    }

	bool getFlagShow(void) const {
        return ssystem->getFlagShow();
    }

	void setFlagTrails(bool b) {
        ssystem->setFlagTrails(b);
    }

	void setFlagAxis(bool b) {
        ssystem->setFlagAxis(b);
    }

	void setFlagHints(bool b) {
        ssystem->setFlagHints(b);
    }

	void setFlagPlanetsOrbits(bool b) {
        ssystem->setFlagPlanetsOrbits(b);
    }

	void setFlagPlanetsOrbits(const std::string &_name, bool b) {
        ssystem->setFlagPlanetsOrbits(_name, b);
    }

	void switchPlanetTexMap(const std::string &name, bool a) {
        ssystem->switchPlanetTexMap(name, a);
    }

	bool getSwitchPlanetTexMap(const std::string &name) {
        return ssystem->getSwitchPlanetTexMap(name);
    }

	void createTexSkin(const std::string &name, const std::string &texName) {
        ssystem->createTexSkin(name, texName);
    }

	bool getFlagPlanetsOrbits(void) const {
        return ssystem->getFlagPlanetsOrbits();
    }

	void setFlagSatellitesOrbits(bool b) {
        ssystem->setFlagSatellitesOrbits(b);
    }

	bool getFlagSatellitesOrbits(void) const {
        return ssystem->getFlagSatellitesOrbits();
    }

	void setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c) {
        ssystem->setBodyColor(englishName, colorName, c);
    }

	const Vec3f getBodyColor(const std::string &englishName, const std::string& colorName) const {
        return ssystem->getBodyColor(englishName, colorName);
    }

	void setDefaultBodyColor(const std::string& colorName, const Vec3f& c) {
        ssystem->setDefaultBodyColor(colorName, c);
    }

	const Vec3f getDefaultBodyColor(const std::string& colorName) const {
        return ssystem->getDefaultBodyColor(colorName);
    }

	bool getHideSatellitesFlag() {
        return ssystem->getHideSatellitesFlag();
    }

	void toggleHideSatellites(bool val) {
        ssystem->toggleHideSatellites(val);
    }

	void setScale(float scale) {
        ssystem->setScale(scale);
    }

	double getSunAltitude(const Navigator * nav) const {
        return ssystem->getSunAltitude(nav);
    }

	double getSunAzimuth(const Navigator * nav) const {
        return ssystem->getSunAzimuth(nav);
    }

	void setPlanetSizeScale(const std::string &name, float s) {
        ssystem->setPlanetSizeScale(name, s);
    }

	void planetTesselation(std::string name, int value) {
        ssystem->planetTesselation(name, value);
    }
    
	const OrbitCreator * getOrbitCreator()const {
        return ssystem->getOrbitCreator();   
    }

	void iniColor(const std::string& _halo, const std::string& _label, const std::string& _orbit, const std::string& _trail) {
        ssystem->iniColor(_halo, _label, _orbit, _trail);
    }

	void iniTess(int minTes, int maxTes, int planetTes, int moonTes, int earthTes) {
        ssystem->iniTess(minTes, maxTes, planetTes, moonTes, earthTes);
    }

	void modelRingInit(int low, int medium, int high) {
        ssystem->modelRingInit(low, medium, high);
    }

	void iniTextures() {
        ssystem->iniTextures();
    }

	void load(const std::string& planetfile) {
        ssystem->load(planetfile);
    }

	void computePositions(double date,const Observer *obs) {
        ssystem->computePositions(date, obs);
    }

	void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr) {
        ssystem->update(delta_time, nav, timeMgr);
    }

	void bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const {
        ssystem->bodyTraceGetAltAz(nav, alt, az);
    }

	void computePreDraw(const Projector * prj, const Navigator * nav) {
        ssystem->computePreDraw(prj, nav);
    }

	void draw(Projector * prj, const Navigator * nav, const Observer* observatory,
	          const ToneReproductor* eye,
	          bool drawHomePlanet ) {
        ssystem->draw(prj, nav, observatory, eye, drawHomePlanet);
    }

	void addBody(stringHash_t & param) {
        ssystem->addBody(param);
    }

	bool removeBody(const std::string &name) {
        return ssystem->removeBody(name);
    }

	bool removeSupplementalBodies(const std::string &name) {
        return ssystem->removeSupplementalBodies(name);
    }

	Object searchByNamesI18(const std::string &planetNameI18n) const {
        return ssystem->searchByNamesI18(planetNameI18n);
    }

	Object getSelected(void) const {
        return ssystem->getSelected();
    }

    std::vector<Object> searchAround(Vec3d v,
	                                  double lim_fov,
	                                  const Navigator * nav,
	                                  const Observer* observatory,
	                                  const Projector * prj,
	                                  bool *default_last_item,
	                                  bool aboveHomePlanet ) const {
                                          return ssystem->searchAround(v, lim_fov, nav, observatory, prj, 
                                          default_last_item, aboveHomePlanet);
                                      }

	void translateNames(Translator& trans) {
        ssystem->translateNames(trans);
    }

	void setDefaultBodyColor(const std::string& halo, const std::string& label, const std::string& orbit, const std::string& trail) {
        ssystem->setDefaultBodyColor(halo, label, orbit, trail);
    }

	std::string getPlanetHashString() {
        return ssystem->getPlanetHashString();
    }

	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const {
        return ssystem->listMatchingObjectsI18n(objPrefix, maxNbItem);
    }

	void setSizeLimit(float scale) {
        ssystem->setSizeLimit(scale);
    }

private:
    SolarSystem* ssystem;				// Manage the solar system
};

#endif