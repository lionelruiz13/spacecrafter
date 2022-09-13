/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 Association Sirius
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
#include <fstream>
#include <string>

#include <math.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <limits.h>

#if defined( CYGWIN )
#include <malloc.h>
#endif

#include "tools/utility.hpp"

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

// bool Utility::isAbsolute(const std::string path)
// {
// #if WIN32
//     return path[1] == ':';
// #else
//     return path[0] == '/';
// #endif
// }


// bool CallSystem::fileExist(const std::string& fileName)
// {
//     if (FILE *file = fopen(fileName.c_str(), "r")) {
//         fclose(file);
//         return true;
//     } else {
//         return false;
//     }
// }


double Utility::hmsToRad( unsigned int h, unsigned int m, double s )
{
	return (double)M_PI/24.*h*2.+(double)M_PI/12.*m/60.+s*M_PI/43200.;
}


double Utility::dmsToRad(int d, int m, double s)
{
	return (double)M_PI/180.*d+(double)M_PI/10800.*m+s*M_PI/648000.;
}


void Utility::spheToRect(double lng, double lat, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}


void Utility::spheToRect(double lng, double lat, double r, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat * r, sin(lng) * cosLat * r, sin(lat) * r);
}


void Utility::spheToRect(float lng, float lat, Vec3f& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}


void Utility::rectToSphe(double *lng, double *lat, const Vec3d& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}


void Utility::rectToSphe(float *lng, float *lat, const Vec3f& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}


// Obtains a Vec3f from a string with the form x,y,z
Vec3f Utility::strToVec3f(const std::string& s)
{
	float x, y, z;
	if (s.empty() || (sscanf(s.c_str(),"%f,%f,%f",&x, &y, &z)!=3)) return v3fNull;
	return Vec3f(x,y,z);
}


// Obtains a string from a Vec3f with the form x,y,z
std::string Utility::vec3fToStr(const Vec3f& v)
{
	std::ostringstream os;
	os << v[0] << "," << v[1] << "," << v[2];
	return os.str();
}


// strips trailing whitespaces from buf.
#define iswhite(c)  ((c)== ' ' || (c)=='\t')
static char *trim(char *x)
{
	char *y;

	if (!x)
		return(x);
	y = x + strlen(x)-1;
	while (y >= x && iswhite(*y))
		*y-- = 0; /* skip white space */
	return x;
}


// salta espacios en blanco
static void skipwhite(char **s)
{
	while (iswhite(**s))
		++(*s);
}


double Utility::getDecAngle(const std::string& str)
{
	const char* s = str.c_str();
	char *mptr, *ptr, *dec, *hh;
	int negative = 0;
	char delim1[] = " :.,;DdHhMm'\n\t\xBA";  // 0xBA was old degree delimiter
	char delim2[] = " NSEWnsew\"\n\t";
	int dghh = 0, minutes = 0;
	double seconds = 0.0, pos;
	short count;

	enum _type {
		HOURS, DEGREES, LAT, LONG
	} type;

	if (s == NULL || !*s)
		return(-0.0);
	count = strlen(s) + 1;
	if ((mptr = (char *) malloc(count)) == NULL)
		return (-0.0);
	ptr = mptr;
	memcpy(ptr, s, count);
	trim(ptr);
	skipwhite(&ptr);

	/* the last letter has precedence over the sign */
	if (strpbrk(ptr,"SsWw") != NULL)
		negative = 1;

	if (*ptr == '+' || *ptr == '-')
		negative = (char) (*ptr++ == '-' ? 1 : negative);
	skipwhite(&ptr);
	if ((hh = strpbrk(ptr,"Hh")) != NULL && hh < ptr + 3)
		type = HOURS;
	else if (strpbrk(ptr,"SsNn") != NULL)
		type = LAT;
	else
		type = DEGREES; /* unspecified, the caller must control it */

	if ((ptr = strtok(ptr,delim1)) != NULL)
		dghh = atoi (ptr);
	else {
		free(mptr);
		return (-0.0);
	}

	if ((ptr = strtok(NULL,delim1)) != NULL) {
		minutes = atoi (ptr);
		if (minutes > 59) {
			free(mptr);
			return (-0.0);
		}
	} else {
		free(mptr);
		return (-0.0);
	}

	if ((ptr = strtok(NULL,delim2)) != NULL) {
		if ((dec = strchr(ptr,',')) != NULL)
			*dec = '.';
		seconds = strtod (ptr, NULL);
		if (seconds >= 60.0) {
			free(mptr);
			return (-0.0);
		}
	}

	if ((ptr = strtok(NULL," \n\t")) != NULL) {
		skipwhite(&ptr);
		if (*ptr == 'S' || *ptr == 'W' || *ptr == 's' || *ptr == 'w') negative = 1;
	}

	free(mptr);

	pos = ((dghh*60+minutes)*60 + seconds) / 3600.0;
	if (type == HOURS && pos > 24.0)
		return (-0.0);
	if (type == LAT && pos > 90.0)
		return (-0.0);
	else if (pos > 180.0)
		return (-0.0);

	if (negative)
		pos = -pos;

	return (pos);

}


