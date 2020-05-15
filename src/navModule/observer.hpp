/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <string>
#include "tools/init_parser.hpp"
#include "tools/vecmath.hpp"

class Body;
class AnchorPoint;

//! @class Observer
//! @brief Classe qui décrit ou se trouve l'observer
//! L'observer se trouve lié à une planète du système solaire
//! L'observer se déplace sur la planète suivant la latitude, la longitude  et l'altitude
//!
class Observer {
public:
	//! Create a new Observer instance which is at a fixed Location
	Observer(const class SolarSystem &ssystem);
	~Observer();
	Observer(Observer const &) = delete;
	Observer& operator = (Observer const &) = delete;

	void setAnchorPoint(const AnchorPoint * _anchor){
		anchor = _anchor;
	}

	//! fixe la planète de l'observer
	// bool setHomePlanet(const std::string &english_name);
	//! fixe la planète de l'observer
	// void setHomePlanet(Body *p);

	bool isOnBody() const;
	
	bool isOnBody(const Body * body) const;

	//! Renvoie un lien vers l'astre ou est localisé l'observer
	//! @return une instance sur l'astre ou nullptr 
	//! @warning nullptr est retourné si l'observer n'est sur aucun astre
	const Body *getHomeBody(void) const;

	//! renvoie le nom anglais de la planète de l'observer
	std::string getHomePlanetEnglishName(void) const;
	//! renvoie le nom I18n de la planète de l'observer
	std::string getHomePlanetNameI18n(void) const;

	//! renvoie un booleen permettant de savoir si la planète de l'observer est soit la terre, soit la lune
	bool isEarth() const;
	bool isSun() const;

	//! renvois la position a laquelle l'observer est attaché
	Vec3d getObserverCenterPoint(void) const;
	
	//! renvoie la position de l'observer dans le systeme de coordonnée du soleil
	Vec3d getHeliocentricPosition(double JD)const;
	
	//! renvoie la distance:  centre de la planete et altitude de l'observer
	double getDistanceFromCenter(void) const;

	//! renvoie la matrice de position equatorial à partir de la position local de la planète
	Mat4d getRotLocalToEquatorial(double jd) const;
	Mat4d getRotLocalToEquatorialFixed(double jd) const;
	//! Compute the z rotation to use from equatorial to geographic coordinates
	Mat4d getRotEquatorialToVsop87(void) const;

	//! savegarde la position de l'observer dans un fichier
	void save(const std::string& file, const std::string& section);
	//! change settings but don't write to files
	void setConf(InitParser &conf, const std::string& section);
	//! charge la position de l'observer d'un fichier de configuration
	void load(const InitParser& conf, const std::string& section);

	//! retourne le nom de l'endroit ou l'on se situe
	// std::string getName(void) const;

	//! fixe la latitude de l'observer sur la planète
	double setLatitude(double l) {
		latitude=l;
		if ( latitude==0.0 ) {
			latitude=1e-6;
		}
		return latitude;
	}

	//! renvoie la latitude de l'observer sur la planète
	double getLatitude(void) const {
		return latitude;
	}

	//! fixe la longitude de l'observer sur la planète
	double setLongitude(double l) {
		longitude=l;
		return(l);
	}

	//! renvoie la longitude de l'observer sur la planète
	double getLongitude(void) const;

	//! fixe l'altitude de l'observer sur la planète
	void setAltitude(double a);
	
	// void setSpacecraft(bool a) {
	// 	spacecraft = a;
	// }

	// bool getSpacecraft() {
	// 	return spacecraft;
	// }

	//! renvoie l'altitude de l'observer sur la planète
	double getAltitude(void) const {
		return altitude;
	}

	//! change le nom du landscape de l'observer
	// void setLandscapeName(const std::string s);

	//! renvoie le nom du landscape de l'observer
	// std::string getLandscapeName(void) const {
	// 	return landscape_name;
	// }

	// sert à retrouver la position initiale de l'observer après qu'il se soit déplacé afin d'y revenir
	//! renvoie la latitude initiale de l'observer à son chargement
	double getDefaultLatitude() {
		return defaultLatitude;
	}
	//! renvoie la longitude initiale de l'observer à son chargement
	double getDefaultLongitude() {
		return defaultLongitude;
	}
	//! renvoie l'altitude initiale de l'observer à son chargement
	double getDefaultAltitude() {
		return defaultAltitude;
	}

	//! Move to relative latitude where home planet is fixed.
	void moveRelLat(double lat, int delay);

	//! Move to relative longitude where home planet is fixed.
	void moveRelLon(double lon, int delay);

	//! Move to relative altitude where home planet is fixed.
	void moveRelAlt(double alt, int delay);

	//! move gradually to a new observation location
	void moveTo(double lat, double lon, double alt, int duration, /*const std::string& _name,*/  bool calculate_duration=0);  // duration in ms
	//! for moving observer position gradually
	void update(int delta_time);

	//! sauvegarde la planete lorsque l'on quitte le mode IN_SOLARSYSTEM
	void saveBodyInSolarSystem();
	//! charge la planete lorsque l'on revient dans le mode IN_SOLARSYSTEM
	void loadBodyInSolarSystem();
	//! fixe l'observer sur la position du soleil (mode IN_GALAXY)
	void fixBodyToSun();
	
	//! returns true if we are on the named body
	bool isOnBodyNamed(const std::string& bodyName); 

private:
	const class SolarSystem &ssystem;
	// std::string name;			//!< Position name

	Body *planet;			//!< la planete ou se trouve l'observer
	Body *planetInSolarSystem; //!< la planete du systeme solaire où l'on se trouvait

	double longitude;		//!< Longitude in degree
	double latitude;		//!< Latitude in degree
	double altitude;			//!< Altitude in meter
	
	// bool spacecraft;

	double defaultLongitude;
	double defaultLatitude;
	double defaultAltitude;
	// std::string m_defaultHome;
	// std::string landscape_name;

	// for changing position
	bool flag_move_to;
	double start_lat, end_lat;
	double start_lon, end_lon;
	double start_alt, end_alt;
	float move_to_coef, move_to_mult;

	const AnchorPoint * anchor = nullptr;
};

#endif // _OBSERVER_H_
