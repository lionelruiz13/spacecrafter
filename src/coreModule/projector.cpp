/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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

#include <GL/glew.h>

#include <iostream>
#include <cstdio>
#include "coreModule/projector.hpp"
#include "tools/s_font.hpp"

#include <unistd.h>
#include <fcntl.h>
#include "tools/fmath.hpp"




Projector::Projector(const int width, const int height, double _fov)
	:fov(1.0), min_fov(0.0001), max_fov(100),
	 zNear(0.1), zFar(10000),
	 flag_auto_zoom(0) 
{
	viewport_radius = -1;  // unset value DIGITALIS
	//~ flip_horz = 1.0;
	//~ flip_vert = 1.0;

	viewport_fov_diameter = std::min(width, height);
	setViewportDisk(width, height);
	setFov(_fov);

	mat_projection.set(1., 0., 0., 0.,
	                   0., 1., 0., 0.,
	                   0., 0., -1, 0.,
	                   0., 0., 0., 1.);
	min_fov = 0.0001;
	max_fov = 359.9;
	setFov(_fov);
}

Projector::~Projector()
{
}

void Projector::setViewportDisk( int w, int h)
{
	int t = std::min(w, h);
	vec_viewport[0] = (w-t)/2;
	vec_viewport[1] = (h-t)/2;
	vec_viewport[2] = h;
	vec_viewport[3] = h;

	viewport_fov_diameter = std::min(w,h);
	viewport_center.set(w/2, h/2,0);
	viewport_radius = viewport_fov_diameter/2;

	glViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);

	// std::cout << "CALLED DISK set viewport\n\n";
	// std::cout << "viewport " << vec_viewport[0] << " " << vec_viewport[1] << " " << vec_viewport[2] << " " << vec_viewport[3] << std::endl;
	// std::cout << "set center " << viewport_center[0] << " with offset " << viewport_center[1] << std::endl;	
}


void Projector::setFov(double f)
{
	if (f>max_fov) f = max_fov;
	if (f<min_fov) f = min_fov;

	fov = f;
	fisheye_scale_factor = 1.0/fov*180./C_PI*2;
}


void Projector::setMaxFov(double max)
{
	if (fov > max) setFov(max);
	max_fov = max;
}


StelGeom::ConvexS Projector::unprojectViewport(void) const
{
	// This is quite ugly, but already better than nothing.
	// Last not least all halfplanes n*x>d really should have d<=0
	// or at least very small d/n.length().
	if ((fov < 90) && fov < 360.0) {
		Vec3d e0,e1,e2,e3;
		bool ok;
		if (fov >= 120.0) {
			unprojectJ2000(viewport_center[0],viewport_center[1],e0);
			StelGeom::ConvexS rval(1);
			rval[0].n = e0;
			rval[0].d = (fov<360.0) ? cos(fov*(C_PI/360.0)) : -1.0;
			return rval;
		}
		ok  = unprojectJ2000(viewport_center[0] - 0.5*viewport_fov_diameter, viewport_center[1] - 0.5*viewport_fov_diameter,e0);
		ok &= unprojectJ2000(viewport_center[0] + 0.5*viewport_fov_diameter, viewport_center[1] + 0.5*viewport_fov_diameter,e2);
		ok &= unprojectJ2000(viewport_center[0] - 0.5*viewport_fov_diameter, viewport_center[1] + 0.5*viewport_fov_diameter,e1);
		ok &= unprojectJ2000(viewport_center[0] + 0.5*viewport_fov_diameter, viewport_center[1] - 0.5*viewport_fov_diameter,e3);

		if (ok) {
			StelGeom::HalfSpace h0(e0^e1);
			StelGeom::HalfSpace h1(e1^e2);
			StelGeom::HalfSpace h2(e2^e3);
			StelGeom::HalfSpace h3(e3^e0);
			if (h0.contains(e2) && h0.contains(e3) &&
			        h1.contains(e3) && h1.contains(e0) &&
			        h2.contains(e0) && h2.contains(e1) &&
			        h3.contains(e1) && h3.contains(e2)) {
				StelGeom::ConvexS rval(4);
				rval[0] = h0;
				rval[1] = h1;
				rval[2] = h2;
				rval[3] = h3;
				return rval;
			} else {
				Vec3d middle;
				if (unprojectJ2000(vec_viewport[0]+0.5*vec_viewport[2], vec_viewport[1]+0.5*vec_viewport[3],middle)) {
					double d = middle*e0;
					double h = middle*e1;
					if (d > h) d = h;
					h = middle*e2;
					if (d > h) d = h;
					h = middle*e3;
					if (d > h) d = h;
					StelGeom::ConvexS rval(1);
					rval[0].n = middle;
					rval[0].d = d;
					return rval;
				}
			}
		}
	}
	StelGeom::ConvexS rval(1);
	rval[0].n = Vec3d(1.0,0.0,0.0);
	rval[0].d = -2.0;
	return rval;
}

