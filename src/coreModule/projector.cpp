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



#include <iostream>
#include <cstdio>
#include <ostream>
#include <iomanip>
#include "coreModule/projector.hpp"
#include "tools/s_font.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "tools/context.hpp"

#include <fcntl.h>
//#include "tools/fmath.hpp"




Projector::Projector(const int width, const int height, double _fov)
	:fov(1.0), min_fov(0.0001), max_fov(350),
	 zNear(0.1), zFar(10000),
	 flag_auto_zoom(0)
{
	viewport_radius = -1;  // unset value DIGITALIS
	viewport_radius_squared = 1;
	//~ flip_horz = 1.0;
	//~ flip_vert = 1.0;

	viewport_fov_diameter = std::min(width, height);
	setViewportDisk(width, height);
	setFov(_fov);

	mat_projection.set(1., 0., 0., 0.,
	                   0., 1., 0., 0.,
	                   0., 0., -1, 0.,
	                   0., 0., 0., 1.);
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
	vec_viewport[2] = t;
	vec_viewport[3] = t;

	viewport_fov_diameter = t;
	viewport_center.set(w/2, h/2,0);
	viewport_radius = viewport_fov_diameter/2;
	viewport_radius_squared = viewport_radius * viewport_radius;

	//glViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);
	this->applyViewport();

	// std::cout << "CALLED DISK set viewport\n\n";
	// std::cout << "viewport " << vec_viewport[0] << " " << vec_viewport[1] << " " << vec_viewport[2] << " " << vec_viewport[3] << std::endl;
	// std::cout << "set center " << viewport_center[0] << " with offset " << viewport_center[1] << std::endl;
}


