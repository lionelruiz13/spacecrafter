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
	return currentSsystemFactory->cameraSave(AppSettings::Instance()->getUserDir() + "anchors/" + name);
}

bool CoreLink::loadCameraPosition(const std::string& filename)
{
	return currentSsystemFactory->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
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
    currentSkyLineMgr->setColor(name, a);
};

void CoreLink::skyGridMgrSetColor(SKYGRID_TYPE name, Vec3f a) {
	currentSkyGridMgr->setColor(name, a);
}

const Vec3f& CoreLink::skyLineMgrGetColor(SKYLINE_TYPE name) {
	return currentSkyLineMgr->getColor(name);
}

const Vec3f& CoreLink::skyGridMgrGetColor(SKYGRID_TYPE name) {
	return currentSkyGridMgr->getColor(name);
}

void CoreLink::skyLineMgrFlipFlagShow(SKYLINE_TYPE name) {
	currentSkyLineMgr->flipFlagShow(name);
}

void CoreLink::skyGridMgrFlipFlagShow(SKYGRID_TYPE name) {
	currentSkyGridMgr->flipFlagShow(name);
}

void CoreLink::skyLineMgrSetFlagShow(SKYLINE_TYPE name, bool value) {
	currentSkyLineMgr->setFlagShow(name, value);
}

void CoreLink::skyGridMgrSetFlagShow(SKYGRID_TYPE name, bool value) {
	currentSkyGridMgr->setFlagShow(name, value);
}

bool CoreLink::skyLineMgrGetFlagShow(SKYLINE_TYPE name) {
	return currentSkyLineMgr->getFlagShow(name);
}

bool CoreLink::skyGridMgrGetFlagShow(SKYGRID_TYPE name) {
	return currentSkyGridMgr->getFlagShow(name);
}

////////////////////////////////////////////////////////////////////////////////
// Oort    ---------------------------
////////////////////////////////////////////////////////////////////////////////
bool CoreLink::oortGetFlagShow() const {
	return core->oort->getFlagShow();
}

void CoreLink::oortSetFlagShow(bool b) {
	core->oort->setFlagShow(b);
}

////////////////////////////////////////////////////////////////////////////////
// Milky Way---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::milkyWaySetFlag(bool b) {
	core->milky_way->setFlagShow(b);
}

bool CoreLink::milkyWayGetFlag() const {
	return core->milky_way->getFlagShow();
}

void CoreLink::milkyWaySetFlagZodiacal(bool b) {
	core->milky_way->setFlagZodiacal(b);
}

bool CoreLink::milkyWayGetFlagZodiacal() const {
	return core->milky_way->getFlagZodiacal();
}

void CoreLink::milkyWaySetIntensity(float f) {
	core->milky_way->setIntensity(f);
}

void CoreLink::milkyWaySetZodiacalIntensity(float f) {
	core->milky_way->setZodiacalIntensity(f);
}

float CoreLink::milkyWayGetIntensity() const {
	return core->milky_way->getIntensity();
}

void CoreLink::milkyWayRestoreDefault() {
	core->milky_way->restoreDefaultMilky();
}

void CoreLink::milkyWaySetDuration(float f) {
	core->milky_way->setFaderDuration(f);
}

void CoreLink::milkyWayRestoreIntensity() {
	core->milky_way->restoreIntensity();
}

void CoreLink::milkyWayChangeState(const std::string& mdir, float _intensity) {
	core->milky_way->changeMilkywayState(mdir, _intensity);
}

void CoreLink::milkyWayChangeStateWithoutIntensity(const std::string& mdir) {
	core->milky_way->changeMilkywayStateWithoutIntensity(mdir);
}

////////////////////////////////////////////////////////////////////////////////
// Meteors---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set Meteor Rate in number per hour
void CoreLink::setMeteorsRate(int f) {
	currentMeteors->setZHR(f);
}

//! Get Meteor Rate in number per hour
int CoreLink::getMeteorsRate() const {
	return currentMeteors->getZHR();
}

void CoreLink::createRadiant(int day, const Vec3f newRadiant) {
	currentMeteors->createRadiant(day, newRadiant);
}

void CoreLink::clearRadiants() {
	currentMeteors->clearRadiants();
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
// Mode Pointers Update    ---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::updateModePointers() {
	// Update all mode-specific collection pointers
	updateSkyDisplayMgrPointer();
	updateStarLinesPointer();
	updateSkyGridMgrPointer();
	updateSkyLineMgrPointer();
	updateMeteorsPointer();
	updateNebulasPointer();
	updateIlluminatesPointer();
	updateDso3dPointer();
	updateStarNavPointer();
	updateSsystemFactoryPointer();
}

