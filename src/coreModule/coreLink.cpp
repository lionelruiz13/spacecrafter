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

#include "coreModule/coreLink.hpp"
#include "tools/app_settings.hpp"
//#include "coreModule/coreFont.hpp"
#include "appModule/fontFactory.hpp"
#include "appModule/space_date.hpp"
#include "coreModule/oort.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/milkyway.hpp"
#include "atmosphereModule/atmosphere.hpp"
#include "coreModule/nebula_mgr.hpp"
#include "coreModule/constellation_mgr.hpp"
#include "coreModule/landscape.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "bodyModule/ssystem_factory.hpp"
#include "inGalaxyModule/dsoNavigator.hpp"
#include "inGalaxyModule/dso3d.hpp"
#include "coreModule/cardinals.hpp"
#include "coreModule/skydisplay_mgr.hpp"
#include "coreModule/meteor_mgr.hpp"
#include "coreModule/illuminate_mgr.hpp"
#include "coreModule/starLines.hpp"
#include "ojmModule/ojm_mgr.hpp"
#include "inGalaxyModule/starNavigator.hpp"
#include "coreModule/ubo_cam.hpp"
#include "coreModule/tully.hpp"
#include "mediaModule/media.hpp"

CoreLink *CoreLink::instance = nullptr;

bool CoreLink::cameraSave(const std::string& name)
{
	return core->currentSsystemFactory->cameraSave(AppSettings::Instance()->getUserDir() + "anchors/" + name);
}

bool CoreLink::loadCameraPosition(const std::string& filename)
{
	return core->currentSsystemFactory->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
}

// void CoreLink::fontUpdateFont(const std::string& _targetName, const std::string& _fontName, const std::string& _sizeValue)
// {
// 	core->fontFactory->updateFont(_targetName, _fontName, _sizeValue);
// }

double CoreLink::getDateYear() const
{
	double jd = core->timeMgr->getJDay();
	int year,month,day,hour,minute;
	double second;

	SpaceDate::DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);
	return year;
}

double CoreLink::getDateMonth() const
{
	double jd = core->timeMgr->getJDay();
	int year,month,day,hour,minute;
	double second;

	SpaceDate::DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);
	return month;
}

double CoreLink::getDateDay() const
{
	double jd = core->timeMgr->getJDay();
	int year,month,day,hour,minute;
	double second;

	SpaceDate::DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);
	return day;
}

double CoreLink::getDateHour() const
{
	double jd = core->timeMgr->getJDay();
	int year,month,day,hour,minute;
	double second;

	SpaceDate::DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);
	return hour;
}

double CoreLink::getDateMinute() const
{
	double jd = core->timeMgr->getJDay();
	int year,month,day,hour,minute;
	double second;

	SpaceDate::DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);
	return minute;
}

double CoreLink::getDateSecond() const
{
	double jd = core->timeMgr->getJDay();
	int year,month,day,hour,minute;
	double second;

	SpaceDate::DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);
	return second;
}

////////////////////////////////////////////////////////////////////////////////
// Skyline et Skygrid---------------------------
////////////////////////////////////////////////////////////////////////////////

void CoreLink::skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a) {
    core->currentSkyLineMgr->setColor(name, a);
};

void CoreLink::skyGridMgrSetColor(SKYGRID_TYPE name, Vec3f a) {
	core->currentSkyGridMgr->setColor(name, a);
}

const Vec3f& CoreLink::skyLineMgrGetColor(SKYLINE_TYPE name) {
	return core->currentSkyLineMgr->getColor(name);
}

const Vec3f& CoreLink::skyGridMgrGetColor(SKYGRID_TYPE name) {
	return core->currentSkyGridMgr->getColor(name);
}

void CoreLink::skyLineMgrFlipFlagShow(SKYLINE_TYPE name) {
	core->currentSkyLineMgr->flipFlagShow(name);
}

void CoreLink::skyGridMgrFlipFlagShow(SKYGRID_TYPE name) {
	core->currentSkyGridMgr->flipFlagShow(name);
}

void CoreLink::skyLineMgrSetFlagShow(SKYLINE_TYPE name, bool value) {
	core->currentSkyLineMgr->setFlagShow(name, value);
}

void CoreLink::skyGridMgrSetFlagShow(SKYGRID_TYPE name, bool value) {
	core->currentSkyGridMgr->setFlagShow(name, value);
}

bool CoreLink::skyLineMgrGetFlagShow(SKYLINE_TYPE name) {
	return core->currentSkyLineMgr->getFlagShow(name);
}

bool CoreLink::skyGridMgrGetFlagShow(SKYGRID_TYPE name) {
	return core->currentSkyGridMgr->getFlagShow(name);
}

////////////////////////////////////////////////////////////////////////////////
// Oort    ---------------------------
////////////////////////////////////////////////////////////////////////////////
bool CoreLink::oortGetFlagShow() const {
	return core->currentOort->getFlagShow();
}

void CoreLink::oortSetFlagShow(bool b) {
	core->currentOort->setFlagShow(b);
}

////////////////////////////////////////////////////////////////////////////////
// Milky Way---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::milkyWaySetFlag(bool b) {
	core->currentMilkyWay->setFlagShow(b);
}

bool CoreLink::milkyWayGetFlag() const {
	return core->currentMilkyWay->getFlagShow();
}

void CoreLink::milkyWaySetFlagZodiacal(bool b) {
	core->currentMilkyWay->setFlagZodiacal(b);
}

bool CoreLink::milkyWayGetFlagZodiacal() const {
	return core->currentMilkyWay->getFlagZodiacal();
}