void Projector::setFov(double f)
{
	if (f>max_fov) f = max_fov;
	if (f<min_fov) f = min_fov;

	fov = f;
	fisheye_scale_factor = 1.0/fov*180./M_PI*2;
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
	if ((fov < 90) /*&& fov < 360.0*/) {
		Vec3d e0,e1,e2,e3;
		// if (fov >= 120.0) {
		// 	unprojectJ2000Normalized(0.f, 0.f, e0);
		// 	StelGeom::ConvexS rval(1);
		// 	rval[0].n = e0;
		// 	rval[0].d = (fov<360.0) ? cos(fov*(M_PI/360.0)) : -1.0;
		// 	return rval;
		// }
		unprojectJ2000Normalized(-1.f, -1.f, e0);
		unprojectJ2000Normalized(+1.f, +1.f, e2);
		unprojectJ2000Normalized(-1.f, +1.f, e1);
		unprojectJ2000Normalized(+1.f, -1.f, e3);

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
			unprojectJ2000Normalized(0.f, 0.f, middle);
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


// ================================ FISHEYE PROJECTION =================================
bool Projector::fisheyeProjectCustom(const Vec3d &v, Vec3d &win, const Mat4d &mat) const
{
	// Apply ModelView transformation
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double depth = win.length();
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	// Handle degenerate case (looking at center)
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

	const double rq = sqrt(rq1);

	// Calculate angle: asin(min(rq/depth, 1))
	double f = asin(std::min(rq/depth, 1.0));
	if (win[2] > 0)
		f = M_PI - f;

	// Standard fisheye projection
	f /= rq * (fov * (M_PI/360.0));
	f *= viewport_radius;

	// Centered coordinates on the viewport
	double dx = win[0] * f;
	double dy = win[1] * f;

	// Check if inside the viewport circle
	bool visible = (dx*dx + dy*dy <= viewport_radius_squared);

	// Final projection
	win[0] = viewport_center[0] + dx;
	win[1] = viewport_center[1] + dy;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	// return (f < 0.97*M_PI*viewport_radius/(fov*(M_PI/360.0))) ? true : false;
	return visible;
}

// ================================ ALLSPHERE PROJECTION =================================
bool Projector::allsphereProjectCustom(const Vec3d &v, Vec3d &win, const Mat4d &mat) const
{
	// Apply ModelView transformation
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double depth = win.length();
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	// Handle degenerate case (looking at center)
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

	const double rq = sqrt(rq1);

	// Calculate angle: asin(min(rq/depth, 1))
	double f = asin(std::min(rq/depth, 1.0));
	if (win[2] > 0)
		f = M_PI - f;

	// Allsphere distortion - high precision polynomial
	// Normalize angle by FOV BEFORE polynomial
	f = (f / (fov * (M_PI/360.0))) * 1200.0;
	f = (((((((((-1.553958085e-26*f + 1.430207232e-22)*f -4.958391394e-19)*f + 8.938737084e-16)*f -9.39081162e-13)*f + 5.979121144e-10)*f -2.293161246e-7)*f + 4.995598119e-5)*f -5.508786926e-3)*f + 1.665135788)*f + 6.526610628e-2;
	f = f / 1200.0;

	// ALLSPHERE: divide only by rq, not by fov (already normalized)
	f /= rq;
	f *= viewport_radius;

	// Centered coordinates on the viewport
	double dx = win[0] * f;
	double dy = win[1] * f;

	// Check if inside the viewport circle
	bool visible = (dx*dx + dy*dy <= viewport_radius_squared);

	// Final projection
	win[0] = viewport_center[0] + dx;
	win[1] = viewport_center[1] + dy;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	// return (f < 0.97*M_PI*viewport_radius/(fov*(M_PI/360.0))) ? true : false;
	return visible;
}

// ================================ EKISOLID PROJECTION =================================
bool Projector::ekisolidProjectCustom(const Vec3d &v, Vec3d &win, const Mat4d &mat) const
{
	// TODO: implement proper EKISOLID formula
	// For now, use FISHEYE
	return fisheyeProjectCustom(v, win, mat);
}

// ================================ ASPHERIC PROJECTION =================================
bool Projector::asphericProjectCustom(const Vec3d &v, Vec3d &win, const Mat4d &mat) const
{
	// Apply ModelView transformation
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double depth = win.length();
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	// Handle degenerate case (looking at center)
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

	const double rq = sqrt(rq1);

	// Calculate angle: asin(min(rq/depth, 1))
	double f = asin(std::min(rq/depth, 1.0));
	if (win[2] > 0)
		f = M_PI - f;

	// halfFov = FOV / 2 in rad
	double halfFov = fov * (M_PI/360.);
	const double tanHalfFovOver2 = tan(halfFov * 0.5);
	// Aspheric projection: r = tan(α/2) / tan(FOV/2)
	f = tan(f * 0.5) / tanHalfFovOver2;

	// Scale factor -> r = f * viewport_radius
	f = (f * viewport_radius) / rq;

	// Centered coordinates on the viewport
	double dx = win[0] * f;
	double dy = win[1] * f;

	// Check if inside the viewport circle
	bool visible = (dx*dx + dy*dy <= viewport_radius_squared);

	// Final projection
	win[0] = viewport_center[0] + dx;
	win[1] = viewport_center[1] + dy;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	// return (f < 0.97*M_PI*viewport_radius/(fov*(M_PI/360.0))) ? true : false;
	return visible;
}

// ================================ DISPATCHER =================================
bool Projector::projectCustom(const Vec3d &v, Vec3d &win, const Mat4d &mat) const
{
	switch(Context::projectionType) {
		case 1: return allsphereProjectCustom(v, win, mat);
		case 2: return ekisolidProjectCustom(v, win, mat);
		case 3: return asphericProjectCustom(v, win, mat);
		default: return fisheyeProjectCustom(v, win, mat);
	}
}

// Helper function to invert ALLSPHERE polynomial distortion using Newton-Raphson
static double invertAllspherePolynomial(double f_distorted) {
	// We need to solve: polynomial(f * 1200) / 1200 = f_distorted
	// Using Newton-Raphson iteration: f_new = f_old - (P(f_old) - target) / P'(f_old)

	double f = f_distorted; // Initial guess
	const int max_iterations = 10;
	const double tolerance = 1e-10;

	for (int i = 0; i < max_iterations; i++) {
		double x = f * 1200.0;

		// Evaluate polynomial P(x)
		double p = (((((((((-1.553958085e-26*x + 1.430207232e-22)*x -4.958391394e-19)*x + 8.938737084e-16)*x -9.39081162e-13)*x + 5.979121144e-10)*x -2.293161246e-7)*x + 4.995598119e-5)*x -5.508786926e-3)*x + 1.665135788)*x + 6.526610628e-2;
		p = p / 1200.0;

		// Evaluate derivative P'(x) * 1200 (chain rule)
		double dp = ((((((((-1.553958085e-26*10*x + 1.430207232e-22*9)*x -4.958391394e-19*8)*x + 8.938737084e-16*7)*x -9.39081162e-13*6)*x + 5.979121144e-10*5)*x -2.293161246e-7*4)*x + 4.995598119e-5*3)*x -5.508786926e-3*2)*x + 1.665135788;

		double error = p - f_distorted;
		if (fabs(error) < tolerance)
			return f;

		f = f - error / dp;
	}

	return f;
}

// ================================ FISHEYE UNPROJECT =================================
void Projector::fisheyeUnproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	const auto pos = VulkanMgr::instance->screenToRect({x, y});
	double length = sqrt(pos.first*pos.first + pos.second*pos.second);
	const double angle_center = length * fov * (M_PI/360.);
	const double r = sin(angle_center);

	if (length) {
		length = r / length;
		v.set(pos.first * length, -pos.second * length, sqrt(1.-r*r));
	} else {
		v.set(0, 0, 1);
	}

	if (angle_center>M_PI_2)
		v[2] = -v[2];

	v.transfo4d(m);
}

// ================================ ALLSPHERE UNPROJECT =================================
void Projector::allsphereUnproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	const auto pos = VulkanMgr::instance->screenToRect({x, y});
	double length = sqrt(pos.first*pos.first + pos.second*pos.second);

	// Apply inverse ALLSPHERE distortion
	// In projection: angle_normalized = (angle / fov) -> polynomial -> result
	// In unprojection: we have length which represents the projected radius
	// We need to: invert polynomial -> multiply by fov to get angle
	const double halfFov = fov * (M_PI/360.);
	double f_normalized = invertAllspherePolynomial(length);  // Invert polynomial
	const double angle_center = f_normalized * halfFov;  // Multiply by fov to get angle
	const double r = sin(angle_center);

	if (length) {
		length = r / length;
		v.set(pos.first * length, -pos.second * length, sqrt(1.-r*r));
	} else {
		v.set(0, 0, 1);
	}

	if (angle_center>M_PI_2)
		v[2] = -v[2];

	v.transfo4d(m);
}

