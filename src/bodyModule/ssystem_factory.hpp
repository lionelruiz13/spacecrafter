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
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ModularBodyPtr.hpp"
#include "experimentalModule/bodyModules/HintModule.hpp"
#include "experimentalModule/bodyModules/AxisModule.hpp"

class Camera;
class ModularSystem;
class EnvironmentManager;
class MilkyWay;
class Atmosphere;
class Landscape;
class ToneReproductor;
struct EnvironmentState;       // experimentalModule/EnvironmentModule.hpp
struct AtmosphereComputeInput; // atmosphereModule/atmosphere.hpp

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

class SSystemFactory: public NoCopy {
public:
    SSystemFactory(Observer *observatory, Navigator *navigation, TimeMgr *timeMgr);
    ~SSystemFactory();

    void loadCamera(const InitParser &conf);

    // Dual-path trace harness (experimentalModule/INTENT.md 11.14): dump the
    // same-frame transform state of BOTH paths (old per-body state + new
    // per-body state matched by english name + camera header) as JSON lines.
    // Script-callable via 'body action dual_dump filename ...'.
    // Precondition: freeze time (timerate rate 0) so the 1s A/B draw toggle
    // cannot make one path's draw-side state stale relative to the other.
    void dumpTracePaths(const std::string &file);
    //! Script-settable rendered-path selection (flag experimental_path):
    //! pins old (false) / new (true) path and stops the A/B auto-toggle.
    void setExperimentalPath(bool newPath) {
        drawModularSystem = newPath;
        pathPinned = true;
    }
    bool getExperimentalPath() const {
        return drawModularSystem;
    }
    // Dual-path seam: re-reference the new-path Camera onto the body the old
    // path's anchor just switched to (warp semantics - the observer keeps its
    // lat/lon/alt meaning on the new body, like the old anchor switch). No-op
    // when the body doesn't exist in the new path.
    void syncCameraReference(const std::string &name);

    // ---- Environment seams (S8 wave, 2026-07-16 - EnvironmentManager.hpp
    // is the aggregation authority; EnvironmentModule.hpp the contract) ----
    // Engine wiring: called once by Core after it creates the shared engines
    // (the factory is constructed before them). Instantiates the manager and
    // the galaxy root's milkyway member.
    void wireEnvironment(MilkyWay *milky, Atmosphere *atmosphere, ToneReproductor *eye);
    // Landscape engine re-seat - called by Core at every landscape swap
    // (setLandscape/loadLandscape; I5).
    void setEnvironmentLandscape(Landscape *landscape);
    // Atmosphere user-flag mirror (old BodyDecor::setAtmosphereState sites).
    void setEnvironmentAtmosphereFlag(bool b);
    // Aggregated per-frame state (valid after update()) - the BodyDecor
    // gate replacement for modular-phase consumers (searchAround, meteors).
    const EnvironmentState &getEnvironmentState() const;
    // New-path input snapshot for Atmosphere::computeColor (built in
    // update(), consumed by the executor's async job - ordered by the work
    // queue push/pop).
    const AtmosphereComputeInput &getEnvironmentAtmosphereInput() const;
    // Frame-positioned environment draws (modular phase; same call
    // positions as the old milky_way->draw / atmosphere->draw+landscape
    // block in the executors - frame-sequencing carrier during migration).
    void drawEnvironmentBackdrop();
    void drawEnvironmentSky();

    SolarSystem * getSolarSystem(void) {
        return ssystem.get();
    }

    void registerFont(s_font* _font) {
        ssystem->registerFont(_font);
        // Dual-path seam (INTENT §9 fonts row): the new path's label font.
        // Same font object - the s_font render cache is shared, so a string
        // rendered by either path is a cache hit for the other.
        HintModule::setFont(_font);
    }

    void setSelectedObject(Object &obj) {
        selected_object = obj;
	}