void CoreLink::milkyWaySetIntensity(float f) {
	core->currentMilkyWay->setIntensity(f);
}

void CoreLink::milkyWaySetZodiacalIntensity(float f) {
	core->currentMilkyWay->setZodiacalIntensity(f);
}

float CoreLink::milkyWayGetIntensity() const {
	return core->currentMilkyWay->getIntensity();
}

void CoreLink::milkyWayRestoreDefault() {
	core->currentMilkyWay->restoreDefaultMilky();
}

void CoreLink::milkyWaySetDuration(float f) {
	core->currentMilkyWay->setFaderDuration(f);
}

void CoreLink::milkyWayRestoreIntensity() {
	core->currentMilkyWay->restoreIntensity();
}

void CoreLink::milkyWayChangeState(const std::string& mdir, float _intensity) {
	core->currentMilkyWay->changeMilkywayState(mdir, _intensity);
}

void CoreLink::milkyWayChangeStateWithoutIntensity(const std::string& mdir) {
	core->currentMilkyWay->changeMilkywayStateWithoutIntensity(mdir);
}

////////////////////////////////////////////////////////////////////////////////
// Meteors---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set Meteor Rate in number per hour
void CoreLink::setMeteorsRate(int f) {
	core->currentMeteors->setZHR(f);
}

//! Get Meteor Rate in number per hour
int CoreLink::getMeteorsRate() const {
	return core->currentMeteors->getZHR();
}

void CoreLink::createRadiant(int day, const Vec3f newRadiant) {
	core->currentMeteors->createRadiant(day, newRadiant);
}

void CoreLink::clearRadiants() {
	core->currentMeteors->clearRadiants();
}

////////////////////////////////////////////////////////////////////////////////
// CardinalsPoints---------------------------
////////////////////////////////////////////////////////////////////////////////
//! Set flag for displaying Cardinals Points
void CoreLink::cardinalsPointsSetFlag(bool b) {
	core->cardinals_points->setFlagShow(b);
}
//! Get flag for displaying Cardinals Points
bool CoreLink::cardinalsPointsGetFlag() const {
	return core->cardinals_points->getFlagShow();
}
//! Set Cardinals Points color
void CoreLink::cardinalsPointsSetColor(const Vec3f& v) {
	core->cardinals_points->setColor(v);
}
//! Get Cardinals Points color
Vec3f CoreLink::cardinalsPointsGetColor() const {
	return core->cardinals_points->getColor();
}

////////////////////////////////////////////////////////////////////////////////
// SkyDisplayMgr    ---------------------------
////////////////////////////////////////////////////////////////////////////////
bool CoreLink::skyDisplayMgrGetFlag(SKYDISPLAY_NAME nameObj) {
	return core->currentSkyDisplayMgr->getFlagShow(nameObj);
}

void CoreLink::skyDisplayMgrSetFlag(SKYDISPLAY_NAME nameObj, bool v) {
	core->currentSkyDisplayMgr->setFlagShow(nameObj,v);
}

void CoreLink::skyDisplayMgrFlipFlag(SKYDISPLAY_NAME nameObj) {
	core->currentSkyDisplayMgr->flipFlagShow(nameObj);
}

void CoreLink::skyDisplayMgrSetColor(SKYDISPLAY_NAME nameObj, const Vec3f& v) {
	core->currentSkyDisplayMgr->setColor(nameObj,v);
}

void CoreLink::skyDisplayMgrClear(SKYDISPLAY_NAME nameObj) {
	core->currentSkyDisplayMgr->clear(nameObj);
}

void CoreLink::skyDisplayMgrLoadData(SKYDISPLAY_NAME nameObj, const std::string& fileName) {
	core->currentSkyDisplayMgr->loadData(nameObj,fileName);
}

void CoreLink::skyDisplayMgrLoadString(SKYDISPLAY_NAME nameObj, const std::string& dataStr) {
	core->currentSkyDisplayMgr->loadString(nameObj,dataStr);
}

bool CoreLink::skyDisplayMgrCheckDraw(){
	if (core->getSelectedPlanetEnglishName() == core->getHomePlanetEnglishName())
		return (true);
	return (false);
}

////////////////////////////////////////////////////////////////////////////////
// StarLines---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set flag for displaying
void CoreLink::starLinesSetFlag(bool b) {
	core->currentStarLines->setFlagShow(b);
}

void CoreLink::starLinesSelectedSetFlag(bool b) {
	core->currentStarLines->setFlagSelected(b);
}

bool CoreLink::starLinesSelectedGetFlag() const {
	return core->currentStarLines->getFlagSelected();
}

//! Get flag for displaying
bool CoreLink::starLinesGetFlag() const {
	return core->currentStarLines->getFlagShow();
}

//! Empty all plot buffers
void CoreLink::starLinesDrop() const {
	core->currentStarLines->drop();
}

//! Loads a set of asterisms from a file
void CoreLink::starLinesLoadData(const std::string &fileName) {
	core->currentStarLines->loadData(fileName);
}

//! Loads an asterism from a line
void CoreLink::starLinesLoadAsterism(std::string record) const {
	core->currentStarLines->loadStringData(record);
}

//! deletes the complete catalog of asterisms
void CoreLink::starLinesClear() {
	core->currentStarLines->clear();
}

void CoreLink::starLinesSaveCat(const std::string &fileName, bool binaryMode){
	core->currentStarLines->saveCat(fileName, binaryMode);
}

void CoreLink::starLinesLoadCat(const std::string &fileName, bool binaryMode){
	core->currentStarLines->loadCat(fileName, binaryMode);
}

