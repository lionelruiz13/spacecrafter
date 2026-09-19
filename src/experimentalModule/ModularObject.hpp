#ifndef MODULAR_OBJECT_HPP_
#define MODULAR_OBJECT_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/object_base.hpp"
#include <utility>

// ObjectBase view of a ModularBody, positions come from the Camera
class ModularObject : public ObjectBase {
public:
    ModularObject() = default;
    explicit ModularObject(ModularBody *b) : body(b) {}

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
    // Return the radius in pixels of a circle containing the body on screen
    virtual float getOnScreenSize(const Projector *prj, const Navigator *nav = NULL, bool orb_only = false) override;

    ModularBodyPtr body;

private:
    int refCount = 0; // Stays 0 when stack-allocated, which is then never deleted

    // Return (alt, az) with N=0, E=90
    std::pair<double, double> altAz() const;
};

#endif /* end of include guard: MODULAR_OBJECT_HPP_ */
