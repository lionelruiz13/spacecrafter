#ifndef MODULAR_OBJECT_HPP_
#define MODULAR_OBJECT_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/object_base.hpp"
#include <utility>

// ObjectBase view of a ModularBody for the selection, the info strings and the UI queries; positions from the Camera
class ModularObject : public ObjectBase {
public:
    ModularObject() = default;
    explicit ModularObject(ModularBody *b) : body(b) {}

    // Owned by its reference count, deleted at zero; body is not owned, it is redirected when the body is removed
    virtual void retain() override {
        ++refCount;
    }
    virtual void release() override {
        if (--refCount == 0)
            delete this;
    }

    virtual std::string getInfoString(const Navigator * nav) const override;
	virtual std::string getShortInfoString(const Navigator *nav) const override;
    virtual std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const override;

    virtual OBJECT_TYPE getType() const override;
    virtual std::string getEnglishName() const override;
	virtual std::string getNameI18n() const override;

    virtual Vec3d getEarthEquPos(const Navigator *nav) const override;
	virtual Vec3d getObsJ2000Pos(const Navigator *nav) const override;

    virtual float getMag(const Navigator *nav) const override;

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
    // Stack-allocated bridges (the dual_dump instrument builds one per body)
    // never see retain/release, so this stays 0 and nothing is deleted.
    int refCount = 0;

    // (alt, az) with az in the reported convention (N=0, E=90); every alt/az report of this class goes through it
    std::pair<double, double> altAz() const;
};

#endif /* end of include guard: MODULAR_OBJECT_HPP_ */