//! @brief Print the passed angle with the format ddÃƒÂ‚Ã‚Â°mm'ss(.ss)"
//! @param angle Angle in radian
//! @param decimal Define if 2 decimal must also be printed
//! @param useD Define if letter "d" must be used instead of Â°
//! @return The corresponding string
std::string Utility::printAngleDMS(double angle, bool decimals, bool useD)
{
	std::ostringstream oss;

	char sign = '+';
	// wchar_t degsign = L'Â°'; ???
	std::string degsign = "°";
	//char degsign = '\u00B0';
	if (useD) degsign = "d";

	angle *= 180./M_PI;

	if (angle<0) {
		angle *= -1;
		sign = '-';
	}

	if (decimals) {
		int d = (int)(0.5+angle*(60*60*100));
		const int centi = d % 100;
		d /= 100;
		const int s = d % 60;
		d /= 60;
		const int m = d % 60;
		d /= 60;

		oss << sign;
		if (d<10) oss << "0";
		oss << d << degsign;
		if (m<10) oss << "0";
		oss << m << "\'";
		if (s<10) oss << "0";
		oss << s << ".";
		if (centi<10) oss << "0";
		oss<< centi << "\"";

//		         L"%lc%.2d%lc%.2d'%.2d.%02d\"",
//		         sign, d, degsign, m, s, centi);
	} else {
		int d = (int)(0.5+angle*(60*60));
		const int s = d % 60;
		d /= 60;
		const int m = d % 60;
		d /= 60;

		oss << sign;
		if (d<10) oss << "0";
		oss<< d << degsign;
		if (m<10) oss << "0";
		oss << m << "\'";
		if (s<10) oss << "0";
		oss<< s << "\"";
//		         L"%lc%.2d%lc%.2d'%.2d\"",
//		         sign, d, degsign, m, s);
	}
	return oss.str();
}


//! @brief Print the passed angle with the format +hhhmmmss(.ss)"
//! @param angle Angle in radian
//! @param decimals Define if 2 decimal must also be printed
//! @return The corresponding string
std::string Utility::printAngleHMS(double angle, bool decimals)
{
	std::ostringstream oss;

	angle = fmod(angle,2.0*M_PI);
	if (angle < 0.0) angle += 2.0*M_PI; // range: [0..2.0*M_PI)
	angle *= 12./M_PI; // range: [0..24)
	if (decimals) {
		angle = 0.5+angle*(60*60*100); // range:[0.5,24*60*60*100+0.5)
		if (angle >= (24*60*60*100)) angle -= (24*60*60*100);
		int h = (int)angle;
		const int centi = h % 100;
		h /= 100;
		const int s = h % 60;
		h /= 60;
		const int m = h % 60;
		h /= 60;
		if (h<10) oss << "0";
		oss << h << "h";
		if (m<10) oss << "0";
		oss << m << "m";
		if (s<10) oss << "0";
		oss << s << ".";
		if (centi<10) oss << "0";
		oss << centi << "s";
//		         L"%.2dh%.2dm%.2d.%02ds",h,m,s,centi);
	} else {
		angle = 0.5+angle*(60*60); // range:[0.5,24*60*60+0.5)
		if (angle >= (24*60*60)) angle -= (24*60*60);
		int h = (int)angle;
		const int s = h % 60;
		h /= 60;
		const int m = h % 60;
		h /= 60;
		if (h<10) oss << "0";
		oss << h << "h";
		if (m<10) oss << "0";
		oss << m << "m";
		if (s<10) oss << "0";
		oss << s << "s";
//		         L"%.2dh%.2dm%.2ds",h,m,s);
	}
	return oss.str();
}

