#ifndef OLD_SKY_VIEW_HPP_
#define OLD_SKY_VIEW_HPP_

#include "coreModule/skyView.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"

// The old path's sky view: each member is the Projector's or the Navigator's own
class OldSkyView final : public SkyView {
public:
	OldSkyView(const Projector *prj, const Navigator *nav) : prj(prj), nav(nav) {}
	bool projectLocal(const Vec3d &v, Vec3d &win) const override {
		return prj->projectLocal(v, win);
	}
	bool projectLocalCheck(const Vec3d &v, Vec3d &win) const override {
		return prj->projectLocalCheck(v, win);
	}
	bool projectEarthEqu(const Vec3d &v, Vec3d &win) const override {
		return prj->projectEarthEqu(v, win);
	}
	bool projectEarthEquFixed(const Vec3d &v, Vec3d &win) const override {
		return prj->projectEarthEquFixed(v, win);
	}
	bool projectEarthEcliptic(const Vec3d &v, Vec3d &win) const override {
		return prj->projectEarthEcliptic(v, win);
	}
	bool projectJ2000(const Vec3d &v, Vec3d &win) const override {
		return prj->projectJ2000(v, win);
	}
	bool projectJ2000Check(const Vec3d &v, Vec3d &win) const override {
		return prj->projectJ2000Check(v, win);
	}
	bool projectJ2000LineCheck(const Vec3d &v1, Vec3d &win1, const Vec3d &v2, Vec3d &win2) const override {
		return prj->projectJ2000LineCheck(v1, win1, v2, win2);
	}
	bool projectJ2000Galactic(const Vec3d &v, Vec3d &win) const override {
		return prj->projectJ2000Galactic(v, win);
	}
	bool projectDomeFixed(const Vec3d &v, Vec3d &win) const override {
		return prj->projectDomeFixed(v, win);
	}
	Vec3d getCursorPosEqu(int x, int y) const override {
		return prj->getCursorPosEqu(x, y);
	}
	double getFov() const override {
		return prj->getFov();
	}
	int getViewportWidth() const override {
		return prj->getViewportWidth();
	}
	int getViewportHeight() const override {
		return prj->getViewportHeight();
	}
	double getViewportRadius() const override {
		return prj->getViewportRadius();
	}
	Vec3d getViewportCenter() const override {
		return prj->getViewportCenter();
	}
	Mat4f getMatProjectionOrtho2D() const override {
		return prj->getMatProjectionOrtho2D();
	}
	void printGravity180(s_font *font, float x, float y, const std::string &str, Vec4f Color, float xshift, float yshift) const override {
		prj->printGravity180(font, x, y, str, Color, xshift, yshift);
	}
	Mat4f getMatLocalToEye() const override {
		return prj->getMatLocalToEye();
	}
	Mat4f getMatJ2000ToEye() const override {
		return prj->getMatJ2000ToEye();
	}
	Mat4f getMatEarthEquToEye() const override {
		return prj->getMatEarthEquToEye();
	}
	Vec3d earthEquToLocal(const Vec3d &v) const override {
		return nav->earthEquToLocal(v);
	}
	Vec3d localToEarthEqu(const Vec3d &v) const override {
		return nav->localToEarthEqu(v);
	}
	Vec3d helioToLocal(const Vec3d &v) const override {
		return nav->helioToLocal(v);
	}
	Vec3d helioToEarthEqu(const Vec3d &v) const override {
		return nav->helioToEarthEqu(v);
	}
	const Vec3d &getPrecEquVision() const override {
		return nav->getPrecEquVision();
	}
private:
	const Projector *prj;
	const Navigator *nav;
};

#endif /* end of include guard: OLD_SKY_VIEW_HPP_ */
