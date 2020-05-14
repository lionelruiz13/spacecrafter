/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003, 2008 Fabien Chereau
 * Copyright (C) 2009-2011 Digitalis Education Solutions, Inc.
 * Copyright (C) 2016 Association Sirius
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

#include <string>
#include <cstdlib>
#include <algorithm>

#include "bodyModule/solarsystem.hpp"
#include "bodyModule/body.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "tools/utility.hpp"
#include "tools/init_parser.hpp"
#include "tools/translator.hpp"
#include "tools/log.hpp"
#include "tools/fmath.hpp"
#include "eventModule/event_manager.hpp"
#include "eventModule/AltitudeEvent.hpp"
#include "mainModule/define_key.hpp"

Observer::Observer(const SolarSystem &ssystem)
	:ssystem(ssystem), planet(0),
	 longitude(0.), latitude(1e-9), altitude(0),
	 defaultLongitude(0), defaultLatitude(1e-9), defaultAltitude(0)
{
	// name = "Anonymous_Location";
	flag_move_to = false;
	planetInSolarSystem = nullptr;

}


Observer::~Observer(){ }

void Observer::setAltitude(double a) {
	altitude=a;
	flag_move_to = 0;
	// on definit l'event : on change d'altitude
	// Nicolas écrit du code magique.
	Event* event = new AltitudeEvent(a);
	EventManager::getInstance()->queue(event);
}


Vec3d Observer::getHeliocentricPosition(double JD)const{
	
	Mat4d mat_local_to_earth_equ = getRotLocalToEquatorial(JD);
	Mat4d mat_earth_equ_to_j2000 = mat_vsop87_to_j2000 * getRotEquatorialToVsop87();
	//~ Mat4d mat_j2000_to_earth_equ = mat_earth_equ_to_j2000.transpose();

	//~ Mat4d mat_helioToEarthEqu =
	    //~ mat_j2000_to_earth_equ *
	    //~ mat_vsop87_to_j2000 *
	    //~ Mat4d::translation(-getObserverCenterPoint());

	Mat4d tmp =
	    mat_j2000_to_vsop87 *
	    mat_earth_equ_to_j2000 *
	    mat_local_to_earth_equ;

	Mat4d mat_local_to_helio =  Mat4d::translation(getObserverCenterPoint()) *
	                      tmp *
	                      Mat4d::translation(Vec3d(0.,0., getDistanceFromCenter()));
	
	return mat_local_to_helio*Vec3d(0.,0.,0.);
}


Vec3d Observer::getObserverCenterPoint(void) const
{
	//TODO touver un meilleur endroit pour l'update
	//anchor->update();
	return anchor->getHeliocentricEclipticPos();
	//return getHomePlanet()->get_heliocentric_ecliptic_pos();
}


double Observer::getDistanceFromCenter(void) const
{
	if(anchor->isOnBody())
		return getHomeBody()->getRadius() + (altitude/(1000*AU));
		//~ return getHomePlanet()->getRadius() + (altitude/(1000*AU));
	else
		return altitude/(1000*AU);
}


Mat4d Observer::getRotLocalToEquatorial(double jd) const
{

	return anchor->getRotLocalToEquatorial(jd,latitude,longitude, altitude);
	/*
	double lat = latitude;
	// TODO: Figure out how to keep continuity in sky as reach poles
	// otherwise sky jumps in rotation when reach poles in equatorial mode
	// This is a kludge
	if ( lat > 89.5 )  lat = 89.5;
	if ( lat < -89.5 ) lat = -89.5;
	return Mat4d::zrotation((getHomePlanet()->getSiderealTime(jd)+longitude)*(C_PI/180.))
	       * Mat4d::yrotation((90.-lat)*(C_PI/180.));*/
}


Mat4d Observer::getRotLocalToEquatorialFixed(double jd) const
{
	double lat = latitude;
	// TODO: Figure out how to keep continuity in sky as reach poles
	// otherwise sky jumps in rotation when reach poles in equatorial mode
	// This is a kludge
	if ( lat > 89.5 )  lat = 89.5;
	if ( lat < -89.5 ) lat = -89.5;
	return Mat4d::zrotation((-longitude)*(C_PI/180.)) * Mat4d::yrotation((90.-lat)*(C_PI/180.));
}