void CoreLink::starLinesLoadHipStar(int name, Vec3f position) {
	core->currentStarLines->loadHipStar(name, position);
}

////////////////////////////////////////////////////////////////////////////////
// Illuminate---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::illuminateSetSize (double value) {
	core->currentIlluminates->setDefaultSize(value);
}

void CoreLink::illuminateLoadConstellation(const std::string& abbreviation, double size, double rotation) {
	core->currentIlluminates->loadConstellation(abbreviation, size, rotation);
}
void CoreLink::illuminateLoadConstellation(const std::string& abbreviation,const Vec3f& color, double size, double rotation) {
	core->currentIlluminates->loadConstellation(abbreviation, color, size, rotation);
}
void CoreLink::illuminateLoadAllConstellation(double size, double rotation) {
	core->currentIlluminates->loadAllConstellation(size, rotation);
}

void CoreLink::illuminateLoad(int number, double size, double rotation) {
	core->currentIlluminates->load(number, size, rotation);
}

void CoreLink::illuminateLoad(int number, const Vec3f& _color, double size, double rotation) {
	core->currentIlluminates->load(number, _color, size, rotation);
}

void CoreLink::illuminateRemove(unsigned int name) 	{
	core->currentIlluminates->remove(name);
}

void CoreLink::illuminateRemoveConstellation(const std::string abbreviation) 	{
	core->currentIlluminates->removeConstellation(abbreviation);
}

void CoreLink::illuminateRemoveAllConstellation() 	{
	core->currentIlluminates->removeAllConstellation();
}

void CoreLink::illuminateRemoveAll()
{
	core->currentIlluminates->removeAll();
}

void CoreLink::illuminateChangeTex(const std::string& _fileName)	{
	core->currentIlluminates->changeTex(_fileName);
}

void CoreLink::illuminateRemoveTex()	{
	core->currentIlluminates->removeTex();
}

////////////////////////////////////////////////////////////////////////////////
// Atmosphere---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set flag for displaying Atmosphere
void CoreLink::atmosphereSetFlag(bool b) {
	core->currentBodyDecor->setAtmosphereState(b);
	core->setBodyDecor();
}
//! Get flag for displaying Atmosphere
bool CoreLink::atmosphereGetFlag() const {
	return core->currentBodyDecor->getAtmosphereState();
}

//! Set atmosphere fade duration in s
void CoreLink::atmosphereSetFadeDuration(float f) {
	core->currentAtmosphere->setFaderDuration(f);
}

//! Set default atmosphere fade duration
void CoreLink::atmosphereSetDefaultFadeDuration() {
	core->currentAtmosphere->setDefaultFaderDuration();
}

//! Set moon brightness
void CoreLink::moonSetBrightness(double f) {
	core->currentAtmosphere->setMoonBrightness(f);
}

//! Set default moon brightness
void CoreLink::moonSetDefaultBrightness() {
	core->currentAtmosphere->setDefaultMoonBrightness();
}

//! Set sun brightness
void CoreLink::sunSetBrightness(double f) {
	core->currentSsystemFactory->setSunBrightness(f);
}

//! Set default sun brightness
void CoreLink::sunSetDefaultBrightness() {
	core->currentSsystemFactory->setDefaultSunBrightness();
}

//! Set flag for activating atmospheric refraction correction
void CoreLink::atmosphericRefractionSetFlag(bool b) {
	core->FlagAtmosphericRefraction = b;
}

//! Get flag for activating atmospheric refraction correction
bool CoreLink::atmosphericRefractionGetFlag() const {
	return core->FlagAtmosphericRefraction;
}

////////////////////////////////////////////////////////////////////////////////
// Time---------------------------
////////////////////////////////////////////////////////////////////////////////
//! Set time speed in JDay/sec
void CoreLink::timeSetSpeed(double ts) {
	core->timeMgr->setTimeSpeed(ts);
}

void CoreLink::timeChangeSpeed(double ts, double duration) {
	core->timeMgr->changeTimeSpeed(ts, duration);
}

//! Get time speed in JDay/sec
double CoreLink::timeGetSpeed() const {
	return core->timeMgr->getTimeSpeed();
}

//! Set the current date in Julian Day
void CoreLink::setJDay(double JD) {
	core->timeMgr->setJDay(JD);
}
//! Get the current date in Julian Day
double CoreLink::getJDay() const {
	return core->timeMgr->getJDay();
}

bool CoreLink::timeGetFlagPause() const {
	return core->timeMgr->getTimePause();
}

void CoreLink::timeSetFlagPause(bool _value) const {
	core->timeMgr->setTimePause(_value);
}

void CoreLink::timeLock() {
	core->timeMgr->lockTime();
}

void CoreLink::timeUnlock() {
	core->timeMgr->unlockTime();
}

////////////////////////////////////////////////////////////////////////////////
// dateSun---------------------------
////////////////////////////////////////////////////////////////////////////////
//! return the JD time when the sun go down
double CoreLink::dateSunRise(double _jd, double _longitude, double _latitude) {
	return core->timeMgr->dateSunRise(_jd,_longitude, _latitude);
}

//! return the JD time when the sun set up
double CoreLink::dateSunSet(double _jd, double _longitude, double _latitude) {
	return core->timeMgr->dateSunSet(_jd,_longitude, _latitude);
}

//! return the JD time when the sun cross the meridian
double CoreLink::dateSunMeridian(double _jd, double _longitude, double _latitude) {
	return core->timeMgr->dateSunMeridian(_jd,_longitude, _latitude);
}

