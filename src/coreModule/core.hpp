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
#include "atmosphereModule/atmosphere.hpp"
#include "bodyModule/body_decor.hpp"
#include "bodyModule/body_trace.hpp"
#include "bodyModule/solarsystem.hpp"
#include "coreModule/backup_mgr.hpp"
#include "coreModule/callbacks.hpp"
#include "coreModule/cardinals.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "coreModule/core.hpp"
#include "coreModule/dso3d.hpp"
#include "coreModule/illuminate_mgr.hpp"
#include "coreModule/landscape.hpp"
#include "coreModule/meteor_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/oort.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/skygrid.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skyline.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/skyDisplay.hpp"
#include "coreModule/starLines.hpp"
#include "coreModule/starNavigator.hpp"
#include "mediaModule/text_mgr.hpp"
#include "coreModule/time_mgr.hpp"
#include "coreModule/tully.hpp"
#include "coreModule/ubo_cam.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "starModule/geodesic_grid.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "tools/init_parser.hpp"
#include "mainModule/define_key.hpp"
#include "tools/object.hpp"
#include "renderGL/shader.hpp"
#include "coreModule/sky_localizer.hpp"
#include "renderGL/stateGL.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/utility.hpp"
#include "tools/no_copy.hpp"

class StarNavigator;
class CoreExecutor;
class BodyDecor;
class Landscape;
class Translator;
class Tully;
class Oort;
class Dso3d;
class Media;
class StarLines;
class BodyTrace;
class CoreFont;

//!  @brief Main class for application core processing.
//!
//! Manage all the objects to be used in the program.
//! This class is the main API of the program. It must be documented using doxygen.
class Core: public NoCopy {
public:
	friend class CoreExecutor;
	friend class CoreExecutorInSolarSystem;
	friend class CoreExecutorInGalaxy;
	friend class CoreExecutorInUniverse;

	friend class CoreLink;
	friend class CoreBackup;

	//! Possible mount modes
	enum MOUNT_MODE { MOUNT_ALTAZIMUTAL, MOUNT_EQUATORIAL };

	//! Inputs are the locale directory and root directory and callback function for recording actions
	Core(int width, int height, Media* _media, const mBoost::callback <void, std::string> & recordCallback);
	virtual ~Core();

	//! Init and load all main core components from the passed config file.
	void init(const InitParser& conf);

	//! Update all the objects in current mode 
	void update(int delta_time);

	//! Update current mode
	void updateMode();

	//! Draw all the objects in current mode 
	void draw(int delta_time);

	//! Set the sky culture from I18 name
	//! Returns false and doesn't change if skyculture is invalid
	bool setSkyCulture(const std::string& cultureName);

	//! Set the current sky culture from the passed directory
	bool setSkyCultureDir(const std::string& culturedir);

	std::string getSkyCultureDir() {
		return skyCultureDir;
	}

	//! Get the current sky culture I18 name
	std::string getSkyCulture() const {
		return skyloc->directoryToSkyCultureI18(skyCultureDir);
	}

	void setInitialSkyCulture() {
		//printf("Culture %s\n",initialvalue.initial_skyCulture.c_str());
		setSkyCultureDir(initialvalue.initial_skyCulture);
	}

	void setInitialSkyLocale() {
		//printf("Locale %s\n",initialvalue.initial_skyLocale.c_str());
		setSkyLanguage(initialvalue.initial_skyLocale);
	}

	//! Get the I18 available sky culture names
	std::string getSkyCultureListI18() const {
		return skyloc->getSkyCultureListI18();
	}

	std::string getSkyCultureHash() const {
		return skyloc->getSkyCultureHash();
	}

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

	// double getViewOffset() {
	// 	return navigation->getViewOffset();
	// }

	// //! set environment rotation around observer
	// void setHeading(double heading, int duration=0) {
	// 	navigation->changeHeading(heading, duration);
	// }

	// void setDefaultHeading() {
	// 	navigation->setDefaultHeading();
	// }

	// double getHeading() {
	// 	return navigation->getHeading();
	// }

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

	// Viewing direction function : 1 move, 0 stop.
	void turnRight(int);
	void turnLeft(int);
	void turnUp(int);
	void turnDown(int);
	void zoomIn(int);
	void zoomOut(int);
	void raiseHeight(int);
	void lowerHeight(int);

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
	void unSelect(void) {
		selected_object=nullptr;
		old_selected_object=nullptr;
		ssystem->setSelected(Object());
	}

