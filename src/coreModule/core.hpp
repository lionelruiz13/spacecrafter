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
#include "coreModule/skyperson.hpp"
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
#include "tools/io.hpp"
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
        void setMouse(int x, int y);

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
		printf("Culture %s\n",mBackup.initial_skyCulture.c_str());
		setSkyCultureDir(mBackup.initial_skyCulture);
	}

	void setInitialSkyLocale() {
		printf("Locale %s\n",mBackup.initial_skyLocale.c_str());
		setSkyLanguage(mBackup.initial_skyLocale);
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
	void setAutomoveDuration(float f) {
		auto_move_duration = f;
	}
	//! Get automove duration in seconds
	float getAutomoveDuration(void) const {
		return auto_move_duration;
	}

	//! Zoom to the given FOV (in degree)
	void zoomTo(double aim_fov, float move_duration = 1.) {
		projection->zoomTo(aim_fov, move_duration);
	}

	//! Get current FOV (in degree)
	float getFov(void) const {
		return projection->getFov();
	}

	//! If is currently zooming, return the target FOV, otherwise return current FOV
	double getAimFov(void) const {
		return projection->getAimFov();
	}

	//! Set the current FOV (in degree)
	void setFov(double f) {
		projection->setFov(f);
	}

	//! Set the maximum FOV (in degree)
	void setMaxFov(double f) {
		projection->setMaxFov(f);
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

	void tcpGetListMatchingObjects(const std::string& objPrefix, unsigned int maxNbItem=5) const;

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

	void tcpGetSelectedObjectInfo() const;

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

	//! Set rendering flag of antialiased lines
	void setFlagAntialiasLines(bool b) {
		FlagAntialiasLines = b;

		if(b) glEnable(GL_LINE_SMOOTH);
		else glDisable(GL_LINE_SMOOTH);
	}
	//! Get display flag of constellation lines
	bool getFlagAntialiasLines(void) {
		return FlagAntialiasLines;
	}

	void setLineWidth(float w) {
		m_lineWidth = w;
	}
	float getLineWidth() {
		return m_lineWidth;
	}

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

	///////////////////////////////////////////////////////////////////////////////////////
	// Planets flags

	void setFlagLightTravelTime(bool b) {
		ssystem->setFlagLightTravelTime(b);
	}
	bool getFlagLightTravelTime(void) const {
		return ssystem->getFlagLightTravelTime();
	}

	//! Start/stop displaying planets Trails
	void startPlanetsTrails(bool b) {
		ssystem->startTrails(b);
	}

	//! Set selected planets by englishName
	//! @param englishName The planet name or "" to select no planet
	void setPlanetsSelected(const std::string& englishName) {
		ssystem->setSelected(englishName);
	}

	std::string getPlanetHashString(void);

	bool setHomePlanet(const std::string &planet);

	//! Set flag for displaying a scaled Moon
	void setFlagMoonScaled(bool b) {
		ssystem->setFlagMoonScale(b);
	}
	//! Get flag for displaying a scaled Moon
	bool getFlagMoonScaled(void) const {
		return ssystem->getFlagMoonScale();
	}

	//! Set flag for displaying a scaled Sun
	void setFlagSunScaled(bool b) {
		ssystem->setFlagSunScale(b);
	}
	//! Get flag for displaying a scaled Sun
	bool getFlagSunScaled(void) const {
		return ssystem->getFlagSunScale();
	}

	//! Set Moon scale
	void setMoonScale(float f, bool resident = false) {
		if (f<0) ssystem->setMoonScale(1., false);
		else ssystem->setMoonScale(f, resident);
	}
	//! Get Moon scale
	float getMoonScale(void) const {
		return ssystem->getMoonScale();
	}

	//! Set Sun scale
	void setSunScale(float f, bool resident = false) {
		if (f<0) ssystem->setSunScale(1., false);
		else ssystem->setSunScale(f, resident);
	}
	//! Get Moon scale
	float getSunScale(void) const {
		return ssystem->getSunScale();
	}


	//! Set flag for displaying clouds (planet rendering feature)
	void setFlagClouds(bool b) {
		ssystem->setFlagClouds(b);
	}
	//! Get flag for displaying Atmosphere
	bool getFlagClouds(void) const {
		return ssystem->getFlagClouds();
	}

	//! Ajoute year année(s) et month mois à la date actuelle sans toucher aux autres paramètres de la date
	void setJDayRelative(int year, int month);

	// for adding planets
	std::string addSolarSystemBody(stringHash_t& param);

	std::string removeSolarSystemBody(std::string name);

	std::string removeSupplementalSolarSystemBodies();

	void initialSolarSystemBodies() {
		return ssystem->initialSolarSystemBodies();
	}

	//cache une planete
	void setPlanetHidden(std::string name, bool planethidden) {
		ssystem->setPlanetHidden(name, planethidden);
	}

	//indique si la planete est visible 1 ou pas 0
	bool getPlanetHidden(std::string name) {
		return ssystem->getPlanetHidden(name);
	}

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
	std::string removeNebula(const std::string& name);

	//! remove all user added nebulae
	std::string removeSupplementalNebulae();

	///////////////////////////////////////////////////////////////////////////////////////
	// Projection

	//! Print the passed string so that it is oriented in the drection of the gravity
	void printHorizontal(s_font* font, float altitude, float azimuth, const std::string& str, Vec3f textColor, bool outline = 1, /*int justify = 0,*/ bool cache = 0) const {
		font->printHorizontal(projection, altitude, azimuth, str, textColor, /*justify,*/ cache, outline/*, 0, 0*/);

	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Landscape
	void setInitialLandscapeName() {
		setLandscape(mBackup.initial_landscapeName);
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
	Observer* getObservatory(void) {
		return observatory;
	}

	//! Move to a new latitude and longitude on home planet
	void moveObserver(double lat, double lon, double alt, int delay /*, const std::string& name*/) {
		observatory->moveTo(lat, lon, alt, delay/*, name*/);
	}

	//! Move to relative latitude where home planet is fixed.
	void moveRelLatObserver(double lat, int delay) {
		double latimem=observatory->getLatitude()+lat;
		if (latimem>90) latimem=90;
		if (latimem<-90) latimem=-90;
		moveObserver(latimem,observatory->getLongitude(),observatory->getAltitude(),delay/*,observatory->getName()*/);
	}

	//! Move to relative longitude where home planet is fixed.
	void moveRelLonObserver(double lon, int delay) {
		moveObserver(observatory->getLatitude(),observatory->getLongitude()+lon,observatory->getAltitude(),delay/*,observatory->getName()*/);
	}

	//! Move to relative altitude where home planet is fixed.
	void moveRelAltObserver(double alt, int delay) {
		moveObserver(observatory->getLatitude(),observatory->getLongitude(),observatory->getAltitude()+alt,delay/*,observatory->getName()*/);
	}

	//! change the Heading value
	void moveHeadingRelative(float f) {
		navigation->setHeading(navigation->getHeading() + f);
	}

	//! Set Meteor Rate in number per hour
	void setMeteorsRate(int f) {
		meteors->setZHR(f);
	}

	//! Get Meteor Rate in number per hour
	int getMeteorsRate(void) const {
		return meteors->getZHR();
	}

	void selectZodiac();

	///////////////////////////////////////////////////////////////////////////////////////
	// Others
	//! Load color scheme from the given ini file and section name
	void setColorScheme(const std::string& skinFile, const std::string& section);

	//! Load font scheme from ini file
	void setFontScheme(void);

	double getZoomSpeed() {
		return vzm.zoom_speed;
	}
	float getAutoMoveDuration() {
		return auto_move_duration;
	}

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

	mCity_Mgr	*mCity;					//!for using this class MUST BE PRIVATE

	void getmBackup();					//! get the current variables to struct Backup
	void setmBackup();					//! set a previous Backup  directly in use
	
	void switchMode(const std::string &mode);


	//tcp
	void tcpConfigure(ServerSocket * _tcp);
	void tcpGetStatus(std::string value) const;
	void tcpGetPlanetsStatus() const;

	//! return tcpPosition
	void tcpGetPosition();
	
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

	////////////////////////////////////////////////////////////////////////////////
	// Body---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void BodyOJMLoad(const std::string &mode, const std::string &name, const std::string &filename, const std::string &pathFile, const Vec3f &Position, const float multiplier) {
		ojmMgr->load(mode, name, filename, pathFile, Position, multiplier);
	}

	void BodyOJMRemove(const std::string &mode, const std::string &name){
		ojmMgr->remove(mode, name);
	}

	void BodyOJMRemoveAll(const std::string &mode){
		ojmMgr->removeAll(mode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Camera---------------------------
	////////////////////////////////////////////////////////////////////////////////
	
	bool cameraAddAnchor(stringHash_t& param) {
		return anchorManager->addAnchor(param); 
	}

	bool cameraRemoveAnchor(const std::string &name) {
		return anchorManager->removeAnchor(name);
	}

	bool cameraSwitchToAnchor(const std::string &name) {
		return anchorManager->switchToAnchor(name);
	}

	bool cameraMoveToPoint(double x, double y, double z){
		return anchorManager->setCurrentAnchorPos(Vec3d(x,y,z));
	}
	
	bool cameraMoveToPoint(double x, double y, double z, double time){
		return anchorManager->moveTo(Vec3d(x,y,z),time);
	}
	
	bool cameraMoveToBody(const std::string& bodyName, double time, double alt = -1.0){

		if(bodyName == "selected"){
			return anchorManager->moveToBody(getSelectedPlanetEnglishName(), time, alt);
		}

		if(bodyName == "default"){
			return anchorManager->moveToBody(ssystem->getEarth()->getEnglishName(), time, alt);
		}

		return anchorManager->moveToBody(bodyName,time, alt);
	}
	
	bool cameraMoveRelativeXYZ( double x, double y, double z) {
		return anchorManager->moveRelativeXYZ(x,y,z);
	}
	
	bool cameraTransitionToPoint(const std::string& name){
		return anchorManager->transitionToPoint(name);
	}
	
	bool cameraTransitionToBody(const std::string& name){

		if(name == "selected"){
			return anchorManager->transitionToBody(getSelectedPlanetEnglishName());
		}

		return anchorManager->transitionToBody(name);
	}

	bool cameraSave(const std::string& name = "anchor"){
		return anchorManager->saveCameraPosition(settings->getUserDir() + "anchors/" + name);
	}
	
	bool loadCameraPosition(const std::string& filename){
		return anchorManager->loadCameraPosition(settings->getUserDir() + "anchors/" + filename);
	}
	
	bool lookAt(double az, double alt, double time = 1.){
		return navigation->lookAt(az, alt, time);
	}
	
	bool cameraSetFollowRotation(const std::string& name, bool value){
		return anchorManager->setFollowRotation(value);
	}

	void cameraSetRotationMultiplierCondition(float v) {
		anchorManager->setRotationMultiplierCondition(v);
	}

	bool cameraAlignWithBody(const std::string& name, double duration){
		return anchorManager->alignCameraToBody(name,duration);
	}

	////////////////////////////////////////////////////////////////////////////////
	// CardinalsPoints---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Cardinals Points
	void cardinalsPointsSetFlag(bool b) {
		cardinals_points->setFlagShow(b);
	}
	//! Get flag for displaying Cardinals Points
	bool cardinalsPointsGetFlag(void) const {
		return cardinals_points->getFlagShow();
	}

	//! Set Cardinals Points color
	void cardinalsPointsSetColor(const Vec3f& v) {
		cardinals_points->setColor(v);
	}
	//! Get Cardinals Points color
	Vec3f cardinalsPointsGetColor(void) const {
		return cardinals_points->getColor();
	}


	////////////////////////////////////////////////////////////////////////////////
	// Constellations---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set display flag of constellation lines
	void constellationSetFlagLines(bool b) {
		asterisms->setFlagLines(b);
	}
	//! Get display flag of constellation lines
	bool constellationGetFlagLines(void) {
		return asterisms->getFlagLines();
	}

	//! Set display flag of constellation art
	void constellationSetFlagArt(bool b) {
		asterisms->setFlagArt(b);
	}
	//! Get display flag of constellation art
	bool constellationGetFlagArt(void) {
		return asterisms->getFlagArt();
	}

	//! Set display flag of constellation names
	void constellationSetFlagNames(bool b) {
		asterisms->setFlagNames(b);
	}
	//! Get display flag of constellation names
	bool constellationGetFlagNames(void) {
		return asterisms->getFlagNames();
	}

	//! Set display flag of constellation boundaries
	void constellationSetFlagBoundaries(bool b) {
		asterisms->setFlagBoundaries(b);
	}
	//! Get display flag of constellation boundaries
	bool constellationGetFlagBoundaries(void) {
		return asterisms->getFlagBoundaries();
	}
	Vec3f constellationGetColorBoundaries(void) const {
		return asterisms->getBoundaryColor();
	}

	//! Set constellation art intensity
	void constellationSetArtIntensity(float f) {
		asterisms->setArtIntensity(f);
	}
	//! Get constellation art intensity
	float constellationGetArtIntensity(void) const {
		return asterisms->getArtIntensity();
	}

	//! Set constellation art intensity
	void constellationSetArtFadeDuration(float f) {
		asterisms->setArtFadeDuration(f);
	}
	//! Get constellation art intensity
	float constellationGetArtFadeDuration(void) const {
		return asterisms->getArtFadeDuration();
	}

	//! Set whether selected constellation is drawn alone
	void constellationSetFlagIsolateSelected(bool b) {
		asterisms->setFlagIsolateSelected(b);
	}

	//! Get whether selected constellation is drawn alone
	bool constellationGetFlagIsolateSelected(void) {
		return asterisms->getFlagIsolateSelected();
	}

	//! Set whether to draw the names for the selected stars or every star
	void starSetFlagIsolateSelected(bool b) {
		return hip_stars->setFlagIsolateSelected(b);
	}

	//! Get whether to draw the names for the selected stars or every star
	bool starGetFlagIsolateSelected(void) {
		return hip_stars->getFlagIsolateSelected();
	}

	//! Get constellation line color
	Vec3f constellationGetColorLine() const {
		return asterisms->getLineColor();
	}
	//! Set constellation line color
	void constellationSetColorLine(const Vec3f& v) {
		asterisms->setLineColor(v);
	}

	//! Get constellation names color
	Vec3f constellationGetColorNames() const {
		return asterisms->getLabelColor();
	}
	//! Set constellation names color
	void constellationSetColorNames(const Vec3f& v) {
		asterisms->setLabelColor(v);
	}

	//! Set constellation names color
	void constellationSetColorNames(const std::string &argName, const Vec3f& v) {
		asterisms->setLabelColor(argName, v);
	}

	//! Get constellation art color
	Vec3f constellationGetColorArt() const {
		return asterisms->getArtColor();
	}
	//! Set constellation line color
	void constellationSetColorArt(const Vec3f& v) {
		asterisms->setArtColor(v);
	}

	void constellationSetColorBoundaries(const Vec3f& v) {
		asterisms->setBoundaryColor(v);
	}

	void constellationSetLineColor(const std::string &argName, const Vec3f& v) {
		asterisms->setLineColor(argName, v);
	}

	void constellationSetArtIntensity(const std::string &argName, float intensity) {
		asterisms->setArtIntensity(argName, intensity);
	}

	////////////////////////////////////////////////////////////////////////////////
	// dateSun---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! return the JD time when the sun go down
	double dateSunRise(double _jd, double _longitude, double _latitude) {
		return timeMgr->dateSunRise(_jd,_longitude, _latitude);
	}

	//! return the JD time when the sun set up
	double dateSunSet(double _jd, double _longitude, double _latitude) {
		return timeMgr->dateSunSet(_jd,_longitude, _latitude);
	}

	//! return the JD time when the sun cross the meridian
	double dateSunMeridian(double _jd, double _longitude, double _latitude) {
		return timeMgr->dateSunMeridian(_jd,_longitude, _latitude);
	}


	////////////////////////////////////////////////////////////////////////////////
	// DSO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! hide a particular DSO
	void dsoSelectName(std::string DSOName, bool hide) const {
		return nebulas->selectName(hide, DSOName);
	}

	//! hide all DSO
	void dsoHideAll() const {
		nebulas->hideAll();
	}

	//! show (unhide) all DSO
	void dsoShowAll() const {
		nebulas->showAll();
	}

	//! select all DSO in constellationName to be hidden or showed
	void dsoSelectConstellation(bool hide, std::string constellationName) const {
		nebulas->selectConstellation(hide, constellationName);
	}

	//! select all DSO with typeName to be hidden or showed
	void dsoSelectType(bool hide, std::string typeName) const {
		nebulas->selectType(hide, typeName);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Fog---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Fog
	void fogSetFlag(bool b) {
		landscape->setFlagShowFog(b);
	}
	//! Get flag for displaying Fog
	bool fogGetFlag(void) const {
		return landscape->getFlagShowFog();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Illuminate---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool illuminateLoad(std::string filename, double ra, double de, double angular_size, std::string name, double r, double g, double b, float rotation);

	std::string illuminateRemove(const std::string& name);
	std::string illuminateRemoveAll();

	void illuminateSetSize (double value) {
		illuminate_size=value;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Landscape---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Get flag for displaying Landscape
	void landscapeSetFlag(bool b) {
		landscape->setFlagShow(b);
	}
	//! Get flag for displaying Landscape
	bool landscapeGetFlag(void) const {
		return landscape->getFlagShow();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Milky Way---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Milky Way
	void milkyWaySetFlag(bool b) {
		milky_way->setFlagShow(b);
	}
	//! Get flag for displaying Milky Way
	bool milkyWayGetFlag(void) const {
		return milky_way->getFlagShow();
	}

	//! Set flag for displaying Zodiacal Light
	void milkyWaySetFlagZodiacal(bool b) {
		milky_way->setFlagZodiacal(b);
	}
	//! Get flag for displaying Zodiacal Light
	bool milkyWayGetFlagZodiacal(void) const {
		return milky_way->getFlagZodiacal();
	}

	//! Set Milky Way intensity
	void milkyWaySetIntensity(float f) {
		milky_way->setIntensity(f);
	}
	//! Get Milky Way intensity
	float milkyWayGetIntensity(void) const {
		return milky_way->getIntensity();
	}

	void milkyWayRestoreDefault() {
		milky_way->restoreDefaultMilky();
	}

	void milkyWaySetDuration(float f) {
		milky_way->setFaderDuration(f*1000);
	}

	void milkyWayRestoreIntensity() {
		milky_way->restoreIntensity();
	}

	void milkyWayUseIris(bool useIt) {
		milky_way->useIrisTexture(useIt);
	}

	//! Change Milkyway texture
	void milkyWayChange(std::string mdir, float _intensity) {
		milky_way->changeMilkywayState(mdir, _intensity);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Nebulae---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Nebulae
	void nebulaSetFlag(bool b) {
		nebulas->setFlagShow(b);
		dso3d->setFlagShow(b);
	}
	//! Get flag for displaying Nebulae
	bool nebulaGetFlag(void) const {
		return nebulas->getFlagShow();
	}

	//! Set flag for displaying Nebulae Hints
	void nebulaSetFlagHints(bool b) {
		nebulas->setFlagHints(b);
	}
	//! Get flag for displaying Nebulae Hints
	bool nebulaGetFlagHints(void) const {
		return nebulas->getFlagHints();
	}

	//! Set Nebulae Hints circle scale
	void nebulaSetCircleScale(float f) {
		nebulas->setNebulaCircleScale(f);
	}
	//! Get Nebulae Hints circle scale
	float nebulaGetCircleScale(void) const {
		return nebulas->getNebulaCircleScale();
	}

	//! Set flag for displaying Nebulae as bright
	void nebulaSetFlagBright(bool b) {
		nebulas->setFlagBright(b);
	}
	//! Get flag for displaying Nebulae as brigth
	bool nebulaGetFlagBright(void) const {
		return nebulas->getFlagBright();
	}

	//! Set maximum magnitude at which nebulae hints are displayed
	void nebulaSetMaxMagHints(float f) {
		nebulas->setMaxMagHints(f);
	}
	//! Get maximum magnitude at which nebulae hints are displayed
	float nebulaGetMaxMagHints(void) const {
		return nebulas->getMaxMagHints();
	}

	//! return the color for the DSO object
	Vec3f nebulaGetColorLabels(void) const {
		return nebulas->getLabelColor();
	}

	//! return the color of the DSO circle
	Vec3f nebulaGetColorCircle(void) const {
		return nebulas->getCircleColor();
	}

	void nebulaSetPictoSize(int value) const {
		nebulas->setPictoSize(value);
	}

	//!set Flag DSO Name who display DSO name
	void nebulaSetFlagNames (bool value) {
		nebulas->setNebulaNames(value);
	}

	//!get flag DSO Name who display DSO name
	bool nebulaGetFlagNames () {
		return nebulas->getNebulaNames();
	}

	void nebulaSetColorLabels(const Vec3f& v) {
		nebulas->setLabelColor(v);
	}
	void nebulaSetColorCircle(const Vec3f& v) {
		nebulas->setCircleColor(v);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Observatory---------------------------
	////////////////////////////////////////////////////////////////////////////////

	std::string observatoryGetLandscapeName() {
		return observatory->getLandscapeName();
	}

	double observatoryGetLatitude() {
		return observatory->getLatitude();
	}

	double observatoryGetLongitude() {
		return observatory->getLongitude();
	}

	double observatoryGetAltitude() {
		return observatory->getAltitude();
	}

	void observatorySetLatitude(double l) {
		observatory->setLatitude(l);
	}

	void observatorySetLongitude(double l) {
		observatory->setLongitude(l);
	}

	void observatorySetAltitude(double l) {
		observatory->setAltitude(l);
	}
	
	void observatorySetSpacecraft(double l) {
		observatory->setSpacecraft(bool(l));
	}

	void observatorySaveBodyInSolarSystem() {
		observatory->saveBodyInSolarSystem();
	}

	void observatoryLoadBodyInSolarSystem() {
		observatory->loadBodyInSolarSystem();
	}

	void observatoryFixBodyToSun() {
		observatory->fixBodyToSun();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Oort    ---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool oortGetFlagShow() {
		return oort->getFlagShow();
	}

	void oortSetFlagShow(bool b) {
		oort->setFlagShow(b);
	}

	////////////////////////////////////////////////////////////////////////////////
	// PersonXX---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Personalized Azimuthal Line
	void personalSetFlag(bool b) {
		personal->setFlagShow(b);
	}
	//! Get flag for displaying Personalized Azimuthal Line
	bool personalGetFlag(void) const {
		return personal->getFlagShow();
	}
	Vec3f personalGetColor(void) const {
		return personal->getColor();
	}

	void personalLoad(std::string filename) const {
		personal->loadData(filename);
	}

	void personalClear() const {
		personal->clear();
	};

	//! Set flag for displaying Personalized Equatorial Line
	void personeqSetFlag(bool b) {
		personeq->setFlagShow(b);
	}
	//! Get flag for displaying Personalized Equatorial Line
	bool personeqGetFlag(void) const {
		return personeq->getFlagShow();
	}
	Vec3f personeqGetColor(void) const {
		return personeq->getColor();
	}

	void personeqLoad(std::string filename) const {
		personeq->loadData(filename);
	}

	void personeqClear() const {
		personeq->clear();
	};
	
	//! Set flag for displaying Nautical Azimuthal Line
	void nauticalSetFlag(bool b) {
		nautical->setFlagShow(b);
	}
	//! Get flag for displaying Nautical Azimuthal Line
	bool nauticalGetFlag(void) const {
		return nautical->getFlagShow();
	}
	Vec3f nauticalGetColor(void) const {
		return nautical->getColor();
	}

	//! Set flag for displaying Nautical Equatorial Line
	void nauticeqSetFlag(bool b) {
		nauticeq->setFlagShow(b);
	}
	//! Get flag for displaying Nautical Equatorial Line
	bool nauticeqGetFlag(void) const {
		return nauticeq->getFlagShow();
	}
	Vec3f nauticeqGetColor(void) const {
		return nauticeq->getColor();
	}

	//! Set flag for displaying Mouse Position
	void objCoordSetFlag(bool b) {
		objCoord->setFlagShow(b);
	}
	//! Get flag for displaying Mouse Position
	bool objCoordGetFlag(void) const {
		return objCoord->getFlagShow();
	}
	Vec3f objCoordGetColor(void) const {
		return objCoord->getColor();
	}

	//! Set flag for activating atmospheric refraction correction
	void atmosphericRefractionSetFlag(bool b) {
		FlagAtmosphericRefraction = b;
	}

	//! Get flag for activating atmospheric refraction correction
	bool atmosphericRefractionGetFlag(void) const {
		return FlagAtmosphericRefraction;
	}

	//! Set flag for displaying Mouse Position
	void mouseCoordSetFlag(bool b) {
		mouseCoord->setFlagShow(b);
	}
	//! Get flag for displaying Mouse Position
	bool mouseCoordGetFlag(void) const {
		return mouseCoord->getFlagShow();
	}

	Vec3f mouseCoordGetColor(void) const {
		return mouseCoord->getColor();
	}

	//! Set flag for displaying Angular Distance
	void angDistSetFlag(bool b) {
		angDist->setFlagShow(b);
	}
	//! Get flag for displaying Angular Distance
	bool angDistGetFlag(void) const {
		return angDist->getFlagShow();
	}
	Vec3f angDistGetColor(void) const {
		return angDist->getColor();
	}

	//! Set flag for displaying Angular Distance
	void loxodromySetFlag(bool b) {
		loxodromy->setFlagShow(b);
	}
	//! Get flag for displaying Angular Distance
	bool loxodromyGetFlag(void) const {
		return loxodromy->getFlagShow();
	}
	Vec3f loxodromyGetColor(void) const {
		return loxodromy->getColor();
	}

	//! Set flag for displaying Angular Distance
	void orthodromySetFlag(bool b) {
		orthodromy->setFlagShow(b);
	}
	//! Get flag for displaying Angular Distance
	bool orthodromyGetFlag(void) const {
		return orthodromy->getFlagShow();
	}
	Vec3f orthodromyGetColor(void) const {
		return orthodromy->getColor();
	}

	void personalSetColor(const Vec3f& v) {
		personal->setColor(v);
	}
	void personeqSetColor(const Vec3f& v) {
		personeq->setColor(v);
	}
	void nauticalSetColor(const Vec3f& v) {
		nautical->setColor(v);
	}
	void nauticeqSetColor(const Vec3f& v) {
		nauticeq->setColor(v);
	}
	void objCoordSetColor(const Vec3f& v) {
		objCoord->setColor(v);
	}
	void mouseCoordSetColor(const Vec3f& v) {
		mouseCoord->setColor(v);
	}
	void angDistSetColor(const Vec3f& v) {
		angDist->setColor(v);
	}
	void loxodromySetColor(const Vec3f& v) {
		loxodromy->setColor(v);
	}
	void orthodromySetColor(const Vec3f& v) {
		orthodromy->setColor(v);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Planets---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Planets
	void planetsSetFlag(bool b) {
		ssystem->setFlagPlanets(b);
	}
	//! Get flag for displaying Planets
	bool planetsGetFlag(void) const {
		return ssystem->getFlagPlanets();
	}

	//! Set flag for displaying Planets Trails
	void planetsSetFlagTrails(bool b) {
		ssystem->setFlagTrails(b);
	}
	//! Get flag for displaying Planets Trails
	bool planetsGetFlagTrails() const {
		return ssystem->getFlagTrails();
	}

	//! Set flag for displaying Planets Axis
	void planetsSetFlagAxis(bool b) {
		ssystem->setFlagAxis(b);
	}
	//! Get flag for displaying Planets Axis
	bool planetsGetFlagAxis(void) const {
		return ssystem->getFlagAxis();
	}


	//! Set flag for displaying Planets Hints
	void planetsSetFlagHints(bool b) {
		ssystem->setFlagHints(b);
	}
	//! Get flag for displaying Planets Hints
	bool planetsGetFlagHints(void) const {
		return ssystem->getFlagHints();
	}

	//! Set flag for displaying Planets Orbits
	void planetsSetFlagOrbits(bool b) {
		ssystem->setFlagPlanetsOrbits(b);
	}

	//! Set flag for displaying Planet name Orbit
	void planetsSetFlagOrbits(const std::string &_name, bool b) {
		ssystem->setFlagPlanetsOrbits(_name, b);
	}

	//! Switch 
	void planetSwitchTexMap(const std::string &_name, bool b) {
		if (_name=="selected") ssystem->switchPlanetTexMap(selected_object.getEnglishName(), b); 
		else ssystem->switchPlanetTexMap(_name, b);
	}

	//! Switch 
	bool planetGetSwitchTexMap(const std::string &_name) {
		if (_name=="selected") return ssystem->getSwitchPlanetTexMap(selected_object.getEnglishName());
		else return ssystem->getSwitchPlanetTexMap(_name);
	}

	void planetCreateTexSkin(const std::string &name, const std::string &texName){
		ssystem->createTexSkin(name, texName);
	}

	//! Get flag for displaying Planets Orbits
	bool planetsGetFlagOrbits(void) const {
		return ssystem->getFlagPlanetsOrbits();
	}

	//! Set flag for displaying Satellites Orbits
	void satellitesSetFlagOrbits(bool b) {
		ssystem->setFlagSatellitesOrbits(b);
	}

	//! Get flag for displaying Satellites Orbits
	bool satellitesGetFlagOrbits(void) const {
		return ssystem->getFlagSatellitesOrbits();
	}
	//! Set flag for displaying Planets & Satellites Orbits
	void planetSetFlagOrbits(bool b) {
		ssystem->setFlagSatellitesOrbits(b);
		ssystem->setFlagPlanetsOrbits(b);
		//ssystem->setFlagOrbits(b);
	}

	void planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const {
		ssystem->setBodyColor(englishName, color, c);
	}

	Vec3f planetGetColor(const std::string& englishName, const std::string& color) const {
		return ssystem->getBodyColor(englishName, color);
	}

	void planetSetDefaultColor(const std::string& color, Vec3f c) const {
		ssystem->setDefaultBodyColor(color, c);
	}

	Vec3f planetGetDefaultColor(const std::string& colorName) const {
		return ssystem->getDefaultBodyColor(colorName);
	}

	bool hideSatellitesFlag(){
		return ssystem->getHideSatellitesFlag();
	}

	void setHideSatellites(bool val){
		ssystem->toggleHideSatellites(val);
	}

	//! Set base planets display scaling factor
	void planetsSetScale(float f) {
		ssystem->setScale(f);
	}
	//! Get base planets display scaling factor
	float planetsGetScale(void) const {
		return ssystem->getScale();
	}

	//! Set planets viewer scaling factor
	void planetSetSizeScale(std::string name, float f) {
		ssystem->setPlanetSizeScale(name, f);
	}

	//! Get planets viewer scaling factor
	float planetGetSizeScale(std::string name) {
		return ssystem->getPlanetSizeScale(name);
	}

	////////////////////////////////////////////////////////////////////////////////
	// StarLines---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying
	void starLinesSetFlag(bool b) {
		starLines->setFlagShow(b);
	}

	//! Get flag for displaying
	bool starLinesGetFlag(void) const {
		return starLines->getFlagShow();
	}

	//! Vide tous les tampons de tracé
	void starLinesDrop(void) const {
		starLines->drop();
	}

	//! Charge un ensemble d'asterismes d'un fichier
	void starLinesLoadData(const std::string &fileName) {
		starLines->loadData(fileName);
	}

	//! Charge un asterisme à partir d'une ligne
	void starLinesLoadAsterism(std::string record) const {
		starLines->loadStringData(record);
	}

	//! supprime le cata logue complet des asterismes
	void starLinesClear() {
		starLines->clear();
	}

	void starLinesLoadCat(const std::string &fileName){
		starLines->loadHipCatalogue(fileName);
	}

	void starLinesLoadBinCat(const std::string &fileName){
		starLines->loadHipBinCatalogue(fileName);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Skyline et Skygrid---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void skyLineMgrSetColor(std::string name, Vec3f a) {
		skyLineMgr->setColor(name, a);
	}

	void skyGridMgrSetColor(std::string name, Vec3f a) {
		skyGridMgr->setColor(name, a);
	}

	const Vec3f& skyLineMgrGetColor(std::string name) {
		return skyLineMgr->getColor(name);
	}

	const Vec3f& skyGridMgrGetColor(std::string name) {
		return skyGridMgr->getColor(name);
	}

	void skyLineMgrFlipFlagShow(std::string name) {
		skyLineMgr->flipFlagShow(name);
	}

	void skyGridMgrFlipFlagShow(std::string name) {
		skyGridMgr->flipFlagShow(name);
	}

	void skyLineMgrSetFlagShow(std::string name, bool value) {
		skyLineMgr->setFlagShow(name, value);
	}

	void skyGridMgrSetFlagShow(std::string name, bool value) {
		skyGridMgr->setFlagShow(name, value);
	}

	bool skyLineMgrGetFlagShow(std::string name) {
		return skyLineMgr->getFlagShow(name);
	}

	bool skyGridMgrGetFlagShow(std::string name) {
		return skyGridMgr->getFlagShow(name);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Stars---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starSetFlag(bool b) {
		hip_stars->setFlagStars(b);
		starNav->setFlagStars(b);
	}

	bool starGetFlag(void) const {
		return hip_stars->getFlagStars();
	}

	void starSetTraceFlag(bool b) {
		hip_stars->setFlagTrace(b);
	}

	bool starGetTraceFlag(void) const {
		return hip_stars->getFlagTrace();
	}

	void starSetColorTable(int p, Vec3f a) {
		hip_stars->setColorStarTable(p,a);
	}

	void starSetDuration(float f) {
		return hip_stars->setFaderDuration(f);
	}

	void starSetFlagName(bool b) {
		hip_stars->setFlagNames(b);
	}
	bool starGetFlagName(void) const {
		return hip_stars->getFlagNames();
	}

	void starSetLimitingMag(float f) {
		hip_stars->setMagConverterMaxScaled60DegMag(f);
	}
	float starGetLimitingMag(void) const {
		return hip_stars->getMagConverterMaxScaled60DegMag();
	}

	void starSetFlagSciName(bool b) {
		hip_stars->setFlagSciNames(b);
	}
	bool starGetFlagSciName(void) const {
		return hip_stars->getFlagSciNames();
	}

	void starSetFlagTwinkle(bool b) {
		hip_stars->setFlagTwinkle(b);
	}
	bool starGetFlagTwinkle(void) const {
		return hip_stars->getFlagTwinkle();
	}

	void starSetMaxMagName(float f) {
		hip_stars->setMaxMagName(f);
	}
	float starGetMaxMagName(void) const {
		return hip_stars->getMaxMagName();
	}

	void starSetSizeLimit(float f) {
		starNav->setStarSizeLimit(f);
		setStarSizeLimit(f);
	}

	void starSetMaxMagSciName(float f) {
		hip_stars->setMaxMagName(f);
	}
	float starGetMaxMagSciName(void) const {
		return hip_stars->getMaxMagName();
	}

	void starSetScale(float f) {
		starNav->setScale(f);
		hip_stars->setScale(f);
	}
	float starGetScale(void) const {
		return hip_stars->getScale();
	}

	void starSetMagScale(float f) {
		starNav->setMagScale(f);
		hip_stars->setMagScale(f);
	}
	float starGetMagScale(void) const {
		return hip_stars->getMagScale();
	}

	void starSetTwinkleAmount(float f) {
		hip_stars->setTwinkleAmount(f);
	}
	float  starGetTwinkleAmount(void) const {
		return hip_stars->getTwinkleAmount();
	}

	////////////////////////////////////////////////////////////////////////////////
	// StarNavigator---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starNavigatorClear(){
		starNav->clear();
	}

	void starNavigatorLoad(const std::string &fileName, bool binaryMode){
		starNav->loadData(fileName, binaryMode);
	}

	void starNavigatorLoadRaw(const std::string &fileName){
		starNav->loadRawData(fileName);
	}

	void starNavigatorLoadOther(const std::string &fileName){
		starNav->loadOtherData(fileName);
	}

	void starNavigatorSave(const std::string &fileName, bool binaryMode){
		starNav->saveData(fileName, binaryMode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// SunTrace---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying SunTrace
	void bodyTraceSetFlag(bool b) const {
		bodytrace->setFlagShow(b);
	}
	//! Get flag for displaying SunTrace
	bool bodyTraceGetFlag(void) const {
		return bodytrace->getFlagShow();
	}

	void bodyPenUp() const {
		bodytrace->upPen();
	}

	void bodyPenDown() const {
		bodytrace->downPen();
	}

	void bodyPenToggle() const {
		bodytrace->togglePen();
	}

	void bodyTraceClear () const {
		bodytrace->clear();
	}

	void bodyTraceHide(std::string value) const;

	void bodyTraceBodyChange(std::string bodyName) const {
		if (bodyName=="selected") ssystem->bodyTraceBodyChange(selected_object.getEnglishName()); else ssystem->bodyTraceBodyChange(bodyName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Text_usr---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool textAdd(std::string name, std::string text, int altitude, int azimuth, std::string textSize, Vec3f &color, int duration) {
		return text_usr->add(name, text,altitude, azimuth, textSize, color, duration);
	}

	bool textAdd(std::string name, std::string text, int altitude, int azimuth, std::string textSize, int duration) {
		return text_usr->add(name, text,altitude, azimuth, textSize, duration);
	}

	void textDel(std::string name) {
		text_usr->del(name);
	}

	void textClear() {
		text_usr->clear();
	}

	void textNameUpdate(std::string name, std::string text) {
		text_usr->nameUpdate(name, text);
	}

	void textDisplay(std::string name , bool displ) {
		text_usr->textDisplay(name, displ);
	}

	void textFadingDuration(int a) {
		text_usr->setFadingDuration(a);
	}

	void textSetDefaultColor(const Vec3f& v) {
		text_usr->setColor(v);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Tully---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void tullySetFlagShow(bool v) {
		tully->setFlagShow(v);
	}

	bool tullyGetFlagShow() {
		return tully->getFlagShow();
	}

	void tullySetColor(const std::string &colorMode)
	{
		if (colorMode=="white")
			tully->setWhiteColor(true);
		if (colorMode=="custom")
			tully->setWhiteColor(false);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Time---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set time speed in JDay/sec
	void timeSetSpeed(double ts) {
		timeMgr->setTimeSpeed(ts);
	}

	void timeChangeSpeed(double ts, double duration) {
		timeMgr->changeTimeSpeed(ts, duration);
	}

	//! Get time speed in JDay/sec
	double timeGetSpeed(void) const {
		return timeMgr->getTimeSpeed();
	}

	void timeLoadSpeed(void) const {
		return timeMgr->loadTimeSpeed();
	}
	void timeSaveSpeed() const  {
		timeMgr-> saveTimeSpeed();
	}

	//! Set the current date in Julian Day
	void setJDay(double JD) {
		timeMgr->setJDay(JD);
	}
	//! Get the current date in Julian Day
	double getJDay(void) const {
		return timeMgr->getJDay();
	}

	bool timeGetFlagPause() const {
		return timeMgr->getTimePause();
	}

	void timeSetFlagPause(bool _value) const {
		timeMgr->setTimePause(_value);
	}

	double timeGetMultiplier() const {
		return timeMgr->getTimeMultiplier();
	}
	void timeSetMultiplier(double _value) {
		timeMgr->setTimeMultiplier(_value);
	}
	void timeResetMultiplier() {
		timeMgr->setTimeMultiplier(1.0);
	};

	////////////////////////////////////////////////////////////////////////////////
	// UBO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void uboSetAmbientLight(float v) {
		ubo_cam->setAmbientLight(v);
	}

	float uboGetAmbientLight() {
		return ubo_cam->getAmbientLight();
	}

	void saveCurrentConfig(InitParser &conf);
	Vec3f getCursorPosEqu(int x, int y);

	void imageDraw();
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
	void tcpSend(std::string msg ) const;

	AppSettings * settings;				//! endroit unique pour les chemins des fichiers dans l'application
	ServerSocket *tcp;
	bool enable_tcp ;

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

	std::string FontFileNameGeneral;			//! The font file used by default during initialization
	std::string FontFileNamePlanet;				//! The font for the planet system
	std::string FontFileNameConstellation;		//! The font for all asterims
	std::string FontFileNameMenu;
	std::string FontFileNameText;
	double FontSizeText;
	double FontSizeGeneral;
	double FontSizePlanet;
	double FontSizeConstellation;
	double FontSizeCardinalPoints;

	std::string skyCultureDir;			// The directory containing data for the culture used for constellations, etc..
	Translator skyTranslator;			// The translator used for astronomical object naming

	CoreExecutor* currentExecutor = nullptr;
	CoreExecutor* executorInSolarSystem = nullptr;
	CoreExecutor* executorInGalaxy = nullptr;
	CoreExecutor* executorInUniverse = nullptr;

	// Main elements of the program
	Navigator * navigation;				// Manage all navigation parameters, coordinate transformations etc..
	TimeMgr* timeMgr;				// Manage date and time
	Observer * observatory;			// Manage observer position
	Projector * projection;				// Manage the projection mode and matrix
	Object selected_object;			// The selected object
	int mouseX;
	int mouseY;
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
	Oort * oort;						//! oort cloud
	Dso3d *dso3d;						//! dso catalog for in_galaxy
	Tully *tully;						//! tully galaxies
	SkyPerson * personal;				// Personal azimuth drawing
	SkyPerson * personeq;				// Personal equatorial drawing
	SkyPerson * nautical;				// Nautical azimuth drawing
	SkyPerson * nauticeq;				// Nautical equatorial drawing
	SkyPerson * objCoord;				// Mouse position drawing
	SkyPerson * mouseCoord;				// Mouse position drawing
	SkyPerson * angDist;				// Angular distance drawing
	SkyPerson * loxodromy;				// Loxodromy drawing
	SkyPerson * orthodromy;				// Orthodromy drawing
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

	float sky_brightness;				// Current sky Brightness in ?
	bool object_pointer_visibility;		// Should selected object pointer be drawn
	std::string getCursorPos(int x, int y);  //not used now

	// Increment/decrement smoothly the vision field and position
	void updateMove(int delta_time);
	bool FlagEnableZoomKeys;
	bool FlagEnableMoveKeys;
	bool FlagAtmosphericRefraction = false;
	bool flagNav = false; 				// define the NAV version edition
	bool FlagAntialiasLines;            // whether to antialias all line drawing
	bool FlagManualZoom;				// Define whether auto zoom can go further
	bool firstTime= true;               // For init to track if reload or first time setup
	std::string defaultLandscape; 
	std::string tempLandscape; 
	// double deltaAlt, deltaAz, deltaFov, deltaHeight;	// View movement
	// double move_speed, zoom_speed;		// Speed of movement and zooming
	ViewZoomMove vzm;					// var for ViewZoomMove
	float InitFov;						// Default viewing FOV
	Vec3d InitViewPos;					// Default viewing direction
	float auto_move_duration;			// Duration of movement for the auto move to a selected objectin seconds
	float m_lineWidth;                  // width to use when drawing any line

	// bool FlagShaders;                   // whether to render using shaders

	//! size of Illuminate star
	double illuminate_size;

	//! Backup Manage
	backupWorkspace mBackup;			// variable used to remember various indicators in use

	void inimBackup();					// init at NULL all var
	float lightPollutionLimitingMagnitude;  // Defined naked eye limiting magnitude (due to light pollution)
	
	AnchorManager * anchorManager=nullptr;
};

#endif // _CORE_H_