void Projector::setClippingPlanes(double znear, double zfar)
{
	zNear = znear;
	zFar = zfar;
}


bool Projector::checkInViewport(const Vec3d& pos) const
{
	return 	(pos[1]>vec_viewport[1] && pos[1]<(vec_viewport[1] + vec_viewport[3]) &&
	         pos[0]>vec_viewport[0] && pos[0]<(vec_viewport[0] + vec_viewport[2]));
}

// to support large object check
bool Projector::checkInMask(const Vec3d& pos, const int object_pixel_radius) const
{
	float radius = sqrt( powf(pos[0]-viewport_center[0], 2) + powf(pos[1]-viewport_center[1], 2));
	return 	(radius - 1.75 * object_pixel_radius <= viewport_radius);  // 1.75 is safety factor
}



void Projector::changeFov(double deltaFov)
{
	if (deltaFov) setFov(fov+deltaFov);
}


bool Projector::projectCustom(const Vec3d &v,Vec3d &win, const Mat4d &mat) const
{
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double depth = win.length();
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	if (rq1 <= 0 ) {
		if (win[2] < 0.0) {
			win[0] = viewport_center[0];
			win[1] = viewport_center[1];
			win[2] = 1.0;
			return true;
		}
		win[0] = viewport_center[0];
		win[1] = viewport_center[1];
		win[2] = -1e99;
		return false;
	}

	const double oneoverh = 1.0/sqrt(rq1);

	const double a = C_PI_2 + atan(win[2]*oneoverh);

	double f = a * fisheye_scale_factor;
	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*C_PI) ? true : false;

}

bool Projector::projectCustomFixedFov(const Vec3d &v,Vec3d &win, const Mat4d &mat) const
{
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	const double depth = win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	if (rq1 <= 0 ) {
		if (win[2] < 0.0) {
			win[0] = viewport_center[0];
			win[1] = viewport_center[1];
			win[2] = 1.0;
			return true;
		}
		win[0] = viewport_center[0];
		win[1] = viewport_center[1];
		win[2] = -1e99;
		return false;
	}

	const double oneoverh = 1.0/sqrt(rq1);

	const double a = C_PI_2 + atan(win[2]*oneoverh);

	// TODO this is not exact, should use init fov
	double f = a / C_PI_2;

	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*C_PI) ? true : false;

}


void Projector::unproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	double d = getViewportRadius();

	//	printf("unproject x,y: %f, %f   cx,cy: %f, %f\n", x, y, center[0], center[1]);
	v[0] = (x - viewport_center[0]); //shear_horz;
	v[1] = y - viewport_center[1];
	v[2] = 0;

	double length = v.length()/d;

	//  printf("viewport radius = %f, length = %f \n", d, length);

	double angle_center = length * fov/2*C_PI/180;
	double r = sin(angle_center);

	if (length!=0) {
		v.normalize();
		v*= r;
		v[2] = sqrt(1.-(v[0]*v[0]+v[1]*v[1]));
	} else {
		v.set(0.,0.,1.);
	}

	if (angle_center>C_PI_2) v[2] = -v[2];

	v.transfo4d(m);
}



// Set the standard modelview matrices used for projection
void Projector::setModelViewMatrices(	const Mat4d& _mat_earth_equ_to_eye,
                                        const Mat4d& _mat_earth_equ_to_eye_fixed,
                                        const Mat4d& _mat_helio_to_eye,
                                        const Mat4d& _mat_local_to_eye,
                                        const Mat4d& _mat_j2000_to_eye,
                                        const Mat4d& _mat_dome,
                                        const Mat4d& _mat_dome_fixed)
{
	mat_earth_equ_to_eye = _mat_earth_equ_to_eye;
	mat_earth_equ_to_eye_fixed = _mat_earth_equ_to_eye_fixed;
	mat_j2000_to_eye = _mat_j2000_to_eye;
	mat_helio_to_eye = _mat_helio_to_eye;
	mat_local_to_eye = _mat_local_to_eye;
	mat_dome = _mat_dome;
	mat_dome_fixed = _mat_dome_fixed;

	inv_mat_earth_equ_to_eye = (mat_projection*mat_earth_equ_to_eye).fastInverse();
	inv_mat_earth_equ_to_eye_fixed = (mat_projection*mat_earth_equ_to_eye).fastInverse();
	inv_mat_j2000_to_eye = (mat_projection*mat_j2000_to_eye).fastInverse();
	inv_mat_helio_to_eye = (mat_projection*mat_helio_to_eye).fastInverse();
	inv_mat_local_to_eye = (mat_projection*mat_local_to_eye).fastInverse();
	inv_mat_dome = (mat_projection*mat_dome).fastInverse();
	inv_mat_dome_fixed = (mat_projection*mat_dome_fixed).fastInverse();
}

