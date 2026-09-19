#ifndef SURFACE_POINT_ORBIT_LOADER_HPP_
#define SURFACE_POINT_ORBIT_LOADER_HPP_

#include "experimentalModule/OrbitLoader.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ModularBodyPtr.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"

// surface_point: a point in the parent's SURFACE frame; co-rotation comes from the grounded fold, never from this orbit
// Keys (degrees/km): orbit_lon, orbit_lat, orbit_alt = km above the parent's datum surface
// Optional linear ascent, all three together: orbit_alt_end (km), orbit_ascent_start (JD), orbit_ascent_duration (days)
class SurfacePointOrbit : public Orbit {
public:
    SurfacePointOrbit(ModularBody *parent, Vec3d direction,
                      double tStart, double tDuration,
                      double lonDeg, double latDeg, double altKm, double altEndKm) :
        parent(parent), direction(direction),
        tStart(tStart), tDuration(tDuration),
        lonDeg(lonDeg), latDeg(latDeg), altKm(altKm), altEndKm(altEndKm)
    {
    }

    // Round-trips the DATA keys (degrees/km), not the derived AU state -
    // the save is a data-surface serialization (ell_orbit precedent).
    virtual std::string saveOrbit() const override
    {
        std::ostringstream os;
        os << "coord_func = surface_point" << std::endl;
        os << "orbit_lon = " << lonDeg << std::endl;
        os << "orbit_lat = " << latDeg << std::endl;
        os << "orbit_alt = " << altKm << std::endl;
        if (tDuration > 0) {
            os << "orbit_alt_end = " << altEndKm << std::endl;
            os << "orbit_ascent_start = " << tStart << std::endl;
            os << "orbit_ascent_duration = " << tDuration << std::endl;
        }
        return os.str();
    }

    // Reads the parent's UNSCALED datum at every evaluation; display scaling is ModularBody::getDisplayEclipticPos()'s
    virtual void positionAtTimevInVSOP87Coordinates(double JD0, double JD, double *v) const override
    {
        ModularBody *p = parent;
        const double datum = p ? p->getDatumRadiusRaw() : 0;
        double alt = altKm;
        if (tDuration > 0) {
            const double f = (JD - tStart) / tDuration;
            if (f >= 1)
                alt = altEndKm;
            else if (f > 0)
                alt = altKm * (1 - f) + altEndKm * f;
        }
        const double r = datum + alt / AU;
        v[0] = direction[0] * r;
        v[1] = direction[1] * r;
        v[2] = direction[2] * r;
    }
private:
    // Destruction-notified; a parent replaced by name redirects this orbit to the replacement
    const ModularBodyPtr parent;
    const Vec3d direction; // unit vector in the parent's surface frame
    const double tStart;   // JD
    const double tDuration; // days
    const double lonDeg, latDeg, altKm, altEndKm; // data keys, for saveOrbit
};

class SurfacePointOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        ModularBody *parent = ModularBody::findBodyOnce(params["parent"]);
        if (!parent) {
            cLog::get()->write("surface_point orbit of '" + params["name"]
                + "': parent '" + params["parent"] + "' not found - altitude is measured from "
                "0 instead of the parent's datum surface. Declare the parent before this body.",
                LOG_TYPE::L_WARNING);
        }
        if (!(params["relation"] == "grounded" || Utility::isTrue(params["bound_to_surface"]))) {
            cLog::get()->write("Body '" + params["name"] + "' uses coord_func=surface_point "
                "without the grounded relation: the point will NOT co-rotate with the parent's "
                "surface. To fix: add relation = grounded (or bound_to_surface = true).",
                LOG_TYPE::L_WARNING);
        }
        const double lonDeg = Utility::strToDouble(params["orbit_lon"]);
        const double latDeg = Utility::strToDouble(params["orbit_lat"]);
        Vec3d direction;
        Utility::spheToRect(lonDeg * (M_PI / 180), latDeg * (M_PI / 180), direction);
        const double altKm = Utility::strToDouble(params["orbit_alt"]);
        double altEndKm = altKm, tStart = 0, tDuration = 0;
        const bool haveEnd = !params["orbit_alt_end"].empty();
        const bool haveStart = !params["orbit_ascent_start"].empty();
        const bool haveDuration = !params["orbit_ascent_duration"].empty();
        if (haveEnd && haveStart && haveDuration) {
            altEndKm = Utility::strToDouble(params["orbit_alt_end"]);
            tStart = Utility::strToDouble(params["orbit_ascent_start"]);
            tDuration = Utility::strToDouble(params["orbit_ascent_duration"]);
            if (tDuration <= 0) {
                cLog::get()->write("Body '" + params["name"] + "': orbit_ascent_duration must be "
                    "> 0 (days). The ascent is disabled, altitude stays at orbit_alt.",
                    LOG_TYPE::L_ERROR);
                altEndKm = altKm;
                tDuration = 0;
            }
        } else if (haveEnd || haveStart || haveDuration) {
            cLog::get()->write("Body '" + params["name"] + "': incomplete ascent ramp - "
                "orbit_alt_end, orbit_ascent_start and orbit_ascent_duration are required "
                "together. The ascent is disabled, altitude stays at orbit_alt. To fix: "
                "declare all three, or none.", LOG_TYPE::L_ERROR);
        }
        return std::make_unique<SurfacePointOrbit>(parent, direction, tStart, tDuration,
                                                   lonDeg, latDeg, altKm, altEndKm);
    }
};

#endif /* end of include guard: SURFACE_POINT_ORBIT_LOADER_HPP_ */
