#ifndef MODULAR_OBJECT_HPP_
#define MODULAR_OBJECT_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/object_base.hpp"
#include <utility>

class ModularObject : public ObjectBase {
public:
    virtual std::string getInfoString(const Navigator * nav) const override;
	virtual std::string getShortInfoString(const Navigator *nav) const override;
    virtual std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const override;

    virtual OBJECT_TYPE getType() const override;
    virtual std::string getEnglishName() const override;
	virtual std::string getNameI18n() const override;

    virtual Vec3d getEarthEquPos(const Navigator *nav) const override;
	virtual Vec3d getObsJ2000Pos(const Navigator *nav) const override;

    virtual float getMag(const Navigator *nav) const override;

    // UI/scripting compatibility surface (G11) - the queries CoreLink and the
    // selection/pointer paths consume. All are Camera-based conversions of the
    // body's observed position (the new-path idiom of this file), not
    // Navigator-based like the old Body.
    virtual void getAltAz(const Navigator *nav, double *alt, double *az) const override;
    virtual void getRaDeValue(const Navigator *nav, double *ra, double *de) const override;
    virtual Vec3f getRGB() const override;
    virtual double getCloseFov(const Navigator *nav) const override;
    virtual double getSatellitesFov(const Navigator *nav) const override;
    virtual double getParentSatellitesFov(const Navigator *nav) const override;
    // Radius in pixels of a circle containing the body on screen (consumed by
    // the pointer drawing, among others).
    virtual float getOnScreenSize(const Projector *prj, const Navigator *nav = NULL, bool orb_only = false) override;

    ModularBodyPtr body;

private:
    // Single authority (I2) for the alt/az REPORTING convention at this object
    // surface. The old path exposed azimuth in the "N=0, E=90" convention and
    // applied the conversion at EACH reporting site (Body::getAltAz body.cpp:381,
    // getInfoString body.cpp:341, getShortInfoNavString body.cpp:431 - the old
    // path itself duplicated it). Camera::observedPosToAltAz returns the RAW
    // Camera-frame az, whose zero differs from the old raw frame by -π/2
    // [measured: harness/b9_azconv.py -> az_old = π/2 − az_raw over 234/234
    // non-degenerate bodies at ≤3e-5°], so the conversion that reproduces the
    // old report is az = π/2 − az_raw (mod 2π) - NOT the 3π−az of the old raw→
    // report step (that constant is frame-specific to the old raw frame; §11.4
    // flagged exactly this, §11.60). getAltAz / getInfoString /
    // getShortInfoNavString all route here so the convention cannot desync.
    // Returns (alt, az) with az in the old-path convention.
    std::pair<double, double> altAz() const;
};

#endif /* end of include guard: MODULAR_OBJECT_HPP_ */
