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

#include <sstream>
#include "appModule/space_date.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include "tools/utility.hpp"
#include "spacecrafter.hpp"



std::string SpaceDate::getPrintableTimeNav(double jd, double latitude, double longitude) const
{
	std::ostringstream os;
	double sidereal;
	double T;
	double LST;
	double c,r,l,lct,m;

	T = (jd - 2451545.0) / 36525.0;
	/* calc mean angle */
	sidereal = 280.46061837 + (360.98564736629 * (jd - 2451545.0)) + (0.000387933 * T * T) - (T * T * T / 38710000.0);
	while (sidereal>=360) sidereal-=360;
	while (sidereal<0)    sidereal+=360;
	LST=sidereal+longitude;
	while (LST>=360) 
		LST-=360;
	while (LST<0)
		LST+=360;
	m=357.5291+0.98560028*(jd-2451545);
	c=1.9148*sin(m*3.1415926/180)+0.02*sin(2*m*3.1415926/180)+0.0003*sin(3*m*3.1415926/180)/12;
	l=280.4665+0.98564736*(jd-2451545)+c;
	r=-2.468*sin(2*l*3.1415926/180)+0.053*sin(4*l*3.1415926/180)-0.0014*sin(6*l*3.1415926/180);
	lct=-longitude+c+r;
	while (lct>360) 
		lct-=360;
	while(lct<0) 
		lct+=360;

	os << getPrintableDateLocal(jd) << " " << getPrintableTimeLocal(jd) << "  UTC " << getPrintableTimeUTC(jd) << " GST " << Utility::printAngleHMS(sidereal*3.1415926/180) << " LST " << Utility::printAngleHMS(LST*3.1415926/180);
	os << " LCT " << getPrintableTimeUTC(jd+longitude/360);
	os << "@" << "Lat: " << Utility::printAngleDMS(latitude*3.1415926/180) << " Lon: " << Utility::printAngleDMS(longitude*3.1415926/180)<< " Eq. Time: ";
	if ((c+r)>=0) os << Utility::printAngleHMS((c+r)*3.1415926/180);
	if ((c+r)<0) os << "-" << Utility::printAngleHMS(-(c+r)*3.1415926/180);
	os << " TLocalPass: " << Utility::printAngleHMS(3.1415926+(c+r)*3.1415926/180-longitude*3.1415926/180) << " TGMPass: " << Utility::printAngleHMS(3.1415926+(c+r)*3.1415926/180);
	return os.str();
}


// Return the time zone name taken from system locale
std::string SpaceDate::TimeZoneNameFromSystem(double JD)
{
	// Windows will crash if date before 1970
	// And no changes on Linux before that year either
	// TODO: ALSO, on Win XP timezone never changes anyway???
	if (JD < 2440588 ) JD = 2440588;

	// The timezone name depends on the day because of the summer time
	time_t rawtime = TimeTFromJulian(JD);

	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);
	static char timez[255];
	timez[0] = 0;
	myStrftime(timez, 254, "%Z", timeinfo);
	return timez;
}


// Return the number of hours to add to gmt time to get the local time in day JD
// taking the parameters from system. This takes into account the daylight saving
// time if there is. (positive for Est of GMT)
// TODO : %z in strftime only works posix compliant systems (not Win32)
// Fixed 31-05-2004 Now use the extern variables set by tzset()
// Revised again on 10-14-2010. Extern variable was not working on MinGW. Now use
// Win32 API GetTimeZoneInformation.
float SpaceDate::GMTShiftFromSystem(double JD, bool _local)
{
	#ifdef LINUX
	// if( !AppSettings::Instance()->Windows() ) {
		struct tm * timeinfo;

		if (!_local) {
			// JD is UTC
			struct tm rawtime;
			TimeTmFromJulian(JD, &rawtime);
			time_t ltime = timegm(&rawtime);
			timeinfo = localtime(&ltime);
		} else {
			time_t rtime;
			rtime = TimeTFromJulian(JD);
			timeinfo = localtime(&rtime);
		}

		static char heure[20];
		heure[0] = '\0';

		myStrftime(heure, 19, "%z", timeinfo);

		heure[5] = '\0';
		float min = 1.f/60.f * atoi(&heure[3]);
		heure[3] = '\0';
		return min + atoi(heure);
	// } else {
	#else
		// Win32 specific. Stub function and structs exist for compilation on other platforms
		TIME_ZONE_INFORMATION info;
		GetTimeZoneInformation(&info);
		return -(info.Bias + info.DaylightBias) / 60;
	// }
	#endif
}

