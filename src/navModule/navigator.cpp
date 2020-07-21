/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
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

#include "bodyModule/solarsystem.hpp"
//#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "tools/object.hpp"
#include "tools/log.hpp"
//#include "tools/fmath.hpp"




Navigator::Navigator() : view_offset_transition(0), flag_traking(0),
	flag_lock_equ_pos(0), flag_auto_move(0), view_offset(0), heading(0.),
	flag_change_heading(0), start_heading(0), end_heading(0), move_to_mult(0)
{
	local_vision=Vec3d(1.,0.,0.);
	equ_vision=Vec3d(1.,0.,0.);
	prec_equ_vision=Vec3d(1.,0.,0.);  // not correct yet...
	viewing_mode = VIEW_HORIZON;  // default

}


Navigator::~Navigator() {}


void Navigator::updateVisionVector(int delta_time,const Object &selected)
{
	if (flag_auto_move) {
		//double ra_aim, de_aim, ra_start, de_start, ra_now, de_now;

		if ( zooming_mode != -1 && selected) {
			// if zooming in, object may be moving so be sure to zoom to latest position
			move.aim = selected.getEarthEquPos(this);
			move.aim.normalize();
		}
		// Use a smooth function
		float smooth = 4.f;
		double c;

		if (zooming_mode == 1) {
			if ( move.coef > .9 ) c = 1;
			else c = 1 - pow(1.-1.11*(move.coef),3);

		} else if (zooming_mode == -1) {

			c =  move.coef*move.coef*move.coef*move.coef;
		} else c = atanf(smooth * 2.*move.coef-smooth)/atanf(smooth)/2+0.5;

		// view offset
		if (zooming_mode != -1) {
			if (view_offset_transition < 1 ) view_offset_transition = c;
		} else {
			if (view_offset_transition > 0 ) view_offset_transition = 1 - c;
		}

		// Rewrote look transistion code, 201104
		Vec3d v1, v2, v3, v4;

		if (move.local_pos) {
			v1 = move.start;
			v2 = move.aim;
		} else {
			v1 = earthEquToLocal(move.start);
			v2 = earthEquToLocal(move.aim);
		}

		// just in case is not already normalized
		v1.normalize();
		v2.normalize();

		// prevent one situation where doesn't work
		if(v1 == v2 ) v1 = Mat4d::xrotation(0.0001) * v1;

		v4 = v1^v2;
		v3 = v4^v1;  // in plane of v1 and v2 and orthogonal to v1
		v3.normalize();

		double angle = atan2( v4.length(), v1.dot(v2) );

		local_vision = (v1*cos(angle*c) + v3*sin(angle*c));
		equ_vision = localToEarthEqu(local_vision);


		if (move.coef>=1.) {
			flag_auto_move=0;
			if (move.local_pos) {
				local_vision=move.aim;
				equ_vision=localToEarthEqu(local_vision);
			} else {
				equ_vision=move.aim;
				local_vision=earthEquToLocal(equ_vision);
			}
		}

		// Allow full transition to occur
		move.coef+=move.speed*delta_time;
		if (move.coef>1) move.coef = 1;
	} else {
		if (flag_traking && selected) { // Equatorial vision vector locked on selected object
			equ_vision= selected.getEarthEquPos(this);
			local_vision=earthEquToLocal(equ_vision);
		} else {
			if (flag_lock_equ_pos) { // Equatorial vision vector locked
				// Recalc local vision vector
				local_vision=earthEquToLocal(equ_vision);
			} else { // Local vision vector locked
				// Recalc equatorial vision vector
				equ_vision=localToEarthEqu(local_vision);
			}
		}
	}

	prec_equ_vision = mat_earth_equ_to_j2000*equ_vision;
}

bool Navigator::lookAt(double az, double alt, double time){

	Vec3d vision(-1,0,0);
	Vec3d rot(0,-1,0);

	vision = Mat4d::zrotation(- az * M_PI / 180.) * vision;
	rot = Mat4d::zrotation(- az * M_PI / 180.) * rot;

	vision = Mat4d::rotation(rot, alt * M_PI / 180.) * vision;

	moveTo(vision, time, true,0);

	return true;
}

void Navigator::setLocalVision(const Vec3d& _pos)
{
	// Transition vision vector by view offset as needed
	local_vision = Mat4d::yrotation(-view_offset * M_PI_2 * view_offset_transition) * _pos;

	equ_vision=localToEarthEqu(local_vision);
	prec_equ_vision = mat_earth_equ_to_j2000*equ_vision;
}