////////////////////////////////////////////////////////////////////////////////
// for TCP usage  ---------------------------
////////////////////////////////////////////////////////////////////////////////

std::string CoreLink::getConstellationSelectedShortName() const {
	return core->currentAsterisms->getSelectedShortName();
}

std::string CoreLink::getPlanetsPosition() const {
	return core->currentSsystemFactory->getPlanetsPosition();
}

std::string CoreLink::tcpGetPosition() const {
	char tmp[512];
	memset(tmp, '\0', 512);
	sprintf(tmp,"%2.2f;%3.2f;%10.2f;%10.6f;%10.6f;",
		core->observatory->getLatitude(), core->observatory->getLongitude(),
		core->observatory->getAltitude(), core->timeMgr->getJDay(),
		core->navigation->getHeading());
	return tmp;
}

////////////////////////////////////////////////////////////////////////////////
// BodyOJM---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::BodyOJMLoad(const std::string &mode, const std::string &name, const std::string &filename, const std::string &pathFile, const Vec3f &Position, const float multiplier) {
	core->ojmMgr->load(mode, name, filename, pathFile, Position, multiplier);
}

void CoreLink::BodyOJMRemove(const std::string &mode, const std::string &name){
	core->ojmMgr->remove(mode, name);
}

void CoreLink::BodyOJMRemoveAll(const std::string &mode){
	core->ojmMgr->removeAll(mode);
}

////////////////////////////////////////////////////////////////////////////////
// Stars---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::starSetFlag(bool b) {
	core->currentHipStars->setFlagShow(b);
	core->currentStarNav->setFlagStars(b);
}

bool CoreLink::starGetFlag() const {
	return core->currentHipStars->getFlagShow();
}

void CoreLink::starSetTraceFlag(bool b) {
	core->currentHipStars->setFlagTrace(b);
}

bool CoreLink::starGetTraceFlag() const {
	return core->currentHipStars->getFlagTrace();
}

void CoreLink::starSetColorTable(int p, Vec3f a) {
	core->currentHipStars->setColorStarTable(p,a);
}

void CoreLink::starSetDuration(float f) {
	return core->currentHipStars->setFaderDuration(f);
}

void CoreLink::starNavSetDuration(float f) {
	return core->currentStarNav->setFaderDuration(f);
}

void CoreLink::starSetFlagName(bool b) {
	core->currentHipStars->setFlagNames(b);
}
bool CoreLink::starGetFlagName() const {
	return core->currentHipStars->getFlagNames();
}

void CoreLink::starNavSetFlagName(bool b) {
	core->currentStarNav->setFlagNames(b);
}

bool CoreLink::starNavGetFlagName() const {
	return core->currentStarNav->getFlagNames();
}

void CoreLink::starSetLimitingMag(float f) {
	core->currentHipStars->setMagConverterMaxScaled60DegMag(f);
}

float CoreLink::starGetLimitingMag() const {
	return core->currentHipStars->getMagConverterMaxScaled60DegMag();
}

void CoreLink::starSetFlagTwinkle(bool b) {
	core->currentHipStars->setFlagTwinkle(b);
}
bool CoreLink::starGetFlagTwinkle() const {
	return core->currentHipStars->getFlagTwinkle();
}

void CoreLink::starSetMaxMagName(float f) {
	core->currentHipStars->setMaxMagName(f);
}
float CoreLink::starGetMaxMagName() const {
	return core->currentHipStars->getMaxMagName();
}

void CoreLink::starNavSetMaxMagName(float f) {
	core->currentStarNav->setMaxMagName(f);
}
float CoreLink::starNavGetMaxMagName() const {
	return core->currentStarNav->getMaxMagName();
}

void CoreLink::starSetSizeLimit(float f) {
	core->currentStarNav->setStarSizeLimit(f);
	core->setStarSizeLimit(f);
}

void CoreLink::starSetScale(float f) {
	core->currentStarNav->setScale(f);
	core->currentHipStars->setScale(f);
}

float CoreLink::starGetScale() const {
	return core->currentHipStars->getScale();
}

void CoreLink::starSetMagScale(float f) {
	core->currentStarNav->setMagScale(f);
	core->currentHipStars->setMagScale(f);
}

float CoreLink::starGetMagScale() const {
	return core->currentHipStars->getMagScale();
}

void CoreLink::starSetTwinkleAmount(float f) {
	core->currentHipStars->setTwinkleAmount(f);
}

float CoreLink::starGetTwinkleAmount() const {
	return core->currentHipStars->getTwinkleAmount();
}

float CoreLink::getMag(int hip) {
	return core->currentHipStars->getMag(hip);
}

float CoreLink::getBaseMag(int hip) {
	return core->currentHipStars->getBaseMag(hip);
}

////////////////////////////////////////////////////////////////////////////////
// StarNavigator---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::starNavigatorClear(){
	core->currentStarNav->clear();
}

void CoreLink::starNavigatorLoad(const std::string &fileName, bool binaryMode){
	core->currentStarNav->loadData(fileName, binaryMode);
}

void CoreLink::starNavigatorLoadRaw(const std::string &fileName){
	core->currentStarNav->loadRawData(fileName);
}

void CoreLink::starNavigatorLoadOther(const std::string &fileName){
	core->currentStarNav->loadOtherData(fileName);
}

void CoreLink::starNavigatorSave(const std::string &fileName, bool binaryMode){
	core->currentStarNav->saveData(fileName, binaryMode);
}

void CoreLink::starNavigatorHideStar(int hip){
	if (!isDrawingHipStarMgr)
		core->currentStarNav->hideStar(hip);
}