void CoreLink::updateSkyGridMgrPointer() {
	currentSkyGridMgr = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxSkyGridMgr.get() 
		: core->skyGridMgr.get();
}

void CoreLink::updateSkyLineMgrPointer() {
	currentSkyLineMgr = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxSkyLineMgr.get() 
		: core->skyLineMgr.get();
}

void CoreLink::updateSkyDisplayMgrPointer() {
	currentSkyDisplayMgr = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxSkyDisplayMgr.get() 
		: core->skyDisplayMgr.get();
}

void CoreLink::updateStarLinesPointer() {
	currentStarLines = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxStarLines.get() 
		: core->starLines.get();
}

void CoreLink::updateMeteorsPointer() {
	currentMeteors = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxMeteors.get() 
		: core->meteors.get();
}

void CoreLink::updateNebulasPointer() {
	currentNebulas = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxNebulas.get() 
		: core->nebulas.get();
}

void CoreLink::updateIlluminatesPointer() {
	currentIlluminates = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxIlluminates.get() 
		: core->illuminates.get();
}

void CoreLink::updateDso3dPointer() {
	currentDso3d = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxDso3d.get() 
		: core->dso3d.get();
}

void CoreLink::updateStarNavPointer() {
	currentStarNav = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxStarNav.get() 
		: core->starNav.get();
}

void CoreLink::updateSsystemFactoryPointer() {
	currentSsystemFactory = (core->getFlagIngalaxy() == MODULE::IN_SANDBOX) 
		? core->sandboxSsystemFactory 
		: core->ssystemFactory;
}

////////////////////////////////////////////////////////////////////////////////
// SkyDisplayMgr    ---------------------------
////////////////////////////////////////////////////////////////////////////////
bool CoreLink::skyDisplayMgrGetFlag(SKYDISPLAY_NAME nameObj) {
	return currentSkyDisplayMgr->getFlagShow(nameObj);
}

void CoreLink::skyDisplayMgrSetFlag(SKYDISPLAY_NAME nameObj, bool v) {
	currentSkyDisplayMgr->setFlagShow(nameObj,v);
}

void CoreLink::skyDisplayMgrFlipFlag(SKYDISPLAY_NAME nameObj) {
	currentSkyDisplayMgr->flipFlagShow(nameObj);
}

void CoreLink::skyDisplayMgrSetColor(SKYDISPLAY_NAME nameObj, const Vec3f& v) {
	currentSkyDisplayMgr->setColor(nameObj,v);
}

void CoreLink::skyDisplayMgrClear(SKYDISPLAY_NAME nameObj) {
	currentSkyDisplayMgr->clear(nameObj);
}

void CoreLink::skyDisplayMgrLoadData(SKYDISPLAY_NAME nameObj, const std::string& fileName) {
	currentSkyDisplayMgr->loadData(nameObj,fileName);
}

void CoreLink::skyDisplayMgrLoadString(SKYDISPLAY_NAME nameObj, const std::string& dataStr) {
	currentSkyDisplayMgr->loadString(nameObj,dataStr);
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
	currentStarLines->setFlagShow(b);
}

void CoreLink::starLinesSelectedSetFlag(bool b) {
	currentStarLines->setFlagSelected(b);
}

bool CoreLink::starLinesSelectedGetFlag() const {
	return currentStarLines->getFlagSelected();
}

//! Get flag for displaying
bool CoreLink::starLinesGetFlag() const {
	return currentStarLines->getFlagShow();
}

//! Empty all plot buffers
void CoreLink::starLinesDrop() const {
	currentStarLines->drop();
}

//! Loads a set of asterisms from a file
void CoreLink::starLinesLoadData(const std::string &fileName) {
	currentStarLines->loadData(fileName);
}

//! Loads an asterism from a line
void CoreLink::starLinesLoadAsterism(std::string record) const {
	currentStarLines->loadStringData(record);
}

//! deletes the complete catalog of asterisms
void CoreLink::starLinesClear() {
	currentStarLines->clear();
}

void CoreLink::starLinesSaveCat(const std::string &fileName, bool binaryMode){
	currentStarLines->saveCat(fileName, binaryMode);
}

void CoreLink::starLinesLoadCat(const std::string &fileName, bool binaryMode){
	currentStarLines->loadCat(fileName, binaryMode);
}

void CoreLink::starLinesLoadHipStar(int name, Vec3f position) {
	currentStarLines->loadHipStar(name, position);
}

////////////////////////////////////////////////////////////////////////////////
// Illuminate---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::illuminateSetSize (double value) {
	currentIlluminates->setDefaultSize(value);
}

