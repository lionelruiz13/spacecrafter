# include "coreModule/coreLink.hpp"
#include "tools/app_settings.hpp"
//#include "coreModule/coreFont.hpp"
#include "appModule/fontFactory.hpp"
#include "appModule/space_date.hpp"

CoreLink::CoreLink(Core * _core)
{
    core= _core;
}

CoreLink::~CoreLink()
{}

bool CoreLink::cameraSave(const std::string& name)
{
	return core->ssystemFactory->cameraSave(AppSettings::Instance()->getUserDir() + "anchors/" + name);
}

bool CoreLink::loadCameraPosition(const std::string& filename)
{
	return core->ssystemFactory->loadCameraPosition(AppSettings::Instance()->getUserDir() + "anchors/" + filename);
}

void CoreLink::fontUpdateFont(const std::string& _targetName, const std::string& _fontName, const std::string& _sizeValue)
{
	core->fontFactory->updateFont(_targetName, _fontName, _sizeValue);
}

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
