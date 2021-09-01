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
#include "tools/no_copy.hpp"

class Body;
class AnchorPoint;

//! @class Observer
//! @brief Gère la position de la caméra
//! L'Observer est basée sur une Ancre, avec un déplacement relatif à l'ancre en latitude, longitude et altitude
//!

class Observer: public NoCopy {
public:
	//! Create a new Observer instance which is at a fixed Location
	Observer();
	~Observer();

	void setAnchorPoint(const AnchorPoint * _anchor){
		anchor = _anchor;
	}

	bool isOnBody() const;

	bool isOnBody(const Body * body) const;

	//! Renvoie un lien vers l'astre ou est localisé l'observer
	//! @return une instance sur l'astre ou nullptr
	//! @warning nullptr est retourné si l'observer n'est sur aucun astre
	const Body *getHomeBody() const;

	//! renvoie le nom anglais de la planète de l'observer
	std::string getHomePlanetEnglishName() const;
	//! renvoie le nom I18n de la planète de l'observer
	std::string getHomePlanetNameI18n() const;

	//! renvoie un booleen permettant de savoir si la planète de l'observer est soit la terre, soit la lune
	bool isEarth() const;
	bool isSun() const;

	//! renvois la position a laquelle l'observer est attaché
	Vec3d getObserverCenterPoint() const;

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
	//void save(const std::string& file, const std::string& section);
	//! change settings but don't write to files
	void setConf(InitParser &conf, const std::string& section);
	//! charge la position de l'observer d'un fichier de configuration
	void load(const InitParser& conf, const std::string& section);

	//! fixe la latitude de l'observer sur la planète
	void setLatitude(double l) {
		latitude=l;
		if ( latitude<=0.0 ) {
			latitude=1e-6;
		}
		if ( latitude >= 90.0)
			latitude = 90.0;
	}

	//! renvoie la latitude de l'observer sur la planète
	double getLatitude() const {
		return latitude;
	}

	//! fixe la longitude de l'observer sur la planète
	void setLongitude(double l) {
		longitude=l;
	}

	//! renvoie la longitude de l'observer sur la planète
	double getLongitude() const;
	double getLongitudeForDisplay() const;

	//! fixe l'altitude de l'observer sur la planète
	void setAltitude(double a);

	//! renvoie l'altitude de l'observer sur la planète
	double getAltitude(void) const {
		return altitude;
	}

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
  
  
	//! returns true if we are on the named body
	bool isOnBodyNamed(const std::string& bodyName);

private:
	double longitude;			//!< Longitude in degree
	double latitude;			//!< Latitude in degree
	double altitude;			//!< Altitude in meter

	double defaultLongitude;
	double defaultLatitude;
	double defaultAltitude;

	// for changing position
	bool flag_move_to;
	double start_lat, end_lat;
	double start_lon, end_lon;
	double start_alt, end_alt;
	float move_to_coef, move_to_mult;

	const AnchorPoint * anchor = nullptr;
};

#endif // _OBSERVER_H_
