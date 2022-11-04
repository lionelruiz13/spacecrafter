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
	return core->ssystemFactory->cameraSave(AppSettings::Instance()->getUserDir() + "anchors/" + name);
}

bool CoreLink::loadCameraPosition(const std::string& filename)
{
	return core->ssystemFactory->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
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

////////////////////////////////////////////////////////////////////////////////
// Skyline et Skygrid---------------------------
////////////////////////////////////////////////////////////////////////////////

void CoreLink::skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a) {
    core->skyLineMgr->setColor(name, a);
};

void CoreLink::skyGridMgrSetColor(SKYGRID_TYPE name, Vec3f a) {
	core->skyGridMgr->setColor(name, a);
}

const Vec3f& CoreLink::skyLineMgrGetColor(SKYLINE_TYPE name) {
	return core->skyLineMgr->getColor(name);
}

const Vec3f& CoreLink::skyGridMgrGetColor(SKYGRID_TYPE name) {
	return core->skyGridMgr->getColor(name);
}

void CoreLink::skyLineMgrFlipFlagShow(SKYLINE_TYPE name) {
	core->skyLineMgr->flipFlagShow(name);
}

void CoreLink::skyGridMgrFlipFlagShow(SKYGRID_TYPE name) {
	core->skyGridMgr->flipFlagShow(name);
}

void CoreLink::skyLineMgrSetFlagShow(SKYLINE_TYPE name, bool value) {
	core->skyLineMgr->setFlagShow(name, value);
}

void CoreLink::skyGridMgrSetFlagShow(SKYGRID_TYPE name, bool value) {
	core->skyGridMgr->setFlagShow(name, value);
}

bool CoreLink::skyLineMgrGetFlagShow(SKYLINE_TYPE name) {
	return core->skyLineMgr->getFlagShow(name);
}

bool CoreLink::skyGridMgrGetFlagShow(SKYGRID_TYPE name) {
	return core->skyGridMgr->getFlagShow(name);
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
	core->meteors->setZHR(f);
}

//! Get Meteor Rate in number per hour
int CoreLink::getMeteorsRate() const {
	return core->meteors->getZHR();
}

void CoreLink::createRadiant(int day, const Vec3f newRadiant) {
	core->meteors->createRadiant(day, newRadiant);
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
	return core->skyDisplayMgr->getFlagShow(nameObj);
}

void CoreLink::skyDisplayMgrSetFlag(SKYDISPLAY_NAME nameObj, bool v) {
	core->skyDisplayMgr->setFlagShow(nameObj,v);
}

void CoreLink::skyDisplayMgrFlipFlag(SKYDISPLAY_NAME nameObj) {
	core->skyDisplayMgr->flipFlagShow(nameObj);
}

void CoreLink::skyDisplayMgrSetColor(SKYDISPLAY_NAME nameObj, const Vec3f& v) {
	core->skyDisplayMgr->setColor(nameObj,v);
}

void CoreLink::skyDisplayMgrClear(SKYDISPLAY_NAME nameObj) {
	core->skyDisplayMgr->clear(nameObj);
}

void CoreLink::skyDisplayMgrLoadData(SKYDISPLAY_NAME nameObj, const std::string& fileName) {
	core->skyDisplayMgr->loadData(nameObj,fileName);
}

void CoreLink::skyDisplayMgrLoadString(SKYDISPLAY_NAME nameObj, const std::string& dataStr) {
	core->skyDisplayMgr->loadString(nameObj,dataStr);
}

////////////////////////////////////////////////////////////////////////////////
// StarLines---------------------------
////////////////////////////////////////////////////////////////////////////////

//! Set flag for displaying
void CoreLink::starLinesSetFlag(bool b) {
	core->starLines->setFlagShow(b);
}

void CoreLink::starLinesSelectedSetFlag(bool b) {
	core->starLines->setFlagSelected(b);
}

bool CoreLink::starLinesSelectedGetFlag() const {
	return core->starLines->getFlagSelected();
}

//! Get flag for displaying
bool CoreLink::starLinesGetFlag() const {
	return core->starLines->getFlagShow();
}

//! Empty all plot buffers
void CoreLink::starLinesDrop() const {
	core->starLines->drop();
}

//! Loads a set of asterisms from a file
void CoreLink::starLinesLoadData(const std::string &fileName) {
	core->starLines->loadData(fileName);
}