// Update auto_zoom if activated
void Projector::updateAutoZoom(int delta_time, bool manual_zoom)
{
	if (flag_auto_zoom) {
		// Use a smooth function
		double c;

// - manual zoom out (semi auto actually) requires fast at start to be smooth
		if ( manual_zoom || zoom_move.start > zoom_move.aim ) {
			// slow down as approach final view
			c = 1 - (1-zoom_move.coef)*(1-zoom_move.coef)*(1-zoom_move.coef);
		} else {
			// speed up as leave zoom target
			c = (zoom_move.coef)*(zoom_move.coef)*(zoom_move.coef);
		}

		setFov(zoom_move.start + (zoom_move.aim - zoom_move.start) * c);
		zoom_move.coef+=zoom_move.speed*delta_time;
		if (zoom_move.coef>=1.) {
			flag_auto_zoom = 0;
			setFov(zoom_move.aim);
		}
	}
}

// Zoom to the given field of view
void Projector::zoomTo(double aim_fov, float move_duration)
{

	if ( flag_auto_zoom && fabs(zoom_move.aim - aim_fov) <= .0000001f ) {
		//    cout << "Already zooming here\n";
		return;  // already zooming to this fov!
	}

	zoom_move.aim=aim_fov;
	zoom_move.start=fov;
	zoom_move.speed=1.f/(move_duration*1000);
	zoom_move.coef=0.;
	flag_auto_zoom = true;
}

//! Override glVertex3f
//! Here is the main trick for texturing in fisheye mode : The trick is to compute the
//! new coordinate in orthographic projection which will simulate the fisheye projection.
void Projector::sVertex3(double x, double y, double z, const Mat4d& mat) const
{
	Vec3d win;
	Vec3d v(x,y,z);
	projectCustom(v, win, mat);

	// Can be optimized by avoiding matrix inversion if it's always the same
	gluUnProject(win[0],win[1],win[2],mat,mat_projection,vec_viewport,&v[0],&v[1],&v[2]);
	//~ cout << "out " << v[0] << " " << v[1] << " " << v[2] << endl;
	glVertex3dv(v);
}

Vec3d Projector::sVertex3v(double x, double y, double z, const Mat4d& mat) const
{
	Vec3d win;
	Vec3d v(x,y,z);
	projectCustom(v, win, mat);

	// Can be optimized by avoiding matrix inversion if it's always the same
	gluUnProject(win[0],win[1],win[2],mat,mat_projection,vec_viewport,&v[0],&v[1],&v[2]);
	// cout << "out " << v[0] << " " << v[1] << " " << v[2] << endl;
	return v;
}

void Projector::printGravity180(s_font* font, float x, float y, const std::string& str, Vec4f Color, bool speed_optimize, float xshift, float yshift) const
{
	static float dx, dy, d, theta, psi;

	// ASSUME disk viewport
	dx = x - viewport_center[0];
	dy = y - viewport_center[1];
	d = sqrt(dx*dx + dy*dy);

	// If the text is too far away to be visible in the screen return
	if (d>myMax(vec_viewport[3], vec_viewport[2])*2) return;


	theta = C_PI + atan2f(dx, dy - 1);
	psi = atan2f((float)font->getStrLen(str)/str.length(),d + 1) * 180./C_PI;

	if (psi>5) psi = 5;

	Mat4f MVP = getMatProjectionOrtho2D();
	Mat4f TRANSFO = Mat4f::translation( Vec3f(x,y,0) );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -theta);
	TRANSFO = TRANSFO*Mat4f::translation( Vec3f(xshift, -yshift, 0) );
	TRANSFO = TRANSFO*Mat4f::scaling( Vec3f(1, -1, 1) );

	font->print(0, 0, str, Color, MVP*TRANSFO ,0, speed_optimize);  // ASSUME speed optimized strings should be cached
}

