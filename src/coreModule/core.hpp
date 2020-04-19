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
#include "coreModule/mCity_mgr.hpp"
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
#include "coreModule/text_mgr.hpp"
#include "coreModule/time_mgr.hpp"
#include "coreModule/tully.hpp"
#include "coreModule/ubo_cam.hpp"
#include "navModule/anchor_manager.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "starModule/geodesic_grid.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "tools/app_settings.hpp"
#include "tools/init_parser.hpp"
//#include "tools/io.hpp"
#include "tools/object.hpp"
#include "tools/shader.hpp"
#include "tools/sky_localizer.hpp"
#include "tools/stateGL.hpp"
#include "tools/tone_reproductor.hpp"
#include "tools/utility.hpp"

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
class Core {
public:
	friend class CoreExecutor;
	friend class CoreExecutorInSolarSystem;
	friend class CoreExecutorInGalaxy;
	friend class CoreExecutorInUniverse;

	friend class CoreLink;
	friend class CoreFont;
	friend class CoreBackup;

	//! Possible mount modes
	enum MOUNT_MODE { MOUNT_ALTAZIMUTAL, MOUNT_EQUATORIAL };

	//! Inputs are the locale directory and root directory and callback function for recording actions
	Core(AppSettings* _settings, int width, int height, Media* _media, const mBoost::callback <void, std::string> & recordCallback);
	virtual ~Core();
	Core(Core const &) = delete;
	Core& operator = (Core const &) = delete;

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

