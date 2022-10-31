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
#include "bodyModule/body.hpp"
#include "navModule/anchor_point.hpp"
#include "navModule/anchor_point_body.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "tools/utility.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"
#include "eventModule/event_recorder.hpp"
#include "eventModule/CoreEvent.hpp"
#include "mainModule/define_key.hpp"
#include "coreModule/coreLink.hpp"

// Slowdown factor used for eye relative movements
constexpr double eyeMoveDivisor = 4*1000*AU;

Observer::Observer()
	: longitude(0.), latitude(1e-9), altitude(0),
	defaultLongitude(0), defaultLatitude(1e-9), defaultAltitude(0)
{
	flag_move_to = false;
	anchorAlt = std::make_shared<AnchorPoint>();
}


Observer::~Observer(){ }

void Observer::setAltitude(double a) {
	if (a < 0.1)
		a = 0.1;
	altitude=a;
	flag_move_to = 0;
	Event* event = new AltitudeEvent(a);
	EventRecorder::getInstance()->queue(event);
}


Vec3d Observer::getHeliocentricPosition(double JD) const
{
	return (flag_eye_relative_mode) ? anchor->getHeliocentricEclipticPos() : Mat4d::translation(getObserverCenterPoint()) * getRotEquatorialToVsop87() * getRotLocalToEquatorial(JD) * Vec3d(0., 0., getDistanceFromCenter());
}
// anchor->getHeliocentricEclipticPos() - anchorAlt->getHeliocentricEclipticPos() = getRotEquatorialToVsop87() * getRotLocalToEquatorial(JD) * Vec3d(0, 0, 1)

Vec3d Observer::getObserverCenterPoint(void) const
{
	return (flag_eye_relative_mode) ? anchorAlt->getHeliocentricEclipticPos() : anchor->getHeliocentricEclipticPos();
}


double Observer::getDistanceFromCenter(void) const
{
	if (flag_eye_relative_mode) {
		return (anchorAlt->getHeliocentricEclipticPos() - anchor->getHeliocentricEclipticPos()).length();
	} else {
		if (anchor->isOnBody())
			return anchor->getBody()->getRadius() + (altitude/(1000*AU));
		else
			return altitude/(1000*AU);
	}
}

void Observer::setDistanceFromCenter(double distance)
{
	auto &tmp = (flag_eye_relative_mode) ? anchorAlt : anchor;
	if (tmp->isOnBody())
		altitude = (distance - tmp->getBody()->getRadius()) * (1000*AU);
	else
		altitude = distance*(1000*AU);
}

Mat4d Observer::getRotLocalToEquatorial(double jd) const
{
	return anchor->getRotLocalToEquatorial(jd,latitude,longitude, altitude);
}


Mat4d Observer::getRotLocalToEquatorialFixed(double jd) const
{
	double lat = latitude;
	if ( lat > 89.5 )  lat = 89.5;
	if ( lat < -89.5 ) lat = -89.5;
	return Mat4d::zrotation((-longitude)*(M_PI/180.)) * Mat4d::yrotation((90.-lat)*(M_PI/180.));
}

Mat4d Observer::getRotEquatorialToVsop87(void) const
{
	return anchor->getRotEquatorialToVsop87() * rotator.getCachedMatrix();
}

bool Observer::isOnBody() const{
	return anchor->isOnBody();
}

bool Observer::isOnBody(std::shared_ptr<Body> body)const{
	return anchor->isOnBody(body);
}

std::shared_ptr<Body> Observer::getHomeBody(void) const
{
	if (anchor==nullptr)
		return nullptr;
	// if we are not on a Body
	if (!anchor->isOnBody())
		return nullptr;
	// otherwise a Body is returned
	return anchor->getBody();
}

void Observer::load(const InitParser& conf, const std::string& section)
{
	this->setLatitude( Utility::getDecAngle(conf.getStr(section, SCK_LATITUDE)) );
	defaultLatitude = latitude;
	longitude = defaultLongitude = Utility::getDecAngle(conf.getStr(section, SCK_LONGITUDE));
	altitude = defaultAltitude = conf.getDouble(section, SCK_ALTITUDE);

	// stop moving and stay put
	flag_move_to = 0;
}

// change settings but don't write to files
void Observer::setConf(InitParser & conf, const std::string& section)
{
	conf.setStr(section + ":" + SCK_LATITUDE, Utility::printAngleDMS(latitude*M_PI/180.0, true, true));
	conf.setStr(section + ":" + SCK_LONGITUDE, Utility::printAngleDMS(longitude*M_PI/180.0,true, true));
	conf.setDouble(section + ":" + SCK_ALTITUDE, altitude);

	// saving values so new defaults to track
	defaultLatitude = latitude;
	defaultLongitude = longitude;
	defaultAltitude = altitude;
}


