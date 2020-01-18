/*
* Spacecrafter astronomy simulation and visualization
*
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
*/

#include "coreModule/mCity_mgr.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"


mCity::mCity(const std::string& _name, const std::string& _state, const std::string& _country,
             double _longitude, double _latitude, float zone, int _showatzoom, int _altitude) :
	name(_name), state(_state), country(_country), latitude(_latitude), longitude(_longitude), showatzoom(_showatzoom), altitude(_altitude)
{
}

mCity_Mgr::mCity_Mgr(double _proximity) : proximity(_proximity)
{
}

mCity_Mgr::~mCity_Mgr()
{
	std::vector<mCity*>::iterator iter;
	for (iter = cities.begin(); iter!=cities.end(); iter++) {
		delete (*iter);
	}
}

void mCity_Mgr::addmCity(const std::string& _name, const std::string& _state,
                         const std::string& _country, double _longitude, double _latitude, float _zone, int _showatzoom, int _altitude)
{
	mCity *city = new mCity(_name, _state, _country, _longitude, _latitude, _zone, _showatzoom, _altitude);
	cities.push_back(city);
}

int mCity_Mgr::getNearest(double _longitude, double _latitude)
{
	double dist, closest=10000.;
	int index = -1;
	int i;
	std::string name;

	if (cities.size() == 0) return -1;

	i = 0;
	while (i < (int)cities.size()) {
		name = cities[i]->getName();
		dist = powf(_latitude - cities[i]->getLatitude(),2.f) +
		       powf(_longitude - cities[i]->getLongitude(),2.f);
		dist = powf(dist,0.5f);
		if (index == -1) {
			closest = dist;
			index = i;
		} else if (dist < closest) {
			closest = dist;
			index = i;
		}
		i++;
	}
	if (closest < proximity)
		return index;
	else
		return -1;
}

mCity *mCity_Mgr::getmCity(unsigned int _index)
{
	if (_index < cities.size())
		return cities[_index];
	else
		return nullptr;
}

void mCity_Mgr::setProximity(double _proximity)
{
	if (_proximity < 0)
		proximity = MCITIES_PROXIMITY;
	else
		proximity = _proximity;
}

void mCity_Mgr::getCoordonnatemCity(const std::string name, const std::string country, double &longitude, double &latitude, int &altitude)
{
	int i=0;
	int n=(int)cities.size();

	while (i < n) {
		if ((cities[i]->getName() == name) && ( cities[i]->getCountry() == country)) {
			longitude=cities[i]->getLongitude();
			latitude=cities[i]->getLatitude();
			altitude=cities[i]->getAltitude();
			i=n;
			//cout << "Found City ! " << longitude << ":" << latitude << ":" << altitude << endl;
		}
		i++;
	}
	if (i==n) { //cout << "No city with name " << name << " and country " << country << endl;
		cLog::get()->write("No city with name " + name + " and country " + country , LOG_TYPE::L_WARNING);
		longitude = 0.0;
		latitude  = 0.0;
		altitude  = -100.0;
	}
}


void mCity_Mgr::loadCities(const std::string & fileName)
{
	float time;

	char linetmp[200];
	char cname[50],cstate[50],ccountry[50],clat[20],clon[20],ctime[20];
	int showatzoom;
	int alt;

	//cout << "Loading mCities data...";
	cLog::get()->write("Loading mCities data...", LOG_TYPE::L_INFO);
	FILE *fic = fopen(fileName.c_str(), "r");
	if (!fic) {
		cLog::get()->write("Can't open "+ fileName, LOG_TYPE::L_ERROR);
		return; // no art, but still loaded constellation data
	}

	int line = 0;
	int drop = 0;
	while (!feof(fic)) {

		if (fgets(linetmp, 100, fic) != NULL) {

			if (linetmp[0] != '#') {
				if (sscanf(linetmp, "%s %s %s %s %s %d %s %d\n", cname, cstate, ccountry, clat,
				           clon, &alt, ctime, &showatzoom) != 8) {
					drop++;
					cLog::get()->write("mCity : error while loading city data in line " + Utility::intToString(line) , LOG_TYPE::L_WARNING);
				} else { // no error all seem good
					std::string name, state, country;
					name = cname;
					for (std::string::size_type i=0; i<name.length(); ++i) {
						if (name[i]=='_') name[i]=' ';
					}
					state = cstate;
					for (std::string::size_type i=0; i<state.length(); ++i) {
						if (state[i]=='_') state[i]=' ';
					}
					country = ccountry;
					for (std::string::size_type i=0; i<country.length(); ++i) {
						if (country[i]=='_') country[i]=' ';
					}
					if (ctime[0] == 'x')
						time = 0;
					else
						sscanf(ctime,"%f",&time);
					addmCity(name, state, country, Utility::getDecAngle(clon), Utility::getDecAngle(clat), time, showatzoom, alt);
				}
			}
			line++;
		}
	}
	fclose(fic);

	std::stringstream oss;
	oss << "(" << line << " mcities loaded " << drop << " dropped )";
	cLog::get()->write(oss.str(), LOG_TYPE::L_INFO);
}