/* Calculate the day of the week.
 * Returns 0 = Sunday .. 6 = Saturday */
unsigned int SpaceDate::DayOfWeek (const ln_date *date)
{
	double JD;
	/* get julian day */
	JD = JulianDay(date) + 1.5;
	return (int)JD % 7;
}

/* Calculate julian day from system time. */
double SpaceDate::JulianFromSys(void)
{
	ln_date date;
	/* get sys date */
	LnDateFromSys(&date);
	return JulianDay(&date);
}


/* Calculate gmt date from system date.
 * param : date Pointer to store date. */
void SpaceDate::LnDateFromSys(ln_date * date)
{
	time_t rawtime;
	struct tm * ptm;

	/* get current time */
	time ( &rawtime );

	/* convert to gmt time representation */
	ptm = gmtime ( &rawtime );

	/* fill in date struct */
	date->seconds = ptm->tm_sec;
	date->minutes = ptm->tm_min;
	date->hours = ptm->tm_hour;
	date->days = ptm->tm_mday;
	date->months = ptm->tm_mon + 1;
	date->years = ptm->tm_year + 1900;
}


// Calculate time_t from julian day
time_t SpaceDate::TimeTFromJulian(double JD)
{
	struct tm loctime;
	ln_date date;

	JulianToDate(JD, &date);

	loctime.tm_sec = floor(date.seconds);
	loctime.tm_min = date.minutes;
	loctime.tm_hour = date.hours;
	loctime.tm_mday =date.days;
	loctime.tm_mon = date.months -1;
	loctime.tm_year = date.years - 1900;
	loctime.tm_isdst = -1;

	return mktime(&loctime);
}

double SpaceDate::JulianDay (const ln_date * date)
{
	if( !date )
		return 0;
	else
		return JulianDayFromDateTime(date->years, date->months, date->days, date->hours, date->minutes, date->seconds);
}

//-----------------------------------------------------------------------------
// convert calendar to Julian date		Year Zero and 4000-year rule added by SGS
// (Julian day number algorithm adapted from Press et al.)
// Adapted JS source by Steve Glennie-Smith to C++ by Trystan Larey-Williams
// This algorithm supports negative Julian dates
//-----------------------------------------------------------------------------
double SpaceDate::JulianDayFromDateTime(const int y, const int m, const int d, const int h, const int mn, const double s)
{
	double julianYear, julianMonth, centry;
	int correctedDay = d;

	// Gregorian calendar undefined for given date range. Rather than return error
	// just snap to valid date. Snap to the 'further' valid date to support scrolling
	// through dates linearly.
	if( y == 1582 && m == 10 ) {
		if (d > 4 && d < 8)
			correctedDay = 15;
		else if (d > 4 && d < 15)
			correctedDay = 4;
	}

	// The Julian calendar has 13 months
	if( m > 2 ) {
		julianYear = y;
		julianMonth = m + 1;
	} else {
		julianYear = y - 1;
		julianMonth = m + 13;
	}

	// Calculate days (integer) portion of Julian date
	double jdays = floor(365.25 * julianYear) + floor(30.6001 * julianMonth) + correctedDay + 1720995;

	// Correct for 'lost 10 days' in Julian -> Gregorian calendar switch
	if( correctedDay + 31 * (m + 12 * y) >= 588829 ) {
		centry = floor(0.01 * julianYear);
		jdays += 2 - centry + floor(0.25 * centry) - floor(0.025 * centry);
	}

	// Correct for half-day offset
	double dayfrac = h / 24.0 - 0.5;
	if (dayfrac < 0.0) {
		dayfrac += 1.0;
		--jdays;
	}

	// Calculate the time (decimal) portion of Julian date
	double jtime = dayfrac + (mn + s / 60.0) / (60.0 * 24.0);

	// Return composite Julian date
	return jdays + jtime;
}

// convert string int ISO 8601-like format [+/-]YYYY-MM-DDThh:mm:ss (no timzone offset)
// to julian day
int SpaceDate::StringToJday(const std::string& date, double &rjd)
{
	char tmp;
	int year, month, day, hour, minute;
	double second;
	year = month = day = hour = minute = second = 0;

	std::istringstream dstr( date );

	// TODO better error checking
	dstr >> year >> tmp >> month >> tmp >> day >> tmp >> hour >> tmp >> minute >> tmp >> second;

	if ( 	//year < -100000 || year > 100000 ||
	    month < 1 || month > 12 ||
	    day < 1 || day > 31 ||
	    hour < 0 || hour > 23 ||
	    minute < 0 || minute > 59 ||
	    second < 0 || second > 59) {
		return 0;
	}

	rjd = JulianDayFromDateTime( year, month, day, hour, minute, second );

	return 1;
}

