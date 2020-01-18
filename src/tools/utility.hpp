/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#ifndef _S_UTILITY_H_
#define _S_UTILITY_H_

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <ctime>
#include "tools/vecmath.hpp"


template <typename T> T myMin(T a, T b){
   if(a<b)
      return a;
   else
      return b;
};

template <typename T> T myMax(T a, T b){
   if(a>b)
      return a;
   else
      return b;
};


template<typename T> class RangeMap {
public:
	RangeMap(T sourceHigh, T sourceLow, T targetHigh, T targetLow) {
		m_sHigh = sourceHigh;
		m_sLow = sourceLow;
		m_tHigh = targetHigh;
		m_tLow = targetLow;
	}
	T Map(T n) {
		return (((n - m_sLow) * (m_tHigh - m_tLow)) / (m_sHigh - m_sLow)) + m_tLow;
	}

private:
	T m_sHigh;
	T m_sLow;
	T m_tHigh;
	T m_tLow;
};


typedef std::map< std::string, std::string > stringHash_t;
typedef stringHash_t::const_iterator stringHashIter_t;

class Utility {
public:
	// static int s_round( float value );

	//! @brief Convert an angle in hms format to radian
	//! @param h hour component
	//! @param m minute component
	//!	@param s second component
	//! @return angle in radian
	static double hmsToRad(unsigned int h, unsigned int m, double s);

	//! @brief Convert an angle in dms format to radian
	//! @param d degree component
	//! @param m arcmin component
	//!	@param s arcsec component
	//! @return angle in radian
	static double dmsToRad(int d, int m, double s);

	//! @brief Obtains a Vec3f from a string
	//! @param s the string describing the Vector with the form "x,y,z"
	//! @return The corresponding vector
	static Vec3f strToVec3f(const std::string& s);

	//! @brief Obtains a string from a Vec3f
	//! @param v The vector
	//! @return the string describing the Vector with the form "x,y,z"
	static std::string vec3fToStr(const Vec3f& v);

	//! @brief Print the passed angle with the format dd°mm'ss(.ss)"
	//! @param angle Angle in radian
	//! @param decimal Define if 2 decimal must also be printed
	//! @param useD Define if letter "d" must be used instead of °
	//! @return The corresponding string
	static std::string printAngleDMS(double angle, bool decimals = false, bool useD = false);

	//! @brief Print the passed angle with the format +hh:mm:ss(.ss)"
	//! @param angle Angle in radian
	//! @param decimals Define if 2 decimal must also be printed
	//! @return The corresponding string
	static std::string printAngleHMS(double angle, bool decimals = false);
	// static ln_date setAngleHMS(ln_date current_date, double angle, bool decimals);
	//! @brief Format the double value to a string (with current locale)
	//! @param d The input double value
	//! @return The matching string
	static std::string doubleToString(double d);

	//! @brief Format the int value to a string (with current locale)
	//! @param i The input int value
	//! @return The matching string
	static std::string intToString(int i);

	//! returns true if the given path is absolute
    static bool isAbsolute(const std::string path);

	static void spheToRect(double lng, double lat, Vec3d& v);
	static void spheToRect(double lng, double lat, double r, Vec3d& v);
	static void spheToRect(float lng, float lat, Vec3f& v);
	static void rectToSphe(double *lng, double *lat, const Vec3d& v);
	static void rectToSphe(float *lng, float *lat, const Vec3f& v);

	static float strToFloat(const std::string& str);
	static float strToFloat(const std::string& str, float default_value);

	static double strToDouble(const std::string& str);
	static double strToDouble(const std::string& str, double default_value);
	// always positive
	static double strToPosDouble(const std::string& str);
	// true, 1 vs false, 0
	static bool strToBool(const std::string& str);
	static bool strToBool(const std::string& str, bool default_value);

	static int strToInt(const std::string& str);
	static int strToInt(const std::string& str, int default_value);

	/* Obtains Latitude, Longitude, RA or Declination from a string. */
	static double getDecAngle(const std::string&);

	static std::string doubleToStr(double dbl);
	static std::string floatToStr(float dbl);
	static long int strToLong(const std::string& str);

	static float clamp( float value, float min, float max ) ;
};

#endif