void CoreLink::starNavigatorShowStar(int hip){
	if (!isDrawingHipStarMgr)
		core->currentStarNav->showStar(hip);
}

void CoreLink::starNavigatorShowAllStar(){
	core->currentStarNav->showAllStar();
}

////////////////////////////////////////////////////////////////////////////////
// UBO---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::uboSetAmbientLight(float v) {
	core->uboCam->setAmbientLight(v);
}

float CoreLink::uboGetAmbientLight() {
	return core->uboCam->getAmbientLight();
}

////////////////////////////////////////////////////////////////////////////////
// DSO---------------------------
////////////////////////////////////////////////////////////////////////////////

//! hide a particular DSO
void CoreLink::dsoSelectName(std::string DSOName, bool hide) const {
	return core->currentNebulas->selectName(hide, DSOName);
}

//! hide all DSO
void CoreLink::dsoHideAll() const {
	core->currentNebulas->hideAll();
}

//! show (unhide) all DSO
void CoreLink::dsoShowAll() const {
	core->currentNebulas->showAll();
}

//! select all DSO in constellationName to be hidden or showed
void CoreLink::dsoSelectConstellation(bool hide, std::string constellationName) const {
	core->currentNebulas->selectConstellation(hide, constellationName);
}

//! select all DSO with typeName to be hidden or showed
void CoreLink::dsoSelectType(bool hide, std::string typeName) const {
	core->currentNebulas->selectType(hide, typeName);
}

//! Insert a volumetric dso from script
void CoreLink::dsoNavInsert(std::map<std::string, std::string> &args) {
	core->currentDsoNav->insert(args);
}

//! Override dsoNavigator resources, allow loading another set of volumetric dso
void CoreLink::dsoNavOverrideCurrent(const std::string& tex_file, const std::string &tex3d_file, int depth) {
	core->currentDsoNav->overrideCurrent(tex_file, tex3d_file, depth);
}

//! Define the main volumetric object to draw
void CoreLink::dsoNavSetupVolumetric(std::map<std::string, std::string> &args, int defaultColorDepth) {
	core->currentDsoNav->setupVolumetric(args, defaultColorDepth);
}

////////////////////////////////////////////////////////////////////////////////
// Nebulae---------------------------
////////////////////////////////////////////////////////////////////////////////
//! Set flag for displaying Nebulae
void CoreLink::nebulaSetFlag(bool b) {
	core->currentNebulas->setFlagShow(b);
	core->currentDso3d->setFlagShow(b);
}

void CoreLink::dso3dSetDuration(float f) {
	return core->currentDso3d->setFaderDuration(f);
}

void CoreLink::dso3dSetFlagName(bool b) {
	core->currentDso3d->setFlagNames(b);
}

bool CoreLink::dso3dGetFlagName() const {
	return core->currentDso3d->getFlagNames();
}

//! Get flag for displaying Nebulae
bool CoreLink::nebulaGetFlag() const {
	return core->currentNebulas->getFlagShow();
}

//! Set flag for displaying Nebulae Hints
void CoreLink::nebulaSetFlagHints(bool b) {
	core->currentNebulas->setFlagHints(b);
}
//! Get flag for displaying Nebulae Hints
bool CoreLink::nebulaGetFlagHints() const {
	return core->currentNebulas->getFlagHints();
}

//! Set flag for displaying Nebulae as bright
void CoreLink::nebulaSetFlagBright(bool b) {
	core->currentNebulas->setFlagBright(b);
}
//! Get flag for displaying Nebulae as brigth
bool CoreLink::nebulaGetFlagBright() const {
	return core->currentNebulas->getFlagBright();
}

//! Set maximum magnitude at which nebulae hints are displayed
void CoreLink::nebulaSetMaxMagHints(float f) {
	core->currentNebulas->setMaxMagHints(f);
}
//! Get maximum magnitude at which nebulae hints are displayed
float CoreLink::nebulaGetMaxMagHints() const {
	return core->currentNebulas->getMaxMagHints();
}

//! return the color for the DSO object
Vec3f CoreLink::nebulaGetColorLabels() const {
	return core->currentNebulas->getLabelColor();
}

//! return the color of the DSO circle
Vec3f CoreLink::nebulaGetColorCircle() const {
	return core->currentNebulas->getCircleColor();
}

//!set Flag DSO Name who display DSO name
void CoreLink::nebulaSetFlagNames (bool value) {
	core->currentNebulas->setNebulaNames(value);
}

//!get flag DSO Name who display DSO name
bool CoreLink::nebulaGetFlagNames () {
	return core->currentNebulas->getNebulaNames();
}

void CoreLink::nebulaSetColorLabels(const Vec3f& v) {
	core->currentNebulas->setLabelColor(v);
}
void CoreLink::nebulaSetColorCircle(const Vec3f& v) {
	core->currentNebulas->setCircleColor(v);
}

void CoreLink::nebulaSetFlagIsolateSelected(bool b) {
	return core->currentNebulas->setFlagIsolateSelected(b);
}

bool CoreLink::nebulaGetFlagIsolateSelected() {
	return core->currentNebulas->getFlagIsolateSelected();
}


////////////////////////////////////////////////////////////////////////////////
// Tully---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::tullySetFlagShow(bool v) {
	core->currentTully->setFlagShow(v);
}

bool CoreLink::tullyGetFlagShow() {
	return core->currentTully->getFlagShow();
}

void CoreLink::tullySetWhiteColor(bool value)
{
	core->currentTully->setWhiteColor(value);
}