// ================================ EKISOLID UNPROJECT =================================
void Projector::ekisolidUnproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	// TODO: implement proper EKISOLID formula
	// For now, use FISHEYE
	fisheyeUnproject(x, y, m, v);
}

// ================================ ASPHERIC UNPROJECT =================================
void Projector::asphericUnproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	const auto pos = VulkanMgr::instance->screenToRect({x, y});
	double length = sqrt(pos.first*pos.first + pos.second*pos.second);

	// Inverse aspheric projection: from r = tan(α/2) / tan(FOV/2), get α = 2 * atan(r * tan(FOV/2))
	const double halfFov = fov * (M_PI/360.);
	const double tanHalfFovOver2 = tan(halfFov * 0.5);
	const double angle_center = 2.0 * atan(length * tanHalfFovOver2);
	const double r = sin(angle_center);

	if (length) {
		length = r / length;
		v.set(pos.first * length, -pos.second * length, sqrt(1.-r*r));
	} else {
		v.set(0, 0, 1);
	}

	if (angle_center>M_PI_2)
		v[2] = -v[2];

	v.transfo4d(m);
}

// ================================ FISHEYE UNPROJECT NORMALIZED =================================
void Projector::fisheyeUnprojectNormalized(double x, double y, const Mat4d& m, Vec3d& v) const
{
	double length = sqrt(x*x + y*y);
	const double angle_center = length * fov * (M_PI/360.);
	const double r = sin(angle_center);

	if (length) {
		length = r / length;
		v.set(x * length, y * length, sqrt(1.-r*r));
	} else {
		v.set(0, 0, 1);
	}

	if (angle_center>M_PI_2)
		v[2] = -v[2];

	v.transfo4d(m);
}

