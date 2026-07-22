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
#include "experimentalModule/bodyModules/StarModule.hpp" // sun-scale halo seam (§11.44)
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
    //! Startup selection of the rendered body path (beta_features.ini,
    //! [dual_path] render_path). The DEFAULT is NEW+pinned: the new path is
    //! what a user gets with no configuration at all, and nothing alternates
    //! on its own. ALTERNATE is the A/B comparison harness (1 s swap) and is
    //! opt-in only - it exists to make divergence visible, not to be run in.
    //! Caller responsibility: call once at init, before the first draw; the
    //! script command (setExperimentalPath) overrides it at any later time.
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
        // New-path mirror of old setHaloSize(200)/(200+SunScale*40)
        // (solarsystem.hpp:100/103) - the STAR module's big-halo size; the
        // ssystem.ini big_halo_size is dead for the Sun (INTENT §11.44).
        StarModule::setSunHaloSize(b ? 200.f + ssystem->getSunScale() * 40.f : 200.f);
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
        // Dual-path mirror (was old-path-only - the 11.15c seam class):
        // new-path hide/show move ownership between the parent's relation
        // lists (ModularBody, INTENT 11.36).
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
        // Both-paths mirror (solarsystem_selected.cpp:62): TrailModule::show is
        // the global display master (new bodies + the anyActive phase gate); the
        // per-body faders follow it unless a live per-name override. The
        // selected-body FOCUS filter (old else-branch): when a NON-star body is
        // selected and trails are on, only the selected body + its children keep
        // the master; every other body's trail is overridden off. The star
        // (isStar - old getCenterObject) selected => global, like old.
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
        // The planet grid rides the axis flag: old Body::setFlagAxis sets BOTH
        // flag_axis AND flag_planet_grid=b (body.cpp:229-234) - there is no
        // independent setFlagPlanetGrid. Mirror it on the new path (INTENT §11.42).
        PlanetGridModule::show = b;
    }

    // Grid color seam (INTENT §11.42): sets the GLOBAL grid colors (old
    // observable: sky-manager colors shared by every body). NEW-path only for
    // now - old's grid colors stay sky-manager-driven (body.cpp:1261-1264); the
    // color-authority unification (should the new path read the sky managers?
    // should the old path gain this seam?) is the SUSPENDED structural choice.
    void setPlanetGridColor(const Vec3f &meridian, const Vec3f &parallel) {
        PlanetGridModule::setColors(meridian, parallel);
    }

    // Grid tropic/polar-circle seam (INTENT §11.57, B23 / Q16): the tropic
    // circles ride the LINE_TROPIC sky-line flag, the polar circles the
    // LINE_CIRCLE_POLAR flag - old body.cpp:1257-1258 polled show+color from the
    // sky managers each draw. Pushed each frame by Core::syncPlanetGridSkyState
    // (a per-frame poll of the current sky-line state, faithful to old's read).
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
        // New-path mirror (S6 Textures seam, INTENT 9/11.46): old routes to
        // Body::switchMapSkin (tex_current swap); new = per-module skin state
        // (BodyModule::switchTexSkin, MESH modules consume). Same findBody
        // guard as the orbit per-name seam (exception-on-miss tolerated like
        // the old searchByEnglishName null return).
        try {
            if (ModularBody *body = ModularBody::findBody(name))
                body->switchTexSkin(a);
        } catch (...) {}
    }

	bool getSwitchPlanetTexMap(const std::string &name) {
        // Old stays the getter authority pre-switchover: both paths are
        // driven by the same commands, so the states agree by construction
        // (the new-path state lives per-module; give it a getter at removal).
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
        // Both-paths seam (INTENT §11.65): route the same runtime recolor to
        // the new path's per-instance storage - HALO on the body, LABEL/ORBIT/
        // TRAIL on the modules (self-select on the channel, I4). "all"
        // broadcasts (old SolarSystemColor::setBodyColor "all" iterates the
        // body map); findBody is nullptr-on-miss, so a bogus name is a no-op
        // on both paths.
        const BodyColorType t = parseBodyColorType(colorName);
        if (t != BodyColorType::NONE) {
            if (englishName == "all")
                ModularBody::forEach([&](ModularBody &b){ b.setColor(t, c); });
            else if (ModularBody *body = ModularBody::findBody(englishName))
                body->setColor(t, c);
        }
    }

	const Vec3f getBodyColor(const std::string &englishName, const std::string& colorName) const {
        // Getter stays old-authority pre-switchover (§11.46 precedent): the
        // dual-write above keeps both paths' values in lock-step, so old's
        // value IS the new value; a new-path getter lands at old-path removal.
        return ssystemColor->getBodyColor(englishName, colorName);
    }

	void setDefaultBodyColor(const std::string& colorName, const Vec3f& c) {
        ssystemColor->setDefaultBodyColor(colorName, c);
        // Both-paths seam (INTENT §11.65): mirror the runtime default to the
        // new module statics (the loaders read them for FUTURE body_action_load
        // bodies) + the body-owned halo default. Matches old EXACTLY: setting a
        // default recolors NO existing body (draw reads the per-instance
        // member), only bodies created afterwards - the 4-arg iniColor twin.
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
        // New-path mirror of the planet_scale seam (§11.35 gap, closed T7 §11.45).
        // Old setPlanetSizeScale -> Body::setSphereScale sets radius=initialRadius*s;
        // the new path carries it as per-body scaling (scaledRadius=radius*scaling),
        // which feeds both the drawn size AND the observer altitude reference - the
        // identical mechanism as moon_scale/sun_scale above (setSphereScale is the
        // very same old sink, solarsystem.hpp:118). Without this, planet_scale was
        // old-path-only (harness ab_orientation.py used moon_scale for exactly this).
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

	//! New-path frame entry (camera + environment aggregation), called from
	//! Executor::update in EVERY executor mode - never from the mode modules.
	//! The new path's "in galaxy / in universe" is reference-chain state (G2),
	//! not an executor mode: gating this behind the solar/stellar modules froze
	//! the camera's multi-shell reference cascades mid-flight the moment the
	//! dual-routed moveto flipped the OLD executor's altitude mode
	//! (INTENT 11.36, scene-E mw_out2). Retires with the executors (frame task).
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

	void addBody(stringHash_t &param);

    //! Re-read the camera's current system from its data file, KEEPING the
    //! current observation state [vixy 2026-07-21, Q27: "keep current state
    //! (camera + date), do not reset to start-up"]. What survives: the
    //! simulated date (untouched - a reload never speaks to the clock), the
    //! camera pose (longitude/latitude/altitude or free-flight position,
    //! view direction, heading, fov), the reference body, the tracked body
    //! and the new-path selection. What changes: the bodies themselves, which
    //! are the point - edits to the file take effect, bodies removed from it
    //! disappear, bodies added to it appear.
    //! Identity across the rebuild is BY NAME: the body OBJECTS are destroyed
    //! and recreated, so every camera-side reference is re-seated here (I5 -
    //! this class owns those references; ModularBodyPtr keeps them merely
    //! valid, pointing at the surviving ancestor, which is not the same thing
    //! as keeping the state). A name that the reloaded file no longer defines
    //! cannot be re-seated: the reference then stays where ModularBodyPtr
    //! redirected it (the system node) and the loss is traced as an error.
    //! Returns false when the current system has no data file to reload from.
    bool reloadCurrentSystem();

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

    //! Which path draws the bodies. DEFAULT = new path [vixy 2026-07-21:
    //! "we should change from auto-switching to defaulting to the new mode"].
    //! These two defaults are the no-configuration behaviour and must stay
    //! consistent with setRenderPathMode(NEW).
    bool drawModularSystem = true;
    // Path pinned: the 1s A/B auto-toggle runs ONLY when this is false, which
    // now requires an explicit opt-in (beta_features.ini ALTERNATE) or the
    // script command. Alternation was the default until 2026-07-21 - it made
    // every unconfigured run flicker between two paths, which is a comparison
    // harness being used as a product default [vixy: 2026-07-12 for the script
    // pin, 2026-07-21 for the default].
    bool pathPinned = true;
private:
    //! Select current system
    void selectSystem();
    std::unique_ptr<SolarSystem> ssystem;				// Manage the solar system
    std::unique_ptr<SolarSystemColor> ssystemColor;
    std::unique_ptr<SolarSystemTex> ssystemTex;
    std::unique_ptr<SolarSystemScale> ssystemScale;
    std::unique_ptr<SolarSystemSelected> ssystemSelected;
    std::unique_ptr<SolarSystemDisplay> ssystemDisplay;

    // The tree root and single eternal node (INTENT 6.6 resolved): the
    // universe owns everything; every other node has a parent to delegate
    // remnant ModularBodyPtr to on destruction. Destroyed only at factory
    // teardown (declared BEFORE camera/environment: members destruct in
    // reverse order, so every ModularBodyPtr holder releases first).
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
