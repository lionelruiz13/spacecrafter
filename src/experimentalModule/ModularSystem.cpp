#include "ModularSystem.hpp"
#include "ModuleLoaderMgr.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"
#include "tools/context.hpp"
#include "meshModules/bodyShaderInterface.hpp" // MAX_SHADOW_CASTERS_PER_RECEIVER
#include <algorithm>

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

// Shadow orchestration - the WHICH half of G7 (class comment + shadow-paths.md
// B2; every ported formula carries its old-path line reference).
void ModularSystem::computeShadows(Renderer &renderer)
{
    ShadowService &service = renderer.shadow;
    if (!ShadowService::enabled || !star || star->distance == 0)
        return;
    service.ensureInit(renderer);
    if (!service)
        return;
    const Vec3f L = ModularBody::getLightPosition();
    const float sunRadius = star->getRadius();
    // Caster candidates: one pass over the system (bodies whose modules
    // declare a PROJECT trait; MINOR_BODY and light sources exempt).
    constexpr uint32_t PROJECT_MASK = BMT_PROJECT_G1_SHADOW | BMT_PROJECT_G8_SHADOW | BMT_PROJECT_BISHADOW;
    static std::vector<ModularBody *> casters; // scratch, system-draw scoped
    casters.clear();
    for (ModularBody *body : sortedSystemBodies) {
        if (!body || body->distance == 0 || body->isStar() || body->bodyType == BodyType::MINOR_BODY)
            continue;
        uint32_t traits = 0;
        for (auto *m : body->nearComponents)
            traits |= m->getTraits();
        if (traits & PROJECT_MASK)
            casters.push_back(body);
    }
    if (casters.empty())
        return;
    struct Pair {
        ModularBody *caster;
        float smooth;   // penumbra growth radius at the receiver (AU)
        float rank;     // casterRadius / smooth - occlusion ordering key
    };
    static std::vector<Pair> pairs; // scratch
    for (ModularBody *body : sortedSystemBodies) {
        if (!body || body->distance == 0)
            break; // sorted: unevaluated bodies are at the tail (drawSystem rule)
        // Receiver gate: drawn this frame (the ModularBody::draw entry test),
        // not MINOR/light-source. NOT gated by screen size beyond drawing:
        // the occlusion criterion below is the shadow-relevance gate [vixy].
        if (!(*body && body->screenSize > 0.0015f) || body->isStar() || body->bodyType == BodyType::MINOR_BODY)
            continue;
        body->receivedShadows.clear();
        const Vec3f rpos = body->getObservedPosition();
        const Vec3f v1 = rpos - L;
        const float sd1 = v1.lengthSquared();
        const float r1 = body->getRadius();
        // Light-cylinder corridor test, ported (solarsystem_display.cpp:85-134).
        const float cst1 = r1 + sunRadius;
        const float cst2 = -sunRadius / sd1;
        pairs.clear();
        for (ModularBody *caster : casters) {
            if (caster == body)
                continue;
            const Vec3f v2 = caster->getObservedPosition() - L;
            const float d = v1.dot(v2);
            if (d > 0 && d < sd1) {
                const float corridor = cst1 + d * cst2 + caster->getRadius();
                if (((v2 - v1 * (d / sd1)) / corridor).lengthSquared() < 1) {
                    // Penumbra growth radius (solarsystem_display.cpp:189-194):
                    // sunRadius * axialDist / |v1| == sunCoef * distToMainBody.
                    const float smooth = sunRadius * (sd1 - d) / sd1;
                    // Peak-occlusion >= 1/16 gate [vixy]: penumbra within 4x
                    // caster radius (the old `smoothRadius < bounding*4`).
                    if (smooth < caster->getRadius() * 4)
                        pairs.push_back({caster, smooth, caster->getRadius() / smooth});
                }
            }
        }
        if (pairs.empty())
            continue;
        // Occlusion-ordered: what the pool/shader caps drop is least visible.
        std::sort(pairs.begin(), pairs.end(), [](const Pair &a, const Pair &b) {
            return a.rank > b.rank;
        });
        // Receiver sun frame - world-tied basis (camera-independence:
        // ShadowProjection.hpp header): z along light->receiver, x from the
        // receiver's spin axis (mat column 2; column 1 fallback near
        // degeneracy), y completing.
        const Vec3f z = v1 / sqrtf(sd1);
        Vec3f axis(body->mat.r[8], body->mat.r[9], body->mat.r[10]);
        Vec3f x = axis ^ z;
        if (x.lengthSquared() < 1e-8f) {
            axis = Vec3f(body->mat.r[4], body->mat.r[5], body->mat.r[6]);
            x = axis ^ z;
        }
        x.normalize();
        const Vec3f y = z ^ x;
        body->receivedShadows.row0 = Vec4f(x[0], x[1], x[2], -x.dot(rpos));
        body->receivedShadows.row1 = Vec4f(y[0], y[1], y[2], -y.dot(rpos));
        for (const Pair &p : pairs) {
            if (body->receivedShadows.entries.size() >= MAX_SHADOW_CASTERS_PER_RECEIVER)
                break; // receiver shader array cap - aligned with the budget
            ModularBody *caster = p.caster;
            const Vec3f cpos = caster->getObservedPosition();
            // Cache key: CASTER-LOCAL light direction (columns of the
            // orthonormal rotation part dot the vector = transpose apply) -
            // the old heliocentric getLocalSunDirection equivalent,
            // camera-independent by construction.
            const Vec3f toLight = L - cpos;
            const Vec3f lightDirLocal(
                caster->mat.r[0] * toLight[0] + caster->mat.r[1] * toLight[1] + caster->mat.r[2] * toLight[2],
                caster->mat.r[4] * toLight[0] + caster->mat.r[5] * toLight[1] + caster->mat.r[6] * toLight[2],
                caster->mat.r[8] * toLight[0] + caster->mat.r[9] * toLight[1] + caster->mat.r[10] * toLight[2]);
            const float size = caster->getRadius() + p.smooth;
            // Blur radius in shadow-map pixels (body.cpp:1227 + the
            // drawShadower *halfShadowRes fold).
            const float radiusPx = p.smooth / size * Context::instance->shadowRes * 0.5f;
            bool reused = false;
            const int idx = service.acquire(caster, radiusPx, lightDirLocal, &reused);
            if (idx < 0)
                continue; // pool exhausted or blur bank still building (both logged/transient)
            if (!reused) {
                // Silhouette matrix: rows(x,y,z) . rot3(caster->mat) .
                // diag(r, r, r*(1-oblateness)) / size - maps the caster mesh
                // into shadow-map NDC (old: lookAt*model*scaling mat3,
                // body.cpp:1222-1230, same algebra in the eye frame).
                Mat4f frame = Mat4f::identity(); // off-cells MUST be zero
                frame.r[0] = x[0]; frame.r[4] = x[1]; frame.r[8] = x[2];
                frame.r[1] = y[0]; frame.r[5] = y[1]; frame.r[9] = y[2];
                frame.r[2] = z[0]; frame.r[6] = z[1]; frame.r[10] = z[2];
                Mat4f rot = caster->mat;
                rot.r[12] = rot.r[13] = rot.r[14] = 0;
                const float s = caster->getRadius() / size;
                const Mat4f sil = frame * rot * Mat4f::scaling(Vec3f(s, s, s * caster->getOneMinusOblateness()));
                for (auto *m : caster->nearComponents) {
                    if (m->getTraits() & PROJECT_MASK)
                        m->drawShadow(renderer, caster, sil, idx);
                }
            }
            body->receivedShadows.entries.push_back({caster,
                {x.dot(cpos - rpos), y.dot(cpos - rpos)},
                size, static_cast<uint8_t>(idx), caster->getShadowAbsorbtion()});
        }
    }
}

