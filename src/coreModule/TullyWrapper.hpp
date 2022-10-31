/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - 2020 all rights reserved
*
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <thread>

#include "inGalaxyModule/starNavigator.hpp"
#include "inGalaxyModule/starManager.hpp"
#include "navModule/navigator.hpp"
#include "starModule/hip_star_mgr.hpp"
#include "starModule/hip_star.hpp"
#include "tools/vecmath.hpp"
#include "tools/object_base.hpp"

class TullyWrapper : public ObjectBase {
public:
    TullyWrapper(Vec3f pos, std::string name, OBJECT_TYPE type) : pos(pos), name(name), type(type) {}
    virtual ~TullyWrapper() = default;

    virtual void retain() override {
        ++refCount;
    }
    virtual void release() override {
        if (--refCount == 0)
            delete this;
    }

    virtual std::string getInfoString(const Navigator * nav) const override;

    virtual std::string getShortInfoString(const Navigator *nav) const override;

    virtual std::string getShortInfoNavString(const Navigator *nav, const TimeMgr * timeMgr, const Observer* observatory) const override {
        return " ";
    }

    virtual OBJECT_TYPE getType(void) const override{
		return type;
	}

    std::string getNameI18n(void) const override{
		return "";
	}

    virtual std::string getEnglishName() const override {
        return name;
    }


    virtual Vec3d getEarthEquPos(const Navigator *nav) const override {
        auto tmp = nav->helioToEarthPosEqu(pos);
        return tmp;
	}

    virtual Vec3d getObsJ2000Pos(const Navigator *nav) const override {
        return nav->earthEquToJ2000(getEarthEquPos(nav));
    }

    virtual float getMag(const Navigator *nav) const override;

private:
    // observer's position in parsec
    int refCount = 0;
	Vec3f pos;
    std::string name;
    OBJECT_TYPE type;
};
