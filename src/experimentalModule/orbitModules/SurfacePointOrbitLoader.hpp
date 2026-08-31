#ifndef SURFACE_POINT_ORBIT_LOADER_HPP_
#define SURFACE_POINT_ORBIT_LOADER_HPP_

#include "experimentalModule/OrbitLoader.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ModularBodyPtr.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"

// B24 composition provider (INTENT S11.78; spelling `surface_point` +
// ramp keys pending Vixy sign-off, the B28 protocol): a point ON a body's
// surface, expressed in the parent's SURFACE frame - the co-rotation comes
// from the GROUNDED fold (ModularBody::computeBodyToSurface, the parent's
// exact spin state: epoch, offset, precession - one authority), never from
// this provider. This is the structural rover/launchpad form; the legacy
// `location_orbit` instead self-rotates with a degraded approximation (raw
// JD, no epoch, offset frozen at construction) and silently DOUBLE-applies
// spin if combined with a grounded body (the S11.78(c) trap).
//
// Keys (data, degrees/km like every legacy key):
//   orbit_lon, orbit_lat        - planetographic position on the parent
//   orbit_alt                   - km above the parent's DATUM surface (B10:
//                                 datum_radius is the altitude zero-point)
// Optional linear ascent ramp (the "rocket going up" of the mandate) - all
// three required together:
//   orbit_alt_end               - km, altitude at the end of the ramp
//   orbit_ascent_start          - JD at which the ascent begins
//   orbit_ascent_duration       - days; alt ramps orbit_alt -> orbit_alt_end
//                                 over [start, start+duration], clamped both
//                                 sides. (The dead legacy `linearOrbit` has
//                                 its lerp weights SWAPPED - defect recorded,
//                                 NOT reproduced here, S11.52(b).)
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

    // THE MODEL LAYER (D21 [vixy 2026-08-22] via S11.149(c1)/(c2); S2(a)'s
    // two-layer rule): the parent's UNSCALED datum, read AT EVERY EVALUATION.
    //
    // It used to be `datum + altKm/AU` folded into a `const double` at LOAD
    // from `parent->getAltitudeReference()` - the SCALED datum - and replayed
    // every frame. That was wrong twice over. (i) It is a closed load-time
    // latch, the fourth instance of the class B15/B19/B32 already closed, and
    // the one thing "grounded children inherit scaling" cannot mean: `scaling`
    // is a 5 s ASmooth ramp and a baked constant cannot inherit a ramp - which
    // is also why the same scene had DIFFERENT geometry by load route
    // (`body action reload` MOVED a composed rover by up to
    // radius x (scale-1) = 6949.6 km on the shipped Moon, S11.101(f)).
    // (ii) It put a DISPLAY flag inside a PHYSICAL position: the position the
    // orbit, the shadow geometry and every model-position consumer read
    // depended on `flag moon_scaled` - the D8 leak S11.101(f)(iii) recorded
    // against `ModularBody.hpp`'s own "Just visual scaling" contract.
    //
    // Reading the RAW datum live fixes both, and it costs one float load: the
    // display half now lives where it belongs, in
    // ModularBody::getDisplayEclipticPos(), which dilates THIS output for the
    // drawn chain only. Reading it live also makes the runtime
    // `body name <parent> datum_radius <km>` seam (B10) reach the bodies
    // standing on that parent, which a baked value silently ignored.
    //
    // The ascent ramp now lerps the ALTITUDE (what the data keys mean) rather
    // than the datum-inclusive radius; algebraically identical, since the datum
    // is a constant of the lerp: datum + (a(1-f) + b f) == (datum+a)(1-f) +
    // (datum+b) f.
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
    // Lifetime (I5): a non-owning reference to a body this orbit outlives only
    // if nothing destroys the parent - ModularBodyPtr is the project's
    // destruction-notified form, and it is also what makes a body REPLACED by
    // name (the loader's replace path) redirect this orbit to the replacement
    // instead of leaving it on freed memory.
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
        // The co-rotation precondition, checked at the one place that knows
        // both sides (S2(f)): this provider emits a surface-frame point, so a
        // body that is NOT grounded would sit frozen in the parent's
        // non-spinning frame - the inverse of the location_orbit double-spin
        // trap. Load proceeds (the position is still well-defined); the log
        // names the fix.
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
