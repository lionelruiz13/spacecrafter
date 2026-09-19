/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jeremy Calvo
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
#include <functional>

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
#include "experimentalModule/CameraAnchors.hpp" // new-path named anchors (B4)
#include "experimentalModule/bodyModules/StarModule.hpp" // sun-scale halo seam (S11.44)
#include "experimentalModule/bodyModules/HintModule.hpp"
#include "experimentalModule/bodyModules/AxisModule.hpp"
#include "experimentalModule/bodyModules/RingModule.hpp"
#include "experimentalModule/bodyModules/OrbitModule.hpp"
#include "experimentalModule/bodyModules/TrailModule.hpp"
#include "experimentalModule/bodyModules/PlanetGridModule.hpp"

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
 * \author Jeremy Calvo
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

    void dumpTracePaths(const std::string &file,
                        const std::function<void(std::ostream &)> &extraHeader = {});
    enum class RenderPathMode { NEW, OLD, ALTERNATE };
    void setRenderPathMode(RenderPathMode mode) {
        drawModularSystem = (mode != RenderPathMode::OLD);
        pathPinned = (mode != RenderPathMode::ALTERNATE);
    }
    //! Script-settable rendered-path selection (flag experimental_path):
    //! pins old (false) / new (true) path and stops the A/B auto-toggle.
    void setExperimentalPath(bool newPath) {
        drawModularSystem = newPath;
        pathPinned = true;
    }
    bool getExperimentalPath() const {
        return drawModularSystem;
    }
    void syncCameraReference(const std::string &name);

    void wireEnvironment(MilkyWay *milky, Atmosphere *atmosphere, ToneReproductor *eye);
    // Landscape engine re-seat - called by Core at every landscape swap
    // (setLandscape/loadLandscape; I5).
    void setEnvironmentLandscape(Landscape *landscape);
    // Atmosphere user-flag mirror (old BodyDecor::setAtmosphereState sites).
    void setEnvironmentAtmosphereFlag(bool b);
    // Aggregated per-frame state (valid after update()) - the BodyDecor
    // gate replacement for modular-phase consumers (searchAround, meteors).
    const EnvironmentState &getEnvironmentState() const;
    const AtmosphereComputeInput &getEnvironmentAtmosphereInput() const;
    void drawEnvironmentBackdrop();
    void drawEnvironmentSky();

    SolarSystem * getSolarSystem(void) {
        return ssystem.get();
    }

    void registerFont(s_font* _font) {
        ssystem->registerFont(_font);
        HintModule::setFont(_font);
    }

    void setSelectedObject(Object &obj) {
        selected_object = obj;
	}

	//! Set selected planet by english name or "" to select none
	void setSelected(const std::string& englishName) {
        ssystemSelected->setSelected((englishName));
        newSelectedBody = englishName.empty() ? nullptr
                        : ModularBody::findBodyOnce(englishName);
	}

	void setSelected(const Object &obj);

    Object searchObjectByEnglishName(const std::string &englishName) const;

    Object searchNewOnlyObjectAt(int x, int y) const;

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

	// [merge] script-variable helpers (theirs' feature): distance/magnitude of the selected object.
	double getSelectedDistance(const Navigator * nav) const {
		Vec3d pos = ssystemSelected->getSelected().getObsJ2000Pos(nav);
		return pos.length();
	}
	double getSelectedMagnitude(const Navigator * nav) const {
		return ssystemSelected->getSelected().getMag(nav);
	}

    void setFlagLightTravelTime(bool b) {
		ssystemDisplay->setFlagLightTravelTime(b);
		ModularBody::flagLightTravelTime = b; // dual-path: both paths must model the same light
	}

    bool getFlagLightTravelTime(void) const {
        return ssystemDisplay->getFlagLightTravelTime();
    }

    void startTrails(bool b);

    void setFlagMoonScale(bool b) {
        ssystem->setFlagMoonScale(b);
        commandDisplayScale("Moon", b ? ssystem->getMoonScale() : 1.f);
    }

    bool getFlagMoonScale(void) const {
        return ssystem->getFlagMoonScale();
    }

	void setFlagSunScale(bool b) {
        ssystem->setFlagSunScale(b);
        commandDisplayScale("Sun", b ? ssystem->getSunScale() : 1.f);
        mirrorSunHaloSize();
    }

	bool getFlagSunScale(void) const {
        return ssystem->getFlagSunScale();
    }

	void setMoonScale(float f, bool resident = false) {
        ssystem->setMoonScale(f, resident);
        if (ssystem->getFlagMoonScale())
            commandDisplayScale("Moon", f);
    }

	float getMoonScale(void) const {
        return ssystem->getMoonScale();
    }

	void setSunScale(float f, bool resident = false) {
        ssystem->setSunScale(f, resident);
        if (ssystem->getFlagSunScale())
            commandDisplayScale("Sun", f);
    }

	float getSunScale(void) const {
        return ssystem->getSunScale();
    }

    void initDisplayScaling(bool flagMoonScale, double moonScale,
                            bool flagSunScale, double sunScale);

    void generatePendingTwins();

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
        if (ModularBody *body = ModularBody::findBody(name)) {
            if (planethidden)
                body->hide();
            else
                body->show();
        }
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
        TrailModule::setGlobalShow(b);
        ModularBody *selected = ModularBody::getSelected();
        if (b && selected && !selected->isStar()) {
            ModularBody::forEach([&](ModularBody &body) {
                if (!(&body == selected || body.getParent() == selected))
                    body.setFlagTrail(false);
            });
        }
    }

	void setFlagAxis(bool b) {
        currentSystem->setFlagAxis(b);
        AxisModule::show = b; // both-paths seam, like setFlagHints
        PlanetGridModule::show = b;
    }

    void setPlanetGridColor(const Vec3f &meridian, const Vec3f &parallel) {
        PlanetGridModule::setColors(meridian, parallel);
    }

    void setPlanetGridTropicPolar(bool showTropics, bool showPolarCircles,
                                  const Vec3f &tropic, const Vec3f &polarCircle) {
        PlanetGridModule::setTropicPolar(showTropics, showPolarCircles, tropic, polarCircle);
    }

	void setFlagHints(bool b) {
        ssystemSelected->setFlagHints(b);
        HintModule::show = b; // both-paths seam, like setFlagLightTravelTime
    }

    void setFlagIsolateSelected(bool b) {ssystemSelected->setFlagIsolateSelected(b);}

    bool getFlagIsolateSelected() {return ssystemSelected->getFlagIsolateSelected();}

	void setFlagPlanetsOrbits(bool b) {
        ssystemSelected->setFlagPlanetsOrbits(b);
        OrbitModule::setGlobalPlanets(b); // both-paths seam (bumps the override generation)
    }

	void setFlagPlanetsOrbits(const std::string &_name, bool b) {
        ssystemSelected->setFlagPlanetsOrbits(_name, b);
        // New-path per-name (findBody is static; exception-on-miss, tolerate it
        // like the old searchByEnglishName null return).
        try {
            if (ModularBody *body = ModularBody::findBody(_name))
                body->setFlagOrbit(b);
        } catch (...) {}
    }

	void switchPlanetTexMap(const std::string &name, bool a) {
        ssystemTex->switchPlanetTexMap(name, a);
        try {
            if (ModularBody *body = ModularBody::findBody(name))
                body->switchTexSkin(a);
        } catch (...) {}
    }

	bool getSwitchPlanetTexMap(const std::string &name) {
        return ssystemTex->getSwitchPlanetTexMap(name);
    }

	void createTexSkin(const std::string &name, const std::string &texName) {
        ssystemTex->createTexSkin(name, texName);
        // New-path mirror (S6 Textures seam): contract parity with old
        // Body::createTexSkin - creating/replacing never activates.
        try {
            if (ModularBody *body = ModularBody::findBody(name))
                body->createTexSkin(texName);
        } catch (...) {}
    }

	bool getFlagPlanetsOrbits(void) const {
        return ssystemSelected->getFlagPlanetsOrbits();
    }

	void setFlagSatellitesOrbits(bool b) {
        ssystemSelected->setFlagSatellitesOrbits(b);
        OrbitModule::setGlobalSatellites(b); // both-paths seam
    }

	bool getFlagSatellitesOrbits(void) const {
        return ssystemSelected->getFlagSatellitesOrbits();
    }

	void setBodyColor(const std::string &englishName, const std::string& colorName, const Vec3f& c) {
        ssystemColor->setBodyColor(englishName, colorName, c);
        const BodyColorType t = parseBodyColorType(colorName);
        if (t != BodyColorType::NONE) {
            if (englishName == "all")
                ModularBody::forEach([&](ModularBody &b){ b.setColor(t, c); });
            else if (ModularBody *body = ModularBody::findBody(englishName))
                body->setColor(t, c);
        }
    }

    bool setBodyDatumRadius(const std::string &englishName, double km);
    bool setBodyGroundRadius(const std::string &englishName, double km);

	const Vec3f getBodyColor(const std::string &englishName, const std::string& colorName) const {
        return ssystemColor->getBodyColor(englishName, colorName);
    }

	void setDefaultBodyColor(const std::string& colorName, const Vec3f& c) {
        ssystemColor->setDefaultBodyColor(colorName, c);
        const BodyColorType t = parseBodyColorType(colorName);
        if (t == BodyColorType::LABEL || t == BodyColorType::ALL)
            HintModule::defaultLabelColor = c;
        if (t == BodyColorType::ORBIT || t == BodyColorType::ALL)
            OrbitModule::defaultColor = c;
        if (t == BodyColorType::TRAIL || t == BodyColorType::ALL)
            TrailModule::defaultColor = c;
        if (t == BodyColorType::HALO || t == BodyColorType::ALL)
            ModularBody::setDefaultHaloColor(c);
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
        if (ModularBody *body = ModularBody::findBody(name))
            body->setScaling(s);
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
        // Both-paths seam (setFlagHints idiom): the new-path RingModule LOD
        // slice counts mirror the same config values the old Ring gets.
        RingModule::setLodSlices(low, medium, high);
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

	void updateExperimental(int delta_time, const TimeMgr* timeMgr);

	void bodyTraceGetAltAz(const Navigator *nav, double *alt, double *az) const {
        ssystem->bodyTraceGetAltAz(nav, alt, az);
    }

	void computePreDraw(const Projector * prj, const Navigator * nav) {
        ssystemDisplay->computePreDraw(prj, nav);
    }

	void draw(Projector * prj, const Navigator * nav, const Observer* observatory,
	          const ToneReproductor* eye,
	          bool drawHomePlanet );

	void drawExperimental();

	void createExperimentalOort(unsigned int nbr, const Vec3f &color);
	inline bool hasExperimentalOort() const { return experimentalOortInstantiated; }

	void addBody(stringHash_t &param);

    bool reloadCurrentSystem();

    bool saveCurrentSystem(const std::string &filename);

    void preloadBody(stringHash_t &param);

	bool removeBody(const std::string &name) {
        if (auto body = ModularBody::findBodyOnce(name))
            body->remove(true);
        return currentSystem->removeBody(name);
    }

    bool removeSupplementalBodies(const std::string &name);

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
        OrbitModule::defaultColor = Utility::strToVec3f(orbit);     // both-paths seam
        TrailModule::defaultColor = Utility::strToVec3f(trail);     // both-paths seam
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

    const AnchorManager *readAnchorManager() const {
        return currentSystem->getAnchorManager().get();
    }
    const CameraAnchors *readCameraAnchors() const {
        return cameraAnchors.get();
    }

    bool cameraAddAnchor(stringHash_t& param) {
        const bool oldOk = currentSystem->getAnchorManager()->addAnchor(param);
        const bool newOk = cameraAnchors->add(param);
        return oldOk || newOk;
    }

    bool cameraRemoveAnchor(const std::string &name) {
        const bool oldOk = currentSystem->getAnchorManager()->removeAnchor(name);
        const bool newOk = cameraAnchors->remove(name);
        return oldOk || newOk;
	}

    bool cameraSwitchToAnchor(const std::string &name) {
		const bool oldOk = currentSystem->getAnchorManager()->switchToAnchor(name);
		const bool newOk = camera ? cameraAnchors->switchTo(name, *camera) : false;
		if (oldOk && !newOk)
			syncCameraReference(name); // old-only anchor: keep the pre-B4 seam
		return oldOk || newOk;
	}

    bool cameraMoveToPoint(double x, double y, double z){
		const bool oldOk = currentSystem->getAnchorManager()->setCurrentAnchorPos(Vec3d(x,y,z));
		const bool newOk = camera
			? cameraAnchors->placeCurrentAt(Vec3d(x,y,z), *camera, timeMgr->getJDay()) : false;
		return oldOk || newOk;
	}

	bool cameraMoveToPoint(double x, double y, double z, double time){
		const bool oldOk = currentSystem->getAnchorManager()->moveTo(Vec3d(x,y,z),time);
		const bool newOk = camera
			? cameraAnchors->travelToPoint(Vec3d(x,y,z), time, *camera, timeMgr->getJDay()) : false;
		return oldOk || newOk;
	}

    bool cameraMoveToBody(const std::string& bodyName, double time, double alt) {
        const bool oldOk = currentSystem->getAnchorManager()->moveToBody(bodyName, time, alt);
        const bool newOk = camera
            ? cameraAnchors->travelToBody(bodyName, time, alt, *camera, timeMgr->getJDay()) : false;
        return oldOk || newOk;
    }

    bool cameraMoveRelativeXYZ( double x, double y, double z) {
		return currentSystem->getAnchorManager()->moveRelativeXYZ(x,y,z);
	}

    bool cameraTransitionToPoint(const std::string& name){
		const bool oldOk = currentSystem->getAnchorManager()->transitionToPoint(name);
		const bool newOk = camera
			? cameraAnchors->transitionToPoint(name, *camera, timeMgr->getJDay()) : false;
		return oldOk || newOk;
	}

    bool cameraTransitionToBody(const std::string& name) {
        const bool oldOk = currentSystem->getAnchorManager()->transitionToBody(name);
        const bool newOk = camera ? cameraAnchors->transitionToBody(name, *camera) : false;
        return oldOk || newOk;
    }

    bool cameraSetFollowRotation(const std::string &name, bool value){
		const bool oldOk = currentSystem->getAnchorManager()->setFollowRotation(value);
		const bool newOk = camera ? cameraAnchors->setFollowRotation(name, value, *camera) : false;
		return oldOk || newOk;
	}

    void cameraSetRotationMultiplierCondition(float v) {
		currentSystem->getAnchorManager()->setRotationMultiplierCondition(v);
	}

    bool cameraAlignWithBody(const std::string& name, double duration){
		return currentSystem->getAnchorManager()->alignCameraToBody(name,duration);
	}

    void anchorManagerInit(const InitParser &conf) {
        const std::string anchorFile = "anchor.ini";
        currentSystem->getAnchorManager()->setRotationMultiplierCondition(conf.getDouble(SCS_NAVIGATION, SCK_STALL_RADIUS_UNIT));
		currentSystem->getAnchorManager()->load(anchorFile);
		cameraAnchors->load(anchorFile);
		currentSystem->getAnchorManager()->initFirstAnchor(conf.getStr(SCS_INIT_LOCATION, SCK_HOME_PLANET));
    }

    void updateAnchorManager() {
        currentSystem->getAnchorManager()->update();
        cameraAnchors->update(timeMgr->getJDay());
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
    void loadSystem(const std::string &path, stringHash_t &params, const std::string &section);
    std::unique_ptr<ProtoSystem> &createSystem(const std::string &mode);
    void createModularSystem(const std::string &name, const std::string &filename, const Vec3d &pos);
    static std::string composedPathOf(const std::string &node) {
        return "modularSystem/" + node + ".ini";
    }
    static std::string composedTwinPathOf(const std::string &node) {
        return composedPathOf(node) + ".disabled";
    }

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

    bool drawModularSystem = true;
    // B5 S6.9 pilot: set true by createExperimentalOort (gated OFF by default);
    // read by hasExperimentalOort() for the dual-path oort seam.
    bool experimentalOortInstantiated = false;
    bool pathPinned = true;
private:
    static void commandDisplayScale(const char *bodyName, float scale) {
        if (ModularBody *body = ModularBody::findBody(bodyName))
            body->setScaling(scale);
    }
    void mirrorSunHaloSize() {
        StarModule::setSunHaloSize(ssystem->getFlagSunScale()
            ? 200.f + ssystem->getSunScale() * 40.f : 200.f);
    }
    static bool fileOwnsDisplayScale(const char *bodyName) {
        const ModularBody *body = ModularBody::findBody(bodyName);
        return body && body->isComposedDeclared();
    }
    void initBodyDisplayScale(const char *bodyName, const char *configKey,
                              bool flag, double value);
    void announceDeprecatedScale(const char *bodyName, const char *configKey,
                                 bool flag, double value);
    void restoreDisplayScaling();
    //! Select current system
    void selectSystem();
    std::vector<std::pair<std::string, std::string>> pendingTwins;
    bool twinsUnblocked = false;
    std::unique_ptr<SolarSystem> ssystem;				// Manage the solar system
    std::unique_ptr<SolarSystemColor> ssystemColor;
    std::unique_ptr<SolarSystemTex> ssystemTex;
    std::unique_ptr<SolarSystemScale> ssystemScale;
    std::unique_ptr<SolarSystemSelected> ssystemSelected;
    std::unique_ptr<SolarSystemDisplay> ssystemDisplay;

    std::unique_ptr<ModularSystem> universe;
    ModularSystem *milkyway; // handle into the tree (universe's INNER child)
    std::unique_ptr<ProtoSystem> galacticSystem;
    std::shared_ptr<AnchorManager> galacticAnchorMgr;

    // Modular systems live IN the tree (milkyway's INNER children - G2);
    // handles by name for the factory's system surface.
    std::map<std::string, ModularSystem *> modularSystemOf;
    std::map<std::string, std::unique_ptr<ProtoSystem>> systems;
    std::map<std::string, Vec3d> systemOffsets;

	std::unique_ptr<ObjLMgr> objLMgr=nullptr;					// represents the light objects of the ss

	std::shared_ptr<BodyTrace> bodytrace;				// the pen bodytrace
    Observer *observatory;
    Navigator *navigation;
    TimeMgr *timeMgr;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<CameraAnchors> cameraAnchors;
    // Environment aggregation authority (created by wireEnvironment - the
    // shared engines don't exist yet at factory construction).
    std::unique_ptr<EnvironmentManager> environment;

    ProtoSystem * currentSystem;
    bool inSystem = true;
    Object selected_object;
    ModularBodySelector newSelectedBody;
};

#endif