// ================================ ALLSPHERE UNPROJECT NORMALIZED =================================
void Projector::allsphereUnprojectNormalized(double x, double y, const Mat4d& m, Vec3d& v) const
{
	double length = sqrt(x*x + y*y);

	// Apply inverse ALLSPHERE distortion
	// In projection: angle_normalized = (angle / fov) -> polynomial -> result
	// In unprojection: we have length which represents the projected radius
	// We need to: invert polynomial -> multiply by fov to get angle
	const double halfFov = fov * (M_PI/360.);
	double f_normalized = invertAllspherePolynomial(length);  // Invert polynomial
	const double angle_center = f_normalized * halfFov;  // Multiply by fov to get angle
	const double r = sin(angle_center);

	if (length) {
		length = r / length;
		v.set(x * length, y * length, sqrt(1.-r*r));
	} else {
		v.set(0, 0, 1);
	}

	if (angle_center>M_PI_2)
		v[2] = -v[2];

	v.transfo4d(m);
}

// ================================ EKISOLID UNPROJECT NORMALIZED =================================
void Projector::ekisolidUnprojectNormalized(double x, double y, const Mat4d& m, Vec3d& v) const
{
	// TODO: implement proper EKISOLID formula
	// For now, use FISHEYE
	fisheyeUnprojectNormalized(x, y, m, v);
}

// ================================ ASPHERIC UNPROJECT NORMALIZED =================================
void Projector::asphericUnprojectNormalized(double x, double y, const Mat4d& m, Vec3d& v) const
{
	double length = sqrt(x*x + y*y);

	// Inverse aspheric projection: from r = tan(α/2) / tan(FOV/2), get α = 2 * atan(r * tan(FOV/2))
	const double halfFov = fov * (M_PI/360.);
	const double tanHalfFovOver2 = tan(halfFov * 0.5);
	const double angle_center = 2.0 * atan(length * tanHalfFovOver2);
	const double r = sin(angle_center);

	if (length) {
		length = r / length;
		v.set(x * length, y * length, sqrt(1.-r*r));
	} else {
		v.set(0, 0, 1);
	}

	if (angle_center>M_PI_2)
		v[2] = -v[2];

	v.transfo4d(m);
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

	const double a = M_PI_2 + atan(win[2]*oneoverh);

	// TODO this is not exact, should use init fov
	double f = a / M_PI_2;

	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*M_PI) ? true : false;

}

// ================================ UNPROJECT DISPATCHER =================================
void Projector::unproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	// Invert x for rear projection here, this unproject is used for mouse picking
	if (Context::rearProjection) {
		// Invert x screen coordinate [0, viewport_width] for rear projection
		x = 2.0 * viewport_center[0] - x;
	}
	switch(Context::projectionType) {
		case 1: allsphereUnproject(x, y, m, v); break;
		case 2: ekisolidUnproject(x, y, m, v); break;
		case 3: asphericUnproject(x, y, m, v); break;
		default: fisheyeUnproject(x, y, m, v); break;
	}
}

// ================================ UNPROJECT NORMALIZED DISPATCHER =================================
void Projector::unprojectNormalized(double x, double y, const Mat4d& m, Vec3d& v) const
{
	// Never invert x for rear projection here, this unproject is used to render things like
	// sky textures that should not be inverted (it's inverted by the vulkan blit ("global" inversion))
	switch(Context::projectionType) {
		case 1: allsphereUnprojectNormalized(x, y, m, v); break;
		case 2: ekisolidUnprojectNormalized(x, y, m, v); break;
		case 3: asphericUnprojectNormalized(x, y, m, v); break;
		default: fisheyeUnprojectNormalized(x, y, m, v); break;
	}
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
	//gluUnProject(win[0],win[1],win[2],mat,mat_projection,vec_viewport,&v[0],&v[1],&v[2]);
	//~ cout << "out " << v[0] << " " << v[1] << " " << v[2] << endl;
	//glVertex3dv(v);
}