void CoreLink::illuminateLoadConstellation(const std::string& abbreviation, double size, double rotation) {
	currentIlluminates->loadConstellation(abbreviation, size, rotation);
}
void CoreLink::illuminateLoadConstellation(const std::string& abbreviation,const Vec3f& color, double size, double rotation) {
	currentIlluminates->loadConstellation(abbreviation, color, size, rotation);
}
void CoreLink::illuminateLoadAllConstellation(double size, double rotation) {
	currentIlluminates->loadAllConstellation(size, rotation);
}

void CoreLink::illuminateLoad(int number, double size, double rotation) {
	currentIlluminates->load(number, size, rotation);
}

void CoreLink::illuminateLoad(int number, const Vec3f& _color, double size, double rotation) {
	currentIlluminates->load(number, _color, size, rotation);
}

void CoreLink::illuminateRemove(unsigned int name) 	{
	currentIlluminates->remove(name);
}

void CoreLink::illuminateRemoveConstellation(const std::string abbreviation) 	{
	currentIlluminates->removeConstellation(abbreviation);
}

void CoreLink::illuminateRemoveAllConstellation() 	{
	currentIlluminates->removeAllConstellation();
}

void CoreLink::illuminateRemoveAll()
{
	currentIlluminates->removeAll();
}

void CoreLink::illuminateChangeTex(const std::string& _fileName)	{
	currentIlluminates->changeTex(_fileName);
}

void CoreLink::illuminateRemoveTex()	{
	currentIlluminates->removeTex();
}


	////////////////////////////////////////////////////////////////////////////////
	// Atmosphere---------------------------
	////////////////////////////////////////////////////////////////////////////////

//! Set flag for displaying Atmosphere
void CoreLink::atmosphereSetFlag(bool b) {
	core->bodyDecor->setAtmosphereState(b);
	core->setBodyDecor();
}
//! Get flag for displaying Atmosphere
bool CoreLink::atmosphereGetFlag() const {
	return core->bodyDecor->getAtmosphereState();
}

//! Set atmosphere fade duration in s
void CoreLink::atmosphereSetFadeDuration(float f) {
	core->atmosphere->setFaderDuration(f);
}

//! Set default atmosphere fade duration
void CoreLink::atmosphereSetDefaultFadeDuration() {
	core->atmosphere->setDefaultFaderDuration();
}

//! Set moon brightness
void CoreLink::moonSetBrightness(double f) {
	core->atmosphere->setMoonBrightness(f);
}

//! Set default moon brightness
void CoreLink::moonSetDefaultBrightness() {
	core->atmosphere->setDefaultMoonBrightness();
}

//! Set sun brightness
void CoreLink::sunSetBrightness(double f) {
	currentSsystemFactory->setSunBrightness(f);
}

//! Set default sun brightness
void CoreLink::sunSetDefaultBrightness() {
	currentSsystemFactory->setDefaultSunBrightness();
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
	return core->asterisms->getSelectedShortName();
}

std::string CoreLink::getPlanetsPosition() const {
	return currentSsystemFactory->getPlanetsPosition();
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
	core->hip_stars->setFlagShow(b);
	currentStarNav->setFlagStars(b);
}

bool CoreLink::starGetFlag() const {
	return core->hip_stars->getFlagShow();
}

void CoreLink::starSetTraceFlag(bool b) {
	core->hip_stars->setFlagTrace(b);
}

bool CoreLink::starGetTraceFlag() const {
	return core->hip_stars->getFlagTrace();
}

void CoreLink::starSetColorTable(int p, Vec3f a) {
	core->hip_stars->setColorStarTable(p,a);
}

void CoreLink::starSetDuration(float f) {
	return core->hip_stars->setFaderDuration(f);
}

void CoreLink::starNavSetDuration(float f) {
	return currentStarNav->setFaderDuration(f);
}

void CoreLink::starSetFlagName(bool b) {
	core->hip_stars->setFlagNames(b);
}
bool CoreLink::starGetFlagName() const {
	return core->hip_stars->getFlagNames();
}

void CoreLink::starNavSetFlagName(bool b) {
	currentStarNav->setFlagNames(b);
}

bool CoreLink::starNavGetFlagName() const {
	return currentStarNav->getFlagNames();
}

void CoreLink::starSetLimitingMag(float f) {
	core->hip_stars->setMagConverterMaxScaled60DegMag(f);
}

float CoreLink::starGetLimitingMag() const {
	return core->hip_stars->getMagConverterMaxScaled60DegMag();
}

void CoreLink::starSetFlagTwinkle(bool b) {
	core->hip_stars->setFlagTwinkle(b);
}
bool CoreLink::starGetFlagTwinkle() const {
	return core->hip_stars->getFlagTwinkle();
}

