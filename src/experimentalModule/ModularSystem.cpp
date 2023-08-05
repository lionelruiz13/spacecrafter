#include "ModularSystem.hpp"

ModularSystem::ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info) :
    ModularBody(parent, info), star(this)
{
    isNotIsolated = false;
}

void ModularBody::cleanUp()
{
    // TODO Use another sorting algorithm for elements far from their final position
    ModularBody **pos = sortedSystemBodies.data();
    ModularBody ** const end = pos + sortedSystemBodies.size();
    do {
        if (*pos == nullptr)
            break;
    } while (++pos < end);
    ModularBody **dst = pos;
    while (++pos < end) {
        if (*pos)
            *(dst++) = *pos;
    };
    sortedSystemBodies.resize(dst - sortedSystemBodies.data());
    needCleanUp = false;
}

void ModularSystem::updateSystem()
{
    star->updateAsLightSource();
    if (needCleanUp)
        cleanUp();
    { // SORT
        // Use dual bubble sort algorithm - average complexity of O(N) as bodies stay mostly sorted between frames
    	// This come with a higher complexity at the frame newly showing multiple bodies (due to addBody)
        ModularBody **pos = sortedSystemBodies.data();
        ModularBody ** const begin = pos - 1;
        ModularBody ** const end = begin + sortedSystemBodies.size();
    	ModularBody **swapPos;
    	ModularBody *tmp;
    	do {
    		if (*pos[0] < *pos[1]) {
    			swapPos = pos;
    			tmp = pos[1];
    			do { // Backward loop, bring up the body
    				pos[1] = pos[0];
    			} while (--pos != begin && *pos[0] < *pos[1]);
    			pos[1] = tmp;
    			pos = swapPos;
    		}
    	} while (++pos < end);
    }
}

void ModularSystem::drawSystem(Renderer &renderer)
{
    for (auto &module : inComponents)
        module.draw(renderer, this, matrix);
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        (**pos).draw(renderer);
    }
    for (auto &module : nearComponents)
        module.draw(renderer, this, matrix);
}

