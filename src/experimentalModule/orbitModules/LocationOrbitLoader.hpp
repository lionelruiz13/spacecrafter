#include "experimentalModule/OrbitLoader.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "bodyModule/orbit.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"

// Return nullptr on failure, a throw would select the default loader
class LocationOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        ModularBody *parent = ModularBody::findBodyOnce(params["parent"]);

        if (!parent) {
            cLog::get()->write("location_orbit orbit of '" + params["name"] + "': parent '"
                + params["parent"] + "' is not a loaded body, so there is no radius, sidereal "
                "day or spin phase to build the orbit from. This body is NOT created on this "
                "path. To fix: declare parent = <a body loaded before this one> ('none' is not "
                "a body - a location_orbit must sit on something).", LOG_TYPE::L_ERROR);
            return nullptr;
        }

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
