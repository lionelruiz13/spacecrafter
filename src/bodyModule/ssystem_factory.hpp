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

#include <memory>

#include "tools/no_copy.hpp"
#include "bodyModule/solarsystem.hpp"
#include "bodyModule/solarsystem_color.hpp"
#include "tools/ScModule.hpp"
#include "tools/app_settings.hpp"
#include "bodyModule/solarsystem_tex.hpp"
#include "bodyModule/solarsystem_scale.hpp"
#include "bodyModule/solarsystem_selected.hpp"
#include "bodyModule/solarsystem_display.hpp"
#include "bodyModule/body_trace.hpp"
#include "mainModule/define_key.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "navModule/anchor_manager.hpp"

/**
 * \file ssystem_factory.hpp
 * \brief Handle solar system functions
 * \author Jérémy Calvo
 * \version 1
 *
 * \class SSystemFactory
 *
 * \brief Wrapper for all Solar Systems functions
 * 
 * Allows to add systems and to select the active one
 *
*/

class ThreadContext;

class SSystemFactory: public NoCopy {
public:
    SSystemFactory(ThreadContext *_context, Observer *observatory, Navigator *navigation, TimeMgr *timeMgr);
    ~SSystemFactory();

    SolarSystem * getSolarSystem(void) {
        return ssystem.get();
    }

    void registerFont(s_font* _font) {
        ssystem->registerFont(_font);
    }

	//! Set selected planet by english name or "" to select none
	void setSelected(const std::string& englishName) {
		ssystemSelected->setSelected((englishName));
	}

    //! Set selected object from its pointer
	void setSelected(const Object &obj) {
        ssystemSelected->setSelected(obj);
    }

    //! Get base planets display limit in pixels
	float getSizeLimit(void) const {
		return ssystemScale->getSizeLimit();
	}

	void bodyTraceBodyChange(const std::string &bodyName){
        currentSystem->bodyTraceBodyChange(bodyName);
    }

	std::string getPlanetsPosition() {
        return currentSystem->getPlanetsPosition();
    }

    std::shared_ptr<Body> getEarth(void) const {
		return ssystem->getEarth();
	}

	std::shared_ptr<Moon> getMoon(void) const {
        return ssystem->getMoon();
    }

    void setFlagLightTravelTime(bool b) {
		ssystemDisplay->setFlagLightTravelTime(b);
	}

    bool getFlagLightTravelTime(void) const {
        return ssystemDisplay->getFlagLightTravelTime();
    }

    void startTrails(bool b) {
        currentSystem->startTrails(b);
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
        currentSystem->setFlagClouds(b);
    }

    bool getFlag(BODY_FLAG name) {
        if (name == BODY_FLAG::F_AXIS || name == BODY_FLAG::F_CLOUDS)
            return currentSystem->getFlag(name);
        else
            return ssystemSelected->getFlag(name);
    }

	void initialSolarSystemBodies() {
        currentSystem->initialSolarSystemBodies();
        ssystemTex->resetTesselationParams();
    }

	void setPlanetHidden(const std::string &name, bool planethidden) {
        currentSystem->setPlanetHidden(name, planethidden);
    }

	bool getPlanetHidden(const std::string &name) {
        return currentSystem->getPlanetHidden(name);
    }

	void setFlagPlanets(bool b) {
        ssystemDisplay->setFlagPlanets(b);
    }

	bool getFlagShow(void) const {
        return ssystemDisplay->getFlagShow();
    }

	void setFlagTrails(bool b) {
        ssystemSelected->setFlagTrails(b);
    }

	void setFlagAxis(bool b) {
        currentSystem->setFlagAxis(b);
    }

	void setFlagHints(bool b) {
        ssystemSelected->setFlagHints(b);
    }

	void setFlagPlanetsOrbits(bool b) {
        ssystemSelected->setFlagPlanetsOrbits(b);
    }

	void setFlagPlanetsOrbits(const std::string &_name, bool b) {
        ssystemSelected->setFlagPlanetsOrbits(_name, b);
    }

	void switchPlanetTexMap(const std::string &name, bool a) {
        ssystemTex->switchPlanetTexMap(name, a);
    }

	bool getSwitchPlanetTexMap(const std::string &name) {
        return ssystemTex->getSwitchPlanetTexMap(name);
    }

	void createTexSkin(const std::string &name, const std::string &texName) {
        ssystemTex->createTexSkin(name, texName);
    }

	bool getFlagPlanetsOrbits(void) const {
        return ssystemSelected->getFlagPlanetsOrbits();
    }

	void setFlagSatellitesOrbits(bool b) {
        ssystemSelected->setFlagSatellitesOrbits(b);
    }

	bool getFlagSatellitesOrbits(void) const {
        return ssystemSelected->getFlagSatellitesOrbits();
    }

