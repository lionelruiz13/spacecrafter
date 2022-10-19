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

class Star3DWrapper : public ObjectBase {
public:
    Star3DWrapper(starInfo *star, Vec3f pos) : star(star), pos(pos) {}
    virtual ~Star3DWrapper() = default;

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
		return OBJECT_STAR;
	}

    std::string getNameI18n(void) const override{
		return "";
	}

    virtual std::string getEnglishName() const override {
        return "";
    }

    virtual void getRaDeValue(const Navigator *nav,double *ra, double *de) const override {
        *ra = star->pmRA;
        *de = star->pmDE;
    }

    virtual Vec3d getEarthEquPos(const Navigator *nav) const override {
        auto tmp = nav->helioToEarthPosEqu(Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180) * star->posXYZ);
        tmp[0] = -tmp[0];
        return tmp;
	}

    virtual Vec3d getObsJ2000Pos(const Navigator *nav) const override {
        return nav->earthEquToJ2000(getEarthEquPos(nav));
    }

    virtual float getMag(const Navigator *nav) const override;

    float getBV(void) const {
		return star->B_V;
	}

private:
    int refCount = 0;
    // observer's position in parsec
    starInfo *star;
	Vec3f pos;
};