void CoreLink::starSetMaxMagName(float f) {
	core->hip_stars->setMaxMagName(f);
}
float CoreLink::starGetMaxMagName() const {
	return core->hip_stars->getMaxMagName();
}

void CoreLink::starNavSetMaxMagName(float f) {
	currentStarNav->setMaxMagName(f);
}
float CoreLink::starNavGetMaxMagName() const {
	return currentStarNav->getMaxMagName();
}

void CoreLink::starSetSizeLimit(float f) {
	currentStarNav->setStarSizeLimit(f);
	core->setStarSizeLimit(f);
}

void CoreLink::starSetScale(float f) {
	currentStarNav->setScale(f);
	core->hip_stars->setScale(f);
}

float CoreLink::starGetScale() const {
	return core->hip_stars->getScale();
}

void CoreLink::starSetMagScale(float f) {
	currentStarNav->setMagScale(f);
	core->hip_stars->setMagScale(f);
}

float CoreLink::starGetMagScale() const {
	return core->hip_stars->getMagScale();
}

void CoreLink::starSetTwinkleAmount(float f) {
	core->hip_stars->setTwinkleAmount(f);
}

float CoreLink::starGetTwinkleAmount() const {
	return core->hip_stars->getTwinkleAmount();
}

float CoreLink::getMag(int hip) {
	return core->hip_stars->getMag(hip);
}

float CoreLink::getBaseMag(int hip) {
	return core->hip_stars->getBaseMag(hip);
}

////////////////////////////////////////////////////////////////////////////////
// StarNavigator---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::starNavigatorClear(){
	currentStarNav->clear();
}

void CoreLink::starNavigatorLoad(const std::string &fileName, bool binaryMode){
	currentStarNav->loadData(fileName, binaryMode);
}

void CoreLink::starNavigatorLoadRaw(const std::string &fileName){
	currentStarNav->loadRawData(fileName);
}

void CoreLink::starNavigatorLoadOther(const std::string &fileName){
	currentStarNav->loadOtherData(fileName);
}

void CoreLink::starNavigatorSave(const std::string &fileName, bool binaryMode){
	currentStarNav->saveData(fileName, binaryMode);
}

void CoreLink::starNavigatorHideStar(int hip){
	if (!isDrawingHipStarMgr)
		currentStarNav->hideStar(hip);
}

void CoreLink::starNavigatorShowStar(int hip){
	if (!isDrawingHipStarMgr)
		currentStarNav->showStar(hip);
}

void CoreLink::starNavigatorShowAllStar(){
	currentStarNav->showAllStar();
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
	return core->nebulas->selectName(hide, DSOName);
}

//! hide all DSO
void CoreLink::dsoHideAll() const {
	core->nebulas->hideAll();
}

//! show (unhide) all DSO
void CoreLink::dsoShowAll() const {
	core->nebulas->showAll();
}

//! select all DSO in constellationName to be hidden or showed
void CoreLink::dsoSelectConstellation(bool hide, std::string constellationName) const {
	core->nebulas->selectConstellation(hide, constellationName);
}

//! select all DSO with typeName to be hidden or showed
void CoreLink::dsoSelectType(bool hide, std::string typeName) const {
	core->nebulas->selectType(hide, typeName);
}

//! Insert a volumetric dso from script
void CoreLink::dsoNavInsert(std::map<std::string, std::string> &args) {
	core->dsoNav->insert(args);
}

//! Override dsoNavigator resources, allow loading another set of volumetric dso
void CoreLink::dsoNavOverrideCurrent(const std::string& tex_file, const std::string &tex3d_file, int depth) {
	core->dsoNav->overrideCurrent(tex_file, tex3d_file, depth);
}

//! Define the main volumetric object to draw
void CoreLink::dsoNavSetupVolumetric(std::map<std::string, std::string> &args, int defaultColorDepth) {
	core->dsoNav->setupVolumetric(args, defaultColorDepth);
}

////////////////////////////////////////////////////////////////////////////////
// Nebulae---------------------------
////////////////////////////////////////////////////////////////////////////////
//! Set flag for displaying Nebulae
void CoreLink::nebulaSetFlag(bool b) {
	currentNebulas->setFlagShow(b);
	currentDso3d->setFlagShow(b);
}

void CoreLink::dso3dSetDuration(float f) {
	return currentDso3d->setFaderDuration(f);
}

void CoreLink::dso3dSetFlagName(bool b) {
	currentDso3d->setFlagNames(b);
}

bool CoreLink::dso3dGetFlagName() const {
	return currentDso3d->getFlagNames();
}

