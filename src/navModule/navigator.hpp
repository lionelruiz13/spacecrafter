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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#ifndef _NAVIGATOR_H_
#define _NAVIGATOR_H_

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"

extern const Mat4d mat_j2000_to_vsop87;
extern const Mat4d mat_vsop87_to_j2000;

class Object;
class Observer;

//! @class Class which manages a navigation context
//! @brief  viewing direction/fov and coordinate changes

class Navigator: public NoCopy {
public:

	enum VIEWING_MODE_TYPE {
		VIEW_HORIZON,
		VIEW_EQUATOR
	};
	//! Create and initialise to default a navigation context
	Navigator(/*Observer* obs*/);
	virtual ~Navigator();

	void updateTransformMatrices(Observer* position, double _JDay);
	void updateVisionVector(int delta_time,const Object &selected);

	//! Update the view matrices
	void updateViewMat(double fov);

	//! Move to the given position in equatorial or local coordinate depending on _local_pos value
	void moveTo(const Vec3d& _aim, float move_duration = 1., bool _local_pos = false, int zooming = 0);

	//! Flags controls
	void setFlagTraking(int v) {
		flag_traking=v;
	}
	int getFlagTraking() const {
		return flag_traking;
	}
	void setFlagLockEquPos(int v) {
		flag_lock_equ_pos=v;
	}
	int getFlagLockEquPos() const {
		return flag_lock_equ_pos;
	}

	//! Get vision direction
	const Vec3d& getEquVision() const { //unused
		return equ_vision;
	}
	//! Get vision direction taking in account the precession of Earth in equatorial mode
	const Vec3d& getPrecEquVision() const {
		return prec_equ_vision;
	}
	const Vec3d& getLocalVision() const { //unused
		return local_vision;
	}

	void setLocalVision(const Vec3d& _pos);

	//! Return the observer heliocentric position
	Vec3d getObserverHelioPos() const;

	//! Transform vector from local coordinate to equatorial
	Vec3d localToEarthEqu(const Vec3d& v) const {
		return mat_local_to_earth_equ*v;
	}

	//! Transform vector from equatorial coordinates to local (it is fixed to the sky)
	Vec3d earthEquToLocal(const Vec3d& v) const {
		return mat_earth_equ_to_local*v;
	}

	//! Transform vector from equatorial coordinates to local fixed to the screen (0ad=center screen Y) (0de=equator)
	Vec3d earthEquToLocalFixed(const Vec3d& v) const {// cyp : une seule apparition
		return mat_earth_equ_to_local_fixed*v;
	}

	//! Transform vector from equatorial (earth centered) coordinates to the sky (ad,de) without precession shifting
	Vec3d earthEquToJ2000(const Vec3d& v) const {
		return mat_earth_equ_to_j2000*v;
	}

	//! Transform vector from equatorial (earth centered) coordinates to the sky (ad,de) without precession shifting
	Vec3d j2000ToEarthEqu(const Vec3d& v) const {
		return mat_j2000_to_earth_equ*v;
	}

	//! Transform vector from heliocentric coordinate to local
	Vec3d helioToLocal(const Vec3d& v) const {// erreur seg
		return mat_helio_to_local*v;
	}

	//! Transform vector from heliocentric coordinate to earth equatorial, only needed in meteor.cpp
	Vec3d helioToEarthEqu(const Vec3d& v) const {
		return mat_helioToEarthEqu*v;
	}

	//! Transform vector from heliocentric coordinate to false equatorial : equatorial coordinate but centered on the observer position (usefull for objects close to earth)
	Vec3d helioToEarthPosEqu(const Vec3d& v) const {
		return mat_local_to_earth_equ*mat_helio_to_local*v;
	}

	//! Return the view matrix for some coordinate systems
	const Mat4d& getHelioToEyeMat() const {
		return mat_helio_to_eye;
	}
	const Mat4d& getEarthEquToEyeMat() const {
		return mat_earth_equ_to_eye;
	}
	const Mat4d& getEarthEquToEyeMatFixed() const {
		return mat_earth_equ_to_eye_fixed;
	}
	const Mat4d& getLocalToEyeMat() const {
		return mat_local_to_eye;
	}
	const Mat4d& getJ2000ToEyeMat() const {
		return mat_j2000_to_eye;
	}
	//! Return fixed dome matrix (no heading adjustment)
	const Mat4d& getDomeFixedMat() const {
		return mat_dome_fixed;
	}
	//! Return dome matrix adjusted for current heading
	const Mat4d& geTdomeMat() const {
		return mat_dome;
	}