void SpaceDate::TimeTmFromJulian(double JD, struct tm * tm_time)
{
	if( !tm_time )
		return;

	ln_date date;
	JulianToDate(JD, &date);
	tm_time->tm_sec = floor(date.seconds);
	tm_time->tm_min = date.minutes;
	tm_time->tm_hour = date.hours;
	tm_time->tm_mday = date.days;
	tm_time->tm_mon = date.months - 1;
	tm_time->tm_year = date.years - 1900;
	tm_time->tm_isdst = -1;
}

void SpaceDate::JulianToDate(double jd, ln_date *date)
{
	if( !date )
		return;

	DateTimeFromJulianDay( jd, &date->years, &date->months, &date->days, &date->hours, &date->minutes, &date->seconds);
}

//-----------------------------------------------------------------------------
// convert Julian date to calendar date
// (algorithm adapted from Hatcher, D.A., 1984, QJRAS 25, 53)
// (algorithm adapted from Press et al.)
// Adapted JS source by Steve Glennie-Smith to C++ by Trystan Larey-Williams
// This algorithm supports negative Julian dates
//-----------------------------------------------------------------------------
void SpaceDate::DateTimeFromJulianDay(double jd, int *year, int *month, int *day, int *hour, int *minute, double *second)
{
	double j1, j2, j3, j4, j5;

	// get the date from the Julian day number
	double jday = floor(jd);
	double jtime = jd - jday;

	// correction for half day offset
	// SGS: Bug - this was originally after Gregorian date check.  Add 5ms to correct rounding errors
	double dayfract = jtime + 0.500000058;
	if (dayfract >= 1.0) {
		dayfract -= 1.0;
		++jday;
	}

	// Gregorian calendar correction.  4000 year correction added by SGS
	if( jday >= 2299161 ) {
		// centry is the number of complete *Gregorian centuries* since 2nd March 0000
		double centry = floor( (jday - 1721119.25) / 36524.225 );
		j1 = jday - 2 + centry - floor(0.25*centry) + floor(0.025*centry);
	} else
		j1 = jday;

	// Calculate year, month, day
	j2 = j1 + 1524.0;
	j3 = floor(6680.0 + (j2 - 2439992.1) / 365.25);
	j4 = floor(j3 * 365.25);
	j5 = floor((j2 - j4) / 30.6001);
	*day = floor(j2 - j4 - floor (j5*30.6001));
	*month = floor(j5 - 1.0);
	*year = floor(j3 - 4715.0);

	// Back to 12 mo. from 13 mo calendar
	if( *month > 12 )
		*month -= 12;
	if( *month > 2 )
		--(*year);

	// Calculate Hours, minutes, seconds
	int ms  = floor(dayfract * 8640000);
	*second  = ms % 6000;
	ms = (ms - *second) / 6000;
	*second /= 100;
	*minute = ms % 60;
	*hour  = (ms - *minute) / 60;
}

// Return the time in ISO 8601 format that is : %Y-%m-%dT%H:%M:%S
std::string SpaceDate::ISO8601TimeUTC(double jd, bool dateOnly)
{
	int year, month, day, hour, minute;
	double second;
	DateTimeFromJulianDay(jd, &year, &month, &day, &hour, &minute, &second);

	char isotime[255];

	if( dateOnly )
		sprintf( isotime, "%d/%d/%d", year, month, day );
	else
		sprintf( isotime, "%d-%d-%dT%d:%d:%d", year, month, day, hour, minute, (int)floor(second) );

	return isotime;
}

// Wrapper around strftime to force ISO format on systems that don't support negative timestamps
// and/or years with more that four digits. Windows supports neither, and probably never will.
// Note that when compiling on MinGW for a Windows build, the MSCRT implementation of strftime is used.
size_t SpaceDate::myStrftime(char *s, size_t max, const char *fmt, const struct tm *tm)
{
	if( !tm || !fmt || !s )
		return 0;

	#ifdef WIN32
	// if( AppSettings::Instance()->Windows() ) {
		std::string sfmt(fmt);
		int size;
		size_t pos = sfmt.find("%Y");

		// Date only in current locale. Can only handle on Windows by returning ISO date.
		if( sfmt == "%x" ) {
			std::string iso = 
				ISO8601TimeUTC(JulianDayFromDateTime(tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec), true);
			size = std::min(max, iso.length());
			strncpy( s, iso.c_str(), size );
			s[size] = '\0';
		} else if( pos != std::string::npos ) {
			// If we're given an explicit format with a year specified, we can splice it out and place it back
			// in after the strftime call. Not looking for little 'y' (two digit year) specifier since it's
			// nonsensical in this application.
			sfmt = sfmt.replace(pos, 2, "YY");
			size = strftime(s, max, sfmt.c_str(), tm);
			std::string ret(s);
			pos = ret.find("YY");
			std::ostringstream ss;
			ss << tm->tm_year+1900;
			ret = ret.replace( pos, 2, ss.str() );  // ss ===> boost::lexical_cast<string>(tm->tm_year+1900) );
			size = std::min(max, ret.length());
			strncpy( s, ret.c_str(), size );
			s[size] = '\0';
		} else
			size = strftime(s, max, fmt, tm);

		return size;
	// } else // Non-windows OS
	#else
		return strftime(s, max, fmt, tm);
	#endif
}