void ModularSystem::drawSystem(Renderer &renderer)
{
    computeShadows(renderer);
    for (auto &module : inComponents)
        module->draw(renderer, this, mat);
    renderer.beginBodyDraw();
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        if ((**pos).distance == 0)
            break; // Skip bodies not evaluated on update
        (**pos).draw(renderer);
    }
    // Selection pointer (S2b): queued here, recorded by endBodyDraw on top of
    // everything (old path drew it after the whole system). Independent of the
    // body draw loop by design - the pointer's purpose is exactly the bodies
    // too small to reach their own draw call (halo-only). Gates here: the
    // selected body belongs to THIS system; screen state fresh (visible =>
    // update ran this frame); center inside the viewport disk (the old
    // projectEarthEqu screen test). Policy rules (visibility flag, 10%
    // suppression, breathing, resolution scale) live in the service.
    if (ModularBody *sel = ModularBody::getSelected()) {
        if (systemOf(sel) == this && *sel) {
            const auto &p = sel->getScreenPos();
            if (p.first*p.first + p.second*p.second <= 1.f) {
                // px full diameter = screenSize * 2 * viewportRadius (the old
                // getOnScreenSize form - drawHalo screen_r note)
                renderer.drawPointer(p, sel->getScreenSize() * 2.f * ModularBody::viewportRadius);
            }
        }
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

    ModularBody *parent = this;
    if (parentName.empty()) {
        cLog::get()->write("No parent specified for " + englishName + ", assume parent is " + this->englishName + " (Specify 'none' to suppress this warning)", LOG_TYPE::L_WARNING);
    } else if (parentName != "none") {
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
            .period=Utility::strToFloat(param["rot_periode"], Utility::strToFloat(param["orbit_period"], 24.f))/24.f,
            .offset=Utility::strToFloat(param["rot_rotation_offset"],0.),
            .epoch=Utility::strToDouble(param["rot_epoch"], J2000),
            .obliquity=rot_obliquity,
            .ascendingNode=rot_asc_node,
            .precessionRate=Utility::strToFloat(param["rot_precession_rate"],0.)*static_cast<float>(M_PI/(180*36525)),
            .sidereal_period=Utility::strToDouble(param["orbit_visualization_period"],0.),
            .axialTilt=Utility::strToFloat(param["axial_tilt"], 0.)
        },
        .haloColor=param["color"].empty() ? defaultHaloColor : Utility::strToVec3f(param["color"]),
        .albedo=Utility::strToFloat(param["albedo"]),
        .radius=radius/static_cast<float>(AU),
        //.innerRadius=Utility::strToFloat(param["min_distance"], radius*1.002f)/static_cast<float>(AU),
        .oblateness=Utility::strToFloat(param["oblateness"], 0.0),
        .solLocalDay=Utility::strToFloat(param["sol_local_day"],1.0),

        .shadowAbsorbtion=(param.find("shadow_color") != param.end()) ? Utility::strToVec3f(param["shadow_color"]) : Vec3f{1, 1, 1},
        .brightness=Utility::strToFloat(param["brightness"], 0.0),

        .bodyType=strToBodyType(param["type"]),
        .isHaloEnabled=Utility::isTrue(param["halo"]),
    .altitudeRelativeToRadius=Utility::isFalse(param["solid"])
    };
	if (!createInfo.orbit) {
		cLog::get()->write("Invalid orbit '" + param["coord_func"] + "' for body '" + englishName + "', skip loading this body.", LOG_TYPE::L_ERROR);
		return;
	}

    // Old-path parity: the old path applies Earth/Moon specificities keyed on
    // (type, name) with no data gate (solarsystem.cpp SolarSystem::addBody), and
    // standard ssystem.ini carries no "hardcoded" key. Key absent -> parity
    // default (apply); key present -> explicit data wins (false blocks).
    // Position-layer consequence when skipped: Earth falls back to the generic
    // rot formula instead of apparent sidereal time -> observer placed ~49 deg
    // off in longitude (measured, harness 2026-07-11). Name-keying remains the
    // acknowledged quarantine (INTENT.md 5.5).
    {
        const auto hardcodedIt = param.find("hardcoded");
        if (hardcodedIt == param.end() || Utility::isTrue(hardcodedIt->second))
            applyHardcodedContent(createInfo, param);
    }
    ModularBody *body = parent->createChild(createInfo);
    // Binary-orbit completion (EMB class): if this body is the declared
    // secondary of its parent's BinaryOrbit, wire its orbit in - without this
    // the primary sits at the BARYCENTER (old/new Earth delta confirmed as
    // exactly -ratio*moon_ecl, harness 2026-07-11). Generic: any body pair, the
    // pairing is declared by the primary's orbit loader/data, not hardcoded here.
    if (auto *binary = dynamic_cast<BinaryOrbit *>(parent->orbit.get())) {
        if (!binary->hasSecondaryOrbit() && binary->getSecondaryName() == englishName) {
            cLog::get()->write("Adding " + englishName + " to " + parent->getEnglishName() + " binary orbit.", LOG_TYPE::L_INFO);
            binary->setSecondaryOrbit(body->orbit.get());
        }
    }
    if (Utility::isTrue(param["bound_to_surface"]))
        body->boundToSurface = true;
    if (Utility::isTrue(param["hidden"]))
        body->hide();
    // star is initialized to the SYSTEM itself (valid light-position default
    // for starless systems), so "unassigned" is star == this, NOT !star - the
    // old !star test was dead and the Sun never became the star (its radius
    // stayed 0 -> zero penumbra, found by the S5 fidelity probe).
    if (Utility::isTrue(param["system_star"]) || (star == this && body->isStar() && parent == this))
        star = body;
    for (auto moduleType : body->deduceBodyModuleList(param))
        ModuleLoaderMgr::instance.loadModule(moduleType, body, param);
    body->updateCache(); // Ensure bounding radius are properly set
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
        // Earth's shadow absorbs G/B more than R - red light diffracted by
        // the atmosphere reaches the umbra (lunar-eclipse color; replaces the
        // old my_moon UmbraColor hardcode). VALUE IS DERIVED, not tuned
        // [visual-fidelity mandate, vixy 2026-07-12]: old composition
        // diffuse*(s + U*(1-s)) with U = UmbraColor(0.4, 0.12, 0) equals the
        // new diffuse*(1 - cov*a) EXACTLY under a = 1-U, cov = 1-s - so
        // {0.6, 0.88, 1.0} reproduces the old model across the whole
        // penumbra; the only residual is coverage-profile shape
        // (shadow-paths.md D2). Explicit shadow_color in the data still wins
        // (loadBody reads it after this call).
        if (param.find("shadow_color") == param.end())
            createInfo.shadowAbsorbtion = Vec3f(0.6f, 0.88f, 1.0f);
    } else if (createInfo.englishName == "Moon") {
        createInfo.bodyType = BodyType::EARTH_MOON;
    }
}
