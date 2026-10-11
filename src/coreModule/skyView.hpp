#ifndef SKY_VIEW_HPP_
#define SKY_VIEW_HPP_

#include "tools/vecmath.hpp"
#include <string>

class s_font;

// The view a sky layer is drawn through, one implementation per path
// Local vectors use the old engine's local axes, helio positions are in AU
// win = (x, y) in render px and z < 1 in front; *Check: also inside the viewport
class SkyView {
public:
	virtual ~SkyView() = default;
	virtual bool projectLocal(const Vec3d &v, Vec3d &win) const = 0;
	virtual bool projectLocalCheck(const Vec3d &v, Vec3d &win) const = 0;
	virtual bool projectEarthEqu(const Vec3d &v, Vec3d &win) const = 0;
	virtual bool projectEarthEquFixed(const Vec3d &v, Vec3d &win) const = 0;
	virtual bool projectEarthEcliptic(const Vec3d &v, Vec3d &win) const = 0;
	virtual bool projectJ2000(const Vec3d &v, Vec3d &win) const = 0;
	virtual bool projectJ2000Check(const Vec3d &v, Vec3d &win) const = 0;
	// Return true if both ends project inside the disc and one inside the viewport
	virtual bool projectJ2000LineCheck(const Vec3d &v1, Vec3d &win1, const Vec3d &v2, Vec3d &win2) const = 0;
	virtual bool projectJ2000Galactic(const Vec3d &v, Vec3d &win) const = 0;
	// Project with the fixed 180-degree dome law, without heading
	virtual bool projectDomeFixed(const Vec3d &v, Vec3d &win) const = 0;
	// Return the earth-equatorial direction under the window px (x, y)
	virtual Vec3d getCursorPosEqu(int x, int y) const = 0;
	// Return the field of view in degrees
	virtual double getFov() const = 0;
	virtual int getViewportWidth() const = 0;
	virtual int getViewportHeight() const = 0;
	virtual double getViewportRadius() const = 0;
	// Return (cx, cy, radius) in px
	virtual Vec3d getViewportCenter() const = 0;
	virtual Mat4f getMatProjectionOrtho2D() const = 0;
	virtual void printGravity180(s_font *font, float x, float y, const std::string &str, Vec4f Color, float xshift, float yshift) const = 0;
	virtual Mat4f getMatLocalToEye() const = 0;
	virtual Mat4f getMatJ2000ToEye() const = 0;
	virtual Mat4f getMatEarthEquToEye() const = 0;
	virtual Vec3d earthEquToLocal(const Vec3d &v) const = 0;
	virtual Vec3d localToEarthEqu(const Vec3d &v) const = 0;
	virtual Vec3d helioToLocal(const Vec3d &v) const = 0;
	virtual Vec3d helioToEarthEqu(const Vec3d &v) const = 0;
	// Return the view axis in J2000
	virtual const Vec3d &getPrecEquVision() const = 0;
};

#endif /* end of include guard: SKY_VIEW_HPP_ */