	// Set mouse position
        // void setMouse(int x, int y);

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
		navigation->updateMove(projection, delta_az, delta_alt, projection->getFov(), duration);
	}


	void setViewOffset(double offset);

	double getViewOffset() {
		return navigation->getViewOffset();
	}

	//! set environment rotation around observer
	void setHeading(double heading, int duration=0) {
		navigation->changeHeading(heading, duration);
	}

	void setDefaultHeading() {
		navigation->setDefaultHeading();
	}

	double getHeading() {
		return navigation->getHeading();
	}

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
	std::vector<std::string> listMatchingObjectsI18n(const std::string& objPrefix, unsigned int maxNbItem=5) const;

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

	//void tcpGetSelectedObjectInfo() const;

	void getDeRa(double *ra, double *de) const {
		selected_object.getRaDeValue(navigation,ra,de);
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
	// Rendering settings

	// //! Set rendering flag of antialiased lines
	// void setFlagAntialiasLines(bool b) {
	// 	FlagAntialiasLines = b;

	// 	if(b) glEnable(GL_LINE_SMOOTH);
	// 	else glDisable(GL_LINE_SMOOTH);
	// }
	// //! Get display flag of constellation lines
	// bool getFlagAntialiasLines(void) {
	// 	return FlagAntialiasLines;
	// }

	// void setLineWidth(float w) {
	// 	m_lineWidth = w;
	// }
	// float getLineWidth() {
	// 	return m_lineWidth;
	// }

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
	std::string addSolarSystemBody(stringHash_t& param);
	std::string removeSolarSystemBody(std::string name);
	std::string removeSupplementalSolarSystemBodies();

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
	void printHorizontal(s_font* font, float altitude, float azimuth, const std::string& str, Vec3f textColor , bool cache = true/*, int justify = 0, bool outline */) const {
		font->printHorizontal(projection, altitude, azimuth, str, textColor, cache);//, /*justify, cache, outline, 0, 0*/);
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



	///////////////////////////////////////////////////////////////////////////////////////
	// Observer
	//! Return the current observatory (as a const object)
	// made non const so can track when save data!  Hmmm. 20070215
	// TODO resolve issue
	// Observer* getObservatory(void) {
	// 	return observatory;
	// }

	// //! Move to a new latitude and longitude on home planet
	// void moveObserver(double lat, double lon, double alt, int delay /*, const std::string& name*/) {
	// 	observatory->moveTo(lat, lon, alt, delay/*, name*/);
	// }

	// //! Move to relative latitude where home planet is fixed.
	// void moveRelLatObserver(double lat, int delay) {
	// 	double latimem=observatory->getLatitude()+lat;
	// 	if (latimem>90) latimem=90;
	// 	if (latimem<-90) latimem=-90;
	// 	moveObserver(latimem,observatory->getLongitude(),observatory->getAltitude(),delay/*,observatory->getName()*/);
	// }

	// //! Move to relative longitude where home planet is fixed.
	// void moveRelLonObserver(double lon, int delay) {
	// 	moveObserver(observatory->getLatitude(),observatory->getLongitude()+lon,observatory->getAltitude(),delay/*,observatory->getName()*/);
	// }

	// //! Move to relative altitude where home planet is fixed.
	// void moveRelAltObserver(double alt, int delay) {
	// 	moveObserver(observatory->getLatitude(),observatory->getLongitude(),observatory->getAltitude()+alt,delay/*,observatory->getName()*/);
	// }

	// //! change the Heading value
	// void moveHeadingRelative(float f) {
	// 	navigation->setHeading(navigation->getHeading() + f);
	// }

	// //! Set Meteor Rate in number per hour
	// void setMeteorsRate(int f) {
	// 	meteors->setZHR(f);
	// }

	// //! Get Meteor Rate in number per hour
	// int getMeteorsRate(void) const {
	// 	return meteors->getZHR();
	// }

	void selectZodiac();

	///////////////////////////////////////////////////////////////////////////////////////
	// Others
	//! Load color scheme from the given ini file and section name
	void setColorScheme(const std::string& skinFile, const std::string& section);

	//! Load font scheme from ini file
	void setFontScheme(void);

	void loadFont(int baseSize, const std::string name);


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

	// void getmBackup();					//! get the current variables to struct Backup
	// void setmBackup();					//! set a previous Backup  directly in use
	
	void switchMode(const std::string &mode);


	//tcp
	// void tcpConfigure(ServerSocket * _tcp);
	// void tcpGetStatus(std::string value) const;
	// void tcpGetPlanetsStatus() const;

	//! return tcpPosition
	// void tcpGetPosition();
	
	////////////////////////////////////////////////////////////////////////////////
	// Atmosphere---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Atmosphere
	void atmosphereSetFlag(bool b) {
		bodyDecor->setAtmosphereState(b);
	}
	//! Get flag for displaying Atmosphere
	bool atmosphereGetFlag(void) const {
		return bodyDecor->getAtmosphereState();
	}

	//! set flag for vp Optoma
	void atmosphereSetFlagOptoma(bool b) {
		atmosphere->setFlagOptoma(b);
	}

	//! Get flag for vp Optoma
	bool atmosphereGetFlagOptoma(void) const {
		return atmosphere->getFlagOptoma();
	}

	//! Set atmosphere fade duration in s
	void atmosphereSetFadeDuration(float f) {
		atmosphere->setFaderDuration(f);
	}

	//! Get atmosphere fade duration in s
	float atmosphereGetFadeDuration(void) const {
		return atmosphere->getFaderDuration();
	}

	//! Set flag for activating atmospheric refraction correction
	void atmosphericRefractionSetFlag(bool b) {
		FlagAtmosphericRefraction = b;
	}

	//! Get flag for activating atmospheric refraction correction
	bool atmosphericRefractionGetFlag(void) const {
		return FlagAtmosphericRefraction;
	}

	void saveCurrentConfig(InitParser &conf);
	Vec3f getCursorPosEqu(int x, int y);

	// void imageDraw();
	void textDraw();

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

	//! envoie directement une chaine de caractère au serveur TCP
	// void tcpSend(std::string msg ) const;

	AppSettings * settings;				//! endroit unique pour les chemins des fichiers dans l'application
	// ServerSocket *tcp;
	// bool enable_tcp ;

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
	Observer * observatory;			// Manage observer position
	Projector * projection;				// Manage the projection mode and matrix
	Object selected_object;			// The selected object
	// int mouseX;
	// int mouseY;
	Object old_selected_object;		// The old selected object
	class HipStarMgr * hip_stars;		// Manage the hipparcos stars
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
	Landscape * inactiveLandscape;			// Background landscape to switch smoothly between landscapes
	ToneReproductor * tone_converter;	// Tones conversion between simulation world and display device
	SkyLocalizer *skyloc;				// for sky cultures and locales
	BodyTrace * bodytrace;				// the pen bodytrace
	StarNavigator* starNav; 			// permet le voyage dans les étoiles
	StarLines* starLines;				// permet de tracer des lignes dans la galaxie
	OjmMgr * ojmMgr;					// représente les obj3D 
	mCity_Mgr	*mCity;					//!for using this class MUST BE PRIVATE
	UBOCam* ubo_cam;
	GeodesicGrid* geodesic_grid;
	BodyDecor* bodyDecor = nullptr;
	AnchorManager * anchorManager=nullptr;

	float sky_brightness;				// Current sky Brightness in ?
	bool object_pointer_visibility;		// Should selected object pointer be drawn
	std::string getCursorPos(int x, int y);  //not used now

	// Increment/decrement smoothly the vision field and position
	void updateMove(int delta_time);
	bool FlagEnableZoomKeys;
	bool FlagEnableMoveKeys;
	bool FlagAtmosphericRefraction = false;
	bool flagNav = false; 				// define the NAV version edition
	// bool FlagAntialiasLines;            // whether to antialias all line drawing
	bool FlagManualZoom;				// Define whether auto zoom can go further
	bool firstTime= true;               // For init to track if reload or first time setup
	std::string defaultLandscape; 
	std::string tempLandscape; 
	ViewZoomMove vzm;					// var for ViewZoomMove
	float InitFov;						// Default viewing FOV
	Vec3d InitViewPos;					// Default viewing direction
	float auto_move_duration;			// Duration of movement for the auto move to a selected objectin seconds
	// float m_lineWidth;                  // width to use when drawing any line

	//! Backup Manage
	//backupWorkspace mBackup;			// variable used to remember various indicators in use
	InitialValue initialvalue;			// variable used to remember various string indicators in use

	// void inimBackup();					// init at NULL all var
	float lightPollutionLimitingMagnitude;  // Defined naked eye limiting magnitude (due to light pollution)
};

#endif // _CORE_H_