	//! Set selected planet by english name or "" to select none
	void setSelected(const std::string& englishName) {
        ssystemSelected->setSelected((englishName));
        // Dual-path seam (INTENT §9 selection row): mirror into the new
        // path's selection state (ModularBody::isSelected + the selected-body
        // aggregate consumed by the pointer service). Tolerates bodies the
        // new path doesn't carry (findBodyOnce: nullptr on miss).
        newSelectedBody = englishName.empty() ? nullptr
                        : ModularBody::findBodyOnce(englishName);
	}

    //! Set selected object from its pointer
	void setSelected(const Object &obj) {
        ssystemSelected->setSelected(obj);
        // Old rule mirrored: only bodies carry a body selection; selecting
        // any other object type clears it (SolarSystemSelected::setSelected).
        newSelectedBody = (obj.getType() == OBJECT_BODY)
                        ? ModularBody::findBodyOnce(obj.getEnglishName())
                        : nullptr;
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
		ModularBody::flagLightTravelTime = b; // dual-path: both paths must model the same light
	}

    bool getFlagLightTravelTime(void) const {
        return ssystemDisplay->getFlagLightTravelTime();
    }

    void startTrails(bool b) {
        currentSystem->startTrails(b);
    }

    // Moon/Sun scale: dual-path (I2, one seam) - the new path carries it as
    // per-body scaling (ModularBody::setScaling -> scaledRadius), which feeds
    // both the drawn size and the observer's altitude reference. Without the
    // mirror, an observer on the scaled body sits at radius instead of
    // scale*radius (measured: 5x |eye->Moon| divergence, scene C 2026-07-11).
    void setFlagMoonScale(bool b) {
        ssystem->setFlagMoonScale(b);
        if (ModularBody *moon = ModularBody::findBody("Moon"))
            moon->setScaling(b ? ssystem->getMoonScale() : 1.f);
    }

    bool getFlagMoonScale(void) const {
        return ssystem->getFlagMoonScale();
    }

	void setFlagSunScale(bool b) {
        ssystem->setFlagSunScale(b);
        if (ModularBody *sun = ModularBody::findBody("Sun"))
            sun->setScaling(b ? ssystem->getSunScale() : 1.f);
    }

	bool getFlagSunScale(void) const {
        return ssystem->getFlagSunScale();
    }

	void setMoonScale(float f, bool resident = false) {
        ssystem->setMoonScale(f, resident);
        if (ssystem->getFlagMoonScale()) {
            if (ModularBody *moon = ModularBody::findBody("Moon"))
                moon->setScaling(f);
        }
    }

	float getMoonScale(void) const {
        return ssystem->getMoonScale();
    }

	void setSunScale(float f, bool resident = false) {
        ssystem->setSunScale(f, resident);
        if (ssystem->getFlagSunScale()) {
            if (ModularBody *sun = ModularBody::findBody("Sun"))
                sun->setScaling(f);
        }
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
        // ssystemTex->resetTesselationParams();
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
        AxisModule::show = b; // both-paths seam, like setFlagHints
    }

	void setFlagHints(bool b) {
        ssystemSelected->setFlagHints(b);
        HintModule::show = b; // both-paths seam, like setFlagLightTravelTime
    }

    void setFlagIsolateSelected(bool b) {ssystemSelected->setFlagIsolateSelected(b);}