//! Get flag for displaying Nebulae
bool CoreLink::nebulaGetFlag() const {
	return currentNebulas->getFlagShow();
}

//! Set flag for displaying Nebulae Hints
void CoreLink::nebulaSetFlagHints(bool b) {
	currentNebulas->setFlagHints(b);
}
//! Get flag for displaying Nebulae Hints
bool CoreLink::nebulaGetFlagHints() const {
	return currentNebulas->getFlagHints();
}

//! Set flag for displaying Nebulae as bright
void CoreLink::nebulaSetFlagBright(bool b) {
	currentNebulas->setFlagBright(b);
}
//! Get flag for displaying Nebulae as brigth
bool CoreLink::nebulaGetFlagBright() const {
	return currentNebulas->getFlagBright();
}

//! Set maximum magnitude at which nebulae hints are displayed
void CoreLink::nebulaSetMaxMagHints(float f) {
	currentNebulas->setMaxMagHints(f);
}
//! Get maximum magnitude at which nebulae hints are displayed
float CoreLink::nebulaGetMaxMagHints() const {
	return currentNebulas->getMaxMagHints();
}

//! return the color for the DSO object
Vec3f CoreLink::nebulaGetColorLabels() const {
	return currentNebulas->getLabelColor();
}

//! return the color of the DSO circle
Vec3f CoreLink::nebulaGetColorCircle() const {
	return currentNebulas->getCircleColor();
}

//!set Flag DSO Name who display DSO name
void CoreLink::nebulaSetFlagNames (bool value) {
	currentNebulas->setNebulaNames(value);
}

//!get flag DSO Name who display DSO name
bool CoreLink::nebulaGetFlagNames () {
	return currentNebulas->getNebulaNames();
}

void CoreLink::nebulaSetColorLabels(const Vec3f& v) {
	currentNebulas->setLabelColor(v);
}
void CoreLink::nebulaSetColorCircle(const Vec3f& v) {
	currentNebulas->setCircleColor(v);
}

void CoreLink::nebulaSetFlagIsolateSelected(bool b) {
	return currentNebulas->setFlagIsolateSelected(b);
}

bool CoreLink::nebulaGetFlagIsolateSelected() {
	return currentNebulas->getFlagIsolateSelected();
}


////////////////////////////////////////////////////////////////////////////////
// Tully---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::tullySetFlagShow(bool v) {
	core->tully->setFlagShow(v);
}

bool CoreLink::tullyGetFlagShow() {
	return core->tully->getFlagShow();
}

void CoreLink::tullySetWhiteColor(bool value)
{
	core->tully->setWhiteColor(value);
}

bool CoreLink::tullyGetWhiteColor() {
	return core->tully->getWhiteColor();
}

void CoreLink::tullySetFlagName(bool b) {
	core->tully->setFlagNames(b);
}

bool CoreLink::tullyGetFlagName() const {
	return core->tully->getFlagNames();
}

void CoreLink::tullySetDuration(float f) {
	return core->tully->setFaderDuration(f);
}

////////////////////////////////////////////////////////////////////////////////
// Constellations---------------------------
////////////////////////////////////////////////////////////////////////////////

void CoreLink::constellationSetFlagLines(bool b) {
	core->asterisms->setFlagLines(b);
}

bool CoreLink::constellationGetFlagLines() {
	return core->asterisms->getFlagLines();
}

void CoreLink::constellationSetFlagArt(bool b) {
	core->asterisms->setFlagArt(b);
}

bool CoreLink::constellationGetFlagArt() {
	return core->asterisms->getFlagArt();
}

void CoreLink::constellationSetFlagNames(bool b) {
	core->asterisms->setFlagNames(b);
}

bool CoreLink::constellationGetFlagNames() {
	return core->asterisms->getFlagNames();
}

void CoreLink::constellationSetFlagBoundaries(bool b) {
	core->asterisms->setFlagBoundaries(b);
}

bool CoreLink::constellationGetFlagBoundaries() {
	return core->asterisms->getFlagBoundaries();
}

void CoreLink::mediaSetFlagDualViewport(bool b) {
	return core->media->setDualViewport(b);
}

bool CoreLink::mediaGetFlagDualViewport() {
	return core->media->getDualViewport();
}

Vec3f CoreLink::constellationGetColorBoundaries() const {
	return core->asterisms->getBoundaryColor();
}

void CoreLink::constellationSetArtIntensity(float f) {
	core->asterisms->setArtIntensity(f);
}

float CoreLink::constellationGetArtIntensity() const {
	return core->asterisms->getArtIntensity();
}

void CoreLink::constellationSetArtFadeDuration(float f) {
	core->asterisms->setArtFadeDuration(f);
}