//! Loads an asterism from a line
void CoreLink::starLinesLoadAsterism(std::string record) const {
	core->starLines->loadStringData(record);
}

//! deletes the complete catalog of asterisms
void CoreLink::starLinesClear() {
	core->starLines->clear();
}

void CoreLink::starLinesSaveCat(const std::string &fileName, bool binaryMode){
	core->starLines->saveCat(fileName, binaryMode);
}

void CoreLink::starLinesLoadCat(const std::string &fileName, bool binaryMode){
	core->starLines->loadCat(fileName, binaryMode);
}

void CoreLink::starLinesLoadHipStar(int name, Vec3f position) {
	core->starLines->loadHipStar(name, position);
}

////////////////////////////////////////////////////////////////////////////////
// Illuminate---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::illuminateSetSize (double value) {
	core->illuminates->setDefaultSize(value);
}

void CoreLink::illuminateLoadConstellation(const std::string& abbreviation, double size, double rotation) {
	core->illuminates->loadConstellation(abbreviation, size, rotation);
}
void CoreLink::illuminateLoadConstellation(const std::string& abbreviation,const Vec3f& color, double size, double rotation) {
	core->illuminates->loadConstellation(abbreviation, color, size, rotation);
}
void CoreLink::illuminateLoadAllConstellation(double size, double rotation) {
	core->illuminates->loadAllConstellation(size, rotation);
}

void CoreLink::illuminateLoad(int number, double size, double rotation) {
	core->illuminates->load(number, size, rotation);
}

void CoreLink::illuminateLoad(int number, const Vec3f& _color, double size, double rotation) {
	core->illuminates->load(number, _color, size, rotation);
}

void CoreLink::illuminateRemove(unsigned int name) 	{
	core->illuminates->remove(name);
}

void CoreLink::illuminateRemoveConstellation(const std::string abbreviation) 	{
	core->illuminates->removeConstellation(abbreviation);
}

void CoreLink::illuminateRemoveAllConstellation() 	{
	core->illuminates->removeAllConstellation();
}

void CoreLink::illuminateRemoveAll()
{
	core->illuminates->removeAll();
}

void CoreLink::illuminateChangeTex(const std::string& _fileName)	{
	core->illuminates->changeTex(_fileName);
}

void CoreLink::illuminateRemoveTex()	{
	core->illuminates->removeTex();
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
	return core->ssystemFactory->getPlanetsPosition();
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
	core->starNav->setFlagStars(b);
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
	return core->starNav->setFaderDuration(f);
}

void CoreLink::starSetFlagName(bool b) {
	core->hip_stars->setFlagNames(b);
}
bool CoreLink::starGetFlagName() const {
	return core->hip_stars->getFlagNames();
}

void CoreLink::starNavSetFlagName(bool b) {
	core->starNav->setFlagNames(b);
}

bool CoreLink::starNavGetFlagName() const {
	return core->starNav->getFlagNames();
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
	core->starNav->setMaxMagName(f);
}
float CoreLink::starNavGetMaxMagName() const {
	return core->starNav->getMaxMagName();
}

void CoreLink::starSetSizeLimit(float f) {
	core->starNav->setStarSizeLimit(f);
	core->setStarSizeLimit(f);
}

void CoreLink::starSetScale(float f) {
	core->starNav->setScale(f);
	core->hip_stars->setScale(f);
}

float CoreLink::starGetScale() const {
	return core->hip_stars->getScale();
}

