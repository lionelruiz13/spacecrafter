/*
 * Copyright (C) 2014-2021 of the LSS Team & Association Sirius
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


#ifndef CORELINK_HPP
#define CORELINK_HPP

#include <memory>
#include "coreModule/core.hpp"
#include "inGalaxyModule/dso3d.hpp"

class CoreLink {
public:

//	static void DateTimeFromJulianDay(double jd, int *year, int *month, int *day, int *hour, int *minute, double *second);

	////////////////////////////////////////////////////////////////////////////////
	// StarLines---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying
	void starLinesSetFlag(bool b);

	void starLinesSelectedSetFlag(bool b);

	bool starLinesSelectedGetFlag() const;

	//! Get flag for displaying
	bool starLinesGetFlag() const;
	//! Vide tous les tampons de tracé
	void starLinesDrop() const;
	//! Charge un ensemble d'asterismes d'un fichier
	void starLinesLoadData(const std::string &fileName);
	//! Charge un asterisme à partir d'une ligne
	void starLinesLoadAsterism(std::string record) const;
	//! supprime le catalogue complet des asterismes
	void starLinesClear();

	void starLinesSaveCat(const std::string &fileName, bool binaryMode);

	void starLinesLoadCat(const std::string &fileName, bool binaryMode);

	void starLinesLoadHipStar(int name, Vec3f position);

	////////////////////////////////////////////////////////////////////////////////
	// Skyline et Skygrid---------------------------
	////////////////////////////////////////////////////////////////////////////////
    void skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a);

    void skyGridMgrSetColor(SKYGRID_TYPE name, Vec3f a);

	const Vec3f& skyLineMgrGetColor(SKYLINE_TYPE name);

	const Vec3f& skyGridMgrGetColor(SKYGRID_TYPE name);

	void skyLineMgrFlipFlagShow(SKYLINE_TYPE name);

	void skyGridMgrFlipFlagShow(SKYGRID_TYPE name);

	void skyLineMgrSetFlagShow(SKYLINE_TYPE name, bool value);

	void skyGridMgrSetFlagShow(SKYGRID_TYPE name, bool value);

	bool skyLineMgrGetFlagShow(SKYLINE_TYPE name);

	bool skyGridMgrGetFlagShow(SKYGRID_TYPE name);


	////////////////////////////////////////////////////////////////////////////////
	// Time---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set time speed in JDay/sec
	void timeSetSpeed(double ts);

	void timeChangeSpeed(double ts, double duration);
	//! Get time speed in JDay/sec
	double timeGetSpeed() const;

	void timeLoadSpeed() const;

	void timeSaveSpeed() const;
	//! Set the current date in Julian Day
	void setJDay(double JD);
	//! Get the current date in Julian Day
	double getJDay() const;
	bool timeGetFlagPause() const;
	void timeSetFlagPause(bool _value) const;

	////////////////////////////////////////////////////////////////////////////////
	// dateSun---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! return the JD time when the sun go down
	double dateSunRise(double _jd, double _longitude, double _latitude);
	//! return the JD time when the sun set up
	double dateSunSet(double _jd, double _longitude, double _latitude);
	//! return the JD time when the sun cross the meridian
	double dateSunMeridian(double _jd, double _longitude, double _latitude);

	////////////////////////////////////////////////////////////////////////////////
	// Tully---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void tullySetFlagShow(bool v) {
		core->tully->setFlagShow(v);
	}

	bool tullyGetFlagShow() {
		return core->tully->getFlagShow();
	}

	void tullySetWhiteColor(bool value)
	{
		core->tully->setWhiteColor(value);
	}

	bool tullyGetWhiteColor() {
		return core->tully->getWhiteColor();
	}
	////////////////////////////////////////////////////////////////////////////////
	// Stars---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starSetFlag(bool b) {
		core->hip_stars->setFlagShow(b);
		core->starNav->setFlagStars(b);
	}

	bool starGetFlag() const {
		return core->hip_stars->getFlagShow();
	}

	void starSetTraceFlag(bool b) {
		core->hip_stars->setFlagTrace(b);
	}

	bool starGetTraceFlag() const {
		return core->hip_stars->getFlagTrace();
	}

	void starSetColorTable(int p, Vec3f a) {
		core->hip_stars->setColorStarTable(p,a);
	}

	void starSetDuration(float f) {
		return core->hip_stars->setFaderDuration(f);
	}

	void starSetFlagName(bool b) {
		core->hip_stars->setFlagNames(b);
	}
	bool starGetFlagName() const {
		return core->hip_stars->getFlagNames();
	}

	void starSetLimitingMag(float f) {
		core->hip_stars->setMagConverterMaxScaled60DegMag(f);
	}
	float starGetLimitingMag() const {
		return core->hip_stars->getMagConverterMaxScaled60DegMag();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Illuminate---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void illuminateSetSize (double value);

	void illuminateLoadConstellation(const std::string& abbreviation, double size, double rotation);

	void illuminateLoadConstellation(const std::string& abbreviation,const Vec3f& color, double size, double rotation);

	void illuminateLoadAllConstellation(double size, double rotation);

	void illuminateLoad(int number, double size, double rotation);

	void illuminateLoad(int number, const Vec3f& _color, double size, double rotation);

	void illuminateRemove(unsigned int name);

	void illuminateRemoveConstellation(const std::string abbreviation);

	void illuminateRemoveAllConstellation();

	void illuminateRemoveAll();

	void illuminateChangeTex(const std::string& _fileName);

	void illuminateRemoveTex();

	////////////////////////////////////////////////////////////////////////////////
	// stars
	////////////////////////////////////////////////////////////////////////////////

	void starSetFlagTwinkle(bool b) {
		core->hip_stars->setFlagTwinkle(b);
	}
	bool starGetFlagTwinkle() const {
		return core->hip_stars->getFlagTwinkle();
	}

	void starSetMaxMagName(float f) {
		core->hip_stars->setMaxMagName(f);
	}
	float starGetMaxMagName() const {
		return core->hip_stars->getMaxMagName();
	}

	void starSetSizeLimit(float f) {
		core->starNav->setStarSizeLimit(f);
		core->setStarSizeLimit(f);
	}

	void starSetScale(float f) {
		core->starNav->setScale(f);
		core->hip_stars->setScale(f);
	}
	float starGetScale() const {
		return core->hip_stars->getScale();
	}

	void starSetMagScale(float f) {
		core->starNav->setMagScale(f);
		core->hip_stars->setMagScale(f);
	}
	float starGetMagScale() const {
		return core->hip_stars->getMagScale();
	}

	void starSetTwinkleAmount(float f) {
		core->hip_stars->setTwinkleAmount(f);
	}
	float  starGetTwinkleAmount() const {
		return core->hip_stars->getTwinkleAmount();
	}

	////////////////////////////////////////////////////////////////////////////////
	// StarNavigator---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starNavigatorClear(){
		core->starNav->clear();
	}

	void starNavigatorLoad(const std::string &fileName, bool binaryMode){
		core->starNav->loadData(fileName, binaryMode);
	}

	void starNavigatorLoadRaw(const std::string &fileName){
		core->starNav->loadRawData(fileName);
	}

	void starNavigatorLoadOther(const std::string &fileName){
		core->starNav->loadOtherData(fileName);
	}

	void starNavigatorSave(const std::string &fileName, bool binaryMode){
		core->starNav->saveData(fileName, binaryMode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// SunTrace---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying SunTrace
	void bodyTraceSetFlag(bool b) const {
		core->ssystemFactory->bodyTraceSetFlag(b);
	}
	//! Get flag for displaying SunTrace
	bool bodyTraceGetFlag() const {
		return core->ssystemFactory->bodyTraceGetFlag();
	}

	void bodyPenUp() const {
		core->ssystemFactory->upPen();
	}

	void bodyPenDown() const {
		core->ssystemFactory->downPen();
	}

	void bodyPenToggle() const {
		core->ssystemFactory->togglePen();
	}

	void bodyTraceClear () const {
		core->ssystemFactory->clear();
	}

	void bodyTraceHide(std::string value) const {
		if (value=="all")
			core->ssystemFactory->hide(-1);
		else
			core->ssystemFactory->hide(Utility::strToInt(value));
	}

	void bodyTraceBodyChange(std::string bodyName) const {
		if (bodyName=="selected")
			core->ssystemFactory->bodyTraceBodyChange(core->selected_object.getEnglishName());
		else
			core->ssystemFactory->bodyTraceBodyChange(bodyName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// for TCP usage  ---------------------------
	////////////////////////////////////////////////////////////////////////////////

	std::string getConstellationSelectedShortName() const;

	std::string getPlanetsPosition() const;

	std::string tcpGetPosition() const;

	////////////////////////////////////////////////////////////////////////////////
	// UBO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void uboSetAmbientLight(float v) {
		core->ubo_cam->setAmbientLight(v);
	}

	float uboGetAmbientLight() {
		return core->ubo_cam->getAmbientLight();
	}

	////////////////////////////////////////////////////////////////////////////////
	// DSO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! hide a particular DSO
	void dsoSelectName(std::string DSOName, bool hide) const {
		return core->nebulas->selectName(hide, DSOName);
	}

	//! hide all DSO
	void dsoHideAll() const {
		core->nebulas->hideAll();
	}

	//! show (unhide) all DSO
	void dsoShowAll() const {
		core->nebulas->showAll();
	}

	//! select all DSO in constellationName to be hidden or showed
	void dsoSelectConstellation(bool hide, std::string constellationName) const {
		core->nebulas->selectConstellation(hide, constellationName);
	}

	//! select all DSO with typeName to be hidden or showed
	void dsoSelectType(bool hide, std::string typeName) const {
		core->nebulas->selectType(hide, typeName);
	}

	////////////////////////////////////////////////////////////////////////////////
	// FOV ( projection )
	////////////////////////////////////////////////////////////////////////////////

	//! Zoom to the given FOV (in degree)
	void zoomTo(double aim_fov, float move_duration = 1.) {
		core->projection->zoomTo(aim_fov, move_duration);
	}

	//! Get current FOV (in degree)
	float getFov() const {
		return core->projection->getFov();
	}

	//! If is currently zooming, return the target FOV, otherwise return current FOV
	double getAimFov() const {
		return core->projection->getAimFov();
	}

	//! Set the current FOV (in degree)
	void setFov(double f) {
		core->projection->setFov(f);
	}

	//! Set the maximum FOV (in degree)
	void setMaxFov(double f) {
		core->projection->setMaxFov(f);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Body---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void BodyOJMLoad(const std::string &mode, const std::string &name, const std::string &filename, const std::string &pathFile, const Vec3f &Position, const float multiplier) {
		core->ojmMgr->load(mode, name, filename, pathFile, Position, multiplier);
	}

	void BodyOJMRemove(const std::string &mode, const std::string &name){
		core->ojmMgr->remove(mode, name);
	}

	void BodyOJMRemoveAll(const std::string &mode){
		core->ojmMgr->removeAll(mode);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Camera---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void cameraDisplayAnchor() {
		core->ssystemFactory->cameraDisplayAnchor();
	}

	bool cameraAddAnchor(stringHash_t& param) {
		return core->ssystemFactory->cameraAddAnchor(param);
	}

	bool cameraRemoveAnchor(const std::string &name) {
		return core->ssystemFactory->cameraRemoveAnchor(name);
	}

	bool cameraSwitchToAnchor(const std::string &name) {
		return core->ssystemFactory->cameraSwitchToAnchor(name);
	}

	bool cameraMoveToPoint(double x, double y, double z){
		return core->ssystemFactory->cameraMoveToPoint(x,y,z);
	}

	bool cameraMoveToPoint(double x, double y, double z, double time){
		return core->ssystemFactory->cameraMoveToPoint(x,y,z,time);
	}

	bool cameraMoveToBody(const std::string& bodyName, double time, double alt = -1.0){

		if(bodyName == "selected"){
			return core->ssystemFactory->cameraMoveToBody(core->getSelectedPlanetEnglishName(), time, alt);
		}

		if(bodyName == "default"){
			return core->ssystemFactory->cameraMoveToBody(core->ssystemFactory->getEarth()->getEnglishName(), time, alt);
		}

		return core->ssystemFactory->cameraMoveToBody(bodyName,time, alt);
	}

	bool cameraMoveRelativeXYZ( double x, double y, double z) {
		return core->ssystemFactory->cameraMoveRelativeXYZ(x,y,z);
	}

	bool cameraTransitionToPoint(const std::string& name){
		return core->ssystemFactory->cameraTransitionToPoint(name);
	}

	bool cameraTransitionToBody(const std::string& name){

		if(name == "selected"){
			return core->ssystemFactory->cameraTransitionToBody(core->getSelectedPlanetEnglishName());
		}

		return core->ssystemFactory->cameraTransitionToBody(name);
	}

	bool cameraSave(const std::string& name = "anchor");

	bool loadCameraPosition(const std::string& filename);

	bool lookAt(double az, double alt, double time = 1.){
		return core->navigation->lookAt(az, alt, time);
	}

	bool cameraSetFollowRotation(const std::string& name, bool value){
		return core->ssystemFactory->cameraSetFollowRotation(value);
	}

	void cameraSetRotationMultiplierCondition(float v) {
		core->ssystemFactory->cameraSetRotationMultiplierCondition(v);
	}

	bool cameraAlignWithBody(const std::string& name, double duration){
		return core->ssystemFactory->cameraAlignWithBody(name,duration);
	}

	////////////////////////////////////////////////////////////////////////////////
	// CardinalsPoints---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Cardinals Points
	void cardinalsPointsSetFlag(bool b);
	//! Get flag for displaying Cardinals Points
	bool cardinalsPointsGetFlag() const;
	//! Set Cardinals Points color
	void cardinalsPointsSetColor(const Vec3f& v);
	//! Get Cardinals Points color
	Vec3f cardinalsPointsGetColor() const;

	////////////////////////////////////////////////////////////////////////////////
	// Constellations---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set display flag of constellation lines
	void constellationSetFlagLines(bool b) {
		core->asterisms->setFlagLines(b);
	}
	//! Get display flag of constellation lines
	bool constellationGetFlagLines() {
		return core->asterisms->getFlagLines();
	}

	//! Set display flag of constellation art
	void constellationSetFlagArt(bool b) {
		core->asterisms->setFlagArt(b);
	}
	//! Get display flag of constellation art
	bool constellationGetFlagArt() {
		return core->asterisms->getFlagArt();
	}

	//! Set display flag of constellation names
	void constellationSetFlagNames(bool b) {
		core->asterisms->setFlagNames(b);
	}
	//! Get display flag of constellation names
	bool constellationGetFlagNames() {
		return core->asterisms->getFlagNames();
	}

	//! Set display flag of constellation boundaries
	void constellationSetFlagBoundaries(bool b) {
		core->asterisms->setFlagBoundaries(b);
	}
	//! Get display flag of constellation boundaries
	bool constellationGetFlagBoundaries() {
		return core->asterisms->getFlagBoundaries();
	}
	Vec3f constellationGetColorBoundaries() const {
		return core->asterisms->getBoundaryColor();
	}

	//! Set constellation art intensity
	void constellationSetArtIntensity(float f) {
		core->asterisms->setArtIntensity(f);
	}
	//! Get constellation art intensity
	float constellationGetArtIntensity() const {
		return core->asterisms->getArtIntensity();
	}

	//! Set constellation art intensity
	void constellationSetArtFadeDuration(float f) {
		core->asterisms->setArtFadeDuration(f);
	}
	//! Get constellation art intensity
	float constellationGetArtFadeDuration() const {
		return core->asterisms->getArtFadeDuration();
	}

	//! Set whether selected constellation is drawn alone
	void constellationSetFlagIsolateSelected(bool b) {
		core->asterisms->setFlagIsolateSelected(b);
	}

	//! Get whether selected constellation is drawn alone
	bool constellationGetFlagIsolateSelected() {
		return core->asterisms->getFlagIsolateSelected();
	}

	//! Set whether to draw the names for the selected stars or every star
	void starSetFlagIsolateSelected(bool b) {
		return core->hip_stars->setFlagIsolateSelected(b);
	}

	//! Get whether to draw the names for the selected stars or every star
	bool starGetFlagIsolateSelected() {
		return core->hip_stars->getFlagIsolateSelected();
	}

	//! Get constellation line color
	Vec3f constellationGetColorLine() const {
		return core->asterisms->getLineColor();
	}
	//! Set constellation line color
	void constellationSetColorLine(const Vec3f& v) {
		core->asterisms->setLineColor(v);
	}

	//! Get constellation names color
	Vec3f constellationGetColorNames() const {
		return core->asterisms->getLabelColor();
	}
	//! Set constellation names color
	void constellationSetColorNames(const Vec3f& v) {
		core->asterisms->setLabelColor(v);
	}

	//! Set constellation names color
	void constellationSetColorNames(const std::string &argName, const Vec3f& v) {
		core->asterisms->setLabelColor(argName, v);
	}

	//! Get constellation art color
	Vec3f constellationGetColorArt() const {
		return core->asterisms->getArtColor();
	}
	//! Set constellation line color
	void constellationSetColorArt(const Vec3f& v) {
		core->asterisms->setArtColor(v);
	}

	void constellationSetColorBoundaries(const Vec3f& v) {
		core->asterisms->setBoundaryColor(v);
	}

	void constellationSetLineColor(const std::string &argName, const Vec3f& v) {
		core->asterisms->setLineColor(argName, v);
	}

	void constellationSetArtIntensity(const std::string &argName, float intensity) {
		core->asterisms->setArtIntensity(argName, intensity);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Planets flags

	void setFlagLightTravelTime(bool b) {
		core->ssystemFactory->setFlagLightTravelTime(b);
	}
	bool getFlagLightTravelTime() const {
		return core->ssystemFactory->getFlagLightTravelTime();
	}

	//! Start/stop displaying planets Trails
	void startPlanetsTrails(bool b) {
		core->ssystemFactory->startTrails(b);
	}

	//! Set selected planets by englishName
	//! @param englishName The planet name or "" to select no planet
	void setPlanetsSelected(const std::string& englishName) {
		core->ssystemFactory->setSelected(englishName);
	}

	//! Set flag for displaying a scaled Moon
	void setFlagMoonScaled(bool b) {
		core->ssystemFactory->setFlagMoonScale(b);
	}
	//! Get flag for displaying a scaled Moon
	bool getFlagMoonScaled() const {
		return core->ssystemFactory->getFlagMoonScale();
	}

	//! Set flag for displaying a scaled Sun
	void setFlagSunScaled(bool b) {
		core->ssystemFactory->setFlagSunScale(b);
	}
	//! Get flag for displaying a scaled Sun
	bool getFlagSunScaled() const {
		return core->ssystemFactory->getFlagSunScale();
	}

	//! Set Moon scale
	void setMoonScale(float f, bool resident = false) {
		if (f<0) core->ssystemFactory->setMoonScale(1., false);
		else core->ssystemFactory->setMoonScale(f, resident);
	}
	//! Get Moon scale
	float getMoonScale() const {
		return core->ssystemFactory->getMoonScale();
	}

	//! Set Sun scale
	void setSunScale(float f, bool resident = false) {
		if (f<0) core->ssystemFactory->setSunScale(1., false);
		else core->ssystemFactory->setSunScale(f, resident);
	}
	//! Get Moon scale
	float getSunScale() const {
		return core->ssystemFactory->getSunScale();
	}

	//! Set flag for displaying clouds (planet rendering feature)
	void setFlagClouds(bool b) {
		core->ssystemFactory->setFlagClouds(b);
	}
	//! Get flag for displaying Atmosphere
	bool getFlagClouds() const {
		return core->ssystemFactory->getFlag(BODY_FLAG::F_CLOUDS);
	}

	void initialSolarSystemBodies() {
		return core->ssystemFactory->initialSolarSystemBodies();
	}

	//cache une planete
	void setPlanetHidden(std::string name, bool planethidden) {
		core->ssystemFactory->setPlanetHidden(name, planethidden);
	}

	//indique si la planete est visible 1 ou pas 0
	bool getPlanetHidden(std::string name) {
		return core->ssystemFactory->getPlanetHidden(name);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Planets---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Planets
	void planetsSetFlag(bool b) {
		core->ssystemFactory->setFlagPlanets(b);
	}
	//! Get flag for displaying Planets
	bool planetsGetFlag() const {
		return core->ssystemFactory->getFlagShow();
	}

	//! Set flag for displaying Planets Trails
	void planetsSetFlagTrails(bool b) {
		core->ssystemFactory->setFlagTrails(b);
	}
	//! Get flag for displaying Planets Trails
	bool planetsGetFlagTrails() const {
		return core->ssystemFactory->getFlag(BODY_FLAG::F_TRAIL);
	}

	//! Set flag for displaying Planets Axis
	void planetsSetFlagAxis(bool b) {
		core->ssystemFactory->setFlagAxis(b);
	}
	//! Get flag for displaying Planets Axis
	bool planetsGetFlagAxis() const {
		return core->ssystemFactory->getFlag(BODY_FLAG::F_AXIS);
	}


	//! Set flag for displaying Planets Hints
	void planetsSetFlagHints(bool b) {
		core->ssystemFactory->setFlagHints(b);
	}
	//! Get flag for displaying Planets Hints
	bool planetsGetFlagHints() const {
		return core->ssystemFactory->getFlag(BODY_FLAG::F_HINTS);
	}

	//! Set flag for displaying Planets Orbits
	void planetsSetFlagOrbits(bool b) {
		core->ssystemFactory->setFlagPlanetsOrbits(b);
	}

	//! Set flag for displaying Planet name Orbit
	void planetsSetFlagOrbits(const std::string &_name, bool b) {
		core->ssystemFactory->setFlagPlanetsOrbits(_name, b);
	}

	//! Switch
	void planetSwitchTexMap(const std::string &_name, bool b) {
		if (_name=="selected") core->ssystemFactory->switchPlanetTexMap(core->selected_object.getEnglishName(), b);
		else core->ssystemFactory->switchPlanetTexMap(_name, b);
	}

	//! Switch
	bool planetGetSwitchTexMap(const std::string &_name) {
		if (_name=="selected") return core->ssystemFactory->getSwitchPlanetTexMap(core->selected_object.getEnglishName());
		else return core->ssystemFactory->getSwitchPlanetTexMap(_name);
	}

	void planetCreateTexSkin(const std::string &name, const std::string &texName){
		core->ssystemFactory->createTexSkin(name, texName);
	}

	//! Get flag for displaying Planets Orbits
	bool planetsGetFlagOrbits() const {
		return core->ssystemFactory->getFlagPlanetsOrbits();
	}

	//! Set flag for displaying Satellites Orbits
	void satellitesSetFlagOrbits(bool b) {
		core->ssystemFactory->setFlagSatellitesOrbits(b);
	}

	//! Get flag for displaying Satellites Orbits
	bool satellitesGetFlagOrbits() const {
		return core->ssystemFactory->getFlagSatellitesOrbits();
	}
	//! Set flag for displaying Planets & Satellites Orbits
	void planetSetFlagOrbits(bool b) {
		core->ssystemFactory->setFlagSatellitesOrbits(b);
		core->ssystemFactory->setFlagPlanetsOrbits(b);
		//ssystem->setFlagOrbits(b);
	}

	void planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const {
		core->ssystemFactory->setBodyColor(englishName, color, c);
	}

	Vec3f planetGetColor(const std::string& englishName, const std::string& color) const {
		return core->ssystemFactory->getBodyColor(englishName, color);
	}

	void planetSetDefaultColor(const std::string& color, Vec3f c) const {
		core->ssystemFactory->setDefaultBodyColor(color, c);
	}

	Vec3f planetGetDefaultColor(const std::string& colorName) const {
		return core->ssystemFactory->getDefaultBodyColor(colorName);
	}

	bool hideSatellitesFlag(){
		return core->ssystemFactory->getHideSatellitesFlag();
	}

	void setHideSatellites(bool val){
		core->ssystemFactory->toggleHideSatellites(val);
	}

	//! Set base planets display scaling factor
	void planetsSetScale(float f) {
		core->ssystemFactory->setScale(f);
	}

	//return the Sun altitude
	double getSunAltitude() const {
		return core->ssystemFactory->getSunAltitude(core->navigation);
	}

	//return the Sun azimuth
	double getSunAzimuth() const {
		return core->ssystemFactory->getSunAzimuth(core->navigation);
	}

	double getDateYear() const;
	double getDateMonth() const;
	double getDateDay() const;
	double getDateHour() const;
	double getDateMinute() const;

	//! Set planets viewer scaling factor
	void planetSetSizeScale(std::string name, float f) {
		core->ssystemFactory->setPlanetSizeScale(name, f);
	}

	// send param tesselation, name design the param to change to value
	void planetTesselation(std::string name, int value) {
		core->ssystemFactory->planetTesselation(name,value);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Fog---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Fog
	void fogSetFlag(bool b) {
		core->landscape->fogSetFlagShow(b);
	}
	//! Get flag for displaying Fog
	bool fogGetFlag() const {
		return core->landscape->fogGetFlagShow();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Landscape---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Get flag for displaying Landscape
	void landscapeSetFlag(bool b) {
		core->landscape->setFlagShow(b);
	}
	//! Get flag for displaying Landscape
	bool landscapeGetFlag() const {
		return core->landscape->getFlagShow();
	}

	void rotateLandscape(double rotation) {
		core->landscape->setRotation(rotation);
	}


	std::string landscapeGetName() {
	 	return core->landscape->getName();
	}

	////////////////////////////////////////////////////////////////////////////////
	// Milky Way---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Milky Way
	void milkyWaySetFlag(bool b);
	//! Get flag for displaying Milky Way
	bool milkyWayGetFlag() const;
	//! Set flag for displaying Zodiacal Light
	void milkyWaySetFlagZodiacal(bool b);
	//! Get flag for displaying Zodiacal Light
	bool milkyWayGetFlagZodiacal() const;
	//! Set Milky Way intensity
	void milkyWaySetIntensity(float f);
	//! Set Zodiacal intensity
	void milkyWaySetZodiacalIntensity(float f);
	//! Get Milky Way intensity
	float milkyWayGetIntensity() const;

	void milkyWayRestoreDefault();

	void milkyWaySetDuration(float f);
	
	void milkyWayRestoreIntensity();

	//! Change Milkyway texture
	void milkyWayChangeState(const std::string& mdir, float _intensity);

	//! Change Milkyway texture without intensity
	void milkyWayChangeStateWithoutIntensity(const std::string& mdir);

	////////////////////////////////////////////////////////////////////////////////
	// Nebulae---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Nebulae
	void nebulaSetFlag(bool b) {
		core->nebulas->setFlagShow(b);
		core->dso3d->setFlagShow(b);
	}
	//! Get flag for displaying Nebulae
	bool nebulaGetFlag() const {
		return core->nebulas->getFlagShow();
	}

	//! Set flag for displaying Nebulae Hints
	void nebulaSetFlagHints(bool b) {
		core->nebulas->setFlagHints(b);
	}
	//! Get flag for displaying Nebulae Hints
	bool nebulaGetFlagHints() const {
		return core->nebulas->getFlagHints();
	}

	//! Set flag for displaying Nebulae as bright
	void nebulaSetFlagBright(bool b) {
		core->nebulas->setFlagBright(b);
	}
	//! Get flag for displaying Nebulae as brigth
	bool nebulaGetFlagBright() const {
		return core->nebulas->getFlagBright();
	}

	//! Set maximum magnitude at which nebulae hints are displayed
	void nebulaSetMaxMagHints(float f) {
		core->nebulas->setMaxMagHints(f);
	}
	//! Get maximum magnitude at which nebulae hints are displayed
	float nebulaGetMaxMagHints() const {
		return core->nebulas->getMaxMagHints();
	}

	//! return the color for the DSO object
	Vec3f nebulaGetColorLabels() const {
		return core->nebulas->getLabelColor();
	}

	//! return the color of the DSO circle
	Vec3f nebulaGetColorCircle() const {
		return core->nebulas->getCircleColor();
	}

	//!set Flag DSO Name who display DSO name
	void nebulaSetFlagNames (bool value) {
		core->nebulas->setNebulaNames(value);
	}

	//!get flag DSO Name who display DSO name
	bool nebulaGetFlagNames () {
		return core->nebulas->getNebulaNames();
	}

	void nebulaSetColorLabels(const Vec3f& v) {
		core->nebulas->setLabelColor(v);
	}
	void nebulaSetColorCircle(const Vec3f& v) {
		core->nebulas->setCircleColor(v);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Oort    ---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool oortGetFlagShow() const;

	void oortSetFlagShow(bool b);

	////////////////////////////////////////////////////////////////////////////////
	// SkyDisplayMgr    ---------------------------
	////////////////////////////////////////////////////////////////////////////////
	bool skyDisplayMgrGetFlag(SKYDISPLAY_NAME nameObj);
	
	void skyDisplayMgrSetFlag(SKYDISPLAY_NAME nameObj, bool v);
	
	void skyDisplayMgrFlipFlag(SKYDISPLAY_NAME nameObj);
	
	void skyDisplayMgrSetColor(SKYDISPLAY_NAME nameObj, const Vec3f& v);

	void skyDisplayMgrClear(SKYDISPLAY_NAME nameObj);

	void skyDisplayMgrLoadData(SKYDISPLAY_NAME nameObj, const std::string& fileName);

	void skyDisplayMgrLoadString(SKYDISPLAY_NAME nameObj, const std::string& dataStr);

	////////////////////////////////////////////////////////////////////////////////
	// Observatory---------------------------
	////////////////////////////////////////////////////////////////////////////////
	double observatoryGetLatitude() {
		return core->observatory->getLatitude();
	}

	double observatoryGetLongitude() {
		return core->observatory->getLongitude();
	}

	double observatoryGetLongitudeForDisplay() {
		return core->observatory->getLongitudeForDisplay();
	}

	double observatoryGetAltitude() {
		return core->observatory->getAltitude();
	}

	double observatoryGetDefaultLatitude() {
		return core->observatory->getDefaultLatitude();
	}

	double observatoryGetDefaultLongitude() {
		return core->observatory->getDefaultLongitude();
	}

	double observatoryGetDefaultAltitude() {
		return core->observatory->getDefaultAltitude();
	}

	void observatorySetLatitude(double l) {
		core->observatory->setLatitude(l);
	}

	void observatorySetLongitude(double l) {
		core->observatory->setLongitude(l);
	}

	///////////////////////////////////////////////////////////
	// Fonctions non utilisée ?
	// -------------------------------
	void observatorySetAltitude(double l) {
	 	core->observatory->setAltitude(l);
	}

	std::string getObserverHomePlanetEnglishName() {
		return core->observatory->getHomePlanetEnglishName();
	}

	std::shared_ptr<Body> getObserverHomeBody(){
		return core->observatory->getHomeBody();
	}

	void observerMoveTo(double lat, double lon, double alt, int duration, bool calculate_duration=0) {
		core->observatory->moveTo(lat, lon, alt, duration, calculate_duration);
	}

	//! Move to relative longitude where home planet is fixed.
	void observerMoveRelLon(double lon, int delay) {
		core->observatory->moveRelLon(lon, delay);
	}
	//! Move to relative latitude where home planet is fixed.
	void observerMoveRelLat(double lat, int delay) {
		core->observatory->moveRelLat(lat, delay);
	}
	//! Move to relative altitude where home planet is fixed.
	void observerMoveRelAlt(double alt, int delay) {
		core->observatory->moveRelAlt(alt, delay);
	}

	void observerSetConf(InitParser &conf,const std::string &section) {
		core->observatory->setConf(conf,section);
	}

	void observerDisplayPos() {
		std::cout << core->observatory->getObserverCenterPoint() << std::endl;
	}

	//! change the Heading value
	void moveHeadingRelative(float f) {
		core->navigation->setHeading(core->navigation->getHeading() + f);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Meteors---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set Meteor Rate in number per hour
	void setMeteorsRate(int f);
	//! Get Meteor Rate in number per hour
	int getMeteorsRate() const;

	////////////////////////////////////////////////////////////////////////////////
	// Atmosphere---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Atmosphere
	void atmosphereSetFlag(bool b);
	//! Get flag for displaying Atmosphere
	bool atmosphereGetFlag() const;
	//! Set atmosphere fade duration in s
	void atmosphereSetFadeDuration(float f);
	//! Set flag for activating atmospheric refraction correction
	void atmosphericRefractionSetFlag(bool b);
	//! Get flag for activating atmospheric refraction correction
	bool atmosphericRefractionGetFlag() const;

	////////////////////////////////////////////////////////////////////////////////
	// Navigation -------------
	////////////////////////////////////////////////////////////////////////////////
	double getViewOffset() {
		return core->navigation->getViewOffset();
	}

	//! set environment rotation around observer
	void setHeading(double heading, int duration=0) {
		core->navigation->changeHeading(heading, duration);
	}

	void setDefaultHeading() {
		core->navigation->setDefaultHeading();
	}

	double getHeading() {
		return core->navigation->getHeading();
	}

    CoreLink(std::shared_ptr<Core> _core) {
		core = _core;
	}
    ~CoreLink(){};

private:
    std::shared_ptr<Core> core;
};

#endif