float CoreLink::constellationGetArtFadeDuration() const {
	return core->asterisms->getArtFadeDuration();
}

void CoreLink::constellationSetFlagIsolateSelected(bool b) {
	core->asterisms->setFlagIsolateSelected(b);
}

bool CoreLink::constellationGetFlagIsolateSelected() {
	return core->asterisms->getFlagIsolateSelected();
}

void CoreLink::starSetFlagIsolateSelected(bool b) {
	return core->hip_stars->setFlagIsolateSelected(b);
}

bool CoreLink::starGetFlagIsolateSelected() {
	return core->hip_stars->getFlagIsolateSelected();
}

Vec3f CoreLink::constellationGetColorLine() const {
	return core->asterisms->getLineColor();
}

void CoreLink::constellationSetColorLine(const Vec3f& v) {
	core->asterisms->setLineColor(v);
}

void CoreLink::constellationSetColor(const Vec3f& v){
	core->starLines->setColor(v);
}

Vec3f CoreLink::constellationGetColorNames() const {
	return core->asterisms->getLabelColor();
}

void CoreLink::constellationSetColorNames(const Vec3f& v) {
	core->asterisms->setLabelColor(v);
}

void CoreLink::constellationSetColorNames(const std::string &argName, const Vec3f& v) {
	core->asterisms->setLabelColor(argName, v);
}

Vec3f CoreLink::constellationGetColorArt() const {
	return core->asterisms->getArtColor();
}

void CoreLink::constellationSetColorArt(const Vec3f& v) {
	core->asterisms->setArtColor(v);
}

void CoreLink::constellationSetColorBoundaries(const Vec3f& v) {
	core->asterisms->setBoundaryColor(v);
}

void CoreLink::constellationSetLineColor(const std::string &argName, const Vec3f& v) {
	core->asterisms->setLineColor(argName, v);
}

void CoreLink::constellationSetArtIntensity(const std::string &argName, float intensity) {
	core->asterisms->setArtIntensity(argName, intensity);
}

void CoreLink::bodyTraceSetFlag(bool b) const {
	currentSsystemFactory->bodyTraceSetFlag(b);
}

bool CoreLink::bodyTraceGetFlag() const {
	return currentSsystemFactory->bodyTraceGetFlag();
}

void CoreLink::bodyPenUp() const {
	currentSsystemFactory->upPen();
}

void CoreLink::bodyPenDown() const {
	currentSsystemFactory->downPen();
}

void CoreLink::bodyPenToggle() const {
	currentSsystemFactory->togglePen();
}

void CoreLink::bodyTraceClear () const {
	currentSsystemFactory->clear();
}

void CoreLink::bodyTraceHide(std::string value) const {
	if (value=="all")
		currentSsystemFactory->hide(-1);
	else
		currentSsystemFactory->hide(Utility::strToInt(value));
}

void CoreLink::bodyTraceBodyChange(std::string bodyName) const {
	if (bodyName=="selected")
		currentSsystemFactory->bodyTraceBodyChange(core->selected_object.getEnglishName());
	else
		currentSsystemFactory->bodyTraceBodyChange(bodyName);
}

void CoreLink::cameraDisplayAnchor() {
	currentSsystemFactory->cameraDisplayAnchor();
}

bool CoreLink::cameraAddAnchor(stringHash_t& param) {
	return currentSsystemFactory->cameraAddAnchor(param);
}

bool CoreLink::cameraRemoveAnchor(const std::string &name) {
	return currentSsystemFactory->cameraRemoveAnchor(name);
}

bool CoreLink::cameraSwitchToAnchor(const std::string &name) {
	return currentSsystemFactory->cameraSwitchToAnchor(name);
}

bool CoreLink::cameraMoveToPoint(double x, double y, double z){
	return currentSsystemFactory->cameraMoveToPoint(x,y,z);
}

bool CoreLink::cameraMoveToPoint(double x, double y, double z, double time){
	return currentSsystemFactory->cameraMoveToPoint(x,y,z,time);
}

bool CoreLink::cameraMoveToBody(const std::string& bodyName, double time, double alt){

	if(bodyName == "selected"){
		return currentSsystemFactory->cameraMoveToBody(core->getSelectedPlanetEnglishName(), time, alt);
	}

	if(bodyName == "default"){
		return currentSsystemFactory->cameraMoveToBody(currentSsystemFactory->getEarth()->getEnglishName(), time, alt);
	}

	return currentSsystemFactory->cameraMoveToBody(bodyName,time, alt);
}

bool CoreLink::cameraMoveRelativeXYZ( double x, double y, double z) {
	return currentSsystemFactory->cameraMoveRelativeXYZ(x,y,z);
}