bool CoreLink::tullyGetWhiteColor() {
	return core->currentTully->getWhiteColor();
}

void CoreLink::tullySetFlagName(bool b) {
	core->currentTully->setFlagNames(b);
}

bool CoreLink::tullyGetFlagName() const {
	return core->currentTully->getFlagNames();
}

void CoreLink::tullySetDuration(float f) {
	return core->currentTully->setFaderDuration(f);
}

////////////////////////////////////////////////////////////////////////////////
// Constellations---------------------------
////////////////////////////////////////////////////////////////////////////////

void CoreLink::constellationSetFlagLines(bool b) {
	core->currentAsterisms->setFlagLines(b);
}

bool CoreLink::constellationGetFlagLines() {
	return core->currentAsterisms->getFlagLines();
}

void CoreLink::constellationSetFlagArt(bool b) {
	core->currentAsterisms->setFlagArt(b);
}

bool CoreLink::constellationGetFlagArt() {
	return core->currentAsterisms->getFlagArt();
}

void CoreLink::constellationSetFlagNames(bool b) {
	core->currentAsterisms->setFlagNames(b);
}

bool CoreLink::constellationGetFlagNames() {
	return core->currentAsterisms->getFlagNames();
}

void CoreLink::constellationSetFlagBoundaries(bool b) {
	core->currentAsterisms->setFlagBoundaries(b);
}

bool CoreLink::constellationGetFlagBoundaries() {
	return core->currentAsterisms->getFlagBoundaries();
}

void CoreLink::mediaSetFlagDualViewport(bool b) {
	return core->media->setDualViewport(b);
}

bool CoreLink::mediaGetFlagDualViewport() {
	return core->media->getDualViewport();
}

Vec3f CoreLink::constellationGetColorBoundaries() const {
	return core->currentAsterisms->getBoundaryColor();
}

void CoreLink::constellationSetArtIntensity(float f) {
	core->currentAsterisms->setArtIntensity(f);
}

float CoreLink::constellationGetArtIntensity() const {
	return core->currentAsterisms->getArtIntensity();
}

void CoreLink::constellationSetArtFadeDuration(float f) {
	core->currentAsterisms->setArtFadeDuration(f);
}

float CoreLink::constellationGetArtFadeDuration() const {
	return core->currentAsterisms->getArtFadeDuration();
}

void CoreLink::constellationSetFlagIsolateSelected(bool b) {
	core->currentAsterisms->setFlagIsolateSelected(b);
}

bool CoreLink::constellationGetFlagIsolateSelected() {
	return core->currentAsterisms->getFlagIsolateSelected();
}

void CoreLink::starSetFlagIsolateSelected(bool b) {
	return core->currentHipStars->setFlagIsolateSelected(b);
}

bool CoreLink::starGetFlagIsolateSelected() {
	return core->currentHipStars->getFlagIsolateSelected();
}

Vec3f CoreLink::constellationGetColorLine() const {
	return core->currentAsterisms->getLineColor();
}

void CoreLink::constellationSetColorLine(const Vec3f& v) {
	core->currentAsterisms->setLineColor(v);
}

void CoreLink::constellationSetColor(const Vec3f& v){
	core->currentStarLines->setColor(v);
}

Vec3f CoreLink::constellationGetColorNames() const {
	return core->currentAsterisms->getLabelColor();
}

void CoreLink::constellationSetColorNames(const Vec3f& v) {
	core->currentAsterisms->setLabelColor(v);
}

void CoreLink::constellationSetColorNames(const std::string &argName, const Vec3f& v) {
	core->currentAsterisms->setLabelColor(argName, v);
}

Vec3f CoreLink::constellationGetColorArt() const {
	return core->currentAsterisms->getArtColor();
}

void CoreLink::constellationSetColorArt(const Vec3f& v) {
	core->currentAsterisms->setArtColor(v);
}

void CoreLink::constellationSetColorBoundaries(const Vec3f& v) {
	core->currentAsterisms->setBoundaryColor(v);
}

void CoreLink::constellationSetLineColor(const std::string &argName, const Vec3f& v) {
	core->currentAsterisms->setLineColor(argName, v);
}

void CoreLink::constellationSetArtIntensity(const std::string &argName, float intensity) {
	core->currentAsterisms->setArtIntensity(argName, intensity);
}

void CoreLink::bodyTraceSetFlag(bool b) const {
	core->currentSsystemFactory->bodyTraceSetFlag(b);
}

bool CoreLink::bodyTraceGetFlag() const {
	return core->currentSsystemFactory->bodyTraceGetFlag();
}

void CoreLink::bodyPenUp() const {
	core->currentSsystemFactory->upPen();
}

void CoreLink::bodyPenDown() const {
	core->currentSsystemFactory->downPen();
}

void CoreLink::bodyPenToggle() const {
	core->currentSsystemFactory->togglePen();
}

void CoreLink::bodyTraceClear () const {
	core->currentSsystemFactory->clear();
}

void CoreLink::bodyTraceHide(std::string value) const {
	if (value=="all")
		core->currentSsystemFactory->hide(-1);
	else
		core->currentSsystemFactory->hide(Utility::strToInt(value));
}

void CoreLink::bodyTraceBodyChange(std::string bodyName) const {
	if (bodyName=="selected")
		core->currentSsystemFactory->bodyTraceBodyChange(core->selected_object.getEnglishName());
	else
		core->currentSsystemFactory->bodyTraceBodyChange(bodyName);
}

void CoreLink::cameraDisplayAnchor() {
	core->currentSsystemFactory->cameraDisplayAnchor();
}

