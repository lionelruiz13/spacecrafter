#ifndef MODULAR_OBJECT_HPP_
#define MODULAR_OBJECT_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/object_base.hpp"

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

    ModularBodyPtr body;
};

#endif /* end of include guard: MODULAR_OBJECT_HPP_ */
