/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014 of the Association Sirius
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

#ifndef _MCITY_H_
#define _MCITY_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

#include "tools/translator.hpp"



#define MCITIES_PROXIMITY 10

class mCity {
public:
	mCity(const std::string& _name = "", const std::string& _state = "", const std::string& _country = "",
	      double _longitude = 0.f, double _latitude = 0.f, float zone = 0, int _showatzoom = 0, int _altitude = 0);
	void addmCity(const std::string& _name = "", const std::string& _state = "", const std::string& _country = "",
	              double _longitude = 0.f, double _latitude = 0.f, float zone = 0, int _showatzoom = 0, int _altitude = 0);

	std::string getName(void) {
		return name;
	}
	std::string getNameI18(void) {
		return _(name);
	}
	std::string getState(void) {
		return state;
	}
	std::string getStateI18(void) {
		return _(state.c_str());
	}
	std::string getCountry(void) {
		return country;
	}
	std::string getCountryI18(void) {
		return _(country.c_str());
	}
	double getLatitude(void) {
		return latitude;
	}
	double getLongitude(void) {
		return longitude;
	}
	int getShowAtZoom(void) {
		return showatzoom;
	}
	int getAltitude(void) {
		return altitude;
	}
private:
	std::string name;
	std::string state;
	std::string country;
	double latitude;
	double longitude;
	float zone;
	int showatzoom;
	int altitude;
};

class mCity_Mgr {
public:
	mCity_Mgr(double _proximity = MCITIES_PROXIMITY);
	~mCity_Mgr();
	void addmCity(const std::string& _name, const std::string& _state, const std::string& _country,
	              double _longitude, double _latitude, float _zone, int _showatzoom, int _altitude = 0);
	int getNearest(double _longitude, double _latitude);
	void setProximity(double _proximity);
	mCity *getmCity(unsigned int _index);
	unsigned int size(void) {
		return cities.size();
	}
	void getCoordonnatemCity(const std::string name, const std::string country, double &longitude, double &latitude, int &altitude);
	void loadCities(const std::string & fileName);
private:
	std::vector<mCity*> cities;
	double proximity;
};

#endif
