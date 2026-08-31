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
#include "experimentalModule/Camera.hpp"
#include "EntityCore/Executor/TickMgr.hpp"

namespace SessionFile { class CommandSurface; }

class CoreLink : public TickMgr<CoreLink> {
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

	//! Set the current date in Julian Day
	void setJDay(double JD);
	//! Get the current date in Julian Day
	double getJDay() const;
	bool timeGetFlagPause() const;
	void timeSetFlagPause(bool _value) const;
	//! Acquire a time lock
	void timeLock();
	//! Release a time lock
	void timeUnlock();

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

	bool tullyGetFlagName() const;

	void tullySetFlagName(bool b);

	void tullySetDuration(float f);

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

	void starNavSetDuration(float f);

	void starSetFlagName(bool b);

	bool starNavGetFlagName() const;

	void starNavSetFlagName(bool b);

	bool starGetFlagName() const;

	void starSetLimitingMag(float f);

	float starGetLimitingMag() const;

	void starSetFlagTwinkle(bool b);

	bool starGetFlagTwinkle() const;

	void starSetMaxMagName(float f);

	float starGetMaxMagName() const;

	void starNavSetMaxMagName(float f);

	float starNavGetMaxMagName() const;

	void starSetSizeLimit(float f);

	void starSetScale(float f);

	float starGetScale() const;

	void starSetMagScale(float f);

	float starGetMagScale() const;

	void starSetTwinkleAmount(float f);

	float  starGetTwinkleAmount() const;

	float getMag(int hip);

	float getBaseMag(int hip);

	////////////////////////////////////////////////////////////////////////////////
	// StarNavigator---------------------------
	////////////////////////////////////////////////////////////////////////////////
	void starNavigatorClear();

	void starNavigatorLoad(const std::string &fileName, bool binaryMode);

	void starNavigatorLoadRaw(const std::string &fileName);

	void starNavigatorLoadOther(const std::string &fileName);

	void starNavigatorSave(const std::string &fileName, bool binaryMode);

	void starNavigatorHideStar(int hip);

	void starNavigatorShowStar(int hip);

	void starNavigatorShowAllStar();

	MODULE getFlagIngalaxy(){
		return core->getFlagIngalaxy();
	}

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
	//! Define the main volumetric object to draw
	void dsoNavSetupVolumetric(std::map<std::string, std::string> &args, int defaultColorDepth);

	////////////////////////////////////////////////////////////////////////////////
	// FOV ( projection )
	////////////////////////////////////////////////////////////////////////////////

