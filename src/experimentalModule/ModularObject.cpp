#include "ModularObject.hpp"
#include "ModularBody.hpp"
#include "Camera.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "tools/translator.hpp"
#include <sstream>

// TRANSLATION, and why every msgid below is copied rather than written (S5.111,
// S11.209). Old is the comparison baseline (S11.52(b)), so the label this path
// prints must be the label the old path prints, byte for byte: the msgids here
// are `Body::getInfoString` / `getShortInfoString` / `getShortInfoNavString`'s
// own (body.cpp:331-435), and NOT ONE of them is authored here.
//
// The reason is mechanical, not stylistic. `_()` is NOT gettext: it is
// `Translator::translateUTF8` (tools/translator.cpp:45), a std::map loaded from
// `<localeDir>/<lang>.txt` with IDENTITY FALLBACK for a key it does not hold.
// That catalogue is INSTALLED FIELD DATA (`~/.spacecrafter/language/fr.txt`),
// frozen by D9 and shipped from another repository - this file cannot add a key
// to it. So a msgid that is old's answers exactly as old answers, and a msgid
// that is NOT old's answers in English forever, silently. Copying is the only
// form of this change that can work.
//
// Five of the ten distinct msgids are in fr.txt today ("Magnitude: ", "RA/DE: ",
// "Alt/Az: ", "Distance: ", "AU"); the other five ("SA ", " GHA ", " LHA ",
// " Az/Alt/coA: ", " Day length: ") fall through to English - AND SO DO OLD'S,
// through the same five keys. Wrapping them changes nothing observable today
// and is still required: the day the catalogue gains one of those keys, the two
// paths must move together, which is what having ONE msgid per label buys (I2).
//
// The separators are deliberately NOT wrapped - `" / "`, `"@"`, `"/"`, `" LPA "`,
// `"00h00m00s"`, `"24h00m00s"` - because old does not wrap them either.

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
	const double HA = std::fmod(sidereal+Camera::instance->getLatitude()-tmp.first, M_PI*2);
	const double GHA = std::fmod(sidereal-tmp.first, M_PI*2);
    const double PA = (HA < M_PI) ? HA : (2*M_PI - HA);
    if (tmp.first < 0)
        tmp.first += 2*M_PI;

    oss << _("SA ") << Utility::printAngleDMS(2*M_PI-tmp.first)
	    << _(" GHA ") << Utility::printAngleDMS(GHA)
	    << _(" LHA ") << Utility::printAngleDMS(HA);
	// calculate alt az. Old path prints az/alt/coAlt (coAlt = 90deg-alt) under
	// the "Az/Alt/coA" label [body.cpp:433]; the new path had swapped alt<->az
	// (raw az too) - a port defect flagged at S11.4. Reproduce the old order +
	// convention exactly via the single authority (I2/parity, S11.60).
    const auto aa = altAz();  // (alt, az) in the old-path convention
	oss << "@" << _(" Az/Alt/coA: ") << Utility::printAngleDMS(aa.second) << "/" << Utility::printAngleDMS(aa.first) << "/" << Utility::printAngleDMS(M_PI_2-aa.first) << " LPA " << Utility::printAngleDMS(PA);

    // B27 A5 (S11.73(b), 2026-07-25): the day-length line was keyed on the name
    // "Sun" (old body.cpp:434). RESOLVED to the CAPABILITY isStar(), not the
    // system star: `daytime` above is computed from THIS body's declination and
    // the observer's latitude, i.e. the length of the day this body makes when
    // it is the one lighting the sky - a property of any light source, and the
    // same structural answer getSatellitesFov() already gives ("the old path
    // excluded the Sun by name; the structural equivalent is excluding stars",
    // :162). `getSystemStar()` would have been the WRONG predicate twice over: it
    // would print nothing for a star that is not its system's designated star,
    // and it asks about the SYSTEM's state where the line is about the SELECTED
    // body. Shipped corpus: `type = Sun` is the only STAR-tagged body, so the
    // truth set is unchanged [measured: ssystem.ini, 1 star / 91 sections].
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

std::pair<double, double> ModularObject::altAz() const
{
    // observedPosToAltAz returns (alt, az_raw): rectToSphe(&ret.second,
    // &ret.first, ...) puts latitude(alt) in .first, longitude(az) in .second.
    const auto tmp = Camera::instance->observedPosToAltAz(body->getObservedPosition());
    // Old-path azimuth convention (N=0, E=90). The new raw az zero is offset
    // -pi/2 from the old raw frame [measured, S11.60], so pi/2 - az_raw
    // reproduces Body::getAltAz's az exactly (float32 residual <=3e-5deg).
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
    // Old path excluded the Sun by name; the structural equivalent is excluding
    // the system's PRIMARY (its "satellites" span the whole system). D27 split
    // (S11.113(f)): primacy, not luminosity - a dark primary's children span the
    // system just as widely, and a companion star that orbits something else has
    // an ordinary subsystem worth fitting.
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
    // screenSize is the ratio of the screen taken by the body; the pointer
    // path wants pixels. Viewport radius matches ModularBody::setTranslator's
    // definition (screen width / 2).
    return body->getScreenSize() * ModularBody::getViewportRadius();
}
