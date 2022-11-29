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

#ifndef _PROJECTOR_H_
#define _PROJECTOR_H_


#include "tools/vecmath.hpp"
#include "starModule/sphere_geometry.hpp"
//#include "tools/fmath.hpp"
#include "tools/no_copy.hpp"


class s_font;

// Class which handle projection modes and projection matrix
// Overide some function usually handled by glu
class Projector: public NoCopy  {
public:
	Projector(const int width, const int height, double _fov = 60.);
	~Projector();

	//! Get and set to define and get viewport size
	Vec3d getViewportCenter(void) const {
		return Vec3d(viewport_center[0], viewport_center[1], viewport_radius);
	}
	Vec3f getViewportFloatCenter(void) const {
		return Vec3f((float) viewport_center[0], (float) viewport_center[1], (float) viewport_radius);
	}

	double getViewportRadius(void) const {
		return viewport_radius;
	}

	void setViewportDisk(int w, int h);

	int getViewportPosX(void) const {
		return vec_viewport[0];
	}
	int getViewportPosY(void) const {
		return vec_viewport[1];
	}
	int getViewportWidth(void) const {
		return vec_viewport[2];
	}
	int getViewportHeight(void) const {
		return vec_viewport[3];
	}
	const Vec4i& getViewport(void) const {
		return vec_viewport;
	}

	//! Set the current openGL viewport to projector's viewport
	void applyViewport(void) const {
		// glViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);
		//Renderer::viewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);
	}

	//! Set the Field of View in degree
	void setFov(double f);
	//! Get the Field of View in degree
	double getFov(void) const {
		return fov;
	}

	//! Set the maximum Field of View in degree
	void setMaxFov(double max);
	//! Get the maximum Field of View in degree
	double getMaxFov(void) const {
		return max_fov;
	}

	//! If is currently zooming, return the target FOV, otherwise return current FOV
	double getAimFov(void) const {
		return (flag_auto_zoom ? zoom_move.aim : fov);
	}

	void changeFov(double deltaFov);

	// Update auto_zoom if activated
	void updateAutoZoom(int delta_time, bool manual_zoom = 0);

	// Zoom to the given field of view in degree
	void zoomTo(double aim_fov, float move_duration = 1.);

	void setClippingPlanes(double znear, double zfar);
	void getClippingPlanes(double* zn, double* zf) const {
		*zn = zNear;
		*zf = zFar;
	}

	// Return true if the 2D pos is inside the viewport
	bool checkInViewport(const Vec3d& pos) const;

	// for fisheye disk checking
	bool checkInMask(const Vec3d& pos, const int object_pixel_radius) const;

	// Set the standard modelview matrices used for projection
	void setModelViewMatrices(const Mat4d& _mat_earth_equ_to_eye,
	                            const Mat4d& _mat_earth_equ_to_eye_fixed,
	                            const Mat4d& _mat_helio_to_eye,
	                            const Mat4d& _mat_local_to_eye,
	                            const Mat4d& _mat_j2000_to_eye,
	                            const Mat4d& _mat_dome,
	                            const Mat4d& _mat_dome_fixed);

	// Return in vector "win" the projection on the screen of point v in earth equatorial coordinate
	// according to the current modelview and projection matrices (reimplementation of gluProject)
	// Return true if the z screen coordinate is < 1, ie if it isn't behind the observer
	// except for the _check version which return true if the projected point is inside the screen
	inline bool projectEarthEqu(const Vec3d& v, Vec3d& win) const {
		return projectCustom(v, win, mat_earth_equ_to_eye);
	}
	inline bool projectEarthEcliptic(const Vec3d& v, Vec3d& win) const {
		return projectCustom(v, win, mat_earth_equ_to_eye*Mat4d::xrotation(23.4392803055555555556*(M_PI/180)));
	}
	inline bool projectJ2000Galactic(const Vec3d& v, Vec3d& win) const {
		return projectCustom(v, win, mat_j2000_to_eye*Mat4d::zrotation(14.8595*(M_PI/180))*Mat4d::yrotation(-61.8717*(M_PI/180))*Mat4d::zrotation(55.5*(M_PI/180)));
	}