void CoreLink::starSetMagScale(float f) {
	core->starNav->setMagScale(f);
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

////////////////////////////////////////////////////////////////////////////////
// StarNavigator---------------------------
////////////////////////////////////////////////////////////////////////////////
void CoreLink::starNavigatorClear(){
	core->starNav->clear();
}

void CoreLink::starNavigatorLoad(const std::string &fileName, bool binaryMode){
	core->starNav->loadData(fileName, binaryMode);
}

void CoreLink::starNavigatorLoadRaw(const std::string &fileName){
	core->starNav->loadRawData(fileName);
}

void CoreLink::starNavigatorLoadOther(const std::string &fileName){
	core->starNav->loadOtherData(fileName);
}

void CoreLink::starNavigatorSave(const std::string &fileName, bool binaryMode){
	core->starNav->saveData(fileName, binaryMode);
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
	core->nebulas->setFlagShow(b);
	core->dso3d->setFlagShow(b);
}

void CoreLink::dso3dSetDuration(float f) {
	return core->dso3d->setFaderDuration(f);
}

void CoreLink::dso3dSetFlagName(bool b) {
	core->dso3d->setFlagNames(b);
}

bool CoreLink::dso3dGetFlagName() const {
	return core->dso3d->getFlagNames();
}

//! Get flag for displaying Nebulae
bool CoreLink::nebulaGetFlag() const {
	return core->nebulas->getFlagShow();
}

//! Set flag for displaying Nebulae Hints
void CoreLink::nebulaSetFlagHints(bool b) {
	core->nebulas->setFlagHints(b);
}
//! Get flag for displaying Nebulae Hints
bool CoreLink::nebulaGetFlagHints() const {
	return core->nebulas->getFlagHints();
}

//! Set flag for displaying Nebulae as bright
void CoreLink::nebulaSetFlagBright(bool b) {
	core->nebulas->setFlagBright(b);
}
//! Get flag for displaying Nebulae as brigth
bool CoreLink::nebulaGetFlagBright() const {
	return core->nebulas->getFlagBright();
}

//! Set maximum magnitude at which nebulae hints are displayed
void CoreLink::nebulaSetMaxMagHints(float f) {
	core->nebulas->setMaxMagHints(f);
}
//! Get maximum magnitude at which nebulae hints are displayed
float CoreLink::nebulaGetMaxMagHints() const {
	return core->nebulas->getMaxMagHints();
}

//! return the color for the DSO object
Vec3f CoreLink::nebulaGetColorLabels() const {
	return core->nebulas->getLabelColor();
}

//! return the color of the DSO circle
Vec3f CoreLink::nebulaGetColorCircle() const {
	return core->nebulas->getCircleColor();
}

//!set Flag DSO Name who display DSO name
void CoreLink::nebulaSetFlagNames (bool value) {
	core->nebulas->setNebulaNames(value);
}

//!get flag DSO Name who display DSO name
bool CoreLink::nebulaGetFlagNames () {
	return core->nebulas->getNebulaNames();
}

void CoreLink::nebulaSetColorLabels(const Vec3f& v) {
	core->nebulas->setLabelColor(v);
}
void CoreLink::nebulaSetColorCircle(const Vec3f& v) {
	core->nebulas->setCircleColor(v);
}

void CoreLink::nebulaSetFlagIsolateSelected(bool b) {
	return core->nebulas->setFlagIsolateSelected(b);
}

bool CoreLink::nebulaGetFlagIsolateSelected() {
	return core->nebulas->getFlagIsolateSelected();
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
	core->ssystemFactory->bodyTraceSetFlag(b);
}

bool CoreLink::bodyTraceGetFlag() const {
	return core->ssystemFactory->bodyTraceGetFlag();
}

void CoreLink::bodyPenUp() const {
	core->ssystemFactory->upPen();
}

void CoreLink::bodyPenDown() const {
	core->ssystemFactory->downPen();
}

void CoreLink::bodyPenToggle() const {
	core->ssystemFactory->togglePen();
}

void CoreLink::bodyTraceClear () const {
	core->ssystemFactory->clear();
}

void CoreLink::bodyTraceHide(std::string value) const {
	if (value=="all")
		core->ssystemFactory->hide(-1);
	else
		core->ssystemFactory->hide(Utility::strToInt(value));
}

void CoreLink::bodyTraceBodyChange(std::string bodyName) const {
	if (bodyName=="selected")
		core->ssystemFactory->bodyTraceBodyChange(core->selected_object.getEnglishName());
	else
		core->ssystemFactory->bodyTraceBodyChange(bodyName);
}

void CoreLink::cameraDisplayAnchor() {
	core->ssystemFactory->cameraDisplayAnchor();
}

bool CoreLink::cameraAddAnchor(stringHash_t& param) {
	return core->ssystemFactory->cameraAddAnchor(param);
}

bool CoreLink::cameraRemoveAnchor(const std::string &name) {
	return core->ssystemFactory->cameraRemoveAnchor(name);
}

bool CoreLink::cameraSwitchToAnchor(const std::string &name) {
	return core->ssystemFactory->cameraSwitchToAnchor(name);
}

bool CoreLink::cameraMoveToPoint(double x, double y, double z){
	return core->ssystemFactory->cameraMoveToPoint(x,y,z);
}

bool CoreLink::cameraMoveToPoint(double x, double y, double z, double time){
	return core->ssystemFactory->cameraMoveToPoint(x,y,z,time);
}

bool CoreLink::cameraMoveToBody(const std::string& bodyName, double time, double alt){

	if(bodyName == "selected"){
		return core->ssystemFactory->cameraMoveToBody(core->getSelectedPlanetEnglishName(), time, alt);
	}

	if(bodyName == "default"){
		return core->ssystemFactory->cameraMoveToBody(core->ssystemFactory->getEarth()->getEnglishName(), time, alt);
	}

	return core->ssystemFactory->cameraMoveToBody(bodyName,time, alt);
}

bool CoreLink::cameraMoveRelativeXYZ( double x, double y, double z) {
	return core->ssystemFactory->cameraMoveRelativeXYZ(x,y,z);
}

bool CoreLink::cameraTransitionToPoint(const std::string& name){
	return core->ssystemFactory->cameraTransitionToPoint(name);
}

bool CoreLink::cameraTransitionToBody(const std::string& name){

	if(name == "selected"){
		return core->ssystemFactory->cameraTransitionToBody(core->getSelectedPlanetEnglishName());
	}

	return core->ssystemFactory->cameraTransitionToBody(name);
}

bool CoreLink::cameraSetFollowRotation(const std::string& name, bool value){
	return core->ssystemFactory->cameraSetFollowRotation(value);
}

void CoreLink::cameraSetRotationMultiplierCondition(float v) {
	core->ssystemFactory->cameraSetRotationMultiplierCondition(v);
}

bool CoreLink::cameraAlignWithBody(const std::string& name, double duration){
	return core->ssystemFactory->cameraAlignWithBody(name,duration);
}

void CoreLink::setFlagLightTravelTime(bool b) {
	core->ssystemFactory->setFlagLightTravelTime(b);
}

bool CoreLink::getFlagLightTravelTime() const {
	return core->ssystemFactory->getFlagLightTravelTime();
}

void CoreLink::startPlanetsTrails(bool b) {
	core->ssystemFactory->startTrails(b);
}

void CoreLink::setPlanetsSelected(const std::string& englishName) {
	core->ssystemFactory->setSelected(englishName);
}

void CoreLink::setFlagMoonScaled(bool b) {
	core->ssystemFactory->setFlagMoonScale(b);
}

bool CoreLink::getFlagMoonScaled() const {
	return core->ssystemFactory->getFlagMoonScale();
}

void CoreLink::setFlagSunScaled(bool b) {
	core->ssystemFactory->setFlagSunScale(b);
}

bool CoreLink::getFlagSunScaled() const {
	return core->ssystemFactory->getFlagSunScale();
}

void CoreLink::setMoonScale(float f, bool resident) {
	if (f<0) core->ssystemFactory->setMoonScale(1., false);
	else core->ssystemFactory->setMoonScale(f, resident);
}

float CoreLink::getMoonScale() const {
	return core->ssystemFactory->getMoonScale();
}

void CoreLink::setSunScale(float f, bool resident) {
	if (f<0) core->ssystemFactory->setSunScale(1., false);
	else core->ssystemFactory->setSunScale(f, resident);
}

void CoreLink::setFlagClouds(bool b) {
	core->ssystemFactory->setFlagClouds(b);
}

bool CoreLink::getFlagClouds() const {
	return core->ssystemFactory->getFlag(BODY_FLAG::F_CLOUDS);
}

float CoreLink::getSunScale() const {
	return core->ssystemFactory->getSunScale();
}

void CoreLink::initialSolarSystemBodies() {
	return core->ssystemFactory->initialSolarSystemBodies();
}

void CoreLink::setPlanetHidden(std::string name, bool planethidden) {
	core->ssystemFactory->setPlanetHidden(name, planethidden);
}

bool CoreLink::getPlanetHidden(std::string name) {
	return core->ssystemFactory->getPlanetHidden(name);
}

void CoreLink::planetsSetFlag(bool b) {
	core->ssystemFactory->setFlagPlanets(b);
}

bool CoreLink::planetsGetFlag() const {
	return core->ssystemFactory->getFlagShow();
}

void CoreLink::planetsSetFlagTrails(bool b) {
	core->ssystemFactory->setFlagTrails(b);
}

bool CoreLink::planetsGetFlagTrails() const {
	return core->ssystemFactory->getFlag(BODY_FLAG::F_TRAIL);
}

void CoreLink::planetsSetFlagAxis(bool b) {
	core->ssystemFactory->setFlagAxis(b);
}

bool CoreLink::planetsGetFlagAxis() const {
	return core->ssystemFactory->getFlag(BODY_FLAG::F_AXIS);
}

void CoreLink::planetsSetFlagHints(bool b) {
	core->ssystemFactory->setFlagHints(b);
}

bool CoreLink::planetsGetFlagHints() const {
	return core->ssystemFactory->getFlag(BODY_FLAG::F_HINTS);
}

void CoreLink::planetsSetFlagOrbits(bool b) {
	core->ssystemFactory->setFlagPlanetsOrbits(b);
}

void CoreLink::planetsSetFlagOrbits(const std::string &_name, bool b) {
	core->ssystemFactory->setFlagPlanetsOrbits(_name, b);
}

void CoreLink::planetSwitchTexMap(const std::string &_name, bool b) {
	if (_name=="selected") core->ssystemFactory->switchPlanetTexMap(core->selected_object.getEnglishName(), b);
	else core->ssystemFactory->switchPlanetTexMap(_name, b);
}

bool CoreLink::planetGetSwitchTexMap(const std::string &_name) {
	if (_name=="selected") return core->ssystemFactory->getSwitchPlanetTexMap(core->selected_object.getEnglishName());
	else return core->ssystemFactory->getSwitchPlanetTexMap(_name);
}

void CoreLink::planetCreateTexSkin(const std::string &name, const std::string &texName){
	core->ssystemFactory->createTexSkin(name, texName);
}

bool CoreLink::planetsGetFlagOrbits() const {
	return core->ssystemFactory->getFlagPlanetsOrbits();
}

void CoreLink::satellitesSetFlagOrbits(bool b) {
	core->ssystemFactory->setFlagSatellitesOrbits(b);
}

bool CoreLink::satellitesGetFlagOrbits() const {
	return core->ssystemFactory->getFlagSatellitesOrbits();
}

void CoreLink::planetSetFlagOrbits(bool b) {
	core->ssystemFactory->setFlagSatellitesOrbits(b);
	core->ssystemFactory->setFlagPlanetsOrbits(b);
	//ssystem->setFlagOrbits(b);
}

void CoreLink::planetSetColor(const std::string& englishName, const std::string& color, Vec3f c) const {
	core->ssystemFactory->setBodyColor(englishName, color, c);
}

Vec3f CoreLink::planetGetColor(const std::string& englishName, const std::string& color) const {
	return core->ssystemFactory->getBodyColor(englishName, color);
}

void CoreLink::planetSetDefaultColor(const std::string& color, Vec3f c) const {
	core->ssystemFactory->setDefaultBodyColor(color, c);
}

Vec3f CoreLink::planetGetDefaultColor(const std::string& colorName) const {
	return core->ssystemFactory->getDefaultBodyColor(colorName);
}

bool CoreLink::hideSatellitesFlag(){
	return core->ssystemFactory->getHideSatellitesFlag();
}

void CoreLink::setHideSatellites(bool val){
	core->ssystemFactory->toggleHideSatellites(val);
}

void CoreLink::planetsSetScale(float f) {
	core->ssystemFactory->setScale(f);
}

double CoreLink::getSunAltitude() const {
	return core->ssystemFactory->getSunAltitude(core->navigation);
}

double CoreLink::getSunAzimuth() const {
	return core->ssystemFactory->getSunAzimuth(core->navigation);
}

double CoreLink::getSelectedAZ() const {
	return core->ssystemFactory->getSelectedAZ(core->navigation);
}

double CoreLink::getSelectedALT() const {
	return core->ssystemFactory->getSelectedALT(core->navigation);
}

double CoreLink::getSelectedRA() const {
	return core->ssystemFactory->getSelectedRA(core->navigation);
}

double CoreLink::getSelectedDE() const {
	return core->ssystemFactory->getSelectedDE(core->navigation);
}

double CoreLink::getBodySelected() const {
	return double(core->getSelectedBodyName());
}

void CoreLink::planetSetSizeScale(std::string name, float f) {
	core->ssystemFactory->setPlanetSizeScale(name, f);
}

void CoreLink::planetTesselation(std::string name, int value) {
	core->ssystemFactory->planetTesselation(name,value);
}