	void updateMove(double deltaAz, double deltaAlt, double fov, double duration = 0);

	//! Set type of viewing mode (align with horizon or equatorial coordinates)
	void setViewingMode(VIEWING_MODE_TYPE view_mode);

	VIEWING_MODE_TYPE getViewingMode() const {
		return viewing_mode;
	}

	void setHeading(double _heading) {
		heading = _heading;
	}

	void setDefaultHeading() {
		heading = defaultHeading;
	}

	void setDefaultHeading(double _heading) {
		defaultHeading = _heading;
		while (defaultHeading > 180) defaultHeading -= 360;
		while (defaultHeading < -180) defaultHeading += 360;
	}

	double getHeading() const {
		double h = heading;
		// keep within -180 to 180 for TUI compatibility
		while (h > 180) h -= 360;
		while (h < -180) h += 360;
		return h;
	}

	// NB: always call stel_core method, not this one directly to set so that init view vector gets updated correctly
	void setViewOffset(double _offset) {
		view_offset = _offset;
	}
	double getViewOffset() const {
		return view_offset;
	}

	//! move gradually to a new heading
	void changeHeading(double _heading, int duration);

	//! for moving heading position gradually
	void update(int delta_time);

	bool lookAt(double az, double alt, double time);

	void alignUpVectorTo(const Mat4d& rot, double duration);

private:

	//! Struct used to store data for auto move
	typedef struct {
		Vec3d start;
		Vec3d aim;
		float speed;
		float coef;
		bool local_pos;				//!< Define if the position are in equatorial or altazimutal
	} auto_move;

	float view_offset_transition;   //!< for transitioning to and from using a view offset

	// Matrices used for every coordinate transfo
	Mat4d mat_helio_to_local;		//!< Transform from Heliocentric to Observer local coordinate
	Mat4d mat_local_to_helio;		//!< Transform from Observer local coordinate to Heliocentric
	Mat4d mat_local_to_earth_equ;	//!< Transform from Observer local coordinate to Earth Equatorial
	Mat4d mat_local_to_earth_equ_fixed;	//!< Transform from Observer local coordinate to Earth Equatorial
	Mat4d mat_earth_equ_to_local;	//!< Transform from Observer local coordinate to Earth Equatorial
	Mat4d mat_earth_equ_to_local_fixed;	//!< Transform from Observer local coordinate to Earth Equatorial
	Mat4d mat_helioToEarthEqu;	//!< Transform from Heliocentric to earth equatorial coordinate
	Mat4d mat_equ_to_vsop87;
	Mat4d mat_earth_equ_to_j2000;
	Mat4d mat_j2000_to_earth_equ;

	Mat4d mat_local_to_eye;			//!< {Main matrix} View matrix for observer local drawing

	Mat4d mat_earth_equ_to_eye;		//!< View matrix for geocentric equatorial drawing
	Mat4d mat_earth_equ_to_eye_fixed;		//!< View matrix for geocentric equatorial drawing
	Mat4d mat_j2000_to_eye;			//!< precessed version
	Mat4d mat_helio_to_eye;			//!< View matrix for heliocentric equatorial drawing

	Mat4d mat_dome_fixed;			//!< Dome (fixed alt/az) View matrix
	Mat4d mat_dome; 				//!< Dome (fixed alt/az) View matrix with heading adjustment

	// Vision variables
	Vec3d local_vision, equ_vision, prec_equ_vision;	//!< Viewing direction in local and equatorial coordinates
	Vec3d heading_vector; 			//!< The up vector of the camera in heliocentric coordinates
	int flag_traking;				//!< Define if the selected object is followed
	int flag_lock_equ_pos;			//!< Define if the equatorial position is locked

	// Automove
	auto_move move;					//!< Current auto movement

	int flag_auto_move;				//!< Define if automove is on or off
	int zooming_mode;				//!< 0 : undefined, 1 zooming, -1 unzooming

	VIEWING_MODE_TYPE viewing_mode;   //!< defines if view corrects for horizon, or uses equatorial coordinates

	double view_offset;              //!< To center/zoom away from the center of the viewport
	double heading;                  //!< Rotate the environment around the observer
	double defaultHeading;			 //!< heading comming from config.ini

	// for changing heading
	bool flag_change_heading;
	double start_heading, end_heading;
	float move_to_coef, move_to_mult;
};

#endif //_NAVIGATOR_H_