	void unsetSelectedConstellation(std::string constellation) {
		asterisms->unsetSelected(constellation);
	}

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

	bool getStarEarthEquPosition(int HP, double &az, double &alt) {
		Object star = hip_stars->searchHP(HP).get();
		if (star) {
			Vec3d earthEqu = star.getEarthEquPos(navigation);
			Utility::rectToSphe(&az, &alt, earthEqu);
			return true;
		}
		return false;
	}

	//! Get a 1 line string briefly describing the currently selected object
	std::string getSelectedObjectShortInfo(void) const {
		return selected_object.getShortInfoString(navigation);
	}

	//! Get a 1 line string briefly describing the currently NAV edition selected object
	std::string getSelectedObjectShortInfoNav(void) const {
		return selected_object.getShortInfoNavString(navigation, timeMgr, observatory);
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
	float getPlanetsSizeLimit(void) const {
		return (ssystem->getSizeLimit()-starGetSizeLimit());
	}

	std::string getPlanetHashString(void);
	//Fonctions ci dessus et dessous définit dans core.cpp
	bool setHomePlanet(const std::string &planet);

	//! Ajoute year année(s) et month mois à la date actuelle sans toucher aux autres paramètres de la date
	void setJDayRelative(int year, int month);

	// for adding planets
	void addSolarSystemBody(stringHash_t& param);
	void removeSolarSystemBody(const std::string& name);
	void removeSupplementalSolarSystemBodies();

	//! set flag to display generic Hint or specific DSO type
	void setDsoPictograms (bool value) {
		nebulas->setDisplaySpecificHint(value);
	}
	//! get flag to display generic Hint or specific DSO type
	bool getDsoPictograms () {
		return nebulas->getDisplaySpecificHint();
	}

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
	void setLightPollutionLimitingMagnitude(float mag) {
		lightPollutionLimitingMagnitude = mag;
		float ln = log(mag);
		float lum = 30.0842967491175 -19.9408790405749*ln +2.12969160094949*ln*ln - .2206;
		atmosphere->setLightPollutionLuminance(lum);
	}
	//! Get light pollution limiting magnitude
	float getLightPollutionLimitingMagnitude(void) const {
		return lightPollutionLimitingMagnitude;
	}

	void selectZodiac();

	///////////////////////////////////////////////////////////////////////////////////////
	// Others
	//! Load color scheme from the given ini file and section name
	void setColorScheme(const std::string& skinFile, const std::string& section);

	//! Load font scheme from ini file
	void setFontScheme(void);

	// MAJ de l'UBO ubo_cam
	void uboCamUpdate();

	void setFlagNav(bool a) {
		flagNav=a;
		cardinals_points->setInternalNav(a);
		skyGridMgr->setInternalNav(a);
		skyLineMgr->setInternalNav(a);
	}

	bool getFlagNav() {
		return flagNav;
	}

	void switchMode(const std::string &mode);

	void saveCurrentConfig(InitParser &conf);

	// void imageDraw();
	void textDraw();

	void onAltitudeChange(double value) {
		std::cout << "Modification altitude reçue "<< value << std::endl;
		setBodyDecor();
	}

	void onObserverChange(std::string str) {
		std::cout << "Modification observer to " << str << std::endl;
		setLandscapeToBody();
	}

private:
	struct ViewZoomMove {
		double deltaAlt, deltaAz, deltaFov, deltaHeight;	// View movement
		double move_speed, zoom_speed;		// Speed of movement and zooming
	};

	void ssystemComputePreDraw();
	void atmosphereComputeColor(Vec3d sunPos, Vec3d moonPos);
	void hipStarMgrPreDraw();

	//! Execute all the drawing functions in solarsystem mode
	//! @param delta_time the time increment in ms.
	void drawInSolarSystem(int delta_time);

	//! Execute all the drawing functions in galaxy mode
	//! @param delta_time the time increment in ms.
	void drawInGalaxy(int delta_time);

	//! Execute all the drawing functions in universe mode
	//! @param delta_time the time increment in ms.
	void drawInUniverse(int delta_time);

	void applyClippingPlanes(float clipping_min, float clipping_max); 
	//void postDraw();

	//! Update all the objects in solarsystem mode with respect to the time.
	//! @param delta_time the time increment in ms.
	void updateInSolarSystem(int delta_time);

	//! Update all the objects in galaxy mode with respect to the time.
	//! @param delta_time the time increment in ms.
	void updateInGalaxy(int delta_time);

	//! Update all the objects in universe mode with respect to the time.
	//! @param delta_time the time increment in ms.
	void updateInUniverse(int delta_time);

	//! Callback to record actions
	mBoost::callback<void, std::string> recordActionCallback;

	//! Select passed object
	//! @return true if the object was selected (false if the same was already selected)
	bool selectObject(const Object &obj);

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
	void initCoreFont() const;

	// adpate le landscape automatiquement au body sélectionné
	void setLandscapeToBody();

	//valide les décisions d'affichage des décors du body
	void setBodyDecor();

	// vérifie si le landsacpe est compatible avec le mode auto
	void testLandscapeCompatibleWithAutoMode();

	std::string skyCultureDir;			// The directory containing data for the culture used for constellations, etc..
	Translator skyTranslator;			// The translator used for astronomical object naming

	CoreExecutor* currentExecutor = nullptr;
	CoreExecutor* executorInSolarSystem = nullptr;
	CoreExecutor* executorInGalaxy = nullptr;
	CoreExecutor* executorInUniverse = nullptr;

	CoreFont* coreFont=nullptr;					// gestion complète des fontes du logiciel
	// Main elements of the program
	Navigator * navigation;				// Manage all navigation parameters, coordinate transformations etc..
	TimeMgr* timeMgr;				// Manage date and time
	Observer *observatory;			// Manage observer position it's a pointer to the 3 other Observer
	Observer *obsSolarSystem;		// it's the historical Observer: so nothing to do with it now.
	Observer *obsInGalaxy;			//observer that should be used in InGalaxy mode
	Observer *ObsInUnivers;			//observer that should be used in InUnivers mode
	Projector * projection;				// Manage the projection mode and matrix
	Object selected_object;			// The selected object
	Object old_selected_object;		// The old selected object
	HipStarMgr * hip_stars;		// Manage the hipparcos stars
	ConstellationMgr * asterisms;		// Manage constellations (boundaries, names etc..)
	NebulaMgr * nebulas;				// Manage the nebulas
	IlluminateMgr * illuminates;		// Manage the illuminations
	TextMgr * text_usr;				// manage all user text in dome
	SolarSystem* ssystem;				// Manage the solar system
	Atmosphere * atmosphere;			// Atmosphere
	Media* media;
	SkyGridMgr * skyGridMgr;			//! gestionnaire des grilles
	SkyLineMgr* skyLineMgr;				//! gestionnaire de lignes
	SkyDisplayMgr* skyDisplayMgr; 		//! gestionnaire de skyDisplay
	Oort * oort;						//! oort cloud
	Dso3d *dso3d;						//! dso catalog for in_galaxy
	Tully *tully;						//! tully galaxies
	Cardinals * cardinals_points;		// Cardinals points
	MilkyWay * milky_way;				// Our galaxy
	MeteorMgr * meteors;				// Manage meteor showers
	Landscape * landscape;				// The landscape ie the fog, the ground and "decor"
	ToneReproductor * tone_converter;	// Tones conversion between simulation world and display device
	SkyLocalizer *skyloc;				// for sky cultures and locales
	BodyTrace * bodytrace;				// the pen bodytrace
	StarNavigator* starNav; 			// permet le voyage dans les étoiles
	StarLines* starLines;				// permet de tracer des lignes dans la galaxie
	OjmMgr * ojmMgr;					// représente les obj3D 
	UBOCam* ubo_cam;
	GeodesicGrid* geodesic_grid;
	BodyDecor* bodyDecor = nullptr;
	AnchorManager * anchorManager=nullptr;

	float sky_brightness;				// Current sky Brightness in ?
	bool object_pointer_visibility;		// Should selected object pointer be drawn
	bool autoLandscapeMode=true;		// Define if we use customised landscape or none
	bool FlagEnableZoomKeys;
	bool FlagEnableMoveKeys;
	bool FlagAtmosphericRefraction = false;
	bool flagNav = false; 				// define the NAV version edition
	bool FlagManualZoom;				// Define whether auto zoom can go further
	bool firstTime= true;               // For init to track if reload or first time setup
	ViewZoomMove vzm;					// var for ViewZoomMove
	float InitFov;						// Default viewing FOV
	Vec3d InitViewPos;					// Default viewing direction
	float auto_move_duration;			// Duration of movement for the auto move to a selected objectin seconds
	float lightPollutionLimitingMagnitude;  // Defined naked eye limiting magnitude (due to light pollution)

	InitialValue initialvalue;			// variable used to remember various string indicators in use
};

#endif // _CORE_H_
