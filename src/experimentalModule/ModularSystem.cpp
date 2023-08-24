#include "ModularSystem.hpp"
#include "ModuleLoaderMgr.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"

const Mat4f mat_j2000_to_vsop87(
    Mat4f::xrotation(-23.4392803055555555556*(M_PI/180)) *
    Mat4f::zrotation(0.0000275*(M_PI/180))
);

constexpr uint32_t casify(const char *data) {
	return ((uint32_t) data[0]) | (((uint32_t) data[1]) << 8) | (((uint32_t) data[2]) << 16) | (((uint32_t) data[3]) << 24);
}

#define CASE(name, type) case casify(name): return BodyType::type

static inline BodyType strToBodyType(const std::string &str)
{
	if (str.size() < 3)
		return BodyType::CUSTOM_BODY;
	switch (*(const uint32_t *) str.data()) {
		CASE("Sun", STAR);
		CASE("Star", STAR);
        CASE("Asteroid", MINOR_BODY);
        CASE("KBO", MINOR_BODY);
        CASE("Comet", MINOR_BODY);
		CASE("Planet", CUSTOM_BODY);
		CASE("Moon", CUSTOM_BODY);
		CASE("Dwarf", CUSTOM_BODY);
		CASE("Artificial", CUSTOM_BODY);
		CASE("Observer", ANCHOR);
		CASE("Anchor", ANCHOR);
		CASE("Center", ANCHOR);
		default:
			return BodyType::CUSTOM_BODY;
	}
}

#undef CASE

ModularSystem::ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info) :
    ModularBody(parent, info), star(this)
{
    isNotIsolated = false;
}

void ModularSystem::cleanUp()
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
    if (sortedSystemBodies.size() > 1) { // SORT
        // Use dual bubble sort algorithm - average complexity of O(N) as bodies stay mostly sorted between frames
    	// This come with a higher complexity at the frame newly showing multiple bodies (due to loadBody)
        ModularBody **pos = sortedSystemBodies.data();
        ModularBody ** const begin = pos - 1;
        ModularBody ** const end = begin + sortedSystemBodies.size();
    	do {
            auto tmpDistance = pos[0]->distance;
            if (tmpDistance < pos[1]->distance) {
    			ModularBody **swapPos = pos;
    			ModularBody *tmp = pos[1];
    			do { // Backward loop, bring up the body
    				pos[1] = pos[0];
    			} while (--pos != begin && tmpDistance < pos[1]->distance);
    			pos[1] = tmp;
    			pos = swapPos;
    		}
    	} while (++pos < end);
    }
}

void ModularSystem::drawSystem(Renderer &renderer)
{
    for (auto &module : inComponents)
        module->draw(renderer, this, mat);
    renderer.beginBodyDraw();
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        if ((**pos).distance == 0)
            break; // Skip bodies not evaluated on update
        (**pos).draw(renderer);
    }
    renderer.endBodyDraw();
    for (auto &module : nearComponents)
        module->draw(renderer, this, mat);
}

