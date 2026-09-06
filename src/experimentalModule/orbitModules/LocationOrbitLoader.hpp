#include "experimentalModule/OrbitLoader.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"

// The LEGACY surface-point provider, registered on this path too
// [modules.cpp:49, Vixy's da858612 2025-09-20] and building the very same
// `LocationOrbit` the old path builds -- so every defect of that class is a
// defect of both paths (INTENT S5.21, reach corrected at S11.163(e)).
// One of them, `orbit_lat` reaching spheToRect as radians, is fixed at the
// constructor (orbit.cpp, S11.217) and therefore fixed here.  The other two --
// the missing equatorial->VSOP87 rotation and the frozen linear spin -- are
// held at S11.217 with both readings recorded: making them exact forces a
// choice about what `orbit_lon` means, and by measurement that choice also
// decides where `surface_point`'s ratified key points (the two conventions
// differ by exactly 90 deg; harness/f97_locorbit.py M3, both ways, live).
class LocationOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        // findBodyOnce, not findBody: a body load is a unique search, so it has
        // no business flushing the lookup cache [ModularBody.hpp:1568-1575] -
        // the sibling provider's call, and the same nullptr on a miss.
        ModularBody *parent = ModularBody::findBodyOnce(params["parent"]);

        // NO PARENT, NO ORBIT - S5.141, and it is S5.50's class one provider
        // over (S11.219).  Every one of this orbit's parameters below IS the
        // parent (radius, sidereal day, spin phase at epoch), so a miss has
        // nothing to degrade to: pre-fix the next lines dereferenced nullptr and
        // took the app down.  RETURN, never THROW: ModuleLoaderMgr::loadOrbit
        // wraps this call in `catch (...)` and falls through to the DEFAULT
        // loader [ModuleLoaderMgr.cpp:101-109], so a throw would silently
        // replace a refused location_orbit with a SpecialOrbit - D12's exact
        // opposite.  "No orbit, no body" is then served by the consumer that
        // already says it: ModularSystem::loadBody:1245-1247.
        // The S2(f) line is written HERE because this is the one site holding
        // the coord_func AND the parent name (S11.193, the same anchor argument
        // as the double-spin warning below).
        if (!parent) {
            cLog::get()->write("location_orbit orbit of '" + params["name"] + "': parent '"
                + params["parent"] + "' is not a loaded body, so there is no radius, sidereal "
                "day or spin phase to build the orbit from. This body is NOT created on this "
                "path. To fix: declare parent = <a body loaded before this one> ('none' is not "
                "a body - a location_orbit must sit on something).", LOG_TYPE::L_ERROR);
            return nullptr;
        }

        // The DOUBLE-SPIN TRAP, said out loud at the one place that knows both
        // sides -- the S2(f) anchor for this event is the loader, because it
        // is the only site holding the coord_func AND the relation (S11.193).
        // `LocationOrbit` carries the parent's rotation inside its own
        // position (`lon + JD*JDToRotation`), while a GROUNDED body's position
        // is additionally folded by the parent's spin
        // [ModularBody.hpp:726-727], so the spin lands twice.  Measured on a
        // live pair authored with the same keys, one spelled grounded and one
        // not: 51.32 deg apart (harness/f97_locorbit.py M5, S11.217).  This is
        // the S11.78(c) failure class -- internally coherent, situationally
        // wrong, no error signal -- and until the position halves are exact the
        // signal is what can honestly be given.  The sibling provider warns for
        // the mirror-image mistake [SurfacePointOrbitLoader.hpp].
        if (params["relation"] == "grounded" || Utility::isTrue(params["bound_to_surface"])) {
            cLog::get()->write("Body '" + params["name"] + "' uses coord_func=location_orbit "
                "WITH the grounded relation: location_orbit already turns with the parent, so "
                "the grounded fold applies the parent's rotation a SECOND time and the body "
                "will not stay where it was authored. To fix: drop relation = grounded / "
                "bound_to_surface for this body.", LOG_TYPE::L_WARNING);
        }

		return std::make_unique<LocationOrbit>(
			Utility::strToDouble(params["orbit_lon"]),
			Utility::strToDouble(params["orbit_lat"]),
			Utility::strToDouble(params["orbit_alt"]),
            parent->getRadius(),
            parent->getSiderealDay(),
            parent->getSiderealTime(0)
		);
    }
};