// ln_date Utility::setAngleHMS(ln_date current_date,double angle, bool decimals)
// {
// 	angle = fmod(angle,2.0*M_PI);
// 	if (angle < 0.0) angle += 2.0*M_PI; // range: [0..2.0*M_PI)
// 	angle *= 12./M_PI; // range: [0..24)
// 	if (decimals) {
// 		angle = 0.5+angle*(60*60*100); // range:[0.5,24*60*60*100+0.5)
// 		if (angle >= (24*60*60*100)) angle -= (24*60*60*100);
// 		int h = (int)angle;
// 		//~ const int centi = h % 100;
// 		h /= 100;
// 		const int s = h % 60;
// 		h /= 60;
// 		const int m = h % 60;
// 		h /= 60;
// 		current_date.hours = h;
// 		current_date.minutes = m;
// 		current_date.seconds = s;
// //		         L"%.2dh%.2dm%.2d.%02ds",h,m,s,centi);
// 	} else {
// 		angle = 0.5+angle*(60*60); // range:[0.5,24*60*60+0.5)
// 		if (angle >= (24*60*60)) angle -= (24*60*60);
// 		int h = (int)angle;
// 		const int s = h % 60;
// 		h /= 60;
// 		const int m = h % 60;
// 		h /= 60;
// 		current_date.hours = h;
// 		current_date.minutes = m;
// 		current_date.seconds = s;

// //		         L"%.2dh%.2dm%.2ds",h,m,s);
// 	}
// 	return current_date;
// }


double Utility::strToDouble(const std::string& str)
{
	if (str.empty()) return 0.;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}


float Utility::strToFloat(const std::string& str)
{
	if (str.empty()) return 0.;
	float dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}


float Utility::strToFloat(const std::string& str, float default_value)
{
	if (str.empty()) return default_value;
	float dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}


double Utility::strToDouble(const std::string& str, double default_value)
{
	if (str.empty()) return default_value;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}


// always positive
double Utility::strToPosDouble(const std::string& str)
{
	if (str.empty()) return 0;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	if (dbl < 0 ) dbl *= -1;
	return dbl;
}


bool Utility::strToBool(const std::string& str)
{
	std::string tmp = str;
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
	if (tmp == "true" || tmp == "1" ) return 1;
	else return 0;
}


bool Utility::strToBool(const std::string& str, bool default_value)
{
	if (str.empty()) return default_value;
	return Utility::strToBool(str);
}


int Utility::strToInt(const std::string& str)
{
	if (str.empty()) return 0;
	int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}


int Utility::strToInt(const std::string& str, int default_value)
{
	if (str.empty()) return default_value;
	int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}


long int Utility::strToLong(const std::string& str)
{
	if (str.empty()) return 0;
	long int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}


float Utility::clamp( float value, float min, float max )
{
    return ( value < min ) ? ( min ) : ( ( value>max) ? ( max) : (value) );
}

bool Utility::isBoolean(const std::string &a)
{
	if ( isTrue(a) || isFalse(a) )
		return true;
	else
		return false;
}

bool Utility::isTrue(const std::string &a)
{
	std::string _a=a;
	std::transform(a.begin(), a.end(),_a.begin(), ::tolower);
	if (_a=="true" || _a == "1" || _a== "on" )
		return true;
	else
		return false;
}

bool Utility::isFalse(const std::string &a)
{
	std::string _a=a;
	std::transform(a.begin(), a.end(),_a.begin(), ::tolower);
	if (_a=="false" || _a =="0" || _a=="off" )
		return true;
	else
		return false;
}