void Navigator::updateMove(void *projector, double deltaAz, double deltaAlt, double fov, double duration)
{
	double azVision, altVision;
	//~ cout << "deltaAz " << deltaAz << endl << "deltaAlt " << deltaAlt << endl;

	if ( viewing_mode == VIEW_EQUATOR)
		Utility::rectToSphe(&azVision,&altVision,equ_vision);
	else
		Utility::rectToSphe(&azVision,&altVision,local_vision);

	// if we are moving in the Azimuthal angle (left/right)
	if (deltaAz)
		azVision-=deltaAz;
	if (deltaAlt) {
		if (altVision+deltaAlt <= M_PI_2 && altVision+deltaAlt >= -M_PI_2) altVision+=deltaAlt;
		if (altVision+deltaAlt > M_PI_2) altVision = M_PI_2 - 0.000001;		// Prevent bug
		if (altVision+deltaAlt < -M_PI_2) altVision = -M_PI_2 + 0.000001;	// Prevent bug
	}

	// recalc all the position variables
	if (deltaAz || deltaAlt) {
		if ( viewing_mode == VIEW_EQUATOR) {
			Utility::spheToRect(azVision, altVision, equ_vision);
			local_vision=earthEquToLocal(equ_vision);
		} else {
			Utility::spheToRect(azVision, altVision, local_vision);
			// Calc the equatorial coordinate of the direction of vision wich was in Altazimuthal coordinate
			equ_vision=localToEarthEqu(local_vision);
			prec_equ_vision = mat_earth_equ_to_j2000*equ_vision;
		}
	}

	// Update the final view matrices
	updateViewMat(projector, fov);
}

const Mat4d mat_j2000_to_vsop87(
    Mat4d::xrotation(-23.4392803055555555556*(M_PI/180)) *
    Mat4d::zrotation(0.0000275*(M_PI/180)));

const Mat4d mat_vsop87_to_j2000(mat_j2000_to_vsop87.transpose());


void Navigator::updateTransformMatrices(Observer* position, double _JDay)
{
	mat_local_to_earth_equ = position->getRotLocalToEquatorial(_JDay);
	mat_local_to_earth_equ_fixed = position->getRotLocalToEquatorialFixed(_JDay);

	mat_earth_equ_to_local = mat_local_to_earth_equ.transpose();
	mat_earth_equ_to_local_fixed = mat_local_to_earth_equ_fixed.transpose();

	mat_equ_to_vsop87 = position->getRotEquatorialToVsop87();

	mat_earth_equ_to_j2000 = mat_vsop87_to_j2000 * position->getRotEquatorialToVsop87();
	mat_j2000_to_earth_equ = mat_earth_equ_to_j2000.transpose();

	mat_helioToEarthEqu =
	    mat_j2000_to_earth_equ *
	    mat_vsop87_to_j2000 *
	    Mat4d::translation(-position->getObserverCenterPoint());

	// These two next have to take into account the position of the observer on the earth
	Mat4d tmp =
	    mat_j2000_to_vsop87 *
	    mat_earth_equ_to_j2000 *
	    mat_local_to_earth_equ;

	mat_local_to_helio =  Mat4d::translation(position->getObserverCenterPoint()) *
	                      tmp *
	                      Mat4d::translation(Vec3d(0.,0., position->getDistanceFromCenter()));

	mat_helio_to_local =  Mat4d::translation(Vec3d(0.,0.,-position->getDistanceFromCenter())) *
	                      tmp.transpose() *
	                      Mat4d::translation(-position->getObserverCenterPoint());

	mat_dome_fixed.set(0, -1, 0, 0,
	                   -1, 0, 0, 0,
	                   0, 0, -1, 0,
	                   0, 0, 0, 1);

	mat_dome = Mat4d::zrotation(heading*M_PI/180.f) * mat_dome_fixed;
}



// Update the view matrices
void Navigator::updateViewMat(void *projector, double fov)
{
	Vec3d f;

	if ( viewing_mode == VIEW_EQUATOR) {
		// view will use equatorial coordinates, so that north is always up
		f = equ_vision;
	} else {
		// view will correct for horizon (always down)
		f = local_vision;
	}
	f.normalize();

	Vec3d s(f[1],-f[0],0.);

	if ( viewing_mode == VIEW_EQUATOR) {
		// convert everything back to local coord
		f = local_vision;
		f.normalize();
		s = earthEquToLocal( s );
	}

	Vec3d u(s^f);
	s.normalize();
	u.normalize();

	//~ viewBeforeLookAt.set(f,s,u,-mat_helio_to_local.getVector(3));
	//~ viewBeforLookAt.r[15]=1.0;

	mat_local_to_eye.set(s[0],u[0],-f[0],0.,
	                     s[1],u[1],-f[1],0.,
	                     s[2],u[2],-f[2],0.,
	                     0.,0.,0.,1.);

	/**
	 *  /!\ READ THIS
	 *
	 *  The matrix parameter are given column by colum so the actual matrix is :
	 *
	 * 	[ s[0], s[1], s[2], 0 ]
	 * 	[ u[0], u[1], u[2], 0 ]
	 * 	[-f[0],-f[1],-f[2], 0 ]
	 * 	[   0 ,   0 ,   0 , 1 ]
	 *
	 **/

	// redo view offset
	mat_local_to_eye =  Mat4d::xrotation(view_offset *fov/2.f*M_PI/180.f * view_offset_transition) * mat_local_to_eye;

	// cout << "offset : " << view_offset *fov/2.f*M_PI/180.f * view_offset_transition << endl;

	// heading
	mat_local_to_eye =  Mat4d::zrotation(heading*M_PI/180.f + 0.0001) * mat_local_to_eye;

	// cout << "heading : " << heading*M_PI/180.f + 0.0001 << endl;

	heading_vector = Vec3d(0,1,0);

	heading_vector = mat_equ_to_vsop87 * mat_local_to_earth_equ * mat_local_to_eye.transpose() * heading_vector;

	// cout << heading_vector << endl;

	// cout << "local : " << heading_vector << endl;

	mat_earth_equ_to_eye = mat_local_to_eye*mat_earth_equ_to_local;
	mat_earth_equ_to_eye_fixed = mat_local_to_eye*mat_earth_equ_to_local_fixed;
	mat_helio_to_eye = mat_local_to_eye*mat_helio_to_local;
	mat_j2000_to_eye = mat_earth_equ_to_eye*mat_j2000_to_earth_equ;

	heading_vector = mat_local_to_earth_equ * heading_vector;
	heading_vector.normalize();

	// cout << "equat : " << heading_vector << endl;

}