    bool getFlagIsolateSelected() {return ssystemSelected->getFlagIsolateSelected();}

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
        ModularBody::haloScale = scale; // both-paths seam (old Body::object_scale)
        ssystemScale->setScale(scale);
    }

    double getSunAltitude(const Navigator * nav) const {
        return ssystem->getSunAltitude(nav);
    }

	double getSunAzimuth(const Navigator * nav) const {
        return ssystem->getSunAzimuth(nav);
    }

    double getSelectedAZ(const Navigator * nav) const {
        double alt, az=0;
        ssystemSelected->getSelected().getAltAz(nav, &alt, &az);
    	return az*180./M_PI;
    }

	double getSelectedALT(const Navigator * nav) const {
        double alt=0, az;
    	ssystemSelected->getSelected().getAltAz(nav, &alt, &az);
    	return alt*180./M_PI;
    }

    double getSelectedStarRA(const Navigator *nav) const {
        double ra=0, de;
        selected_object.getRaDeValue(nav, &ra, &de);
        return (ra < 0) ? ra + 360 : ra;//*180.0/M_PI;
    }
    double getSelectedStarDE(const Navigator *nav) const {
        double ra, de=0;
        selected_object.getRaDeValue(nav, &ra, &de);
        return de;
    }

    double getSelectedRA(const Navigator * nav) const {
        double ra=0, de;
        ssystemSelected->getSelected().getRaDeValue(nav, &ra, &de);
    	return ra*180.0/M_PI;
    }

	double getSelectedDE(const Navigator * nav) const {
        double ra, de=0;
    	ssystemSelected->getSelected().getRaDeValue(nav, &ra, &de);
    	return de*180.0/M_PI;
    }

	void setPlanetSizeScale(const std::string &name, float s) {
        ssystemScale->setPlanetSizeScale(name, s);
    }

	void planetTesselation(std::string name, int value) {
        ssystemTex->planetTesselation(name, value);
    }

	const std::shared_ptr<OrbitCreator> getOrbitCreator()const {
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
        currentSystem->selectSystem();
    }

    /**
     * @brief Reload body colors from ssystem.ini file.
     * This function reads the color definitions (label_color, orbit_color, trail_color)
     * from the specified ssystem.ini file and updates the colors of
     * every body in the ssystem.ini file.
     * @param planetfile Path to the ssystem.ini file
    */
    void reloadColors(const std::string& planetfile);

	void computePositions(double date,const Observer *obs) {
        ssystemDisplay->computePositions(date, obs);
    }

	void update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr);

	void bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const {
        ssystem->bodyTraceGetAltAz(nav, alt, az);
    }

	void computePreDraw(const Projector * prj, const Navigator * nav) {
        ssystemDisplay->computePreDraw(prj, nav);
    }

	void draw(Projector * prj, const Navigator * nav, const Observer* observatory,
	          const ToneReproductor* eye,
	          bool drawHomePlanet );

	void addBody(stringHash_t &param);

    void preloadBody(stringHash_t & param) {
        currentSystem->preloadBody(param);
    }

	bool removeBody(const std::string &name) {
        if (auto body = ModularBody::findBodyOnce(name))
            body->remove(true);
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
        ModularBody::setTranslator(trans);
    }

	void setDefaultBodyColor(const std::string& halo, const std::string& label, const std::string& orbit, const std::string& trail) {
        ssystemColor->setDefaultBodyColor(halo, label, orbit, trail);
        HintModule::defaultLabelColor = Utility::strToVec3f(label); // both-paths seam
    }

	std::string getPlanetHashString() {
        return currentSystem->getPlanetHashString();
    }

	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem) const {
        return currentSystem->listMatchingObjectsI18n(objPrefix, maxNbItem);
    }

	void setSizeLimit(float scale) {
        ModularBody::haloSizeLimit = scale; // both-paths seam (old Body::object_size_limit)
        ssystemScale->setSizeLimit(scale);
    }

    //! Return the matching planet pointer if exists or nullptr
	std::shared_ptr<Body> searchByEnglishName(const std::string &planetEnglishName) const {
        return currentSystem->searchByEnglishName(planetEnglishName);
    }

    void setAnchorManager(std::shared_ptr<AnchorManager> _anchorManager) {
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
		bool ret = currentSystem->getAnchorManager()->switchToAnchor(name);
		if (ret)
			syncCameraReference(name); // dual-path: observer body change must reach the new Camera
		return ret;
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
		currentSystem->getAnchorManager()->load("anchor.ini");
		currentSystem->getAnchorManager()->initFirstAnchor(conf.getStr(SCS_INIT_LOCATION, SCK_HOME_PLANET));
    }

    void updateAnchorManager() {
        currentSystem->getAnchorManager()->update();
    }

    bool switchToAnchor(const std::string& anchorName) {
        bool ret = currentSystem->getAnchorManager()->switchToAnchor(anchorName);
        if (ret)
            syncCameraReference(anchorName); // dual-path: observer body change must reach the new Camera
        return ret;
    }

    bool switchToAnchor(const Object &selection) {
        currentSystem->getAnchorManager()->switchToAnchor(selection);
        syncCameraReference(selection.getEnglishName());
        return true;
    }

    bool cameraSave(const std::string& name) {
	    return currentSystem->getAnchorManager()->saveCameraPosition("anchors/" + name);
    }

    bool loadCameraPosition(const std::string& filename) {
	    return currentSystem->getAnchorManager()->loadCameraPosition("anchors/" + filename);
    }

    void changeSystem(const std::string &mode);

    void addSystem(const std::string &name, const std::string &file);

    void loadGalacticSystem(const std::string &path, const std::string &file);
    void loadSystem(const std::string &path, stringHash_t &params);
    std::unique_ptr<ProtoSystem> &createSystem(const std::string &mode);
    void createModularSystem(const std::string &name, const std::string &filename, const Vec3d &pos);

    //! Enter a system (leave the galactic system)
    void enterSystem();

    //! Leave a system (enter in the galactic system)
    void leaveSystem();

    //! Return the selected anchor name
    std::string querySelectedAnchorName();

    double getSunBrightness() {
        return (ssystem->getHaloSize());
    }

    void setDefaultSunBrightness(){
        ssystem->setDefaultHaloSize();
    }

    void setDefaultSunBrightness(double f){
        ssystem->setDefaultHaloSize(f);
        ssystem->setHaloSize(f);
    }

    void setSunBrightness(double f){
        ssystem->setHaloSize(f);
    }

    //! For debugging, should the modular system been drawn ?
    bool drawModularSystem = false;
    // Path pinned by script (flag experimental_path): the 1s A/B auto-toggle
    // stops once a script chose a path - deterministic captures [vixy:
    // 2026-07-12, replaces phase-guessing on screenshots].
    bool pathPinned = false;
