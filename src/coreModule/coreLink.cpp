# include "coreModule/coreLink.hpp"
#include "tools/app_settings.hpp"
//#include "coreModule/coreFont.hpp"
#include "appModule/fontFactory.hpp"
#include "appModule/space_date.hpp"
#include "coreModule/oort.hpp"
#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skygrid_mgr.hpp"
#include "coreModule/milkyway.hpp"

#include "coreModule/cardinals.hpp"

#include "coreModule/meteor_mgr.hpp"

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
	core->milky_way->setFaderDuration(f*1000);
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