	//! Zoom to the given FOV (in degree)
	void zoomTo(double aim_fov, float move_duration = 1.) {
		core->projection->zoomTo(aim_fov, move_duration);
		Camera::instance->setHalfFov(aim_fov*M_PI/360, move_duration);
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
		Camera::instance->setHalfFov(f*M_PI/360);
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
		Camera::instance->lookTo(alt*M_PI/180, az*M_PI/180, time);
		// NEW path (B17): the look_at command arms the view offset -- mirrors old,
		// where navigation->lookAt -> moveTo arms view_offset_transition
		// (navigator.cpp:151,73-78). Discrete arm site (not the per-frame track).
		Camera::instance->armViewOffset(true);
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
	//! Get dual viewport
	bool mediaGetFlagDualViewport();
	//! Set dual viewport
	void mediaSetFlagDualViewport(bool b);
	//! Get display flag of constellation boundaries
	bool constellationGetFlagBoundaries();
  //! Get constellation color boundaries
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
	//! Set constellation color 3D
	void constellationSetColor(const Vec3f& v);
	//! Get constellation color 3D (starLines) - the read half of the above.
	const Vec3f &constellationGetColor() const;
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

	//! Re-read the observer's current system from its data file, keeping the
	//! current observation state (camera + date). Contract and what "keeping
	//! state" covers: SSystemFactory::reloadCurrentSystem.
	//! \return false when the current system has no data file behind it.
	bool reloadSolarSystem();

	//! Write the observer's current system to a composed system file, so what a
	//! script pushed into it this session is there at the next launch as
	//! ordinary authored data (B31 slice 2). Contract, naming rules and what a
	//! save preserves: SSystemFactory::saveCurrentSystem.
	//! \param filename a file NAME (empty = this system's own composed file).
	//! \return false when nothing was written (the reason is logged).
	bool saveSolarSystem(const std::string &filename);

	//! B31 slice 3 - the session file (experimentalModule/SessionFile.hpp).
	//! It lives HERE and not on Core because a restore has to move the observer
	//! through the DUAL seam this class owns (observerMoveTo): the old
	//! Observer still draws the whole sky, and S2 row B19 excludes it from the
	//! file only on the grounds that "setters are dual so it follows".
	//! EXPLICIT ONLY (D33): nothing calls either of these at startup or at
	//! shutdown, and no config key selects one.
	//! `cmds` is the command surface, which owns the names, read halves and
	//! write halves of the bulk value rows (S2 E3/E4/E5). It is passed in
	//! rather than reached for: this facade has no business knowing what a
	//! flag is called.
	bool sessionSave(const std::string &filename, SessionFile::CommandSurface *cmds);
	bool sessionLoad(const std::string &filename, SessionFile::CommandSurface *cmds);

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

	//! Mirror the planet-grid colors onto the new path from the sky-manager
	//! sources the OLD planet grid reads every draw (GRID_EQUATORIAL = meridian,
	//! LINE_EQUATOR = parallel; body.cpp:1261-1264). Both-paths grid COLOR seam
	//! (INTENT S11.42): keeps the new grid's colors identical to what the old
	//! grid renders. The color-authority structural choice is suspended for Vixy.
	void planetsSyncGridColor();


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

	//! Runtime navigation-radius seam (B10 S5.2, S11.79(e) D9key): set a body's
	//! datum_radius / ground_radius (in km, the data-key unit) at runtime.
	//! Returns false when no such body exists (the S2(f) diagnostic hook).
	bool planetSetDatumRadius(const std::string& englishName, double km) const;
	bool planetSetGroundRadius(const std::string& englishName, double km) const;

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

	//return the Sected star RA
	double getSelectedStarRA() const;

	//return the Selected star DE
	double getSelectedStarDE() const;

	double getDateYear() const;
	double getDateMonth() const;
	double getDateDay() const;
	double getDateHour() const;
	double getDateMinute() const;
	double getDateSecond() const;

	int getLanguage() const;

	//return body selected
	double getBodySelected() const;

	//! Set whether to draw the names for the selected planet or every planet
	void bodySetFlagIsolateSelected(bool b);
	//! Get whether to draw the names for the selected planet or every planet
	bool bodyGetFlagIsolateSelected();

	//! Set planets viewer scaling factor
	void planetSetSizeScale(std::string name, float f);

	// send param tesselation, name design the param to change to value
	void planetTesselation(std::string name, int value);

	std::string getSelectedPlanetEnglishName(){
		return (core->getSelectedPlanetEnglishName());
	}

	std::string getHomePlanetEnglishName(){
		return (core->getHomePlanetEnglishName());
	}

	////////////////////////////////////////////////////////////////////////////////
	// Fog---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Set flag for displaying Fog
	void fogSetFlag(bool b);
	//! Get flag for displaying Fog
	bool fogGetFlag() const;

	////////////////////////////////////////////////////////////////////////////////
	// Landscape---------------------------
	////////////////////////////////////////////////////////////////////////////////

	//! Get flag for displaying Landscape
	void landscapeSetFlag(bool b);
	//! Get flag for displaying Landscape
	bool landscapeGetFlag() const;

	void rotateLandscape(double rotation);

	std::string landscapeGetName();

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
	//! Set whether to draw the names for the selected stars or every star
	void nebulaSetFlagIsolateSelected(bool b);
	//! Get whether to draw the names for the selected stars or every star
	bool nebulaGetFlagIsolateSelected();
	//! Set fader duration for dso3d names
	void dso3dSetDuration(float f);
	//! Set flag for displaying dso3d names
	void dso3dSetFlagName(bool b);
	//! Set flag for displaying dso3d names
	bool dso3dGetFlagName() const;

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

	//! The read half of skyDisplayMgrSetColor. The manager has had the getter
	//! all along (skydisplay_mgr.hpp:61); only this facade lacked it, so nine
	//! `color` names had no readback at all (INTENT S11.129).
	const Vec3f &skyDisplayMgrGetColor(SKYDISPLAY_NAME nameObj);

	void skyDisplayMgrClear(SKYDISPLAY_NAME nameObj);

	void skyDisplayMgrLoadData(SKYDISPLAY_NAME nameObj, const std::string& fileName);

	void skyDisplayMgrLoadString(SKYDISPLAY_NAME nameObj, const std::string& dataStr);

	bool skyDisplayMgrCheckDraw();

	////////////////////////////////////////////////////////////////////////////////
	// Observatory---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! The place the path that DRAWS is at, in the legacy triple's units
	//! (degrees, degrees, metres) -- the READ half of the seam observerMoveTo
	//! writes, with the same two conversions in the other direction (I2).
	//! Answers only when the NEW path is the drawn one and a camera exists;
	//! otherwise the old Observer IS the drawn place and each getter below
	//! returns its own authority untouched, conventions included.
	//! Precision note, measured and not hidden: the camera holds the place as
	//! `float` radians and a `float` AU distance, so a drawn-path readout is
	//! quantised to that grid -- 0.10 m of altitude and 1.3e-6deg of latitude at
	//! the shipped Marseille place. That is the resolution the drawn observer
	//! ACTUALLY has; the old double carried more digits about a place that is
	//! not the one on screen (S11.130(f)'s rule, mirrored: report the value at
	//! the precision its owner has).
	bool drawnPlace(double &latDeg, double &lonDeg, double &altMetres) const {
		if (!core->getExperimentalPath() || !Camera::instance)
			return false;
		const Vec3f p = Camera::instance->getPlace();
		lonDeg = p[0] * (180.0 / M_PI);
		latDeg = p[1] * (180.0 / M_PI);
		altMetres = static_cast<double>(p[2]) * (1000.0 * AU);
		return true;
	}

	//! B33 (S11.108(f), the F12 template S11.118(f)): the observer place
	//! readouts ask WHICH PATH DRAWS. Their setters have been dual since the
	//! camera existed (observerMoveTo / observatorySetLatitude & co), so the two
	//! authorities agree until something moves ONE of them -- and one shipped
	//! command does exactly that: `camera action descend` is new-path-only by
	//! design (the old path's free navigation is the anchor-point observatory,
	//! there is nothing to mirror), as is free flight. Measured: two
	//! `camera action descend coef 0.5` leave the old observer at 75 m and the
	//! camera at 18.50 m. Everything that derives a target from these getters --
	//! `moveto` with any absent component, `moveto multiply_alt`/`delta_alt`,
	//! `mode jump ... altitude +-x`, the joypad height axis, the TUI location
	//! callback -- then computes it from a place nothing is drawing from, and
	//! writes it to BOTH paths, i.e. teleports the drawn observer.
	//! No clamp is added here: old's +-90deg latitude clamp and its 0.1 m altitude
	//! floor live in its SETTER, and a readout that clamps would report a place
	//! the camera is not at (the setter asymmetry is recorded, not fixed).
	double observatoryGetLatitude() const {
		double lat, lon, alt;
		if (drawnPlace(lat, lon, alt))
			return lat;
		return core->observatory->getLatitude();
	}

	double observatoryGetLongitude() const {
		double lat, lon, alt;
		if (drawnPlace(lat, lon, alt))
			return lon;
		return core->observatory->getLongitude();
	}

	//! Same readout, with the old getter's own [-180,180) display
	//! normalisation preserved (observer.cpp) -- it is the TUI's convention and
	//! it is what makes this a separate name.
	double observatoryGetLongitudeForDisplay() const {
		double lat, lon, alt;
		if (drawnPlace(lat, lon, alt))
			return lon - floor((lon + 180.) / 360.) * 360.;
		return core->observatory->getLongitudeForDisplay();
	}

	double observatoryGetAltitude() const {
		double lat, lon, alt;
		if (drawnPlace(lat, lon, alt))
			return alt;
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
		Camera::instance->setLatitude(l);
	}

	void observatorySetLongitude(double l) {
		core->observatory->setLongitude(l);
		Camera::instance->setLongitude(l);
	}

	///////////////////////////////////////////////////////////
	// Fonctions non utilisee ?
	// -------------------------------
	void observatorySetAltitude(double l) {
	 	core->observatory->setAltitude(l);
		Camera::instance->setAltitude(l);
	}

	std::string getObserverHomePlanetEnglishName() {
		return core->observatory->getHomePlanetEnglishName();
	}

	std::shared_ptr<Body> getObserverHomeBody(){
		return core->observatory->getHomeBody();
	}

	// [merge] Wrappers kept for theirs' lunar-eclipse skyline feature: experimental's
	// D1=ours CoreLink lacks them, so forward to experimental's ssystemFactory getters.
	std::shared_ptr<Body> ssystemFactoryGetEarth() const {
		return core->ssystemFactory->getEarth();
	}
	std::shared_ptr<Moon> ssystemFactoryGetMoon() const {
		return core->ssystemFactory->getMoon();
	}

	// [merge] script-variable getters (theirs' feature); adapted current* -> experimental's direct pointers.
	double getSelectedDistance() const {
		return core->ssystemFactory->getSelectedDistance(core->navigation);
	}
	double getSelectedMagnitude() const {
		return core->ssystemFactory->getSelectedMagnitude(core->navigation);
	}
	double getCurrentModule() const {
		return double(core->getFlagIngalaxy());
	}
	void starGalaxyLoadCatalog(const std::string &filename); // defined in coreLink.cpp (StarGalaxy incomplete in this header)

	//! New-path camera free-flight mode. Closes the INTENT S2(c)/S11.19c
	//! reachability defect (setFreeMode had no command route - the whole
	//! reference-transition layer was unreachable dynamically). New-path
	//! capability: the old path's free navigation is the anchor-point
	//! observatory, there is nothing to mirror.
	void cameraSetFreeMode(bool b) {
		Camera::instance->setFreeMode(b);
	}

	//! View-directed free descent (B21, S11.72): drive Camera::descend, the
	//! altitude-geometry authority (view ray near / last-selected body far).
	//! coef<1 descends, coef>1 ascends. New-path only -- the old-path free
	//! navigation is the anchor-point observatory, there is nothing to mirror
	//! (same seam as cameraSetFreeMode). This is the ONLY command-reachable
	//! driver of the view-directed descent: multAlt/moveRelAlt are UI-key-only
	//! (B10 finding, S11.71).
	void cameraDescend(float coef) {
		Camera::instance->descend(coef);
	}

	void observerMoveTo(double lat, double lon, double alt, int duration, bool calculate_duration=0) {
		core->observatory->moveTo(lat, lon, alt, duration, calculate_duration);
		Camera::instance->moveTo({static_cast<float>(lon*M_PI/180), static_cast<float>(lat*M_PI/180), static_cast<float>(alt/(1000*AU))}, duration/1000.f, calculate_duration);
	}

	//! Move to relative longitude where home planet is fixed.
	void observerMoveRelLon(double lon, int delay) {
		core->observatory->moveRelLon(lon, delay);
		Camera::instance->moveRelLon(lon*M_PI/180, delay/1000.f);
	}
	//! Move to relative latitude where home planet is fixed.
	void observerMoveRelLat(double lat, int delay) {
		core->observatory->moveRelLat(lat, delay);
		Camera::instance->moveRelLat(lat*M_PI/180, delay/1000.f);
	}
	//! Move to relative altitude where home planet is fixed.
	void observerMoveRelAlt(double alt, int delay) {
		core->observatory->moveRelAlt(alt, delay);
		Camera::instance->moveRelAlt(alt, delay/1000.f);
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
		if (mode) {
			core->bodyDecor->anchorAssign();
		} else {
			core->bindHomePlanet();
		}
	}

	bool getEyeRelativeMode() const {
		return core->observatory->getEyeRelativeMode();
	}

	//! change the Heading value
	void moveHeadingRelative(float f) {
		core->navigation->setHeading(core->navigation->getHeading() + f);
		Camera::instance->moveHeading(f*M_PI/180);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Meteors---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set Meteor Rate in number per hour
	void setMeteorsRate(int f);
	//! Get Meteor Rate in number per hour
	int getMeteorsRate() const;
	//! create new radiant
	void createRadiant(int day, const Vec3f newRadiant);
	//! clear radiants
	void clearRadiants();

	////////////////////////////////////////////////////////////////////////////////
	// Atmosphere---------------------------
	////////////////////////////////////////////////////////////////////////////////
	//! Set flag for displaying Atmosphere
	void atmosphereSetFlag(bool b);
	//! Get flag for displaying Atmosphere
	bool atmosphereGetFlag() const;
	//! Set atmosphere fade duration in s
	void atmosphereSetFadeDuration(float f);
	//! Get atmosphere fade duration in s - the read half of the above.
	float atmosphereGetFadeDuration() const;
	//! Set default atmosphere fade duration
	void atmosphereSetDefaultFadeDuration();
	//! Set moon brightness
	void moonSetBrightness(double f);
	//! Set default moon brightness
	void moonSetDefaultBrightness();
	//! Set sun brightness
	void sunSetBrightness(double f);
	//! Set default sun brightness
	void sunSetDefaultBrightness();
	//! Set flag for activating atmospheric refraction correction
	void atmosphericRefractionSetFlag(bool b);
	//! Get flag for activating atmospheric refraction correction
	bool atmosphericRefractionGetFlag() const;

	////////////////////////////////////////////////////////////////////////////////
	// Navigation -------------
	////////////////////////////////////////////////////////////////////////////////
	//! B33 (S11.108(f), the F12 template S11.118(f)): the zoom/centre offset of
	//! the path that DRAWS, as the fraction of the dome radius both authorities
	//! store it as (no conversion -- Core::setViewOffset is the ONE sink and it
	//! hands the SAME clamped scalar to both, so the units are one unit).
	//! This member is LATENT and says so: no shipped channel writes one path's
	//! offset without the other (the sink since S11.92, the session restore
	//! since S11.130(e)), so today the fix cannot change an observable -- it
	//! stops the readout from BECOMING wrong the moment a channel does, which
	//! is the whole reason the class is being closed rather than patched. Its
	//! divergence has to be injected to be checked, and it was.
	double getViewOffset() const {
		if (!core->getExperimentalPath() || !Camera::instance)
			return core->navigation->getViewOffset();
		return Camera::instance->getViewOffset();
	}

	//! set environment rotation around observer
	void setHeading(double heading, int duration=0) {
		core->navigation->changeHeading(heading, duration);
		// duration is ms here; the camera plans in seconds (smoothed with the
		// constant-min-acceleration law)
		Camera::instance->setHeading(heading*M_PI/180, duration*0.001f);
	}

	void setDefaultHeading() {
		core->navigation->setDefaultHeading();
	}

	//! The environment roll around the observer, in degrees, normalised to
	//! [-180, 180] (the old getter's own TUI-compatibility convention).
	//! B33 (S11.108(f)) + the D28 rider (S11.113(g)): this READS THE PATH THAT
	//! DRAWS. The setter above has always written both authorities; the getter
	//! read only the old Navigator, so under the new path it reported a number
	//! that was not the roll on screen - measured -6.16 deg drawn vs 0 reported
	//! after a reference switch - and `heading delta_azimuth d`
	//! (app_command_interface.cpp) computes its ABSOLUTE target from it.
	//! D28 chose "the new path holds the whole orientation across a reference
	//! switch", which makes the divergence permanent by design and this readout
	//! the thing that makes the choice operable: the operator must be able to
	//! see and command the roll the choice produces. `set heading 0` stays the
	//! standing remedy for the accumulating tilt and is unaffected - it writes
	//! both paths, as before.
	//! Which path draws is asked, not assumed: under `flag experimental_path
	//! off` the old Navigator IS the drawn roll and is what must be reported.
	double getHeading() const {
		if (!core->getExperimentalPath())
			return core->navigation->getHeading();
		double h = Camera::instance->getHeading() * (180.0 / M_PI);
		h -= floor((h + 180.) / 360.) * 360.;
		return h;
	}

	const Vec3d& getLocalVision() const { //unused
		return core->navigation->getLocalVision();
	}

	void setLocalVision(const Vec3d& _pos) {
		core->navigation->setLocalVision(_pos);
		Camera::instance->lookTo(_pos, 0);
	}

	bool getFlagTracking() {
		return (core->getFlagTracking());
	}

	//! B33 READBACK ONLY (INTENT S11.108(f), delivered S11.131) -- what the
	//! CONTROL SURFACE answers for each member of the query half, beside the
	//! value EACH path holds for the same readout, in one frame.
	//! What it is FOR: the rule is that a getter reports the path that DRAWS,
	//! and until this existed no member of the class could be MEASURED against
	//! it on either binary -- `get status position` never replies (S5.47), the
	//! view-offset readout has one live reader and it is the TUI, and the mount
	//! readout has no live reader at all. Each row is
	//! `{"reported": ..., "old": ..., "new": ...}` in the GETTER's own units, so one
	//! dump discriminates by itself: a binary that reads the old authority has
	//! `reported == old` whatever draws; a binary that reads the drawn path has
	//! `reported == new` while the new path draws and `reported == old` under
	//! `flag experimental_path off`.
	//! Const, side-effect-free, dump-channel only (`body action dual_dump`) --
	//! the old render path is unchanged by construction (S11.52(b)).
	void dumpControlSurface(std::ostream &out) const;

    CoreLink(std::shared_ptr<Core> _core) {
		core = _core;
		instance = this;
	}
    ~CoreLink() {
		instance = nullptr;
	};

	//! Whether the rendered frame must be predictible (at the expense of framerate) or not
	inline bool predictibleRendering() const {
		return core->predictibleRendering;
	}

	inline void setPredictibleRendering(bool enable, int framerate) {
		core->setPredictibleRendering(enable, framerate);
	}

	//! Whether App thing HipStarMgr is in use or not
	bool isDrawingHipStarMgr;
	bool isJoypadConnected = false;
	static CoreLink *instance;
private:
    std::shared_ptr<Core> core;
};

#endif
