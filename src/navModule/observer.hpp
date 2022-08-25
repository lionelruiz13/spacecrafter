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
#include <memory>
#include "tools/init_parser.hpp"
#include "tools/vecmath.hpp"
#include "tools/rotator.hpp"
#include "tools/no_copy.hpp"

class Body;
class AnchorPoint;

//! @class Observer
//! @brief Manages the camera's position
//! The Observer is based on an Anchor, with a displacement relative to the anchor in latitude, longitude and altitude
//!

class Observer: public NoCopy {
public:
	//! Create a new Observer instance which is at a fixed Location
	Observer();
	~Observer();

	void setAnchorPoint(std::shared_ptr<AnchorPoint> _anchor);

	std::shared_ptr<AnchorPoint> getAnchorPoint() const {
		return anchor;
	}

	bool isOnBody() const;

	bool isOnBody(std::shared_ptr<Body> body) const;

	//! Returns a link to the star where the observer is located
	//! @return an instance on the star or nullptr
	//! @warning nullptr is returned if the observer is not on any star
	std::shared_ptr<Body> getHomeBody() const;

	//! returns the english name of the planet of the observer
	std::string getHomePlanetEnglishName() const;
	//! returns the I18n name of the planet of the observer
	std::string getHomePlanetNameI18n() const;

	//! returns a boolean allowing to know if the planet of the observer is either the earth or the moon
	bool isEarth() const;
	bool isSun() const;

	//! returns the position to which the observer is attached
	Vec3d getObserverCenterPoint() const;

	//! returns the position of the observer in the sun coordinate system
	Vec3d getHeliocentricPosition(double JD)const;

	//! returns the distance: center of the planet and altitude of the observer
	double getDistanceFromCenter(void) const;
	//! changes the altitude to match the distance between the center of the planet and the desired observer
	void setDistanceFromCenter(double distance);

	//! returns the equatorial position matrix from the local position of the planet
	Mat4d getRotLocalToEquatorial(double jd) const;
	Mat4d getRotLocalToEquatorialFixed(double jd) const;
	//! Compute the z rotation to use from equatorial to geographic coordinates
	Mat4d getRotEquatorialToVsop87(void) const;

	//! save the observer's position in a file
	//void save(const std::string& file, const std::string& section);
	//! change settings but don't write to files
	void setConf(InitParser &conf, const std::string& section);
	//! load the observer's position from a configuration file
	void load(const InitParser& conf, const std::string& section);

	//! set the latitude of the observer on the planet
	void setLatitude(double l) {
		latitude=l;
		if ( latitude==0.0 ) {
			latitude=1e-6;
		}
		if ( latitude <= -90.0)
			latitude = -90.0;
		if ( latitude >= 90.0)
			latitude = 90.0;
	}

	//! renvoie la latitude de l'observer sur la planète
	double getLatitude() const {
		return latitude;
	}

	//! fixes the longitude of the observer on the planet
	void setLongitude(double l) {
		longitude=l;
	}

	//! returns the longitude of the observer on the planet
	double getLongitude() const;
	double getLongitudeForDisplay() const;

	//! fixe l'altitude de l'observer sur la planète
	void setAltitude(double a);

	//! renvoie l'altitude de l'observer sur la planète
	double getAltitude(void) const {
		return altitude;
	}

	// is used to find the initial position of the observer after it has moved in order to return to it
	//! returns the initial latitude of the observer to its load
	double getDefaultLatitude() {
		return defaultLatitude;
	}
	//! returns the initial longitude of the observer at its loading
	double getDefaultLongitude() {
		return defaultLongitude;
	}
	//! returns the initial altitude of the observer at its loading
	double getDefaultAltitude() {
		return defaultAltitude;
	}

	//! Move to relative latitude where home planet is fixed.
	void moveRelLat(double lat, int delay);

	//! Move to relative longitude where home planet is fixed.
	void moveRelLon(double lon, int delay);

	//! Move to relative altitude where home planet is fixed.
	void moveRelAlt(double alt, int delay);

	//! Move gradually to a relative longitude latitude altitude where home planet is fixed/
	void moveRel(double lat, double lon, double alt, int duration, bool calculate_duration=0);

	//! move gradually to a new observation location
	void moveTo(double lat, double lon, double alt, int duration, /*const std::string& _name,*/  bool calculate_duration=0);  // duration in ms

	//! Move to quaternion angle (quaternion angle apply before every other transformations)
	//! @param isMaxDuration is the given duration for a 180° rotation
	void moveTo(const Vec4d &target, int duration, bool isMaxDuration = false);

	//! Move to the quaternion angle relative to the current quaternion angle (quaternion angle apply before every other transformations)
	//! @param isMaxDuration is the given duration for a 180° rotation
	void moveRel(const Vec4d &relTarget, int duration, bool isMaxDuration = false);

	//! Perform a movement relative to eye, only work for instantaneous movements
	void moveEyeRel(const Vec3d &eyeTarget);

	//! Perform a movement relative to current position
	void moveRel3D(const Vec3d &target);

	//! Set quaternion angle (quaternion angle apply before every other transformations)
	void setRotation(const Vec4d &target);

	//! Set quaternion angle relative to the current quaternion angle (quaternion angle apply before every other transformations)
	void setRelRotation(const Vec4d &relTarget);

	//! for moving observer position gradually
	void update(int delta_time);

	//! For switching between quaternion and lon/lat mode
	void setQuaternionMode(bool mode);

	//! Return true if in quaterion mode
	bool getQuaternionMode() const {
		return flag_quaternion_mode;
	}

	//! Tell if altitide movement should be forward movement
	void setEyeRelativeMode(bool mode);

	//! Return true if in quaterion mode
	bool getEyeRelativeMode() const {
		return flag_eye_relative_mode;
	}

	//! returns true if we are on the named body
	bool isOnBodyNamed(const std::string& bodyName);

	void setEyeMatrix(const Mat4d &mat_eye_to_local, const Mat4d &mat_altitude_to_earth_equ) {
		this->mat_eye_to_local = mat_eye_to_local;
		this->mat_altitude_to_earth_equ = mat_altitude_to_earth_equ;
	}
private:
	double longitude;			//!< Longitude in degree
	double latitude;			//!< Latitude in degree
	double altitude;			//!< Altitude in meter

	double defaultLongitude;
	double defaultLatitude;
	double defaultAltitude;

	// for changing position
	bool flag_move_to;
	bool flag_quaternion_mode = false; // True for quaternion movements, false for lon/lat movements
	bool flag_eye_relative_mode = false; // True to redirect movements in eye relative, require quaterion mode to be enabled
	double start_lat, end_lat;
	// double start_lon, current_lon;
	double rel_lon;
	double start_alt, end_alt;
	float move_to_coef, move_to_mult;

	std::shared_ptr<AnchorPoint> anchor = nullptr;
	Rotator<double> rotator;

	Mat4d mat_altitude_to_earth_equ; // observer altitude to anchor equatorial
	Mat4d mat_eye_to_local;
};

#endif // _OBSERVER_H_
