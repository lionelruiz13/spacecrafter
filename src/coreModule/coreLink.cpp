# include "coreModule/coreLink.hpp"
#include "tools/app_settings.hpp"
//#include "coreModule/coreFont.hpp"
#include "appModule/fontFactory.hpp"
#include "appModule/space_date.hpp"

#include "coreModule/skyline_mgr.hpp"
#include "coreModule/skygrid_mgr.hpp"

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