bool CoreLink::cameraAddAnchor(stringHash_t& param) {
	return core->currentSsystemFactory->cameraAddAnchor(param);
}

bool CoreLink::cameraRemoveAnchor(const std::string &name) {
	return core->currentSsystemFactory->cameraRemoveAnchor(name);
}

bool CoreLink::cameraSwitchToAnchor(const std::string &name) {
	return core->currentSsystemFactory->cameraSwitchToAnchor(name);
}

bool CoreLink::cameraMoveToPoint(double x, double y, double z){
	return core->currentSsystemFactory->cameraMoveToPoint(x,y,z);
}

bool CoreLink::cameraMoveToPoint(double x, double y, double z, double time){
	return core->currentSsystemFactory->cameraMoveToPoint(x,y,z,time);
}

bool CoreLink::cameraMoveToBody(const std::string& bodyName, double time, double alt){

	if(bodyName == "selected"){
		return core->currentSsystemFactory->cameraMoveToBody(core->getSelectedPlanetEnglishName(), time, alt);
	}

	if(bodyName == "default"){
		return core->currentSsystemFactory->cameraMoveToBody(core->currentSsystemFactory->getEarth()->getEnglishName(), time, alt);
	}

	return core->currentSsystemFactory->cameraMoveToBody(bodyName,time, alt);
}

bool CoreLink::cameraMoveRelativeXYZ( double x, double y, double z) {
	return core->currentSsystemFactory->cameraMoveRelativeXYZ(x,y,z);
}

bool CoreLink::cameraTransitionToPoint(const std::string& name){
	return core->currentSsystemFactory->cameraTransitionToPoint(name);
}

bool CoreLink::cameraTransitionToBody(const std::string& name){

	if(name == "selected"){
		return core->currentSsystemFactory->cameraTransitionToBody(core->getSelectedPlanetEnglishName());
	}

	return core->currentSsystemFactory->cameraTransitionToBody(name);
}

bool CoreLink::cameraSetFollowRotation(const std::string& name, bool value){
	return core->currentSsystemFactory->cameraSetFollowRotation(value);
}

void CoreLink::cameraSetRotationMultiplierCondition(float v) {
	core->currentSsystemFactory->cameraSetRotationMultiplierCondition(v);
}

bool CoreLink::cameraAlignWithBody(const std::string& name, double duration){
	return core->currentSsystemFactory->cameraAlignWithBody(name,duration);
}

void CoreLink::setFlagLightTravelTime(bool b) {
	core->currentSsystemFactory->setFlagLightTravelTime(b);
}

bool CoreLink::getFlagLightTravelTime() const {
	return core->currentSsystemFactory->getFlagLightTravelTime();
}

void CoreLink::startPlanetsTrails(bool b) {
	core->currentSsystemFactory->startTrails(b);
}

void CoreLink::setPlanetsSelected(const std::string& englishName) {
	core->currentSsystemFactory->setSelected(englishName);
}

void CoreLink::setFlagMoonScaled(bool b) {
	core->currentSsystemFactory->setFlagMoonScale(b);
}

bool CoreLink::getFlagMoonScaled() const {
	return core->currentSsystemFactory->getFlagMoonScale();
}

void CoreLink::setFlagSunScaled(bool b) {
	core->currentSsystemFactory->setFlagSunScale(b);
}

bool CoreLink::getFlagSunScaled() const {
	return core->currentSsystemFactory->getFlagSunScale();
}

void CoreLink::setMoonScale(float f, bool resident) {
	if (f<0) core->currentSsystemFactory->setMoonScale(1., false);
	else core->currentSsystemFactory->setMoonScale(f, resident);
}

float CoreLink::getMoonScale() const {
	return core->currentSsystemFactory->getMoonScale();
}

void CoreLink::setSunScale(float f, bool resident) {
	if (f<0) core->currentSsystemFactory->setSunScale(1., false);
	else core->currentSsystemFactory->setSunScale(f, resident);
}

void CoreLink::setFlagClouds(bool b) {
	core->currentSsystemFactory->setFlagClouds(b);
}

bool CoreLink::getFlagClouds() const {
	return core->currentSsystemFactory->getFlag(BODY_FLAG::F_CLOUDS);
}

float CoreLink::getSunScale() const {
	return core->currentSsystemFactory->getSunScale();
}

void CoreLink::initialSolarSystemBodies() {
	return core->currentSsystemFactory->initialSolarSystemBodies();
}

void CoreLink::setPlanetHidden(std::string name, bool planethidden) {
	core->currentSsystemFactory->setPlanetHidden(name, planethidden);
}

bool CoreLink::getPlanetHidden(std::string name) {
	return core->currentSsystemFactory->getPlanetHidden(name);
}

void CoreLink::planetsSetFlag(bool b) {
	core->currentSsystemFactory->setFlagPlanets(b);
}

bool CoreLink::planetsGetFlag() const {
	return core->currentSsystemFactory->getFlagShow();
}

void CoreLink::planetsSetFlagTrails(bool b) {
	core->currentSsystemFactory->setFlagTrails(b);
}

bool CoreLink::planetsGetFlagTrails() const {
	return core->currentSsystemFactory->getFlag(BODY_FLAG::F_TRAIL);
}

void CoreLink::planetsSetFlagAxis(bool b) {
	core->currentSsystemFactory->setFlagAxis(b);
}

bool CoreLink::planetsGetFlagAxis() const {
	return core->currentSsystemFactory->getFlag(BODY_FLAG::F_AXIS);
}

void CoreLink::planetsSetFlagHints(bool b) {
	core->currentSsystemFactory->setFlagHints(b);
}

