#include "ModularObject.hpp"
#include "ModularBody.hpp"
#include "Camera.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "tools/translator.hpp"
#include <sstream>

namespace {
inline double wrap2pi(double a)
{
    a = std::fmod(a, 2 * M_PI);
    return (a < 0) ? a + 2 * M_PI : a;
}
}

std::string ModularObject::getInfoString(const Navigator *nav) const
{
    std::ostringstream oss;

    oss << body->getEnglishName();  // UI translation can differ from sky translation
	oss.setf(std::ios::fixed);
	oss.precision(1);
	oss << std::endl;

	oss.precision(2);
	oss << _("Magnitude: ") << body->computeMagnitude() << std::endl;

    auto tmp = Camera::instance->observedPosToRaDe(body->getObservedPosition());
	oss << _("RA/DE: ") << Utility::printAngleHMS(tmp.first) << " / " << Utility::printAngleDMS(tmp.second) << std::endl;

    const auto aa = altAz();  // (alt, az) in the old-path convention (I2)
	oss << _("Alt/Az: ") << Utility::printAngleDMS(aa.first) << " / " << Utility::printAngleDMS(aa.second) << std::endl;

	oss.precision(8);
	oss << _("Distance: ") << body->getDistanceToObserver() << " " << _("AU");
    return oss.str();
}

std::string ModularObject::getShortInfoString(const Navigator *nav) const
{
    std::ostringstream oss;
    oss << body->getEnglishName();  // UI translation can differ from sky translation
    oss << " : " << "ModularBody" << " ";
    oss.setf(std::ios::fixed);
    oss.precision(2);
    oss << "  " << _("Magnitude: ") << body->computeMagnitude();

    oss.precision(4);
    oss << "  " <<  _("Distance: ") << body->getDistanceToObserver() << " " << _("AU");
    return oss.str();
}

std::string ModularObject::getShortInfoNavString(const Navigator *nav, const TimeMgr *timeMgr, const Observer *observatory) const
{
    std::ostringstream oss;
    auto tmp = Camera::instance->observedPosToRaDe(body->getObservedPosition());
	oss << _("RA/DE: ") << Utility::printAngleHMS(tmp.first) << " / " << Utility::printAngleDMS(tmp.second) << std::endl;
    double daytime = tan(tmp.second)*tan(Camera::instance->getLatitude()); // partial calculation to determinate if midnight sun or not

    const double jd = timeMgr->getJulian() - 2451545.0;

	const double T = jd / 36525.0;
	/* calc mean angle */
	const double sidereal = (280.46061837 + (360.98564736629 * jd) + (0.000387933 * T * T) - (T * T * T / 38710000.0)) * (M_PI/180.);
    const double HA = wrap2pi(sidereal + Camera::instance->getLongitude() - tmp.first);
    const double GHA = wrap2pi(sidereal - tmp.first);
    const double PA = (HA < M_PI) ? HA : (2*M_PI - HA);
    if (tmp.first < 0)
        tmp.first += 2*M_PI;

    oss << _("SA ") << Utility::printAngleDMS(2*M_PI-tmp.first)
	    << _(" GHA ") << Utility::printAngleDMS(GHA)
	    << _(" LHA ") << Utility::printAngleDMS(HA);
    const auto aa = altAz();  // (alt, az) in the old-path convention
	oss << "@" << _(" Az/Alt/coA: ") << Utility::printAngleDMS(aa.second) << "/" << Utility::printAngleDMS(aa.first) << "/" << Utility::printAngleDMS(M_PI_2-aa.first) << " LPA " << Utility::printAngleDMS(PA);

	if (body->isStar()) {
		oss << _(" Day length: ");
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
    Vec3f ret = Camera::instance->observedToBodyEquPos(body->getObservedPosition());
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

std::pair<double, double> ModularObject::altAz() const
{
    // observedPosToAltAz returns (alt, az_raw): rectToSphe(&ret.second,
    // &ret.first, ...) puts latitude(alt) in .first, longitude(az) in .second.
    const auto tmp = Camera::instance->observedPosToAltAz(body->getObservedPosition());
    double az = std::fmod(M_PI_2 - tmp.second, 2 * M_PI);
    if (az < 0)
        az += 2 * M_PI;
    return { tmp.first, az };
}

void ModularObject::getAltAz(const Navigator *nav, double *alt, double *az) const
{
    const auto aa = altAz();
    *alt = aa.first;
    *az = aa.second;
}

void ModularObject::getRaDeValue(const Navigator *nav, double *ra, double *de) const
{
    const auto tmp = Camera::instance->observedPosToRaDe(body->getObservedPosition());
    *ra = tmp.first;
    *de = tmp.second;
}

Vec3f ModularObject::getRGB() const
{
    return body->getHaloColor();
}

double ModularObject::getCloseFov(const Navigator *nav) const
{
    // Same semantic as the old path: fov fitting 4x the body diameter angle
    return atanf(body->getRadius()*2.f/body->getDistanceToObserver())*(180./M_PI)*4;
}

double ModularObject::getSatellitesFov(const Navigator *nav) const
{
    if (body->hasChildren() && !body->isPrimary()) {
        const float rad = body->getSubsystemRadius();
        if (rad > 0)
            return atanf(rad/body->getDistanceToObserver())*(180./M_PI)*4;
    }
    return -1.;
}

double ModularObject::getParentSatellitesFov(const Navigator *nav) const
{
    if (ModularBody *parent = body->getParent()) {
        if (parent->hasChildren() && !parent->isPrimary()) {   // D27 split, as above
            const float rad = parent->getSubsystemRadius();
            if (rad > 0)
                return atanf(rad/parent->getDistanceToObserver())*(180./M_PI)*4;
        }
    }
    return -1.;
}

float ModularObject::getOnScreenSize(const Projector *prj, const Navigator *nav, bool orb_only)
{
    return body->getScreenSize() * ModularBody::getViewportRadius();
}