Vec3d Projector::sVertex3v(double x, double y, double z, const Mat4d& mat) const
{
	Vec3d win;
	Vec3d v(x,y,z);
	projectCustom(v, win, mat);

	// Can be optimized by avoiding matrix inversion if it's always the same
	//gluUnProject(win[0],win[1],win[2],mat,mat_projection,vec_viewport,&v[0],&v[1],&v[2]);
	// cout << "out " << v[0] << " " << v[1] << " " << v[2] << endl;
	return v;
}

void Projector::printGravity180(s_font* font, float x, float y, const std::string& str, Vec4f Color, float xshift, float yshift) const
{
	static float dx, dy, d, theta;

	// ASSUME disk viewport
	dx = x - viewport_center[0];
	dy = y - viewport_center[1];
	d = sqrt(dx*dx + dy*dy);

	// If the text is too far away to be visible in the screen return
	if (d>viewport_radius + font->getStrLen(str))
		return;

	theta = M_PI + atan2f(dx, dy - 1);

	Mat4f MVP = getMatProjectionOrtho2D();
	Mat4f TRANSFO = Mat4f::translation( Vec3f(x,y,0) );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -theta);
	TRANSFO = TRANSFO*Mat4f::translation( Vec3f(xshift, -yshift, 0) );
	TRANSFO = TRANSFO*Mat4f::scaling( Vec3f(1, -1, 1) );

	font->print(0, 0, str, Color, MVP*TRANSFO ,0);
}

// ---------------------------------------------------------------------------
// READBACK ONLY (INTENT §5.63 / §11.130). See the header for what it is for.
// Const, side-effect-free, called only from the dump channel.
// ---------------------------------------------------------------------------
void Projector::dumpTrace(std::ostream &out) const
{
	const auto prec = out.precision();
	out << std::setprecision(17);
	out << "{\"fov\":" << fov
	    << ",\"aimFov\":" << getAimFov()
	    << ",\"minFov\":" << min_fov
	    << ",\"maxFov\":" << max_fov
	    << ",\"flagAutoZoom\":" << (flag_auto_zoom ? "true" : "false")
	    << ",\"zoomAim\":" << zoom_move.aim
	    << ",\"zoomCoef\":" << zoom_move.coef
	    << ",\"viewport\":[" << vec_viewport[0] << ',' << vec_viewport[1] << ','
	    << vec_viewport[2] << ',' << vec_viewport[3] << ']'
	    << ",\"viewportCenter\":[" << viewport_center[0] << ',' << viewport_center[1]
	    << ',' << viewport_center[2] << ']'
	    << ",\"viewportRadius\":" << viewport_radius
	    << ",\"viewportFovDiameter\":" << viewport_fov_diameter
	    << ",\"fisheyeScaleFactor\":" << fisheye_scale_factor
	    << ",\"projectionType\":\"" << projectionTypeToString(static_cast<ProjectionType>(Context::projectionType)) << '"';
	// The geodesic-zone SELECTOR itself, exactly as HipStarMgr::preDraw asks
	// for it: a convex region of half-spaces in J2000. Two runs that differ
	// here draw DIFFERENT STARS, which is a disjoint lit set rather than a
	// displaced one — the shape §5.63's own correction reports.
	const StelGeom::ConvexS cv = unprojectViewport();
	out << ",\"unprojectViewport\":[";
	for (size_t i = 0; i < cv.size(); ++i) {
		out << "{\"n\":[" << cv[i].n[0] << ',' << cv[i].n[1] << ',' << cv[i].n[2]
		    << "],\"d\":" << cv[i].d << '}' << ((i + 1 < cv.size()) ? "," : "");
	}
	out << "]}";
	out.precision(prec);
}