bool CoreLink::cameraTransitionToPoint(const std::string& name){
	return currentSsystemFactory->cameraTransitionToPoint(name);
}

bool CoreLink::cameraTransitionToBody(const std::string& name){

	if(name == "selected"){
		return currentSsystemFactory->cameraTransitionToBody(core->getSelectedPlanetEnglishName());
	}

	return currentSsystemFactory->cameraTransitionToBody(name);
}

bool CoreLink::cameraSetFollowRotation(const std::string& name, bool value){
	return currentSsystemFactory->cameraSetFollowRotation(value);
}

void CoreLink::cameraSetRotationMultiplierCondition(float v) {
	currentSsystemFactory->cameraSetRotationMultiplierCondition(v);
}

bool CoreLink::cameraAlignWithBody(const std::string& name, double duration){
	return currentSsystemFactory->cameraAlignWithBody(name,duration);
}

void CoreLink::setFlagLightTravelTime(bool b) {
	currentSsystemFactory->setFlagLightTravelTime(b);
}

bool CoreLink::getFlagLightTravelTime() const {
	return currentSsystemFactory->getFlagLightTravelTime();
}

void CoreLink::startPlanetsTrails(bool b) {
	currentSsystemFactory->startTrails(b);
}

void CoreLink::setPlanetsSelected(const std::string& englishName) {
	currentSsystemFactory->setSelected(englishName);
}

void CoreLink::setFlagMoonScaled(bool b) {
	currentSsystemFactory->setFlagMoonScale(b);
}

bool CoreLink::getFlagMoonScaled() const {
	return currentSsystemFactory->getFlagMoonScale();
}

void CoreLink::setFlagSunScaled(bool b) {
	currentSsystemFactory->setFlagSunScale(b);
}

bool CoreLink::getFlagSunScaled() const {
	return currentSsystemFactory->getFlagSunScale();
}

void CoreLink::setMoonScale(float f, bool resident) {
	if (f<0) currentSsystemFactory->setMoonScale(1., false);
	else currentSsystemFactory->setMoonScale(f, resident);
}

float CoreLink::getMoonScale() const {
	return currentSsystemFactory->getMoonScale();
}

void CoreLink::setSunScale(float f, bool resident) {
	if (f<0) currentSsystemFactory->setSunScale(1., false);
	else currentSsystemFactory->setSunScale(f, resident);
}

void CoreLink::setFlagClouds(bool b) {
	currentSsystemFactory->setFlagClouds(b);
}

bool CoreLink::getFlagClouds() const {
	return currentSsystemFactory->getFlag(BODY_FLAG::F_CLOUDS);
}

float CoreLink::getSunScale() const {
	return currentSsystemFactory->getSunScale();
}

void CoreLink::initialSolarSystemBodies() {
	return currentSsystemFactory->initialSolarSystemBodies();
}

void CoreLink::setPlanetHidden(std::string name, bool planethidden) {
	currentSsystemFactory->setPlanetHidden(name, planethidden);
}

bool CoreLink::getPlanetHidden(std::string name) {
	return currentSsystemFactory->getPlanetHidden(name);
}

void CoreLink::planetsSetFlag(bool b) {
	currentSsystemFactory->setFlagPlanets(b);
}

bool CoreLink::planetsGetFlag() const {
	return currentSsystemFactory->getFlagShow();
}

void CoreLink::planetsSetFlagTrails(bool b) {
	currentSsystemFactory->setFlagTrails(b);
}

bool CoreLink::planetsGetFlagTrails() const {
	return currentSsystemFactory->getFlag(BODY_FLAG::F_TRAIL);
}

void CoreLink::planetsSetFlagAxis(bool b) {
	currentSsystemFactory->setFlagAxis(b);
}

bool CoreLink::planetsGetFlagAxis() const {
	return currentSsystemFactory->getFlag(BODY_FLAG::F_AXIS);
}

void CoreLink::planetsSetFlagHints(bool b) {
	currentSsystemFactory->setFlagHints(b);
}

bool CoreLink::planetsGetFlagHints() const {
	return currentSsystemFactory->getFlag(BODY_FLAG::F_HINTS);
}

void CoreLink::planetsSetFlagOrbits(bool b) {
	currentSsystemFactory->setFlagPlanetsOrbits(b);
}

void CoreLink::planetsSetFlagOrbits(const std::string &_name, bool b) {
	currentSsystemFactory->setFlagPlanetsOrbits(_name, b);
}

void CoreLink::planetSwitchTexMap(const std::string &_name, bool b) {
	if (_name=="selected") currentSsystemFactory->switchPlanetTexMap(core->selected_object.getEnglishName(), b);
	else currentSsystemFactory->switchPlanetTexMap(_name, b);
}