	inline bool projectEarthEquFixed(const Vec3d& v, Vec3d& win) const {
		return projectCustom(v, win, mat_earth_equ_to_eye_fixed);
	}
	inline bool projectEarthEquCheck(const Vec3d& v, Vec3d& win) const {	// unused
		return projectCustomCheck(v, win, mat_earth_equ_to_eye);
	}

	inline bool projectEarthEquLineCheck(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const { //unused
		return projectCustomLineCheck(v1, win1, v2, win2, mat_earth_equ_to_eye);
	}

	inline void unprojectEarthEqu(double x, double y, Vec3d& v) const {
		unproject(x, y, inv_mat_earth_equ_to_eye, v);
	}

	inline bool unprojectJ2000(double x, double y, Vec3d& v) const {
		unproject(x, y, inv_mat_j2000_to_eye, v);
		return true;
	}

	// taking account of precession
	inline bool projectJ2000(const Vec3d& v, Vec3d& win) const {
		return projectCustom(v, win, mat_j2000_to_eye);
	}

	inline bool projectJ2000Check(const Vec3d& v, Vec3d& win) const {
		return projectCustomCheck(v, win, mat_j2000_to_eye);
	}

	inline bool projectJ2000LineCheck(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const {
		return projectCustomLineCheck(v1, win1, v2, win2, mat_j2000_to_eye);
	}

	// Same function with input vector v in heliocentric coordinate
	inline bool projectHelioCheck(const Vec3d& v, Vec3d& win) const { //unused
		return projectCustomCheck(v, win, mat_helio_to_eye);
	}

	inline bool projectHelio(const Vec3d& v, Vec3d& win) const {	//unused
		return projectCustom(v, win, mat_helio_to_eye);
	}

	inline bool projectHelioLineCheck(const Vec3d& v1, Vec3d& win1, const Vec3d& v2, Vec3d& win2) const { //unused
		return projectCustomLineCheck(v1, win1, v2, win2, mat_helio_to_eye);
	}

	inline void unprojectHelio(double x, double y, Vec3d& v) const {	//unused
		return unproject(x, y, inv_mat_helio_to_eye, v);
	}

	// Same function with input vector v in local coordinate
	inline bool projectLocal(const Vec3d& v, Vec3d& win) const {
		return projectCustom(v, win, mat_local_to_eye);
	}

	inline bool projectLocalCheck(const Vec3d& v, Vec3d& win) const {
		return projectCustomCheck(v, win, mat_local_to_eye);
	}

	inline void unprojectLocal(double x, double y, Vec3d& v) const {
		unproject(x, y, inv_mat_local_to_eye, v);
	}

	// Same function with input vector v in dome coordinates
	inline bool projectDome(const Vec3d& v, Vec3d& win) const {	//unused
		return projectCustomFixedFov(v, win, mat_dome);
	}

	// Same function without heading
	inline bool projectDomeFixed(const Vec3d& v, Vec3d& win) const {
		return projectCustomFixedFov(v, win, mat_dome_fixed);
	}

	bool projectCustomFixedFov(const Vec3d& v, Vec3d& win, const Mat4d& mat) const;

	// Same function but using a custom modelview matrix
	bool projectCustom(const Vec3d& v, Vec3d& win, const Mat4d& mat) const;

	bool projectCustomCheck(const Vec3f& v, Vec3d& win, const Mat4d& mat) const  {
		return (projectCustom(v, win, mat) && checkInViewport(win));
	}

	// for large objects
	bool projectCustomCheck(const Vec3f& v, Vec3d& win, const Mat4d& mat, const int object_pixel_radius) const {
		return (projectCustom(v, win, mat) && checkInMask(win, object_pixel_radius));
	}

	// project two points and make sure both are in front of viewer and that at least one is on screen
	bool projectCustomLineCheck(const Vec3f& v1, Vec3d& win1, const Vec3f& v2, Vec3d& win2, const Mat4d& mat) const {
		return projectCustom(v1, win1, mat) && projectCustom(v2, win2, mat) && (checkInViewport(win1) || checkInViewport(win2));
	}

	//! Override glVertex3f and glVertex3d
	void sVertex3(double x, double y, double z, const Mat4d& mat) const;
	Vec3d sVertex3v(double x, double y, double z, const Mat4d& mat) const;

	Vec3d getCursorPosEqu(int x, int y) const {
		Vec3d v;
		unprojectEarthEqu(x,getViewportHeight()-y,v);
		return v;
	}

	void printGravity180(s_font* font, float x, float y, const std::string& str, Vec4f Color, float xshift, float yshift) const;

	//! Un-project the entire viewport depending on mapping, maskType,
	//! viewport_fov_diameter, viewport_center, and viewport dimensions.
	StelGeom::ConvexS unprojectViewport(void) const;

	Mat4f getMatProjectionOrtho2D() const {
		return Mat4f::ortho2D((float) vec_viewport[0], (float) (vec_viewport[0] + vec_viewport[2]), (float) vec_viewport[1], (float) (vec_viewport[1] + vec_viewport[3]));
	}

	//! return the current Projection Matrix
	Mat4d getMatProjection() const {
		return mat_projection;
	}

	//! return float zNear, zFar and fov direct for GLSL usage
	Vec3f getClippingFov () const {
		return Vec3f((float) zNear, (float) zFar, (float) fov*(M_PI/360.f));
	}

	Mat4f getMatLocalToEye() const {
		return mat_local_to_eye.convert();
	}
	Mat4f getMatJ2000ToEye() const {
		return mat_j2000_to_eye.convert();
	}
	Mat4f getMatEarthEquToEye() const {
		return mat_earth_equ_to_eye.convert();
	}

protected:
	// Struct used to store data for auto mov
	typedef struct {
		double start;
		double aim;
		float speed;
		float coef;
	} auto_zoom;

	double fov;					// Field of view in degree
	double min_fov;				// Minimum fov in degree
	double max_fov;				// Maximum fov in degree
	double zNear, zFar;			// Near and far clipping planes
	Vec4i vec_viewport;			// Viewport parameters
	Mat4d mat_projection;		// Projection matrix

	Vec3i viewport_center;				// Viewport center in screen pixel
	int viewport_radius;  				// Viewport radius in screen pixels

	Mat4d mat_earth_equ_to_eye;		// Modelview Matrix for earth equatorial projection
	Mat4d mat_earth_equ_to_eye_fixed;		// Modelview Matrix for earth equatorial projection
	Mat4d mat_j2000_to_eye;         // for precessed equ coords
	Mat4d mat_helio_to_eye;			// Modelview Matrix for earth equatorial projection
	Mat4d mat_local_to_eye;			// Modelview Matrix for earth equatorial projection
	Mat4d inv_mat_earth_equ_to_eye;	// Inverse of mat_projection*mat_earth_equ_to_eye
	Mat4d inv_mat_earth_equ_to_eye_fixed;	// Inverse of mat_projection*mat_earth_equ_to_eye
	Mat4d inv_mat_j2000_to_eye;		// Inverse of mat_projection*mat_earth_equ_to_eye
	Mat4d inv_mat_helio_to_eye;		// Inverse of mat_projection*mat_helio_to_eye
	Mat4d inv_mat_local_to_eye;		// Inverse of mat_projection*mat_local_to_eye

	Mat4d mat_dome;
	Mat4d inv_mat_dome;
	// Same without heading adjustment
	Mat4d mat_dome_fixed;
	Mat4d inv_mat_dome_fixed;

	// transformation from screen 2D point x,y to object
	// m is here the already inverted full tranfo matrix
	void unproject(double x, double y, const Mat4d& m, Vec3d& v) const;

	// Automove
	auto_zoom zoom_move;		// Current auto movement
	bool flag_auto_zoom;		// Define if autozoom is on or off

private:
	double viewport_fov_diameter;
	double fisheye_scale_factor;
};

#endif // _PROJECTOR_H_