// Return the observer heliocentric position
Vec3d Navigator::getObserverHelioPos(void) const
{
	Vec3d v(0.,0.,0.);
	return mat_local_to_helio*v;
}


// Move to the given equatorial position
void Navigator::moveTo(const Vec3d& _aim, float move_duration, bool _local_pos, int zooming)
{
	Vec3d tmp = _aim;
	tmp.normalize();
	tmp *= 2.;

	// already moving to correct position
	if ( flag_auto_move && fabs(move.aim[0] - tmp[0]) <= .00000001f
	        && fabs(move.aim[1] - tmp[1]) <= .00000001f
	        && fabs(move.aim[2] - tmp[2]) <= .00000001f ) {
		//    cout << "already moving here\n";
		return;
	}

	zooming_mode = zooming;
	move.aim=_aim;
	move.aim.normalize();
	move.aim*=2.;
	if (_local_pos) {
		move.start=local_vision;
	} else {
		move.start=equ_vision;
	}
	move.start.normalize();
	move.speed=1.f/(move_duration*1000);
	move.coef=0.;
	move.local_pos = _local_pos;
	flag_auto_move = true;
}


// Set type of viewing mode (align with horizon or equatorial coordinates)
void Navigator::setViewingMode(VIEWING_MODE_TYPE view_mode)
{
	viewing_mode = view_mode;
}

// move gradually to a new heading
void Navigator::changeHeading(double _heading, int duration)
{
	flag_change_heading = 1;

	start_heading = heading;
	end_heading = _heading;

	move_to_coef = 1.0f/duration;
	move_to_mult = 0;
}

// for moving heading position gradually
void Navigator::update(int delta_time)
{
	if (flag_change_heading) {
		if (delta_time > 100) return;
		//std::cout << "Changing heading from " << start_heading << " to " << end_heading << " delta_time " << delta_time << std::endl;
		move_to_mult += move_to_coef*delta_time;

		if ( move_to_mult >= 1) {
			move_to_mult = 1;
			flag_change_heading = 0;
		}
		heading = start_heading - move_to_mult*(start_heading-end_heading);
	}
}

void Navigator::alignUpVectorTo(const Mat4d& rotlocalToVsop87, double duration){

	/**
	 *
	 * axis doit être le vecteur représentant la direction de l'axe de la planète
	 * on ramène ce vecteur dans le repère de la caméra (eye). Dans le plan de la
	 * caméra (x vers la droite y vers le haut z vers nous) on peut déterminer l'
	 * angle que fait le vecteur par rapport à l'axe y. Cet de cet angle que l'on
	 * doit tourner le heading de la caméra pour que l'axe soit aligné.
	 *
	 * A noter que quand on passe sur la planète notre heading doit être remis à
	 * zéro
	 *
	 *     opp
	 *    _______
	 *    |     ^ axis
	 *    |    /         y
	 *adj |   /          ^
	 *    |__/           |
	 *    |A/            |
	 *    |/            zo----->x
	 *
	 *
	 * Je n'arrive pas à obtenir l'axe de la planète à l'heure actuelle
	 *
	 **/


	Mat4d rotVsopToEye =
		( 	mat_local_to_eye *
			mat_earth_equ_to_local *
			mat_equ_to_vsop87.transpose()
		).transpose();

	Vec3d axis = mat_vsop87_to_j2000 * rotlocalToVsop87 *  Vec3d(0,0,1);

	axis = rotVsopToEye * axis;

	std::cout << "axis : " << axis << std::endl;

	double opp = axis[0];
	double adj = axis[1];
	double hyp = sqrt(opp*opp + adj*adj);

	double angle = acos(adj/hyp);

	if(opp < 0){
		angle = -angle;
	}

	std::cout << angle * 180/M_PI << std::endl;

	changeHeading(angle * 180/M_PI, (int)(duration*1000));
}