bool CoreLink::planetGetSwitchTexMap(const std::string &_name) {
	if (_name=="selected") return currentSsystemFactory->getSwitchPlanetTexMap(core->selected_object.getEnglishName());
	else return currentSsystemFactory->getSwitchPlanetTexMap(_name);
}

void CoreLink::planetCreateTexSkin(const std::string &name, const std::string &texName){
	currentSsystemFactory->createTexSkin(name, texName);
}

bool CoreLink::planetsGetFlagOrbits() const {
	return currentSsystemFactory->getFlagPlanetsOrbits();
}

void CoreLink::satellitesSetFlagOrbits(bool b) {
	currentSsystemFactory->setFlagSatellitesOrbits(b);
}

bool CoreLink::satellitesGetFlagOrbits() const {
	return currentSsystemFactory->getFlagSatellitesOrbits();
}

void CoreLink::planetSetFlagOrbits(bool b) {
	currentSsystemFactory->setFlagSatellitesOrbits(b);
	currentSsystemFactory->setFlagPlanetsOrbits(b);
	//ssystem->setFlagOrbits(b);
}

void CoreLink::planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const {
	currentSsystemFactory->setBodyColor(englishName, color, c);
}

Vec3f CoreLink::planetGetColor(const std::string& englishName, const std::string& color) const {
	return currentSsystemFactory->getBodyColor(englishName, color);
}

void CoreLink::planetSetDefaultColor(const std::string& color, Vec3f c) const {
	currentSsystemFactory->setDefaultBodyColor(color, c);
}

Vec3f CoreLink::planetGetDefaultColor(const std::string& colorName) const {
	return currentSsystemFactory->getDefaultBodyColor(colorName);
}

bool CoreLink::hideSatellitesFlag(){
	return currentSsystemFactory->getHideSatellitesFlag();
}

void CoreLink::setHideSatellites(bool val){
	currentSsystemFactory->toggleHideSatellites(val);
}

void CoreLink::planetsSetScale(float f) {
	currentSsystemFactory->setScale(f);
}

double CoreLink::getSunAltitude() const {
	return currentSsystemFactory->getSunAltitude(core->navigation);
}

double CoreLink::getSunAzimuth() const {
	return currentSsystemFactory->getSunAzimuth(core->navigation);
}

double CoreLink::getSelectedAZ() const {
	return currentSsystemFactory->getSelectedAZ(core->navigation);
}

double CoreLink::getSelectedALT() const {
	return currentSsystemFactory->getSelectedALT(core->navigation);
}

double CoreLink::getSelectedRA() const {
	return currentSsystemFactory->getSelectedRA(core->navigation);
}

double CoreLink::getSelectedDE() const {
	return currentSsystemFactory->getSelectedDE(core->navigation);
}

double CoreLink::getSelectedStarRA() const {
	return currentSsystemFactory->getSelectedStarRA(core->navigation);
}

double CoreLink::getSelectedStarDE() const {
	return currentSsystemFactory->getSelectedStarDE(core->navigation);
}

int CoreLink::getLanguage() const {
	return double(core->getLanguage());
}

double CoreLink::getBodySelected() const {
	return double(core->getSelectedBodyName());
}

void CoreLink::bodySetFlagIsolateSelected(bool b) {
	currentSsystemFactory->setFlagIsolateSelected(b);
}

bool CoreLink::bodyGetFlagIsolateSelected() {
	return currentSsystemFactory->getFlagIsolateSelected();
}

void CoreLink::planetSetSizeScale(std::string name, float f) {
	currentSsystemFactory->setPlanetSizeScale(name, f);
}

void CoreLink::planetTesselation(std::string name, int value) {
	currentSsystemFactory->planetTesselation(name,value);
}

////////////////////////////////////////////////////////////////////////////////
// Fog---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set flag for displaying Fog
void CoreLink::fogSetFlag(bool b) {
	core->landscape->fogSetFlagShow(b);
}
//! Get flag for displaying Fog
bool CoreLink::fogGetFlag() const {
	return core->landscape->fogGetFlagShow();
}

////////////////////////////////////////////////////////////////////////////////
// Landscape---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Get flag for displaying Landscape
void CoreLink::landscapeSetFlag(bool b) {
	core->landscape->setFlagShow(b);
}
//! Get flag for displaying Landscape
bool CoreLink::landscapeGetFlag() const {
	return core->landscape->getFlagShow();
}

void CoreLink::rotateLandscape(double rotation) {
	core->landscape->setRotation(rotation);
}

std::string CoreLink::landscapeGetName() {
 	return core->landscape->getName();
}
