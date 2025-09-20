#include "ModularObject.hpp"
#include "ModularBody.hpp"
#include "Camera.hpp"
#include <sstream>

std::string ModularObject::getInfoString(const Navigator *nav) const
{
    std::ostringstream oss;

    oss << body->getEnglishName();  // UI translation can differ from sky translation
	oss.setf(std::ios::fixed);
	oss.precision(1);
	oss << std::endl;

	oss.precision(2);
	oss << ("Magnitude: ") << body->computeMagnitude() << std::endl;

    auto tmp = Camera::instance->observedPosToRaDe(body->getObservedPosition());
	oss << ("RA/DE: ") << Utility::printAngleHMS(tmp.first) << " / " << Utility::printAngleDMS(tmp.second) << std::endl;

    tmp = Camera::instance->observedPosToAltAz(body->getObservedPosition());
	oss << ("Alt/Az: ") << Utility::printAngleDMS(tmp.first) << " / " << Utility::printAngleDMS(tmp.second) << std::endl;

	oss.precision(8);
	oss << ("Distance: ") << body->getDistanceToObserver() << " " << ("AU");
    return oss.str();
}

std::string ModularObject::getShortInfoString(const Navigator *nav) const
{
    std::ostringstream oss;
    oss << body->getEnglishName();  // UI translation can differ from sky translation
    oss << " : " << "ModularBody" << " ";
    oss.setf(std::ios::fixed);
    oss.precision(2);
    oss << "  " << ("Magnitude: ") << body->computeMagnitude();

    oss.precision(4);
    oss << "  " <<  ("Distance: ") << body->getDistanceToObserver() << " " << ("AU");
    return oss.str();
}

std::string ModularObject::getShortInfoNavString(const Navigator *nav, const TimeMgr *timeMgr, const Observer *observatory) const
{
    std::ostringstream oss;
    auto tmp = Camera::instance->observedPosToRaDe(body->getObservedPosition());
	oss << ("RA/DE: ") << Utility::printAngleHMS(tmp.first) << " / " << Utility::printAngleDMS(tmp.second) << std::endl;
    double daytime = tan(tmp.second)*tan(Camera::instance->getLatitude()); // partial calculation to determinate if midnight sun or not

    const double jd = timeMgr->getJulian() - 2451545.0;

	const double T = jd / 36525.0;
	/* calc mean angle */
	const double sidereal = (280.46061837 + (360.98564736629 * jd) + (0.000387933 * T * T) - (T * T * T / 38710000.0)) * (M_PI/180.);
	const double HA = std::fmod(sidereal+Camera::instance->getLatitude()-tmp.first, M_PI*2);
	const double GHA = std::fmod(sidereal-tmp.first, M_PI*2);
    const double PA = (HA < M_PI) ? HA : (2*M_PI - HA);
    if (tmp.first < 0)
        tmp.first += 2*M_PI;

    oss << ("SA ") << Utility::printAngleDMS(2*M_PI-tmp.first)
	    << (" GHA ") << Utility::printAngleDMS(GHA)
	    << (" LHA ") << Utility::printAngleDMS(HA);
	// calculate alt az
    tmp = Camera::instance->observedPosToAltAz(body->getObservedPosition());
	oss << "@" << (" Az/Alt/coA: ") << Utility::printAngleDMS(tmp.first) << "/" << Utility::printAngleDMS(tmp.second) << "/" << Utility::printAngleDMS(M_PI_2-tmp.second) << " LPA " << Utility::printAngleDMS(PA);

    // TODO don't rely on englishName for execution path.
    // Do you mean body->isStar(), body == Camera::instance->getCurrentSystem()->getSystemStar() or something else ?
	if (body->getEnglishName() == "Sun") {
		oss << (" Day length: ");
		if (daytime<-1) {
            oss << "00h00m00s";
		} else if (daytime>1) {
            oss << "24h00m00s";
        } else {
			daytime=2*(M_PI-acos(daytime));
			oss << Utility::printAngleHMS(daytime);
		}
	}
	return oss.str();
}

OBJECT_TYPE ModularObject::getType() const
{
    return OBJECT_TYPE::OBJECT_MODULAR;
}

std::string ModularObject::getEnglishName() const
{
    return body->getEnglishName();
}

std::string ModularObject::getNameI18n() const
{
    return body->getNameI18n();
}

Vec3d ModularObject::getEarthEquPos(const Navigator *nav) const
{
    Vec3f ret = Camera::instance->observedToBodyLocalPos(body->getObservedPosition());
    return Vec3d(ret[0], ret[1], ret[2]);
}

Vec3d ModularObject::getObsJ2000Pos(const Navigator *nav) const
{
    return getEarthEquPos(nav); // Not valid
}

float ModularObject::getMag(const Navigator *nav) const
{
    return body->computeMagnitude();
}