void SpaceDate::getYearMonthDaybyJD(double JD, int &Y, int &M, int &D)
{
	double mJD= floor(JD)-fmod(floor(JD),10.0)+0.5;
	double mQ= mJD+0.5;
	double mZ= floor(mQ);
	double mW = floor((mZ - 1867216.25)/36524.25);
	double mX = floor(mW/4);
	double mA = mZ+1+mW-mX;
	double mB = mA+1524;
	double mC = floor((mB-122.1)/365.25);
	double mD = floor(365.25 * mC);
	double mE = floor((mB-mD)/30.6001);
	double mF = floor(30.6001 * mE);
	D= mB-mD-mF+floor(mQ-mZ);
	M= mE-1;
	if (M<0 || M>12) M= mE-13;
	if (M<3) Y = mC-4715;
	else Y=mC-4716;
}

//! Return a string with the UTC date formated according to the dateFormat variable
std::string SpaceDate::getPrintableDateUTC(double JD) const
{
	struct tm time_utc;
	SpaceDate::TimeTmFromJulian(JD, &time_utc);

	static char date[255];
	switch (dateFormat) {
		case S_DATE_SYSTEM_DEFAULT :
			SpaceDate::myStrftime(date, 254, "%x", &time_utc);
			break;
		case S_DATE_MMDDYYYY :
			SpaceDate::myStrftime(date, 254, "%m/%d/%Y", &time_utc);
			break;
		case S_DATE_DDMMYYYY :
			SpaceDate::myStrftime(date, 254, "%d/%m/%Y", &time_utc);
			break;
		case S_DATE_YYYYMMDD :
			SpaceDate::myStrftime(date, 254, "%Y-%m-%d", &time_utc);
			break;
	}
	return date;
}

//! Return a string with the local date formated according to the dateFormat variable
std::string SpaceDate::getPrintableDateLocal(double JD) const
{
	struct tm time_local;

	if (timeZoneMode == S_TZ_GMT_SHIFT)
		SpaceDate::TimeTmFromJulian(JD + GMTShift, &time_local);
	else
		SpaceDate::TimeTmFromJulian(JD + SpaceDate::GMTShiftFromSystem(JD)*0.041666666666, &time_local);

	static char date[255];
	switch (dateFormat) {
		case S_DATE_SYSTEM_DEFAULT :
			SpaceDate::myStrftime(date, 254, "%x", &time_local);
			break;
		case S_DATE_MMDDYYYY :
			SpaceDate::myStrftime(date, 254, "%m/%d/%Y", &time_local);
			break;
		case S_DATE_DDMMYYYY :
			SpaceDate::myStrftime(date, 254, "%d/%m/%Y", &time_local);
			break;
		case S_DATE_YYYYMMDD :
			SpaceDate::myStrftime(date, 254, "%Y-%m-%d", &time_local);
			break;
	}

	return date;
}

//! Return a string with the UTC time formated according to the timeFormat variable
//! @fixme for some locales (french) the %p returns nothing
std::string SpaceDate::getPrintableTimeUTC(double JD) const
{
	struct tm time_utc;
	SpaceDate::TimeTmFromJulian(JD, &time_utc);

	static char heure[255];
	switch (timeFormat) {
		case S_TIME_SYSTEM_DEFAULT :
			SpaceDate::myStrftime(heure, 254, "%X", &time_utc);
			break;
		case S_TIME_24H :
			SpaceDate::myStrftime(heure, 254, "%H:%M:%S", &time_utc);
			break;
		case S_TIME_12H :
			SpaceDate::myStrftime(heure, 254, "%I:%M:%S %p", &time_utc);
			break;
	}
	return heure;
}


