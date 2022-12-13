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
class SkyLocalizer;
class CloudNavigator;
class DsoNavigator;
class OjmMgr;
class StarNavigator;
class UBOCam;
class NebulaMgr;
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

	//! Get the I18 available sky culture names
	std::string getSkyCultureListI18() const;
	std::string getSkyCultureHash() const;

	bool loadSkyCulture(const std::string& culturePath);

	//! Set the landscape
	bool setLandscape(const std::string& new_landscape_name);

	//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
	//! and make it the current landscape
	bool loadLandscape(stringHash_t& param);

	//! @brief Set the sky language and reload the sky objects names with the new translation
	//! This function has no permanent effect on the global locale
	//!@param newSkyLocaleName The name of the locale (e.g fr) to use for sky object labels
	void setSkyLanguage(const std::string& newSkyLocaleName);

	//! Get the current sky language used for sky object labels
	//! @return The name of the locale (e.g fr)
	std::string getSkyLanguage();

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

	//! Set whether sky position is to be locked
	void setFlagLockSkyPosition(bool b) {
		navigation->setFlagLockEquPos(b);
	}
	//! Set whether sky position is locked
	bool getFlagLockSkyPosition(void) {
		return navigation->getFlagLockEquPos();
	}

	//! Set current mount type
	void setMountMode(MOUNT_MODE m) {
		navigation->setViewingMode((m==MOUNT_ALTAZIMUTAL) ? Navigator::VIEW_HORIZON : Navigator::VIEW_EQUATOR);
	}
	//! Get current mount type
	MOUNT_MODE getMountMode(void) {
		return ((navigation->getViewingMode()==Navigator::VIEW_HORIZON) ? MOUNT_ALTAZIMUTAL : MOUNT_EQUATORIAL);
	}
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

	//! Deselect selected object if any
	//! Does not deselect selected constellation
	void unSelect(void);

	void unsetSelectedConstellation(std::string constellation);
	void deselect(void);

	//! Set whether a pointer is to be drawn over selected object
	void setFlagSelectedObjectPointer(bool b) {
		object_pointer_visibility = b;
	}

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

	//! Adds year(s) and month(s) to the current date without affecting the other date parameters
	void setJDayRelative(int year, int month);

	// for adding planets
	void addSolarSystemBody(stringHash_t& param);
	void preloadSolarSystemBody(stringHash_t& param);
	void removeSolarSystemBody(const std::string& name);
	void removeSupplementalSolarSystemBodies();

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

	int getSelectedBodyName() {
		return selected_body_name;
	}

	// Update tickable elements
	void update(int delta_time);
private:
	struct ViewZoomMove {
		double deltaAlt, deltaAz, deltaFov, deltaHeight;	// View movement
		double move_speed, zoom_speed;		// Speed of movement and zooming
		double coefAlt=1, coefAz=1; // Coefficient applied to deltaAlt and deltaAz, used for joystick axis
	};

	void applyClippingPlanes(float clipping_min, float clipping_max);

	//! Callback to record actions
	mBoost::callback<void, std::string> recordActionCallback;

	//! Select passed object
	//! @return true if the object was selected (false if the same was already selected)
	bool selectObject(const Object &obj);

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
	std::list<Tickable<CoreLink> *> updateList;
	GeodesicGrid* geodesic_grid;
	BodyDecor* bodyDecor = nullptr;
	MODULE currentModule = MODULE::SOLAR_SYSTEM;

	float sky_brightness;				// Current sky Brightness in ?
	bool object_pointer_visibility;		// Should selected object pointer be drawn
	bool autoLandscapeMode=true;		// Define if we use customised landscape or none
	bool FlagEnableZoomKeys;
	bool FlagEnableMoveKeys;
	bool FlagAtmosphericRefraction = false;
	bool flagNav = false; 				// define the NAV version edition
	bool FlagManualZoom;				// Define whether auto zoom can go further
	bool firstTime= true;               // For init to track if reload or first time setup
	bool flagEnableTransition = true;
	ViewZoomMove vzm;					// var for ViewZoomMove
	float InitFov;						// Default viewing FOV
	Vec3d InitViewPos;					// Default viewing direction
	float auto_move_duration;			// Duration of movement for the auto move to a selected objectin seconds
	float lightPollutionLimitingMagnitude;  // Defined naked eye limiting magnitude (due to light pollution)
	int selected_body_name=999;

	InitialValue initialvalue;			// variable used to remember various string indicators in use
};

#endif // _CORE_H_