void ModularSystem::loadBody(std::map<std::string, std::string> &param)
{
    // Avoid string copy and map search
    const std::string &englishName = param["name"];
    const std::string &parentName = param["parent"];

    if (englishName.empty()) {
        cLog::get()->write("Can't load unnamed body", LOG_TYPE::L_WARNING);
        return;
    }
    if (!Utility::isTrue(param["replace"]) && ModularBody::exists(englishName)) {
        cLog::get()->write("Skip loading " + englishName + " as it already exists and replace param isn't true", LOG_TYPE::L_WARNING);
        return;
    }
    cLog::get()->write("Loading body " + englishName, LOG_TYPE::L_INFO);

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

    // bool close_orbit = !Utility::isFalse(param["close_orbit"]);
    // float orbit_bounding_radius = Utility::strToFloat(param["orbit_bounding_radius"], -1);
    float radius = Utility::strToFloat(param["radius"]);

    // Use J2000 N pole data if available
	float rot_obliquity = Utility::strToFloat(param["rot_obliquity"],0.)*M_PI/180.;
	float rot_asc_node  = Utility::strToFloat(param["rot_equator_ascending_node"],0.)*M_PI/180.;

	// In J2000 coordinates
	float J2000_npole_ra = Utility::strToFloat(param["rot_pole_ra"],0.)*M_PI/180.;
	float J2000_npole_de = Utility::strToFloat(param["rot_pole_de"],0.)*M_PI/180.;

	// NB: north pole needs to be defined by right hand rotation rule
	if (param["rot_pole_ra"] != "" || param["rot_pole_de"] != "") {
		// cout << "Using north pole data for " << englishName << endl;
		Vec3f J2000_npole;
		Utility::spheToRect(J2000_npole_ra,J2000_npole_de,J2000_npole);

		Vec3f vsop87_pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(J2000_npole));

		float ra, de;
		Utility::rectToSphe(&ra, &de, vsop87_pole);

		rot_obliquity = (M_PI_2 - de);
		rot_asc_node = (ra + M_PI_2);
		//cout << "\tCalculated rotational obliquity: " << rot_obliquity*180./M_PI << endl;
		//cout << "\tCalculated rotational ascending node: " << rot_asc_node*180./M_PI << endl;
	}

    ModularBodyCreateInfo createInfo {
        .orbit=ModuleLoaderMgr::instance.loadOrbit(param),
        .englishName=englishName,
        .re={
            Utility::strToFloat(param["rot_periode"], Utility::strToFloat(param["orbit_period"], 24.f))/24.f,
            Utility::strToFloat(param["rot_rotation_offset"],0.),
            Utility::strToDouble(param["rot_epoch"], J2000),
            rot_obliquity,
            rot_asc_node,
            Utility::strToFloat(param["rot_precession_rate"],0.)*static_cast<float>(M_PI/(180*36525)),
            Utility::strToDouble(param["orbit_visualization_period"],0.),
            Utility::strToFloat(param["axial_tilt"], 0.)
        },
        .haloColor=param["color"].empty() ? defaultHaloColor : Utility::strToVec3f(param["color"]),
        .albedo=Utility::strToFloat(param["albedo"]),
        .radius=radius/static_cast<float>(AU),
        .innerRadius=Utility::strToFloat(param["min_distance"], radius*1.2f)/static_cast<float>(AU),
        .oblateness=Utility::strToFloat(param["oblateness"], 0.0),
        .solLocalDay=Utility::strToFloat(param["sol_local_day"],1.0),
        .bodyType=strToBodyType(param["type"]),
        .isHaloEnabled=Utility::isTrue(param["halo"]),
		.altitudeRelativeToRadius=Utility::isFalse("solid")
    };
	if (!createInfo.orbit) {
		cLog::get()->write("Invalid orbit '" + param["coord_func"] + "' for body '" + englishName + "', skip loading this body.", LOG_TYPE::L_ERROR);
		return;
	}

    if (Utility::isTrue(param["hardcoded"]))
        applyHardcodedContent(createInfo, param);
    ModularBody *body = parent->createChild(createInfo);
    if (Utility::isTrue(param["bound_to_surface"]))
        body->boundToSurface = true;
    if (Utility::isTrue(param["hidden"]))
        body->hide();
    if (Utility::isTrue(param["system_star"]) || (!star && body->isStar() && parent == this))
        star = body;
    for (auto moduleType : body->deduceBodyModuleList(param))
        ModuleLoaderMgr::instance.loadModule(moduleType, body, param);
}

ModularBody *ModularSystem::findBodyAt(const std::pair<float, float> &searchPos) const
{
    float mostLikely = 0;
    ModularBody *ret = nullptr;
    for (auto child : sortedSystemBodies) {
        if (*child) {
            float squaredDistance = child->screenPos.first - searchPos.first;
            squaredDistance *= squaredDistance;
            {
                float tmp = child->screenPos.second - searchPos.second;
                tmp *= tmp;
                squaredDistance += tmp;
            }
            if (squaredDistance < child->screenSize * child->screenSize + 0.0001f) {
                const float likely = std::min(child->screenSize, 0.001f) / squaredDistance;
                if (mostLikely <= likely) {
                    mostLikely = likely;
                    ret = child;
                }
            }
        }
    }
    return ret;
}

void ModularSystem::loadSystem(const std::string &filename)
{
    std::ifstream file(filename);
    if (file) {
        systemFilename = filename;
        stringHash_t bodyParams;
        std::string line;
        while (getline(file, line)) {
            if (line.size() < 2) // Smallest is "[]"
                continue;
            switch (line.front()) {
                case '#':
                    continue;
                case '[':
                    if (!bodyParams.empty()) {
                        loadBody(bodyParams);
                        bodyParams.clear();
                    }
                    break;
                default:
                    if (line.back() == '\r')
                        line.pop_back();
                    {
                        int pos = line.find('=', 2); // Smallest is "a = b", with '=' at index 2
                        bodyParams[line.substr(0, pos-1)] = line.substr(pos+2);
                    }
                    break;
            }
        }
        if (!bodyParams.empty())
            loadBody(bodyParams);
    } else {
        cLog::get()->write("Unable to open file " + filename, LOG_TYPE::L_ERROR);
    }
    cLog::get()->write("(system " + englishName + " loaded)", LOG_TYPE::L_INFO);
	cLog::get()->mark();
}

void ModularSystem::applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param)
{
    if (createInfo.englishName == "Earth") {
        createInfo.bodyType = BodyType::EARTH;
    } else if (createInfo.englishName == "Moon") {
        createInfo.bodyType = BodyType::EARTH_MOON;
    }
}