void ModularSystem::loadBody(std::map<std::string, std::string> &param)
{
    // Avoid string copy and map search
    const std::string &englishName = param["name"];
    const std::string &parentName = param["parent"];
    const std::string &funcname = param["coord_func"];

    if (englishName.empty()) {
        cLog::get()->write("Can't load unnamed body", LOG_TYPE::L_WARNING);
        return;
    }
    cLog::get()->write("Loading body " + englishName, LOG_TYPE::L_INFO);

    if (!Utility::isTrue(param["replace"]) && ModularBody::exists(englishName)) {
        cLog::get()->write("Abort loading " + englishName + " as it already exists and replace param isn't true", LOG_TYPE::L_WARNING);
    }

    ModularBody *parent;
    if (parentName.empty()) {
        cLog::get()->write("No parent specified for " + englishName + ", assume parent is " + this->englishName + " (Specify 'none' to suppress this warning)", LOG_TYPE::L_WARNING);
    } else if (parentName == "none") {
        parent = this;
    } else {
        parent = findBody(parentName);
        if (parent == nullptr) {
            cLog::get()->write("Can't find parent " + parentName + " for " + englishName, LOG_TYPE::L_WARNING);
            return;
        }
    }

    bool close_orbit = !Utility::isFalse(param["close_orbit"]);
    double orbit_bounding_radius = Utility::strToDouble(param["orbit_bounding_radius"], -1);
    float radius = Utility::strToDouble(param["radius"]);

    // Use J2000 N pole data if available
	double rot_obliquity = Utility::strToDouble(param["rot_obliquity"],0.)*M_PI/180.;
	double rot_asc_node  = Utility::strToDouble(param["rot_equator_ascending_node"],0.)*M_PI/180.;

	// In J2000 coordinates
	double J2000_npole_ra = Utility::strToDouble(param["rot_pole_ra"],0.)*M_PI/180.;
	double J2000_npole_de = Utility::strToDouble(param["rot_pole_de"],0.)*M_PI/180.;

	// NB: north pole needs to be defined by right hand rotation rule
	if (param["rot_pole_ra"] != "" || param["rot_pole_de"] != "") {
		// cout << "Using north pole data for " << englishName << endl;
		Vec3d J2000_npole;
		Utility::spheToRect(J2000_npole_ra,J2000_npole_de,J2000_npole);

		Vec3d vsop87_pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(J2000_npole));

		double ra, de;
		Utility::rectToSphe(&ra, &de, vsop87_pole);

		rot_obliquity = (M_PI_2 - de);
		rot_asc_node = (ra + M_PI_2);
		//cout << "\tCalculated rotational obliquity: " << rot_obliquity*180./M_PI << endl;
		//cout << "\tCalculated rotational ascending node: " << rot_asc_node*180./M_PI << endl;
	}

    ModularBodyCreateInfo createInfo {
        .orbit=nullptr,
        .englishName=englishName,
        .re={
            Utility::strToDouble(param["rot_periode"], Utility::strToDouble(param["orbit_period"], 24.))/24.,
            Utility::strToDouble(param["rot_rotation_offset"],0.),
            Utility::strToDouble(param["rot_epoch"], J2000),
            rot_obliquity,
            rot_asc_node,
            Utility::strToDouble(param["rot_precession_rate"],0.)*M_PI/(180*36525),
            Utility::strToDouble(param["orbit_visualization_period"],0.),
            Utility::strToDouble(param["axial_tilt"], 0.)
        },
        .haloColor=param["color"].empty() ? defaultHaloColor : Utility::strToVec3f(param["color"]),
        .albedo=Utility::strToDouble(param["albedo"]),
        .radius=radius/AU,
        .innerRadius=Utility::strToDouble(param["min_distance"], radius*1.2)/AU,
        .oblateness=Utility::strToDouble(param["oblateness"], 0.0),
        .solLocalDay=Utility::strToDouble(param["sol_local_day"],1.0),
        .bodyType=strToBodyType(param["type"]),
        .isHaloEnabled=Utility::isTrue(param["halo"]),
    };

    if (funcname=="earth_custom") {
		// Special case to take care of Earth-Moon Barycenter at a higher level than in ephemeris library

		//cout << "Creating Earth orbit...\n" << endl;
		cLog::get()->write("Creating Earth orbit...", LOG_TYPE::L_INFO);
		std::unique_ptr<SpecialOrbit> sorb = std::make_unique<SpecialOrbit>("emb_special");
		if (!sorb->isValid()) {
			std::string error = std::string("ERROR : can't find position function ") + funcname + std::string(" for ") + englishName + std::string("\n");
			cLog::get()->write(error, LOG_TYPE::L_ERROR);
			return;
		}
		// NB. moon has to be added later
		createInfo.orbit = std::make_unique<BinaryOrbit>(std::move(sorb), 0.0121505677733761);

	} else if(funcname == "lunar_custom") {
		// This allows chaotic Moon ephemeris to be removed once start leaving acurate and sane range

		std::unique_ptr<SpecialOrbit> sorb = std::make_unique<SpecialOrbit>("lunar_special");

		if (!sorb->isValid()) {
			std::string error = std::string("ERROR : can't find position function ") + funcname + std::string(" for ") + englishName + std::string("\n");
			cLog::get()->write(error, LOG_TYPE::L_ERROR);
			return ;
		}

		createInfo.orbit = std::make_unique<MixedOrbit>(std::move(sorb),
		                     Utility::strToDouble(param["orbit_period"]),
		                     SpaceDate::JulianDayFromDateTime(-10000, 1, 1, 1, 1, 1),
		                     SpaceDate::JulianDayFromDateTime(10000, 1, 1, 1, 1, 1),
		                     EARTH_MASS + LUNAR_MASS,
		                     0, 0, 0,
		                     false);

	} else if (funcname == "still_orbit") {
		createInfo.orbit = std::make_unique<stillOrbit>(Utility::strToDouble(param["orbit_x"]),
		                     Utility::strToDouble(param["orbit_y"]),
		                     Utility::strToDouble(param["orbit_z"]));
	} else if (funcname == "location_orbit") {
		createInfo.orbit = std::make_unique<LocationOrbit>(
			Utility::strToDouble(param["orbit_lon"]),
			Utility::strToDouble(param["orbit_lat"]),
			Utility::strToDouble(param["orbit_alt"]),
			parent->getRadius()
		);
	} else {
		createInfo.orbit = orbitCreator->handle(param);
		if (createInfo.orbit == nullptr) {
			std::cout << "something went wrong when creating orbit from "<< englishName << std::endl;
			cLog::get()->write("Error when creating orbit from " + englishName, LOG_TYPE::L_ERROR);
		}
	}

    ModularBody *body = parent->createChild();
    if (Utility::isTrue(param["bound_to_surface"]))
        body->boundToSurface = true;
    if (Utility::isTrue(param["hidden"]))
        body->hide();

    // TODO NEXT TIME !
    // 1 - THIS
    // 2 - BIND TO ssystemFactory (in parallel to current system ?)
    // 3 - BIND TO Observer
}

ModularBody *ModularSystem::findBodyAt(const std::pair<float, float> &searchPos) const
{
    float mostLikely = 0;
    ModularBody *ret = nullptr;
    for (auto &child : sortedSystemBodies) {
        if (child.isVisible & child.isBodyVisible) {
            float squaredDistance = child.screenPos.first - searchPos.first;
            squaredDistance *= squaredDistance;
            {
                float tmp = child.screenPos.second - searchPos.second;
                tmp *= tmp;
                squaredDistance += tmp;
            }
            if (squaredDistance < child.screenSize * child.screenSize + 0.0001) {
                const float likely = std::min(child.screenSize, 0.001) / squaredDistance;
                if (mostLikely <= likely) {
                    mostLikely = likely;
                    ret = &child;
                }
            }
        }
    }
    return ret;
}
