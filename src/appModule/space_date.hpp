
/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/
//! \file space_date.hpp
//! \brief a class for diverse date conversions
//! \author Julien LAFILLE
//! \date april 2018

#ifndef SPACE_DATE_H
#define SPACE_DATE_H

#include <string>
#include "tools/vecmath.hpp"

typedef struct {
	int years; 		/*!< Years. All values are valid */
	int months;		/*!< Months. Valid values : 1 (January) - 12 (December) */
	int days; 		/*!< Days. Valid values 1 - 28,29,30,31 Depends on month.*/
	int hours; 		/*!< Hours. Valid values 0 - 23. */
	int minutes; 	/*!< Minutes. Valid values 0 - 59. */
	double seconds;	/*!< Seconds. Valid values 0 - 59.99999.... */
} ln_date;

class SpaceDate {
public:

	enum S_TIME_FORMAT {S_TIME_24H,	S_TIME_12H,	S_TIME_SYSTEM_DEFAULT};
	
	enum S_DATE_FORMAT {S_DATE_MMDDYYYY, S_DATE_DDMMYYYY, S_DATE_SYSTEM_DEFAULT, S_DATE_YYYYMMDD};
	
	enum S_TZ_FORMAT {S_TZ_CUSTOM, S_TZ_GMT_SHIFT, S_TZ_SYSTEM_DEFAULT};
	
	
	SpaceDate() : 
		timeFormat(S_TIME_SYSTEM_DEFAULT),
		dateFormat(S_DATE_SYSTEM_DEFAULT), 
		timeZoneMode(S_TZ_SYSTEM_DEFAULT),
		GMTShift (0) { }
		
	~SpaceDate(){ }
	
	std::string getPrintableTimeNav(double JDay, double Longitude, double latitude) const;
	
	static size_t myStrftime(char *s, size_t max, const char *fmt, const struct tm *tm);

	static std::string ISO8601TimeUTC(double jd, bool dateOnly = false);
	
	static void DateTimeFromJulianDay(double jd, int *year, int *month, int *day, int *hour, int *minute, double *second);
	
	static void JulianToDate(double jd, ln_date *date);
	
	static void TimeTmFromJulian(double JD, struct tm * tm_time);
	
	static int StringToJday(const std::string& date, double &rjd);
	
	static double JulianDayFromDateTime(const int, const int, const int, const int, const int, const double);
	
	static double JulianDay (const ln_date * date);
	
	static time_t TimeTFromJulian(double JD);
	
	static void LnDateFromSys(ln_date * date);
	
	static double JulianFromSys(void);
	
	static unsigned int DayOfWeek (const ln_date *date);
	
	static float GMTShiftFromSystem(double JD, bool _local=0);
	
	static std::string TimeZoneNameFromSystem(double JD);

	static void getYearMonthDaybyJD(double JD, int &Y, int &M, int &D);

	// These should probably move elsewhere...
	static double getMaxSimulationJD (void) {
		static double maxJD = JulianDayFromDateTime(1000000, 1, 1, 1, 1, 1);
		return maxJD;
	}

	static double getMinSimulationJD (void) {
		static double minJD = JulianDayFromDateTime(-1000000, 1, 1, 1, 1, 1);
		return minJD;
	}
	
	//! Return the current time shift at observer time zone with respect to GMT time
	void setGMTShift(int t) {
		GMTShift=t;
	}
	
	std::string getPrintableDateUTC(double JD) const;
		
	std::string getPrintableDateLocal(double JD) const;
		
	std::string getPrintableTimeUTC(double JD) const;
	
	std::string getPrintableTimeLocal(double JD) const;
		
	float getGMTShift(double JD = 0, bool _local=0) const;
	
	void setCustomTzName(const std::string& tzname);
	
	void setCustomTimezone(const std::string& _time_zone) {
		setCustomTzName(_time_zone);
	}
	
	std::string getCustomTzName(void) const {
		return custom_tz_name;
	}
		
	S_TZ_FORMAT getTzFormat(void) const {
		return timeZoneMode;
	}
	
	//! Return the time in ISO 8601 format that is : %Y-%m-%d %H:%M:%S
	std::string getISO8601TimeLocal(double JD) const;

	std::string getTimeFormatStr(void) const {
		return sTimeFormatToString(timeFormat);
	}
	
	void setTimeFormatStr(const std::string& tf) {
		timeFormat=stringToSTimeFormat(tf);
	}
	
	std::string getDateFormatStr(void) const {
		return sDateFormatToString(dateFormat);
	}
	
	void setDateFormatStr(const std::string& df) {
		dateFormat=stringToSDateFormat(df);
	}
	
	//! Convert string to its associated time format enum
	S_TIME_FORMAT stringToSTimeFormat(const std::string&) const;
	//! Convert string to its associated date format enum
	S_DATE_FORMAT stringToSDateFormat(const std::string& df) const;
	
	void setTimeFormat(S_TIME_FORMAT format){
		timeFormat = format;
	}
	
	void setDateFormat(S_DATE_FORMAT format){
		dateFormat = format;
	}
	
	void setTimeZoneMode(S_TZ_FORMAT format){
		timeZoneMode = format;
	}
	
private:
	//! Convert the date format enum to its associated string
	std::string sDateFormatToString(S_DATE_FORMAT df) const;
	
	//! Convert the time format enum to its associated string
	std::string sTimeFormatToString(S_TIME_FORMAT) const;

	// Date and time variables
	S_TIME_FORMAT timeFormat;
	S_DATE_FORMAT dateFormat;
	S_TZ_FORMAT timeZoneMode;		//! Can be the system default or a user defined value
	std::string custom_tz_name;		//! Something like "Europe/Paris"
	float GMTShift;					//! Time shift between GMT time and local time in hour. (positive for Est of GMT)
};

#endif