	void setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c) {
        ssystemColor->setBodyColor(englishName, colorName, c);
    }

	const Vec3f getBodyColor(const std::string &englishName, const std::string& colorName) const {
        return ssystemColor->getBodyColor(englishName, colorName);
    }

	void setDefaultBodyColor(const std::string& colorName, const Vec3f& c) {
        ssystemColor->setDefaultBodyColor(colorName, c);
    }

	const Vec3f getDefaultBodyColor(const std::string& colorName) const {
        return ssystemColor->getDefaultBodyColor(colorName);
    }

	bool getHideSatellitesFlag() {
        return currentSystem->getHideSatellitesFlag();
    }

	void toggleHideSatellites(bool val) {
        currentSystem->toggleHideSatellites(val);
    }

	void setScale(float scale) {
        ssystemScale->setScale(scale);
    }

	double getSunAltitude(const Navigator * nav) const {
        return ssystem->getSunAltitude(nav);
    }

	double getSunAzimuth(const Navigator * nav) const {
        return ssystem->getSunAzimuth(nav);
    }

	void setPlanetSizeScale(const std::string &name, float s) {
        ssystemScale->setPlanetSizeScale(name, s);
    }

	void planetTesselation(std::string name, int value) {
        ssystemTex->planetTesselation(name, value);
    }
    
	const OrbitCreator * getOrbitCreator()const {
        return currentSystem->getOrbitCreator();   
    }

	void iniColor(const std::string& _halo, const std::string& _label, const std::string& _orbit, const std::string& _trail) {
        ssystemColor->iniColor(_halo, _label, _orbit, _trail);
    }

	void iniTess(int minTes, int maxTes, int planetTes, int moonTes, int earthTes) {
        ssystemTex->iniTess(minTes, maxTes, planetTes, moonTes, earthTes);
    }

	void modelRingInit(int low, int medium, int high) {
        currentSystem->modelRingInit(low, medium, high);
    }

	void iniTextures() {
        ssystemTex->iniTextures();
    }

	void load(const std::string& planetfile) {
        currentSystem->load(planetfile);
    }

	void loadStellar(const std::string& planetfile) {
        stellarSystem->load(planetfile);
    }

	void computePositions(double date,const Observer *obs) {
        ssystemDisplay->computePositions(date, obs);
    }

	void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr) {
        ssystemTex->updateTesselation(delta_time);
        currentSystem->update(delta_time, nav, timeMgr);
	    bodytrace->update(delta_time);
    }

	void bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const {
        ssystem->bodyTraceGetAltAz(nav, alt, az);
    }

	void computePreDraw(const Projector * prj, const Navigator * nav) {
        ssystemDisplay->computePreDraw(prj, nav);
    }

	void draw(Projector * prj, const Navigator * nav, const Observer* observatory,
	          const ToneReproductor* eye,
	          bool drawHomePlanet ) {
    	bodytrace->draw(prj, nav);
        ssystemDisplay->draw(prj, nav, observatory, eye, drawHomePlanet);
    }

	void addBody(stringHash_t & param) {
        currentSystem->addBody(param);
    }

	bool removeBody(const std::string &name) {
        return currentSystem->removeBody(name);
    }

	bool removeSupplementalBodies(const std::string &name) {
        return currentSystem->removeSupplementalBodies(name);
    }

	Object searchByNamesI18(const std::string &planetNameI18n) const {
        return currentSystem->searchByNamesI18(planetNameI18n);
    }

	Object getSelected(void) const {
        return ssystemSelected->getSelected();
    }

    std::vector<Object> searchAround(Vec3d v,
	                                  double lim_fov,
	                                  const Navigator * nav,
	                                  const Observer* observatory,
	                                  const Projector * prj,
	                                  bool *default_last_item,
	                                  bool aboveHomePlanet ) const {
                                          return currentSystem->searchAround(v, lim_fov, nav, observatory, prj, 
                                          default_last_item, aboveHomePlanet);
                                      }

	void translateNames(Translator& trans) {
        currentSystem->translateNames(trans);
    }

	void setDefaultBodyColor(const std::string& halo, const std::string& label, const std::string& orbit, const std::string& trail) {
        ssystemColor->setDefaultBodyColor(halo, label, orbit, trail);
    }

	std::string getPlanetHashString() {
        return currentSystem->getPlanetHashString();
    }

	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const {
        return currentSystem->listMatchingObjectsI18n(objPrefix, maxNbItem);
    }

	void setSizeLimit(float scale) {
        ssystemScale->setSizeLimit(scale);
    }

    //! Return the matching planet pointer if exists or nullptr
	std::shared_ptr<Body> searchByEnglishName(const std::string &planetEnglishName) const {
        return currentSystem->searchByEnglishName(planetEnglishName);
    }

    void setAnchorManager(AnchorManager * _anchorManager) {
        currentSystem->setAnchorManager(_anchorManager);
    }

    void bodyTrace(Navigator * navigation) {
        double alt, az;
        bodyTraceGetAltAz(navigation, &alt, &az);
	    bodytrace->addData(navigation, alt, az);
    }

    void bodyTraceSetFlag(bool b) const {
        bodytrace->setFlagShow(b);
    }

    bool bodyTraceGetFlag() const {
        return bodytrace->getFlagShow();
    }

    void upPen() const {
        bodytrace->upPen();
    }

    void downPen() const {
        bodytrace->downPen();
    }

    void togglePen() const {
        bodytrace->togglePen();
    }

    void clear() const {
        bodytrace->clear();
    }

    void hide(int numberlist) const {
        bodytrace->hide(numberlist);
    }

    void cameraDisplayAnchor() {
        currentSystem->getAnchorManager()->displayAnchor();
    }

    bool cameraAddAnchor(stringHash_t& param) {
        return currentSystem->getAnchorManager()->addAnchor(param);
    }

    bool cameraRemoveAnchor(const std::string &name) {
		return currentSystem->getAnchorManager()->removeAnchor(name);
	}

    bool cameraSwitchToAnchor(const std::string &name) {
		return currentSystem->getAnchorManager()->switchToAnchor(name);
	}

    bool cameraMoveToPoint(double x, double y, double z){
		return currentSystem->getAnchorManager()->setCurrentAnchorPos(Vec3d(x,y,z));
	}

	bool cameraMoveToPoint(double x, double y, double z, double time){
		return currentSystem->getAnchorManager()->moveTo(Vec3d(x,y,z),time);
	}

    bool cameraMoveToBody(const std::string& bodyName, double time, double alt) {
        return currentSystem->getAnchorManager()->moveToBody(bodyName, time, alt);
    }

    bool cameraMoveRelativeXYZ( double x, double y, double z) {
		return currentSystem->getAnchorManager()->moveRelativeXYZ(x,y,z);
	}

    bool cameraTransitionToPoint(const std::string& name){
		return currentSystem->getAnchorManager()->transitionToPoint(name);
	}

    bool cameraTransitionToBody(const std::string& name) {
        return currentSystem->getAnchorManager()->transitionToBody(name);
    }

    bool cameraSetFollowRotation(bool value){
		return currentSystem->getAnchorManager()->setFollowRotation(value);
	}

    void cameraSetRotationMultiplierCondition(float v) {
		currentSystem->getAnchorManager()->setRotationMultiplierCondition(v);
	}

    bool cameraAlignWithBody(const std::string& name, double duration){
		return currentSystem->getAnchorManager()->alignCameraToBody(name,duration);
	}

    void anchorManagerInit(const InitParser &conf) {
        currentSystem->getAnchorManager()->setRotationMultiplierCondition(conf.getDouble(SCS_NAVIGATION, SCK_STALL_RADIUS_UNIT));
		currentSystem->getAnchorManager()->load(AppSettings::Instance()->getUserDir() + "anchor.ini");
		currentSystem->getAnchorManager()->initFirstAnchor(conf.getStr(SCS_INIT_LOCATION, SCK_HOME_PLANET));
    }

    void updateAnchorManager() {
	    currentSystem->getAnchorManager()->update();
    }

    bool switchToAnchor(const std::string& anchorName) {
        return currentSystem->getAnchorManager()->switchToAnchor(anchorName);
    }

    bool cameraSave(const std::string& name) {
	    return currentSystem->getAnchorManager()->saveCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + name);
    }

    bool loadCameraPosition(const std::string& filename) {
	    return currentSystem->getAnchorManager()->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
    }

    void changeSystem(const std::string mode);

    void addSystem(const std::string name, const std::string file);

private:
    std::unique_ptr<SolarSystem> ssystem;				// Manage the solar system
    std::unique_ptr<SolarSystemColor> ssystemColor;
    std::unique_ptr<SolarSystemTex> ssystemTex;
    std::unique_ptr<SolarSystemScale> ssystemScale;
    std::unique_ptr<SolarSystemSelected> ssystemSelected;
    std::unique_ptr<SolarSystemDisplay> ssystemDisplay;
    std::unique_ptr<ProtoSystem> stellarSystem;

    std::map<std::string, std::unique_ptr<ProtoSystem>> systems;

	std::unique_ptr<ObjLMgr> objLMgr=nullptr;					// représente  les objets légers du ss

	BodyTrace * bodytrace;				// the pen bodytrace
    ThreadContext *context;
    Observer *observatory;
    Navigator *navigation;
    TimeMgr *timeMgr;
    
    ProtoSystem * currentSystem;
};

#endif