// move gradually to a new observation location
void Observer::moveTo(double lat, double lon, double alt, int duration, bool calculate_duration)
{
	if (alt!=altitude) {
		Event* event = new AltitudeEvent(alt);
		EventRecorder::getInstance()->queue(event);
	}

	// If calculate_duration is true, scale the duration based on the amount of change
	// Note: Doesn't look at altitude change
	if( calculate_duration ) {
		float deltaDegrees = abs(latitude - lat) + abs(longitude - lon);
		if(deltaDegrees > 1) duration *= deltaDegrees/10.;
		else duration = 250;  // Small change should be almost instantaneous
	}

	if (flag_quaternion_mode)
		moveTo({1, 0, 0, 0}, duration);

	if (duration==0) {
		setLatitude(lat);
		setLongitude(lon);
		setAltitude(alt);

		flag_move_to = 0;
		return;
	}

	start_lat = latitude;
	end_lat = lat;

	// start_lon = longitude;
	// current_lon = longitude;
	rel_lon = lon - longitude;

	start_alt = altitude;
	end_alt = alt;

	flag_move_to = 1;

	move_to_coef = 1.0f/duration;
	move_to_mult = 0;

}

void Observer::moveRelLat(double lat, int delay) {
	if (flag_eye_relative_mode) {
		moveEyeRel(Vec3d(0, lat / 10.f * altitude, 0));
	} else
		moveRel(lat, 0, 0, delay);
}

//! Move to relative longitude where home planet is fixed.
void Observer::moveRelLon(double lon, int delay) {
	if (flag_eye_relative_mode) {
		moveEyeRel(Vec3d(lon / 10.f * altitude, 0, 0));
	} else
		moveRel(0, lon, 0, delay);
}

//! Move to relative altitude where home planet is fixed.
void Observer::moveRelAlt(double alt, int delay) {
	if (flag_eye_relative_mode) {
		moveEyeRel(Vec3d(0, 0, -alt / 1.f));
	} else
		moveTo(latitude, longitude, altitude+alt, delay);
}

void Observer::multAltitude(double coef) {
	if (flag_eye_relative_mode) {
		moveEyeRel(Vec3d(0, 0, altitude * (coef-1)));
	} else
		setAltitude(altitude * coef);
}

void Observer::moveRel(double lat, double lon, double alt, int duration, bool calculate_duration)
{
	if (flag_quaternion_mode) {
		if (duration) {
			rotator.moveRel(Vec4d::zyrotation(lon*(M_PI/180.), -lat*(M_PI/180.)), duration);
			if (alt) {
				if (flag_move_to) {
					end_alt += alt;
				} else {
					end_alt = altitude + alt;
					flag_move_to = 1;
				}
				start_alt = altitude;
				move_to_coef = 1.0f/duration;
				move_to_mult = 0;
			}
		} else {
			rotator.setRelRotation(Vec4d::zyrotation(lon*(M_PI/180.), -lat*(M_PI/180.)));
			if (alt)
				setAltitude(altitude+alt);
		}
	} else {
		double latimem = latitude+lat;
		if (latimem>90) latimem=90;
		if (latimem<-90) latimem=-90;
		moveTo(latimem, longitude+lon, altitude+alt, duration, calculate_duration);
	}
}

void Observer::moveEyeRel(const Vec3d &eyeTarget)
{
	moveRel3D(mat_eye_to_helio_untranslated.multiplyWithoutTranslation(eyeTarget));
}

void Observer::moveRel3D(const Vec3d &target)
{
	anchor->setHeliocentricEclipticPos(anchor->getHeliocentricEclipticPos() + target/eyeMoveDivisor);
	/*
	Vec3d local_to_altitude(0., 0., getDistanceFromCenter());
	Vec3d relativeRotation = Vec3d(0., 0., getDistanceFromCenter()) + target;
	double targetDistanceFromCenter = relativeRotation.length();
	setDistanceFromCenter(targetDistanceFromCenter);
	relativeRotation /= targetDistanceFromCenter;
	double lon, lat;
	Utility::rectToSphe(&lon, &lat, relativeRotation);
	rotator.setPreRotation(mat_altitude_to_earth_equ.toQuaternion().combineQuaternions(Vec4d::zyrotation(lon, lat)));
	// Should perform some heading compensation, as it might move...
	if (!rotator.moving())
		rotator.getMatrix(); // Update internal cache, as we use getCachedMatrix later-on
	*/
}

void Observer::moveTo(const Vec4d &target, int duration, bool isMaxDuration)
{
	rotator.moveTo(target, duration, isMaxDuration);
}

void Observer::moveRel(const Vec4d &relTarget, int duration, bool isMaxDuration)
{
	rotator.moveRel(relTarget, duration, isMaxDuration);
}

void Observer::setRotation(const Vec4d &target)
{
	rotator.setRotation(target);
}