Mat4d Observer::getRotEquatorialToVsop87(void) const
{
	return anchor->getRotEquatorialToVsop87();
	//return getHomePlanet()->getRotEquatorialToVsop87();
}


//~ void Observer::load(const std::string& file, const std::string& section)
//~ {
	//~ InitParser conf;
	//~ conf.load(file);
	//~ if (!conf.findEntry(section)) {
		//~ cerr << "ERROR : Can't find observer section " << section << " in file " << file << endl;
		//~ assert(0);
	//~ }
	//~ load(conf, section);
//~ }


bool Observer::setHomePlanet(const std::string &english_name)
{
	Body *p = nullptr;

	if (english_name == "default")
		p = ssystem.searchByEnglishName(m_defaultHome);
	else
		p = ssystem.searchByEnglishName(english_name);

	if (p==nullptr) {
		std::cerr << "Can't set home planet to " + english_name + " because it is unknown\n";
		return false;
	}
	setHomePlanet(p);
	return true;
}


void Observer::setHomePlanet(Body *p)
{
	assert(p); // Assertion enables to track bad calls.
	if (planet != p) {
		planet = p;
		std::string label =  p->getNameI18n();
		if( label.empty() )
			label = p->getEnglishName();
	}
}

bool Observer::isOnBody() const{
	return anchor->isOnBody();
}

bool Observer::isOnBody(const Body * body)const{
	return anchor->isOnBody(body);
}

// const Body *Observer::getHomePlanet(void) const
// {
// 	return planet;
// }


const Body *Observer::getHomeBody(void) const
{
	//temporaire
	if (anchor==nullptr)
		return nullptr;
	// si on n'est pas sur un Body
	if (!anchor->isOnBody())
		return nullptr;
	// sinon on renvoie bien un Body
	return anchor->getBody();
}




void Observer::load(const InitParser& conf, const std::string& section)
{
	// name = _(conf.getStr(section, "name").c_str());

	// for (std::string::size_type i=0; i<name.length(); ++i) {
	// 	if (name[i]=='_') name[i]=' ';
	// }

	m_defaultHome = conf.getStr(section, SCK_HOME_PLANET );


	if (!setHomePlanet(m_defaultHome)) {
		planet = ssystem.getEarth();
	}

	cLog::get()->write("Loading location: on " + planet->getEnglishName(), LOG_TYPE::L_INFO);

	defaultLatitude = setLatitude( Utility::getDecAngle(conf.getStr(section, SCK_LATITUDE)) );
	longitude = defaultLongitude = Utility::getDecAngle(conf.getStr(section, SCK_LONGITUDE));
	altitude = defaultAltitude = conf.getDouble(section, SCK_ALTITUDE);

	// stop moving and stay put
	flag_move_to = 0;

	setLandscapeName(conf.getStr(section, SCK_LANDSCAPE_NAME));

	cLog::get()->write("Landscape is: "+ landscape_name);
}


void Observer::setLandscapeName(const std::string s)
{
	// need to lower case name because config file parser lowercases section names
	std::string x = s;
	transform(x.begin(), x.end(), x.begin(), ::tolower);
	landscape_name = x;
}

//~ void Observer::save(const std::string& file, const std::string& section)
//~ {
	//~ cLog::get()->write("Saving location " +std::string(name) + " to file " + file.c_str(), LOG_TYPE::L_INFO);
	//~ InitParser conf;
	//~ conf.load(file);
	//~ setConf(conf,section);
	//~ conf.save(file);
//~ }


// change settings but don't write to files
void Observer::setConf(InitParser & conf, const std::string& section)
{

	// conf.setStr(section + ":name", std::string(name));
	// conf.setStr(section + ":name", "AutoSavedLocation");
	conf.setStr(section + ":" + SCK_HOME_PLANET, planet->getEnglishName());
	conf.setStr(section + ":" + SCK_LATITUDE, Utility::printAngleDMS(latitude*C_PI/180.0, true, true));
	conf.setStr(section + ":" + SCK_LONGITUDE, Utility::printAngleDMS(longitude*C_PI/180.0,true, true));

	conf.setDouble(section + ":" + SCK_ALTITUDE, altitude);
	conf.setStr(section + ":" + SCK_LANDSCAPE_NAME, landscape_name);

	// saving values so new defaults to track
	defaultLatitude = latitude;
	defaultLongitude = longitude;
	defaultAltitude = altitude;
	m_defaultHome = planet->getEnglishName();
}