//! Return a string with the local time (according to timeZoneMode variable) formated according to the timeFormat variable
std::string SpaceDate::getPrintableTimeLocal(double JD) const
{
	struct tm time_local;

	if (timeZoneMode == S_TZ_GMT_SHIFT)
		SpaceDate::TimeTmFromJulian(JD + GMTShift, &time_local);
	else
		SpaceDate::TimeTmFromJulian(JD + SpaceDate::GMTShiftFromSystem(JD)*0.041666666666, &time_local);

	static char heure[255];
	switch (timeFormat) {
		case S_TIME_SYSTEM_DEFAULT :
			SpaceDate::myStrftime(heure, 254, "%X", &time_local);
			break;
		case S_TIME_24H :
			SpaceDate::myStrftime(heure, 254, "%H:%M:%S", &time_local);
			break;
		case S_TIME_12H :
			SpaceDate::myStrftime(heure, 254, "%I:%M:%S %p", &time_local);
			break;
	}
	return heure;
}

float SpaceDate::getGMTShift(double JD, bool _local) const
{
	if (timeZoneMode == S_TZ_GMT_SHIFT) return GMTShift;
	else return SpaceDate::GMTShiftFromSystem(JD,_local);
}

void SpaceDate::setCustomTzName(const std::string& tzname)
{
	#if LINUX
	custom_tz_name = tzname;
	timeZoneMode = S_TZ_CUSTOM;

	if ( custom_tz_name != "") {
		// set the TZ environement variable and update c locale stuff
		putenv(strdup((std::string("TZ=") + custom_tz_name).c_str()));
		tzset();
	}
	#endif
}

//! Return the time in ISO 8601 format that is : %Y-%m-%d %H:%M:%S
std::string SpaceDate::getISO8601TimeLocal(double JD) const
{
	struct tm time_local;
	if (timeZoneMode == S_TZ_GMT_SHIFT)
		SpaceDate::TimeTmFromJulian(JD + GMTShift, &time_local);
	else
		SpaceDate::TimeTmFromJulian(JD + SpaceDate::GMTShiftFromSystem(JD)*0.041666666666, &time_local);

	static char isotime[255];
	SpaceDate::myStrftime(isotime, 254, "%Y-%m-%d %H:%M:%S", &time_local);
	return isotime;
}


//! Convert the time format enum to its associated string
SpaceDate::S_TIME_FORMAT SpaceDate::stringToSTimeFormat(const std::string& tf) const
{
	if (tf == "system_default") return S_TIME_SYSTEM_DEFAULT;
	if (tf == "24h") return S_TIME_24H;
	if (tf == "12h") return S_TIME_12H;
	cLog::get()->write(" unrecognized time_display_format : " + tf + " system_default used.", LOG_TYPE::L_ERROR);
	return S_TIME_SYSTEM_DEFAULT;
}

//! Convert string to its associated time format enum
std::string SpaceDate::sTimeFormatToString(S_TIME_FORMAT tf) const
{
	if (tf == S_TIME_SYSTEM_DEFAULT) return "system_default";
	if (tf == S_TIME_24H) return "24h";
	if (tf == S_TIME_12H) return "12h";
	std::stringstream oss;
	oss << "ERROR : unrecognized time_display_format value : " << tf << " system_default used.";
	cLog::get()->write(oss.str(),  LOG_TYPE::L_ERROR);
	return "system_default";
}

//! Convert the date format enum to its associated string
SpaceDate::S_DATE_FORMAT SpaceDate::stringToSDateFormat(const std::string& df) const
{
	if (df == "system_default") return S_DATE_SYSTEM_DEFAULT;
	if (df == "mmddyyyy") return S_DATE_MMDDYYYY;
	if (df == "ddmmyyyy") return S_DATE_DDMMYYYY;
	if (df == "yyyymmdd") return S_DATE_YYYYMMDD;  // iso8601
	cLog::get()->write(" unrecognized date_display_format : " + df + " system_default used.",LOG_TYPE::L_ERROR );
	return S_DATE_SYSTEM_DEFAULT;
}

//! Convert string to its associated date format enum
std::string SpaceDate::sDateFormatToString(S_DATE_FORMAT df) const
{
	if (df == S_DATE_SYSTEM_DEFAULT) return "system_default";
	if (df == S_DATE_MMDDYYYY) return "mmddyyyy";
	if (df == S_DATE_DDMMYYYY) return "ddmmyyyy";
	if (df == S_DATE_YYYYMMDD) return "yyyymmdd";
	std::stringstream oss;
	oss << "ERROR : unrecognized date_display_format value : " << df << " system_default used.";
	cLog::get()->write(oss.str(), LOG_TYPE::L_WARNING );
	return "system_default";
}