void Observer::setRelRotation(const Vec4d &relTarget)
{
	rotator.setRelRotation(relTarget);
}

bool Observer::isOnBodyNamed(const std::string& bodyName){

	if(anchor == nullptr)
		return false;

	if(!isOnBody())
		return false;

	std::shared_ptr<Body> b = getHomeBody();

	if(b != nullptr)
		return getHomeBody()->getEnglishName() == bodyName;

	return false;

}

std::string Observer::getHomePlanetEnglishName(void) const
{
	std::shared_ptr<Body> p = getHomeBody();
	return p ? p->getEnglishName() : "";
}


std::string Observer::getHomePlanetNameI18n(void) const
{
	std::shared_ptr<Body> p = getHomeBody();
	return p ? p->getNameI18n() : "";
}

bool Observer::isEarth() const {
	return getHomePlanetEnglishName() == "Earth";
}

bool Observer::isSun() const {
	return getHomePlanetEnglishName() == "Sun";
}

// for moving observer position gradually
void Observer::update(int delta_time)
{
	rotator.update(delta_time, true);
	if (flag_move_to) {
		auto inter_move_to_mult = move_to_mult;
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_move_to = 0;
		}
		inter_move_to_mult = move_to_mult - inter_move_to_mult;

		setLatitude( start_lat - move_to_mult*(start_lat-end_lat) );
		// const double off_lon = longitude - current_lon;
		// current_lon += off_lon;
		// start_lon += off_lon;
		longitude += inter_move_to_mult*rel_lon;
		// current_lon = longitude;
		altitude  = start_alt - move_to_mult*(start_alt-end_alt);
	}
	if (flag_eye_relative_mode) {
		// Update altitude
		setDistanceFromCenter(getDistanceFromCenter());
		// Update longitude and latitude
		auto relPos = anchor->getHeliocentricEclipticPos() - anchorAlt->getHeliocentricEclipticPos();
		relPos = anchorAlt->getRotEquatorialToVsop87().transpose() * relPos;
		Utility::rectToSphe(&longitude, &latitude, relPos);
		longitude *= 180/M_PI;
		latitude *= 180/M_PI;
		if (anchorAlt->isOnBody())
			longitude -= anchorAlt->getBody()->getSiderealTime(CoreLink::instance->getJDay());
	}
}


double Observer::getLongitude() const
{
		return  longitude;
}

double Observer::getLongitudeForDisplay() const
{
	double tmp = longitude;

	tmp -= floor((tmp + 180.) / 360.) * 360.;
	return tmp;
}

void Observer::setQuaternionMode(bool mode)
{
	if (mode != flag_quaternion_mode) {
		flag_quaternion_mode = mode;
		if (mode) {
			// Swap to quaternion mode, nothing to do
		} else {
			auto lonLatRotation = getRotLocalToEquatorial(0).toQuaternion(); // Longitude/latitude quaternion
			longitude = latitude = 0;
			// auto noRotation = getRotLocalToEquatorial(0).toQuaternion(); // Rotation with null longitude/latitude
			auto &quatRotation = rotator.getQuaternion();
			auto fullRotation = quatRotation.combineQuaternions(lonLatRotation); // Current rotation
			// auto baseRotation = quatRotation.combineQuaternions(noRotation); // Offset rotation
			// Compute the latitude/longitude of noRotation
			double offLon, offLat;
			Utility::rectToSphe(&offLon, &offLat, getRotLocalToEquatorial(0) * Vec3d(0, 0, 1));
			Utility::rectToSphe(&longitude, &latitude, Mat4d::fromQuaternion(fullRotation) * Vec3d(0, 0, 1));
			// Compute the latitude/longitude of fullRotation
			// The real rotation can make a difference
			longitude = (longitude - offLon) * 180 / M_PI;
			latitude = (latitude - offLat) * 180 / M_PI;

			// zrotation(longitude) * yrotation(latitude) should give the same position
			// Compensate what remain of the rotation
			setRotation(fullRotation.combineQuaternions(getRotLocalToEquatorial(0).toQuaternion().inverse()));
			// Reinitialize quaterion rotation
			moveTo({1, 0, 0, 0}, 10000, true);
		}
	}
}

void Observer::setEyeRelativeMode(bool mode)
{
	if (flag_eye_relative_mode == mode)
		return;
	if (mode)
		anchorAlt->setHeliocentricEclipticPos(getHeliocentricPosition(CoreLink::instance->getJDay()));
	flag_eye_relative_mode = mode;
	anchor.swap(anchorAlt);
}

void Observer::setAnchorPoint(std::shared_ptr<AnchorPoint> _anchor)
{
	if (flag_eye_relative_mode) {
		anchorAlt = _anchor;
	} else
		anchor = _anchor;
}

// lon = 1.96841976537792
// lat = 46.176495867768431
// sideral = 252.38943488471656
