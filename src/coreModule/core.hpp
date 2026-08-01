/*
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _CORE_H_
#define _CORE_H_

#include <string>
#include "bodyModule/body_common.hpp"
//#include "atmosphereModule/atmosphere.hpp"
#include "atmosphereModule/skybright.hpp"
#include "bodyModule/body_decor.hpp"
#include "bodyModule/body_trace.hpp"
#include "bodyModule/solarsystem.hpp"
//#include "bodyModule/ssystem_factory.hpp"
#include "coreModule/backup_mgr.hpp"
#include "coreModule/callbacks.hpp"
//#include "coreModule/cardinals.hpp"
//#include "coreModule/constellation_mgr.hpp"
//#include "inGalaxyModule/dso3d.hpp"
//#include "coreModule/illuminate_mgr.hpp"
// #include "coreModule/landscape.hpp"
//#include "coreModule/meteor_mgr.hpp"
//#include "coreModule/milkyway.hpp"
//#include "coreModule/nebula_mgr.hpp"
//#include "coreModule/oort.hpp"
#include "coreModule/projector.hpp"
//#include "coreModule/skygrid_mgr.hpp"
//#include "coreModule/skygrid.hpp"
//#include "coreModule/skyline_mgr.hpp"
//#include "coreModule/skyline.hpp"
//#include "coreModule/skydisplay_mgr.hpp"
//#include "coreModule/skyDisplay.hpp"
//#include "coreModule/starLines.hpp"
//#include "inGalaxyModule/starNavigator.hpp"
//#include "inGalaxyModule/cloudNavigator.hpp"
//#include "inGalaxyModule/dsoNavigator.hpp"
//#include "mediaModule/text_mgr.hpp"
//#include "coreModule/time_mgr.hpp"
//#include "coreModule/tully.hpp"
//#include "coreModule/ubo_cam.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
//#include "ojmModule/ojm_mgr.hpp"
#include <iosfwd>
#include "starModule/geodesic_grid.hpp"
//#include "starModule/hip_star_mgr.hpp"
#include "executorModule/executorModule.hpp"

//#include "mainModule/define_key.hpp"
#include "tools/object.hpp"

//#include "coreModule/sky_localizer.hpp"

#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/utility.hpp"
#include "tools/no_copy.hpp"
#include "tools/translator.hpp"
#include "EntityCore/Executor/Tickable.hpp"
#include "experimentalModule/AsyncHub.hpp"

class StarNavigator;
class BodyDecor;
class Landscape;
class Translator;
class Tully;
class Oort;
class Dso3d;
class Media;
class StarLines;
class BodyTrace;
class FontFactory;
class SkyGridMgr;
class SkyLineMgr;
class SkyDisplayMgr;
class Cardinals;
class MeteorMgr;
class MilkyWay;
class IlluminateMgr;
class Atmosphere;
class Skybright;
class SkyLocalizer;
class CloudNavigator;
class DsoNavigator;
class OjmMgr;
class StarNavigator;
class UBOCam;
class NebulaMgr;
class ScriptMgr;
class HipStarMgr;
class ConstellationMgr;
class SSystemFactory;
class StarGalaxy;
class VolumObj3D;
class CoreLink;

//!  @brief Main class for application core processing.
//!
//! Manage all the objects to be used in the program.
//! This class is the main API of the program. It must be documented using doxygen.
class Core: public NoCopy {
public:
	friend class Executor;
	friend class ExecutorModule;
	friend class SolarSystemModule;
	friend class StellarSystemModule;
	friend class InGalaxyModule;
	friend class InUniverseModule;

	friend class CoreLink;
	friend class CoreBackup;

	//! Possible mount modes
	enum MOUNT_MODE { MOUNT_ALTAZIMUTAL, MOUNT_EQUATORIAL };

	//! Inputs are the locale directory and root directory and callback function for recording actions
	Core(int width, int height, std::shared_ptr<Media> _media, std::shared_ptr<FontFactory> _fontFactory, const mBoost::callback <void, std::string> & recordCallback, std::shared_ptr<Observer> _observatory);
	virtual ~Core();

	//! Init and load all main core components from the passed config file.
	void init(const InitParser& conf);

	//! Set the sky culture from I18 name
	//! Returns false and doesn't change if skyculture is invalid
	bool setSkyCulture(const std::string& cultureName);

	//! Set the current sky culture from the passed directory
	bool setSkyCultureDir(const std::string& culturedir);

	std::string getSkyCultureDir() {
		return skyCultureDir;
	}

	//! Get the current sky culture I18 name
	std::string getSkyCulture() const;

	void setInitialSkyCulture() {
		//printf("Culture %s\n",initialvalue.initial_skyCulture.c_str());
		setSkyCultureDir(initialvalue.initial_skyCulture);
	}

	void setInitialSkyLocale() {
		//printf("Locale %s\n",initialvalue.initial_skyLocale.c_str());
		setSkyLanguage(initialvalue.initial_skyLocale);
	}

	// [merge] theirs' subtitle SRT-locale feature (command side).
	void setInitialSrtLocale() {
		setSrtLanguage(initialvalue.initial_srtLocale);
	}

	//! Get the I18 available sky culture names
	std::string getSkyCultureListI18() const;
	std::string getSkyCultureHash() const;

	bool loadSkyCulture(const std::string& culturePath);

	//! Set the landscape
	bool setLandscape(const std::string& new_landscape_name);

	void setLandingLandscape(bool landing, float speed);

	//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
	//! and make it the current landscape
	bool loadLandscape(stringHash_t& param, int landing);

	//! @brief Set the sky language and reload the sky objects names with the new translation
	//! This function has no permanent effect on the global locale
	//!@param newSkyLocaleName The name of the locale (e.g fr) to use for sky object labels
	void setSkyLanguage(const std::string& newSkyLocaleName);

	//! Get the current sky language used for sky object labels
	//! @return The name of the locale (e.g fr)
	std::string getSkyLanguage();

	// [merge] theirs' SRT (subtitle) language, command side; stores/returns the locale.
	void setSrtLanguage(const std::string& newSrtLocaleName) { srtLanguage = newSrtLocaleName; }
	std::string getSrtLanguage() { return srtLanguage; }

	// [merge] theirs' init_fov script command support.
	void setInitFov(double f) { InitFov = f; }

	///////////////////////////////////////////////////////////////////////////////////////
	// Navigation

	//! Set simulation time to current real world time
	void setTimeNow();
	//! Get wether the current simulation time is the real world time
	bool getIsTimeNow(void) const;

	//! Look at observatory position
	void lookAnchor(const std::string &name, double duration);

	//! Set object tracking
	void setFlagTracking(bool b);
	//! Get object tracking
	bool getFlagTracking(void) {
		return navigation->getFlagTraking();
	}

	//! Set whether sky position is to be locked. Both-paths mirror (defined in
	//! core.cpp): old navigation flag + new-path Camera sky-lock (INTENT 11.58).
	void setFlagLockSkyPosition(bool b);
	//! Whether the sky position is locked ON THE PATH THAT DRAWS — B33
	//! (§11.108(f), the F12 template §11.118(f)). Defined in core.cpp: this
	//! header does not see the Camera, and the answer is the Camera's whenever
	//! the new path is the one drawing.
	bool getFlagLockSkyPosition(void);

	//! Set current mount type.
	//! WRITE-HALF GAP, named here because the read half above it is now right
	//! and the pair would otherwise disagree the day this is wired: this writes
	//! the OLD navigator only, while the drawn mount is `Camera::mount`
	//! (`Camera::setMount`, whose transition is a deduce-identical-view
	//! recovery the old setter has no equivalent of). Both are config-only
	//! today — this method and toggleMountMode have ZERO callers in src/, and
	//! both authorities are initialised from the SAME key
	//! ([navigation] viewing_mode: core.cpp for the navigator,
	//! ssystem_factory.cpp for the camera) — which is B35, and the spelling of
	//! the command that would wire it is Vixy's (D15 adjacency). Whoever wires
	//! it makes this dual FIRST; the getter is already asking the right
	//! question. (INTENT §11.131.)
	void setMountMode(MOUNT_MODE m) {
		navigation->setViewingMode((m==MOUNT_ALTAZIMUTAL) ? Navigator::VIEW_HORIZON : Navigator::VIEW_EQUATOR);
	}
	//! Get current mount type — of the path that DRAWS (B33). Defined in
	//! core.cpp, same reason as getFlagLockSkyPosition.
	MOUNT_MODE getMountMode(void);
	//! Toggle current mount mode between equatorial and altazimutal
	void toggleMountMode(void) {
		if (getMountMode()==MOUNT_ALTAZIMUTAL) setMountMode(MOUNT_EQUATORIAL);
		else setMountMode(MOUNT_ALTAZIMUTAL);
	}

	//! Go to the selected object
	void gotoSelectedObject(void) {
		if (selected_object)
			navigation->moveTo( selected_object.getEarthEquPos(navigation), auto_move_duration);
	}

	//! Move view in alt/az (or equatorial if in that mode) coordinates
	void panView(double delta_az, double delta_alt, double duration) {
		setFlagTracking(0);
		navigation->updateMove(delta_az, delta_alt, projection->getFov(), duration);
	}

	//! set zoom/center offset (percent of fov radius)
	void setViewOffset(double offset);

	//! Set automove duration in seconds
	void setAutoMoveDuration(float f) {
		auto_move_duration = f;
	}
	//! Get automove duration in seconds
	float getAutoMoveDuration(void) const {
		return auto_move_duration;
	}

	//! Go and zoom temporarily to the selected object.
	void autoZoomIn(float move_duration = 1.f, bool allow_manual_zoom = 1);

	//! Unzoom to the previous position
	void autoZoomOut(float move_duration = 1.f, bool full = 0, bool allow_manual_zoom = 0);

	//! Set whether auto zoom can go further than normal
	void setFlagManualAutoZoom(bool b) {
		FlagManualZoom = b;
	}
	//! Get whether auto zoom can go further than normal
	bool getFlagManualAutoZoom(void) {
		return FlagManualZoom;
	}
	//! Set whether transition is enabled or not
	void setFlagEnableTransition(bool b) {
		flagEnableTransition = b;
	}
	bool getFlagEnableTransition() const {
		return flagEnableTransition;
	}

	// Viewing direction function : 1 move, 0 stop.
	void turnRight(int);
	void turnLeft(int);
	void turnUp(int);
	void turnDown(int);
	void zoomIn(int);
	void zoomOut(int);
	void raiseHeight(int);
	void lowerHeight(int);

	// Analogic viewing direction functions
	void turnHorizontal(float);
	void turnVertical(float);

	//! Make the first screen position correspond to the second (useful for mouse dragging)
	void dragView(int x1, int y1, int x2, int y2);

	//! Find and select an object near given equatorial position
	//! @return true if a object was found at position (this does not necessarily means it is selected)
	bool findAndSelect(const Vec3d& pos);

	//! Find and select an object near given screen position
	//! @return true if a object was found at position (this does not necessarily means it is selected)
	bool findAndSelect(int x, int y);

	//! Find and select an object from its translated name
	//! @param nameI18n the case sensitive object translated name
	//! @return true if a object was found with the passed name
	bool findAndSelectI18n(const std::string &nameI18n);

	//! Find and select an object based on selection type and standard name or number
	//! @return true if an object was selected
	bool selectObject(const std::string &type, const std::string &id);


	//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
	//! @param objPrefix the case insensitive first letters of the searched object
	//! @param maxNbItem the maximum number of returned object names
	//! @return a vector of matching object name by order of relevance, or an empty vector if nothing match
	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem=5, bool withType= false) const;

	std::string getListMatchingObjects(const std::string& objPrefix, unsigned int maxNbItem=5) const;

	//! Return whether an object is currently selected
	bool getFlagHasSelected(void) {
		return selected_object;
	}

	//! Whether the OLD path must draw the selection pointer this frame.
	//! Dual-path rule (S2b), previously copy-pasted at the four executor draw
	//! sites: in the modular phase BODY pointers are drawn by the new path's
	//! pointer service (Renderer::drawPointer, ModularSystem::drawSystem) and
	//! the old pointer would double-draw at the old path's projected position.
	//! That holds for old-tree bodies (OBJECT_BODY) and for new-only composed
	//! bodies (OBJECT_MODULAR) alike - both are driven by
	//! ModularBody::getSelected() there (B24-select, INTENT §11.106).
	//! Non-body pointers (star/nebula) have no new-path counterpart and stay.
	bool needOldSelectionPointer() const {
		if (!selected_object || !object_pointer_visibility)
			return false;
		if (!ssystemFactory->drawModularSystem)
			return true;
		const OBJECT_TYPE type = selected_object.getType();
		return !(type == OBJECT_BODY || type == OBJECT_MODULAR);
	}

	//! Deselect selected object if any
	//! Does not deselect selected constellation
	void unSelect(void);

	void unsetSelectedConstellation(std::string constellation);
	void deselect(void);

	//! Set whether a pointer is to be drawn over selected object.
	//! Single choke point for the flag - mirrors to the new path's pointer
	//! service (Renderer::showPointer); defined in core.cpp for that reason.
	void setFlagSelectedObjectPointer(bool b);

	std::string getSelectedPlanetEnglishName() const;

	std::string getHomePlanetEnglishName() const;

	//! Get a multiline string describing the currently selected object
	std::string getSelectedObjectInfo(void) const {
		return selected_object.getInfoString(navigation);
	}

	void getDeRa(double *ra, double *de) const {
		selected_object.getRaDeValue(navigation,ra,de);
	}

	bool getStarEarthEquPosition(int HP, double &az, double &alt);

	//! Get a 1 line string briefly describing the currently selected object
	std::string getSelectedObjectShortInfo(void) const {
		return selected_object.getShortInfoString(navigation);
	}

	//! Get a 1 line string briefly describing the currently NAV edition selected object
	std::string getSelectedObjectShortInfoNav(void) const {
		return selected_object.getShortInfoNavString(navigation, timeMgr.get(), observatory.get());
	}


	//! Get a color used to display info about the currently selected object
	Vec3f getSelectedObjectInfoColor(void) const;


	///////////////////////////////////////////////////////////////////////////////////////
	// Stars methods
	void setStarSizeLimit(float);
	float starGetSizeLimit(void) const;

	//! Set base planets display scaling factor
	//! This is additive to star size limit above
	//! since makes no sense to be less
	//! ONLY SET THROUGH THIS METHOD
	void setPlanetsSizeLimit(float f);

	//! Get base planets display scaling factor
	float getPlanetsSizeLimit(void) const;

	std::string getPlanetHashString(void);
	//Functions above and below defined in core.cpp
	void setHomePlanet(const std::string &planet);
	void bindHomePlanet();


	//! Adds year(s) and month(s) to the current date without affecting the other date parameters
	void setJDayRelative(int year, int month);

	// for adding planets
	void addSolarSystemBody(stringHash_t& param);
	void preloadSolarSystemBody(stringHash_t& param);
	void removeSolarSystemBody(const std::string& name);
	void removeSupplementalSolarSystemBodies();
	// Dual-path trace harness (experimentalModule/INTENT.md 11.14)
	void ssystemDualDump(const std::string& file);

	//! The direction the OLD path draws the sky from (§2 row B19's twin).
	//! The star field, the milky way and the nebulae are aimed by this vector
	//! and by nothing the camera holds — measured 107.634 deg away from the
	//! camera in a scene where nothing aimed either path (INTENT §11.130).
	const Vec3d& getSkyVision() const;

	//! Put that direction back where a session recorded it, and make it MEAN
	//! something: the transforms are recomputed from the observer and the date
	//! FIRST, because a restore runs between two frames and the navigator's
	//! matrices are otherwise still the previous frame's — on the launch body,
	//! at the launch date. Same two calls Core::init makes after it moves the
	//! observer; no existing caller of the old path changes.
	void restoreSkyVision(const Vec3d& localVision);

	//! §2 row B10 on BOTH paths. `setViewOffset` is already the one sink the
	//! two §2(c) channels funnel into, so the scalar is its business; what it
	//! cannot do is assert the ARMING LATCH, which D32 saves as a condition and
	//! snaps. The old navigator's latch is otherwise only reachable through an
	//! aim, and a restore must not aim. Session-restore use only.
	void restoreViewOffset(double offset, bool armed);

	//! READBACK ONLY (INTENT §5.63 / §11.130) — the OLD path's view state, as
	//! one JSON object, written into the dual-path dump's header.
	//! What it is FOR: the composed screen of a restored session differs from
	//! the saved one ONLY in old-path sky content (stars / milky way /
	//! nebulae), while every field of the camera dump agrees. The state those
	//! three are drawn from lives in the navigator, the observer, the projector
	//! and the star pipeline, and none of it was observable — so the difference
	//! could be measured but not attributed. Each owner writes its own part;
	//! this method only composes them and adds what Core itself owns.
	//! Const, side-effect-free, dump-channel only: the old render path is
	//! unchanged by construction (§11.52(b)).
	void dumpOldViewState(std::ostream &out) const;
	//! Pin the rendered body path (flag experimental_path): old/new selection
	//! replacing the A/B auto-toggle once used.
	void setExperimentalPath(bool newPath);
	bool getExperimentalPath() const;
	//! Startup path selection from beta_features.ini (see SSystemFactory).
	//! Call once at init; "new"/"old"/"alternate", anything else is refused
	//! with a log line rather than silently falling back - an unrecognised
	//! value is a user error that must be visible, not absorbed.
	void setRenderPathMode(const std::string &mode);

	//! set flag to display generic Hint or specific DSO type
	void setDsoPictograms (bool value);
	//! get flag to display generic Hint or specific DSO type
	bool getDsoPictograms ();

	bool loadNebula(double ra, double de, double magnitude, double angular_size, double rotation,
	                std::string name, std::string filename, std::string credit, double texture_luminance_adjust,
	                double distance, std::string constellation, std::string type);

	//! remove one nebula added by user
	void removeNebula(const std::string& name);
	//! remove all user added nebulae
	void removeSupplementalNebulae();

	bool loadDso2d(int typeDso, std::string name, float size, float alpha, float delta, float distance, int xyz);
	void removeSupplementalDso();

	///////////////////////////////////////////////////////////////////////////////////////
	// Projection

	//! Print the passed string so that it is oriented in the drection of the gravity
	void printHorizontal(s_font* font, float altitude, float azimuth, const std::string& str, Vec3f textColor, TEXT_ALIGN textPos = TEXT_ALIGN::LEFT, bool cache = true) const {
		font->printHorizontal(projection, altitude, azimuth, str, textColor, textPos, cache);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Landscape
	void setInitialLandscapeName() {
		setLandscape(initialvalue.initial_landscapeName);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Atmosphere
	//! Set light pollution limiting magnitude (naked eye)
	void setLightPollutionLimitingMagnitude(float mag);
	//! Get light pollution limiting magnitude
	float getLightPollutionLimitingMagnitude(void) const {
		return lightPollutionLimitingMagnitude;
	}

	void selectZodiac();

	///////////////////////////////////////////////////////////////////////////////////////
	// Others
	//! Load color scheme from the given ini file and section name
	void setColorScheme(const std::string& skinFile, const std::string& section);

	// MAJ de l'UBO uboCam
	void uboCamUpdate();

	void setFlagNav(bool a);

	bool getFlagNav() {
		return flagNav;
	}

	void setFlagAstronomical(bool a);

	bool getFlagAstronomical() {
		return flagAstronomical;
	}

	void setFlagIngalaxy(MODULE a) {
		currentModule = a;
	}

	MODULE getFlagIngalaxy() {
		return currentModule;
	}

	void saveCurrentConfig(InitParser &conf);

	void onObserverChange(std::string str) {
		std::cout << "Modification observer to " << str << std::endl;
		setLandscapeToBody();
	}

	int getLanguage() {
		return language;
	}

	int getSelectedBodyName() {
		return selected_body_name;
	}

	// Update tickable elements
	void update(int delta_time);

	void setPredictibleRendering(bool enable, int framerate);
private:
	//! Zoom the OLD projection to aim_fov AND mirror it to the new-path
	//! Camera (ModularBody::halfFov -> Renderer clipping_fov.z). Both-paths
	//! seam for the auto-zoom fov targets (INTENT 11.40 / 11.15c residual).
	void zoomToBothPaths(double aim_fov, float move_duration);
	struct ViewZoomMove {
		double deltaAlt, deltaAz, deltaFov, deltaHeight;	// View movement
		double move_speed, zoom_speed;		// Speed of movement and zooming
		double coefAlt=1, coefAz=1; // Coefficient applied to deltaAlt and deltaAz, used for joystick axis
	};

	void applyClippingPlanes(float clipping_min, float clipping_max);

	//! Push the current sky-line tropic / polar-circle state (show flags +
	//! colors) to the new-path planet grid, once per frame before the modular
	//! system draws. Reproduces old Body::drawPlanetGrid's per-frame poll of the
	//! sky managers (body.cpp:1257-1258): the planet-grid tropic circles ride
	//! LINE_TROPIC, the polar circles LINE_CIRCLE_POLAR (INTENT §11.57, B23).
	void syncPlanetGridSkyState();

	//! Callback to record actions
	mBoost::callback<void, std::string> recordActionCallback;

	//! Select passed object
	//! @return true if the object was selected (false if the same was already selected)
	bool selectObject(const Object &obj);

	//set name as number for language
	void setLanguage();

	//set name as number for selected_object
	void setSelectedBodyName(const Object &selected_object);

	//! Find any kind of object by the name
	Object searchByNameI18n(const std::string &name) const;

	//! Find in a "clever" way an object from its equatorial position
	Object cleverFind(const Vec3d& pos) const;

	//! Find in a "clever" way an object from its screen position
	Object cleverFind(int x, int y) const;

	std::string getCursorPos(int x, int y);  //not used now
	Vec3f getCursorPosEqu(int x, int y);  //not used now

	// Increment/decrement smoothly the vision field and position
	void updateMove(int delta_time);

	// initialize CoreFont class
	void registerCoreFont() const;

	// automatically adapts the landscape to the selected body
	void setLandscapeToBody();

	//validates the display decisions of the body's scenery
	void setBodyDecor();

	// check if the landsacpe is compatible with the auto mode
	void testLandscapeCompatibleWithAutoMode();

	std::string skyCultureDir;			// The directory containing data for the culture used for constellations, etc..
	Translator skyTranslator;			// The translator used for astronomical object naming

	// external class
	std::shared_ptr<FontFactory> fontFactory;					// complete management of the software fonts
	std::shared_ptr<Observer> observatory;			// Manage observer position it's a pointer to the 3 other Observer
	std::shared_ptr<Media> media;

	// Main elements of the program
	Navigator * navigation;				// Manage all navigation parameters, coordinate transformations etc..
	std::shared_ptr<TimeMgr> timeMgr;				// Manage date and time

	Observer *obsSolarSystem;		// it's the historical Observer: so nothing to do with it now.
	Observer *obsInGalaxy;			//observer that should be used in InGalaxy mode
	Observer *ObsInUnivers;			//observer that should be used in InUnivers mode
	Projector * projection;				// Manage the projection mode and matrix
	Object selected_object;			// The selected object
	Object old_selected_object;		// The old selected object
	std::shared_ptr<HipStarMgr> hip_stars;		// Manage the hipparcos stars
	std::shared_ptr<ConstellationMgr> asterisms;		// Manage constellations (boundaries, names etc..)
	std::unique_ptr<NebulaMgr> nebulas;				// Manage the nebulas
	std::unique_ptr<ScriptMgr> script;
	std::unique_ptr<IlluminateMgr> illuminates;		// Manage the illuminations
	//TextMgr * text_usr;				// manage all user text in dome
	//SolarSystem* ssystem;				// Manage the solar system
	SSystemFactory* ssystemFactory;

	std::shared_ptr<Atmosphere> atmosphere;			// Atmosphere

	std::unique_ptr<SkyGridMgr> skyGridMgr;				//! grid manager
	std::unique_ptr<SkyLineMgr> skyLineMgr;				//! line manager
	std::unique_ptr<SkyDisplayMgr> skyDisplayMgr; 		//! skyDisplay manager
	std::unique_ptr<Oort> oort;			//! oort cloud
	std::unique_ptr<Dso3d> dso3d;		//! dso catalog for in_galaxy
	std::unique_ptr<Tully> tully;		//! tully galaxies
	std::unique_ptr<Cardinals> cardinals_points;	// Cardinals points
	std::shared_ptr<MilkyWay> milky_way;			// Our galaxy
	std::unique_ptr<MeteorMgr> meteors;				// Manage meteor showers
	Landscape * landscape;				// The landscape ie the fog, the ground and "decor"
	ToneReproductor * tone_converter;	// Tones conversion between simulation world and display device
	std::unique_ptr<SkyLocalizer> skyloc;				// for sky cultures and locales
	std::unique_ptr<StarNavigator> starNav; 			// permet le voyage dans les étoiles
	std::unique_ptr<CloudNavigator> cloudNav; 			// draw galaxy gaz clouds
	std::unique_ptr<CloudNavigator> universeCloudNav; 	// draw galaxy gaz clouds when in universe
	std::unique_ptr<StarGalaxy> starGalaxy; 			// draw galaxy stars when in universe
	std::unique_ptr<VolumObj3D> volumGalaxy; 			// draw volumetric galaxy
	std::unique_ptr<DsoNavigator> dsoNav; 				// draw 3d dso when in galaxy
	std::unique_ptr<StarLines> starLines;			// allows to draw lines in the galaxy
	std::unique_ptr<OjmMgr> ojmMgr;					// represents obj3D
	std::unique_ptr<UBOCam> uboCam;
	GeodesicGrid* geodesic_grid;
	BodyDecor* bodyDecor = nullptr;
	AsyncHub transitions;
	MODULE currentModule = MODULE::SOLAR_SYSTEM;

	float sky_brightness;				// Current sky Brightness in ?
	bool object_pointer_visibility;		// Should selected object pointer be drawn
	bool autoLandscapeMode=true;		// Define if we use customised landscape or none
	bool FlagEnableZoomKeys;
	bool FlagEnableMoveKeys;
	bool FlagAtmosphericRefraction = false;
	bool flagNav = false; 				// define the NAV version edition
	bool flagAstronomical = false; 		// define the astronomical version edition
	bool FlagManualZoom;				// Define whether auto zoom can go further
	bool firstTime= true;               // For init to track if reload or first time setup
	bool flagEnableTransition = true;
	bool predictibleRendering = false;  // Whether the rendered frames must be strictly reproductible (ex : recording sequence) or not (ex : realtime use)
	ViewZoomMove vzm;					// var for ViewZoomMove
	float InitFov;						// Default viewing FOV
	std::string srtLanguage;			// [merge] current SRT (subtitle) locale (theirs' feature)
	Vec3d InitViewPos;					// Default viewing direction
	float auto_move_duration;			// Duration of movement for the auto move to a selected objectin seconds
	float lightPollutionLimitingMagnitude;  // Defined naked eye limiting magnitude (due to light pollution)
	int selected_body_name=999;
	int language = 0;

	InitialValue initialvalue;			// variable used to remember various string indicators in use
};

#endif // _CORE_H_