private:
    //! Select current system
    void selectSystem();
    std::unique_ptr<SolarSystem> ssystem;				// Manage the solar system
    std::unique_ptr<SolarSystemColor> ssystemColor;
    std::unique_ptr<SolarSystemTex> ssystemTex;
    std::unique_ptr<SolarSystemScale> ssystemScale;
    std::unique_ptr<SolarSystemSelected> ssystemSelected;
    std::unique_ptr<SolarSystemDisplay> ssystemDisplay;

    ModularSystem *milkyway; // Never destroyed, there is no parent to delegate remnant ModularBodyPtr to
    std::unique_ptr<ProtoSystem> galacticSystem;
    std::shared_ptr<AnchorManager> galacticAnchorMgr;

    // At least for now, systems are stored here
    std::list<ModularSystem> modularSystems;
    std::map<std::string, std::unique_ptr<ProtoSystem>> systems;
    std::map<std::string, Vec3d> systemOffsets;

	std::unique_ptr<ObjLMgr> objLMgr=nullptr;					// represents the light objects of the ss

	std::shared_ptr<BodyTrace> bodytrace;				// the pen bodytrace
    Observer *observatory;
    Navigator *navigation;
    TimeMgr *timeMgr;
    std::unique_ptr<Camera> camera;
    // Environment aggregation authority (created by wireEnvironment - the
    // shared engines don't exist yet at factory construction).
    std::unique_ptr<EnvironmentManager> environment;

    ProtoSystem * currentSystem;
    bool inSystem = true;
    Object selected_object;
    // New-path body selection (dual-path seam, INTENT §9 selection row):
    // the ModularBodySelector maintains ModularBody::isSelected and the
    // getSelected() aggregate; redirect-safe across body replacement (I5).
    ModularBodySelector newSelectedBody;
};

#endif