bool CoreLink::planetsGetFlagHints() const {
	return core->currentSsystemFactory->getFlag(BODY_FLAG::F_HINTS);
}

void CoreLink::planetsSetFlagOrbits(bool b) {
	core->currentSsystemFactory->setFlagPlanetsOrbits(b);
}

void CoreLink::planetsSetFlagOrbits(const std::string &_name, bool b) {
	core->currentSsystemFactory->setFlagPlanetsOrbits(_name, b);
}

void CoreLink::planetSwitchTexMap(const std::string &_name, bool b) {
	if (_name=="selected") core->currentSsystemFactory->switchPlanetTexMap(core->selected_object.getEnglishName(), b);
	else core->currentSsystemFactory->switchPlanetTexMap(_name, b);
}

bool CoreLink::planetGetSwitchTexMap(const std::string &_name) {
	if (_name=="selected") return core->currentSsystemFactory->getSwitchPlanetTexMap(core->selected_object.getEnglishName());
	else return core->currentSsystemFactory->getSwitchPlanetTexMap(_name);
}

void CoreLink::planetCreateTexSkin(const std::string &name, const std::string &texName){
	core->currentSsystemFactory->createTexSkin(name, texName);
}

bool CoreLink::planetsGetFlagOrbits() const {
	return core->currentSsystemFactory->getFlagPlanetsOrbits();
}

void CoreLink::satellitesSetFlagOrbits(bool b) {
	core->currentSsystemFactory->setFlagSatellitesOrbits(b);
}

bool CoreLink::satellitesGetFlagOrbits() const {
	return core->currentSsystemFactory->getFlagSatellitesOrbits();
}

void CoreLink::planetSetFlagOrbits(bool b) {
	core->currentSsystemFactory->setFlagSatellitesOrbits(b);
	core->currentSsystemFactory->setFlagPlanetsOrbits(b);
	//ssystem->setFlagOrbits(b);
}

void CoreLink::planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const {
	core->currentSsystemFactory->setBodyColor(englishName, color, c);
}

Vec3f CoreLink::planetGetColor(const std::string& englishName, const std::string& color) const {
	return core->currentSsystemFactory->getBodyColor(englishName, color);
}

void CoreLink::planetSetDefaultColor(const std::string& color, Vec3f c) const {
	core->currentSsystemFactory->setDefaultBodyColor(color, c);
}

Vec3f CoreLink::planetGetDefaultColor(const std::string& colorName) const {
	return core->currentSsystemFactory->getDefaultBodyColor(colorName);
}

bool CoreLink::hideSatellitesFlag(){
	return core->currentSsystemFactory->getHideSatellitesFlag();
}

void CoreLink::setHideSatellites(bool val){
	core->currentSsystemFactory->toggleHideSatellites(val);
}

void CoreLink::planetsSetScale(float f) {
	core->currentSsystemFactory->setScale(f);
}

double CoreLink::getSunAltitude() const {
	return core->currentSsystemFactory->getSunAltitude(core->navigation);
}

double CoreLink::getSunAzimuth() const {
	return core->currentSsystemFactory->getSunAzimuth(core->navigation);
}

double CoreLink::getSelectedAZ() const {
	return core->currentSsystemFactory->getSelectedAZ(core->navigation);
}

double CoreLink::getSelectedALT() const {
	return core->currentSsystemFactory->getSelectedALT(core->navigation);
}

double CoreLink::getSelectedRA() const {
	return core->currentSsystemFactory->getSelectedRA(core->navigation);
}

double CoreLink::getSelectedDE() const {
	return core->currentSsystemFactory->getSelectedDE(core->navigation);
}

double CoreLink::getSelectedStarRA() const {
	return core->currentSsystemFactory->getSelectedStarRA(core->navigation);
}

double CoreLink::getSelectedStarDE() const {
	return core->currentSsystemFactory->getSelectedStarDE(core->navigation);
}

int CoreLink::getLanguage() const {
	return double(core->getLanguage());
}

double CoreLink::getBodySelected() const {
	return double(core->getSelectedBodyName());
}

void CoreLink::bodySetFlagIsolateSelected(bool b) {
	core->currentSsystemFactory->setFlagIsolateSelected(b);
}

bool CoreLink::bodyGetFlagIsolateSelected() {
	return core->currentSsystemFactory->getFlagIsolateSelected();
}

void CoreLink::planetSetSizeScale(std::string name, float f) {
	core->currentSsystemFactory->setPlanetSizeScale(name, f);
}

void CoreLink::planetTesselation(std::string name, int value) {
	core->currentSsystemFactory->planetTesselation(name,value);
}

////////////////////////////////////////////////////////////////////////////////
// Fog---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set flag for displaying Fog
void CoreLink::fogSetFlag(bool b) {
	core->currentLandscape->fogSetFlagShow(b);
}
//! Get flag for displaying Fog
bool CoreLink::fogGetFlag() const {
	return core->currentLandscape->fogGetFlagShow();
}

////////////////////////////////////////////////////////////////////////////////
// Landscape---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Get flag for displaying Landscape
void CoreLink::landscapeSetFlag(bool b) {
	core->currentLandscape->setFlagShow(b);
}
//! Get flag for displaying Landscape
bool CoreLink::landscapeGetFlag() const {
	return core->currentLandscape->getFlagShow();
}

void CoreLink::rotateLandscape(double rotation) {
	core->currentLandscape->setRotation(rotation);
}

std::string CoreLink::landscapeGetName() {
	return core->currentLandscape->getName();
}
