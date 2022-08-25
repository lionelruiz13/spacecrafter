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
	//! Empty all plot buffers
	void starLinesDrop() const;
	//! Loads a set of asterisms from a file
	void starLinesLoadData(const std::string &fileName);
	//! Loads an asterism from a line
	void starLinesLoadAsterism(std::string record) const;
	//! deletes the complete catalog of asterisms
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
	void tullySetFlagShow(bool v);

	bool tullyGetFlagShow();

	void tullySetWhiteColor(bool value);

	bool tullyGetWhiteColor();

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
	void starSetFlag(bool b);

	bool starGetFlag() const;

	void starSetTraceFlag(bool b);

	bool starGetTraceFlag() const;

	void starSetColorTable(int p, Vec3f a);

	void starSetDuration(float f);

	void starSetFlagName(bool b);

	bool starGetFlagName() const;

	void starSetLimitingMag(float f);

	float starGetLimitingMag() const;

	void starSetFlagTwinkle(bool b);

	bool starGetFlagTwinkle() const;

	void starSetMaxMagName(float f);

	float starGetMaxMagName() const;

	void starSetSizeLimit(float f);

	void starSetScale(float f);

	float starGetScale() const;

	void starSetMagScale(float f);

	float starGetMagScale() const;

	void starSetTwinkleAmount(float f);

	float  starGetTwinkleAmount() const;

	////////////////////////////////////////////////////////////////////////////////
	// StarNavigator---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starNavigatorClear();

	void starNavigatorLoad(const std::string &fileName, bool binaryMode);

	void starNavigatorLoadRaw(const std::string &fileName);

	void starNavigatorLoadOther(const std::string &fileName);

	void starNavigatorSave(const std::string &fileName, bool binaryMode);

	////////////////////////////////////////////////////////////////////////////////
	// SunTrace---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying SunTrace
	void bodyTraceSetFlag(bool b) const;

	//! Get flag for displaying SunTrace
	bool bodyTraceGetFlag() const;

	void bodyPenUp() const;

	void bodyPenDown() const;

	void bodyPenToggle() const;

	void bodyTraceClear () const;

	void bodyTraceHide(std::string value) const;

	void bodyTraceBodyChange(std::string bodyName) const;

	////////////////////////////////////////////////////////////////////////////////
	// for TCP usage  ---------------------------
	////////////////////////////////////////////////////////////////////////////////

	std::string getConstellationSelectedShortName() const;

	std::string getPlanetsPosition() const;

	std::string tcpGetPosition() const;

	////////////////////////////////////////////////////////////////////////////////
	// UBO---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void uboSetAmbientLight(float v);
	float uboGetAmbientLight();

	////////////////////////////////////////////////////////////////////////////////
	// DSO---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! hide a particular DSO
	void dsoSelectName(std::string DSOName, bool hide) const;
	//! hide all DSO
	void dsoHideAll() const;
	//! show (unhide) all DSO
	void dsoShowAll() const;
	//! select all DSO in constellationName to be hidden or showed
	void dsoSelectConstellation(bool hide, std::string constellationName) const;
	//! select all DSO with typeName to be hidden or showed
	void dsoSelectType(bool hide, std::string typeName) const;

	//! Insert a volumetric dso from script
	void dsoNavInsert(std::map<std::string, std::string> &args);
	//! Override dsoNavigator resources, allow loading another set of volumetric dso
	void dsoNavOverrideCurrent(const std::string& tex_file, const std::string &tex3d_file, int depth);

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
	// BodyOJM---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void BodyOJMLoad(const std::string &mode, const std::string &name, const std::string &filename, const std::string &pathFile, const Vec3f &Position, const float multiplier);

	void BodyOJMRemove(const std::string &mode, const std::string &name);

	void BodyOJMRemoveAll(const std::string &mode);

	////////////////////////////////////////////////////////////////////////////////
	// Camera---------------------------
	////////////////////////////////////////////////////////////////////////////////

	void cameraDisplayAnchor();

	bool cameraAddAnchor(stringHash_t& param);

	bool cameraRemoveAnchor(const std::string &name);

	bool cameraSwitchToAnchor(const std::string &name);

	bool cameraMoveToPoint(double x, double y, double z);

	bool cameraMoveToPoint(double x, double y, double z, double time);

	bool cameraMoveToBody(const std::string& bodyName, double time, double alt = -1.0);

	bool cameraMoveRelativeXYZ( double x, double y, double z);

	bool cameraTransitionToPoint(const std::string& name);

	bool cameraTransitionToBody(const std::string& name);

	bool cameraSave(const std::string& name = "anchor");

	bool loadCameraPosition(const std::string& filename);

	bool lookAt(double az, double alt, double time = 1.){
		return core->navigation->lookAt(az, alt, time);
	}

	bool cameraSetFollowRotation(const std::string& name, bool value);

	void cameraSetRotationMultiplierCondition(float v);

	bool cameraAlignWithBody(const std::string& name, double duration);

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
	void constellationSetFlagLines(bool b);
	//! Get display flag of constellation lines
	bool constellationGetFlagLines();
	//! Set display flag of constellation art
	void constellationSetFlagArt(bool b);
	//! Get display flag of constellation art
	bool constellationGetFlagArt();
	//! Set display flag of constellation names
	void constellationSetFlagNames(bool b);
	//! Get display flag of constellation names
	bool constellationGetFlagNames();
	//! Set display flag of constellation boundaries
	void constellationSetFlagBoundaries(bool b);
	//! Get display flag of constellation boundaries
	bool constellationGetFlagBoundaries();
	Vec3f constellationGetColorBoundaries() const;
	//! Set constellation art intensity
	void constellationSetArtIntensity(float f);
	//! Get constellation art intensity
	float constellationGetArtIntensity() const;
	//! Set constellation art intensity
	void constellationSetArtFadeDuration(float f);
	//! Get constellation art intensity
	float constellationGetArtFadeDuration() const;
	//! Set whether selected constellation is drawn alone
	void constellationSetFlagIsolateSelected(bool b);
	//! Get whether selected constellation is drawn alone
	bool constellationGetFlagIsolateSelected();
	//! Set whether to draw the names for the selected stars or every star
	void starSetFlagIsolateSelected(bool b);
	//! Get whether to draw the names for the selected stars or every star
	bool starGetFlagIsolateSelected();
	//! Get constellation line color
	Vec3f constellationGetColorLine() const;
	//! Set constellation line color
	void constellationSetColorLine(const Vec3f& v);
	//! Get constellation names color
	Vec3f constellationGetColorNames() const;
	//! Set constellation names color
	void constellationSetColorNames(const Vec3f& v);
	//! Set constellation names color
	void constellationSetColorNames(const std::string &argName, const Vec3f& v);
	//! Get constellation art color
	Vec3f constellationGetColorArt() const;
	//! Set constellation line color
	void constellationSetColorArt(const Vec3f& v);

	void constellationSetColorBoundaries(const Vec3f& v);

	void constellationSetLineColor(const std::string &argName, const Vec3f& v);

	void constellationSetArtIntensity(const std::string &argName, float intensity);

	///////////////////////////////////////////////////////////////////////////////////////
	// Planets flags

	void setFlagLightTravelTime(bool b);

	bool getFlagLightTravelTime() const;

	//! Start/stop displaying planets Trails
	void startPlanetsTrails(bool b);

	//! Set selected planets by englishName
	//! @param englishName The planet name or "" to select no planet
	void setPlanetsSelected(const std::string& englishName);

	//! Set flag for displaying a scaled Moon
	void setFlagMoonScaled(bool b);

	//! Get flag for displaying a scaled Moon
	bool getFlagMoonScaled() const;

	//! Set flag for displaying a scaled Sun
	void setFlagSunScaled(bool b);

	//! Get flag for displaying a scaled Sun
	bool getFlagSunScaled() const;

	//! Set Moon scale
	void setMoonScale(float f, bool resident = false);

	//! Get Moon scale
	float getMoonScale() const;

	//! Set Sun scale
	void setSunScale(float f, bool resident = false);

	//! Get Moon scale
	float getSunScale() const;

	//! Set flag for displaying clouds (planet rendering feature)
	void setFlagClouds(bool b);

	//! Get flag for displaying Atmosphere
	bool getFlagClouds() const;

	void initialSolarSystemBodies();

	//hides a planet
	void setPlanetHidden(std::string name, bool planethidden);

	//indicates if the planet is visible 1 or not 0
	bool getPlanetHidden(std::string name);

	////////////////////////////////////////////////////////////////////////////////
	// Planets---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Planets
	void planetsSetFlag(bool b);

	//! Get flag for displaying Planets
	bool planetsGetFlag() const;

	//! Set flag for displaying Planets Trails
	void planetsSetFlagTrails(bool b);

	//! Get flag for displaying Planets Trails
	bool planetsGetFlagTrails() const;

	//! Set flag for displaying Planets Axis
	void planetsSetFlagAxis(bool b);

	//! Get flag for displaying Planets Axis
	bool planetsGetFlagAxis() const;


	//! Set flag for displaying Planets Hints
	void planetsSetFlagHints(bool b);

	//! Get flag for displaying Planets Hints
	bool planetsGetFlagHints() const;

	//! Set flag for displaying Planets Orbits
	void planetsSetFlagOrbits(bool b);

	//! Set flag for displaying Planet name Orbit
	void planetsSetFlagOrbits(const std::string &_name, bool b);

	//! Switch
	void planetSwitchTexMap(const std::string &_name, bool b);

	//! Switch
	bool planetGetSwitchTexMap(const std::string &_name);

	void planetCreateTexSkin(const std::string &name, const std::string &texName);

	//! Get flag for displaying Planets Orbits
	bool planetsGetFlagOrbits() const;

	//! Set flag for displaying Satellites Orbits
	void satellitesSetFlagOrbits(bool b);

	//! Get flag for displaying Satellites Orbits
	bool satellitesGetFlagOrbits() const;

	//! Set flag for displaying Planets & Satellites Orbits
	void planetSetFlagOrbits(bool b);

	void planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const;

	Vec3f planetGetColor(const std::string& englishName, const std::string& color) const;

	void planetSetDefaultColor(const std::string& color, Vec3f c) const;

	Vec3f planetGetDefaultColor(const std::string& colorName) const;

	bool hideSatellitesFlag();

	void setHideSatellites(bool val);

	//! Set base planets display scaling factor
	void planetsSetScale(float f);

	//return the Sun altitude
	double getSunAltitude() const;

	//return the Sun azimuth
	double getSunAzimuth() const;

	//return the Sected body AZ
	double getSelectedAZ() const;

	//return the Selected body ALT
	double getSelectedALT() const;

	//return the Sected body RA
	double getSelectedRA() const;

	//return the Selected body DE
	double getSelectedDE() const;

	double getDateYear() const;
	double getDateMonth() const;
	double getDateDay() const;
	double getDateHour() const;
	double getDateMinute() const;

	//! Set planets viewer scaling factor
	void planetSetSizeScale(std::string name, float f);

	// send param tesselation, name design the param to change to value
	void planetTesselation(std::string name, int value);

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
	void nebulaSetFlag(bool b);
	//! Get flag for displaying Nebulae
	bool nebulaGetFlag() const;
	//! Set flag for displaying Nebulae Hints
	void nebulaSetFlagHints(bool b);
	//! Get flag for displaying Nebulae Hints
	bool nebulaGetFlagHints() const;
	//! Set flag for displaying Nebulae as bright
	void nebulaSetFlagBright(bool b);
	//! Get flag for displaying Nebulae as brigth
	bool nebulaGetFlagBright() const;
	//! Set maximum magnitude at which nebulae hints are displayed
	void nebulaSetMaxMagHints(float f);
	//! Get maximum magnitude at which nebulae hints are displayed
	float nebulaGetMaxMagHints() const;
	//! return the color for the DSO object
	Vec3f nebulaGetColorLabels() const;
	//! return the color of the DSO circle
	Vec3f nebulaGetColorCircle() const;
	//!set Flag DSO Name who display DSO name
	void nebulaSetFlagNames (bool value);
	//!get flag DSO Name who display DSO name
	bool nebulaGetFlagNames ();
	void nebulaSetColorLabels(const Vec3f& v);
	void nebulaSetColorCircle(const Vec3f& v);

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

	void setQuaternionMode(bool mode) {
		core->observatory->setQuaternionMode(mode);
	}

	bool getQuaternionMode() const {
		return core->observatory->getQuaternionMode();
	}

	void setEyeRelativeMode(bool mode) {
		core->observatory->setEyeRelativeMode(mode);
	}

	bool getEyeRelativeMode() const {
		return core->observatory->getEyeRelativeMode();
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