// move gradually to a new observation location
void Observer::moveTo(double lat, double lon, double alt, int duration, /*const std::string& _name,*/ bool calculate_duration)
{
	// name = _name;

	if (alt!=altitude) {
		Event* event = new AltitudeEvent(alt);
		EventManager::getInstance()->queue(event);
	}

	// If calculate_duration is true, scale the duration based on the amount of change
	// Note: Doesn't look at altitude change
	if( calculate_duration ) {
		float deltaDegrees = abs(latitude - lat) + abs(longitude - lon);
		if(deltaDegrees > 1) duration *= deltaDegrees/10.;
		else duration = 250;  // Small change should be almost instantaneous
	}

	if (duration==0) {
		setLatitude(lat);
		setLongitude(lon);
		setAltitude(alt);

		flag_move_to = 0;
		return;
	}

	start_lat = latitude;
	end_lat = lat;

	start_lon = longitude;
	end_lon = lon;

	start_alt = altitude;
	end_alt = alt;

	flag_move_to = 1;

	move_to_coef = 1.0f/duration;
	move_to_mult = 0;

}

void Observer::moveRelLat(double lat, int delay) {
	double latimem = latitude+lat;
	if (latimem>90) latimem=90;
	if (latimem<-90) latimem=-90;
	moveTo(latimem, longitude, altitude, delay);
}

//! Move to relative longitude where home planet is fixed.
void Observer::moveRelLon(double lon, int delay) {
	moveTo(latitude, longitude+lon, altitude, delay);
}

//! Move to relative altitude where home planet is fixed.
void Observer::moveRelAlt(double alt, int delay) {
	moveTo(latitude, longitude, altitude+alt, delay);
}

bool Observer::isOnBodyNamed(const std::string& bodyName){
	
	if(anchor == nullptr)
		return false;
	
	if(!isOnBody())
		return false;
	
	const Body * b = getHomeBody();
	
	if(b != nullptr)
		return getHomeBody()->getEnglishName() == bodyName;
	
	return false;
	
}

std::string Observer::getHomePlanetEnglishName(void) const
{
	const Body *p = getHomeBody();
	return p ? p->getEnglishName() : "";
}


std::string Observer::getHomePlanetNameI18n(void) const
{
	const Body *p = getHomeBody();
	return p ? p->getNameI18n() : "";
}

bool Observer::isEarth() const {
	return getHomePlanetEnglishName() == "Earth";
}

bool Observer::isSun() const {
	return getHomePlanetEnglishName() == "Sun";
}

// std::string Observer::getName(void) const
// {
// 	// return name;
// 	std::ostringstream oss;
// 	oss << latitude <<"-"<<longitude<<"-"<< altitude;
// 	return oss.str();
// }


// for moving observer position gradually
void Observer::update(int delta_time)
{
	if (flag_move_to) {
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_move_to = 0;
		}

		setLatitude( start_lat - move_to_mult*(start_lat-end_lat) );
		longitude = start_lon - move_to_mult*(start_lon-end_lon);
		altitude  = start_alt - move_to_mult*(start_alt-end_alt);
	}
}


double Observer::getLongitude(void) const
{
// wrap to proper range
	double tmp = longitude;
	while (tmp > 180) {
		tmp -= 360;
	}
	while (tmp < -180 ) {
		tmp += 360;
	}

	return tmp;
}

void Observer::saveBodyInSolarSystem()
{
	planetInSolarSystem = planet;
}

void Observer::loadBodyInSolarSystem()
{
	if (planetInSolarSystem != nullptr)
		planet=planetInSolarSystem;
}

void Observer::fixBodyToSun()
{
	setHomePlanet("Solar_System");
}
