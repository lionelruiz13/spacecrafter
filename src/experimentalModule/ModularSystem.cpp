#include "ModularSystem.hpp"
#include "ModularSystemFormat.hpp"
#include "ModuleLoader.hpp" // reroute (composed relation= override)
#include "ModuleLoaderMgr.hpp"
#include "environmentModules/LandscapeEnv.hpp"
#include "environmentModules/AtmosphereEnv.hpp"
#include "tools/ini_line.hpp" // the ONE .ini line grammar (INTENT S5.39/D29)
#include "tools/log.hpp"
#include "tools/sc_const.hpp"
#include "tools/context.hpp"
#include "meshModules/bodyShaderInterface.hpp" // MAX_SHADOW_CASTERS_PER_RECEIVER
#include "bodyModules/OrbitModule.hpp" // orbit-pass activity gate (row 8)
#include "bodyModules/TrailModule.hpp" // trail-pass activity gate (row 9)
#include "bodyModules/TailModule.hpp"  // tail-pass activity gate (row 12)
#include <algorithm>
#include <set>    // the names a save target already declares
#include <cfloat> // FLT_MAX (within-body pair rank)
#include <cmath>  // std::sin/cos/atan2 (rot_pole_w0 -> offset conversion), isfinite
#include <sstream> // a float as a file value (fileValue)
#include "EntityCore/Core/VulkanMgr.hpp" // G8-budget overflow log

// Same mapping as the old parse (protosystem.cpp setAtmosphere) - retires
// with the old path; kept local until then (two paths, one data format).
static ATMOSPHERE_MODEL parseAtmosphereModel(const std::string &atmModel)
{
	if (atmModel=="earth_model") return ATMOSPHERE_MODEL::EARTH_MODEL;
	if (atmModel=="venus_model") return ATMOSPHERE_MODEL::VENUS_MODEL;
	if (atmModel=="mars_model") return ATMOSPHERE_MODEL::MARS_MODEL;
	return ATMOSPHERE_MODEL::NONE_MODEL;
}

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

static SiderealTimeModel parseSiderealTimeModel(const std::string &value, const std::string &bodyName)
{
    if (value.empty() || value == "generic")
        return SiderealTimeModel::GENERIC;
    if (value == "earth_apparent")
        return SiderealTimeModel::EARTH_APPARENT;
    cLog::get()->write("Body '" + bodyName + "': unknown sidereal_time = '" + value
        + "'. Valid values are 'generic' (the analytic (jd - epoch)/period spin) and "
        "'earth_apparent' (apparent sidereal time, with nutation - Earth's model). "
        "Falling back to 'generic'. To fix: set sidereal_time to one of those, or "
        "remove it.", LOG_TYPE::L_ERROR);
    return SiderealTimeModel::GENERIC;
}

static const std::string *authored(std::map<std::string, std::string> &param, const char *key)
{
    const auto it = param.find(key);
    return (it == param.end() || it->second.empty()) ? nullptr : &it->second;
}

static std::string fileValue(float v)
{
    std::ostringstream os;
    os << v;
    return os.str();
}

static void diagnose(ModularSystemFormat::Section *origin, const std::string &key,
    const char *reason, const std::string &text, LOG_TYPE severity)
{
    cLog::get()->write(text, severity);
    if (origin)
        origin->annotate(key, reason, text);
}

static void logRetiredTypeCapability(const std::string &bodyName, const std::string &type,
    const char *key, const std::string &legacyValue, const std::string &actingValue,
    ModularSystemFormat::Section *origin = nullptr)
{
    diagnose(origin, "type", key,
        "Body '" + bodyName + "': the composed format does not read capabilities "
        "from type = '" + type + "' (type-as-identity is retired in this format - D14). A legacy "
        "file would give " + std::string(key) + " = " + legacyValue + " here; no " + key
        + " key is declared, so " + key + " = " + actingValue + " acts instead. To keep the legacy "
        "behaviour, declare " + key + " = " + legacyValue + " on this body.", LOG_TYPE::L_WARNING);
}

static SurfaceModel parseSurfaceModel(const std::string &value, const std::string &bodyName)
{
    if (value == "planet")
        return SurfaceModel::PLANET;
    if (value == "lunar")
        return SurfaceModel::LUNAR;
    cLog::get()->write("Body '" + bodyName + "': unknown surface_model = '" + value
        + "'. Valid values are 'planet' (the earth/planet surface-shader lineage: night side, "
        "specular, bump combinations) and 'lunar' (the lunar lineage: tessellated heightmap "
        "displacement). Falling back to 'planet'. To fix: set surface_model to one of those, or "
        "remove it.", LOG_TYPE::L_ERROR);
    return SurfaceModel::PLANET;
}

static bool parseCapabilityFlag(const std::string &value, const std::string &bodyName, const char *key)
{
    if (Utility::isTrue(value))
        return true;
    if (Utility::isFalse(value))
        return false;
    cLog::get()->write("Body '" + bodyName + "': " + std::string(key) + " = '" + value
        + "' is not a boolean. Valid values are true/on/1 and false/off/0. Falling back to false. "
        "To fix: set " + key + " to a boolean, or remove it.", LOG_TYPE::L_ERROR);
    return false;
}

ModularSystem::ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info) :
    ModularBody(parent, info), star(this)
{
    isNotIsolated = false;
    const bool datumDefaulted = (info.datumRadius < 0.f);
    const bool groundDefaulted = (info.groundRadius < 0.f);
    if (datumDefaulted)
        datumRadius = 0.f;
    if (groundDefaulted)
        groundRadius = 0.f;
    if ((datumDefaulted || groundDefaulted) && radius != 0.f) {
        cLog::get()->write("System '" + englishName + "' uses centre-relative navigation "
            "by class default (datum_radius = ground_radius = 0): free-mode altitude is "
            "measured from its centre and free descent reaches the centre. To override, set "
            "datum_radius/ground_radius in data or run 'body name " + englishName
            + " datum_radius <km>'.", LOG_TYPE::L_INFO);
    }
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
    if (ModularBody *s = getSystemStar()) {
        s->useNow();
        s->updateAsLightSource();
    }
    if (ModularBody *sel = ModularBody::getSelected())
        sel->useNow();
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

void ModularSystem::computeShadows(Renderer &renderer)
{
    ShadowService &service = renderer.shadow;
    // getSystemStar: the star==this unassigned sentinel must not act as a
    // light source (starless galaxy/universe systems have no shadows).
    ModularBody *systemStar = getSystemStar();
    if (!ShadowService::enabled || !systemStar || systemStar->distance == 0)
        return;
    service.ensureInit(renderer);
    if (!service)
        return;
    const Vec3f L = ModularBody::getLightPosition();
    const float sunRadius = systemStar->getRadius();
    {
        ModularBody *best = nullptr;
        BodyModule *bestModule = nullptr;
        float bestImportance = 0;
        for (ModularBody *body : sortedSystemBodies) {
            if (!body || body->distance == 0)
                break; // sorted: unevaluated tail
            if (!(*body && body->screenSize > earlyVisibilityGate()) || body->isStar() || body->isMinorBody())
                continue;
            for (auto *m : body->nearComponents) {
                if (m->getTraits() & (BMT_BASIC_SELF_SHADOW | BMT_RGBA8_SELF_SHADOW)) {
                    const float importance = body->screenSize / body->distance;
                    if (importance > bestImportance) {
                        bestImportance = importance;
                        best = body;
                        bestModule = m;
                    }
                    break; // one nomination per body; first such module carries it
                }
            }
        }
        if (best) {
            Vec3f zs = L - best->getObservedPosition();
            zs.normalize();
            Vec3f axis(best->mat.r[8], best->mat.r[9], best->mat.r[10]);
            Vec3f xs = axis ^ zs;
            if (xs.lengthSquared() < 1e-8f) {
                axis = Vec3f(best->mat.r[4], best->mat.r[5], best->mat.r[6]);
                xs = axis ^ zs;
            }
            xs.normalize();
            const Vec3f ys = zs ^ xs;
            Mat4f frame = Mat4f::identity(); // off-cells MUST be zero
            frame.r[0] = xs[0]; frame.r[4] = xs[1]; frame.r[8] = xs[2];
            frame.r[1] = ys[0]; frame.r[5] = ys[1]; frame.r[9] = ys[2];
            frame.r[2] = zs[0]; frame.r[6] = zs[1]; frame.r[10] = zs[2];
            Mat4f rot = best->mat.multiplyFast(best->computeBodyToSurface());
            rot.r[12] = rot.r[13] = rot.r[14] = 0;
            bestModule->drawSelfShadow(renderer, best, frame * rot);
        }
    }
    // Caster candidates: one pass over the system (modules declaring a
    // PROJECT trait; MINOR_BODY and light sources exempt).
    constexpr uint32_t PROJECT_MASK = BMT_PROJECT_G1_SHADOW | BMT_PROJECT_G8_SHADOW;
    struct Caster {
        ModularBody *body;
        BodyModule *module;
        uint32_t traits;   // this module's PROJECT_* bit(s) - selects budget/word
        ShadowCaster info; // module vocabulary: silhouette radius, absorbtion, clip
    };
    static std::vector<Caster> casters; // scratch, system-draw scoped
    casters.clear();
    for (ModularBody *body : sortedSystemBodies) {
        if (!body || body->distance == 0 || body->isStar() || body->isMinorBody())
            continue;
        for (auto *m : body->nearComponents) {
            const uint32_t traits = m->getTraits();
            if (traits & PROJECT_MASK)
                casters.push_back({body, m, traits, m->getShadowCaster(body, L)});
        }
    }
    if (casters.empty())
        return;
    constexpr int MAX_G8_PROJECTIONS = 10;
    int g8Remaining = MAX_G8_PROJECTIONS;
    bool g8Logged = false;
    struct Pair {
        const Caster *caster;
        float smooth;   // penumbra growth radius at the receiver (AU)
        float rank;     // casterRadius / smooth - occlusion ordering key
    };
    static std::vector<Pair> pairs; // scratch
    for (ModularBody *body : sortedSystemBodies) {
        if (!body || body->distance == 0)
            break; // sorted: unevaluated bodies are at the tail (drawSystem rule)
        if (!(*body && body->screenSize > earlyVisibilityGate()) || body->isStar() || body->isMinorBody())
            continue;
        body->receivedShadows.clear();
        // ... and only surfaces that SAMPLE: a body with no BMT_RECEIVE_SHADOW
        // module would get entries nothing consumes (dead fills).
        uint32_t recvTraits = 0;
        for (auto *m : body->nearComponents)
            recvTraits |= m->getTraits();
        if (!(recvTraits & BMT_RECEIVE_SHADOW))
            continue;
        const Vec3f rpos = body->getObservedPosition();
        const Vec3f v1 = rpos - L;
        const float sd1 = v1.lengthSquared();
        const float r1 = body->getRadius();
        // Light-cylinder corridor test, ported (solarsystem_display.cpp:85-134).
        const float cst1 = r1 + sunRadius;
        const float cst2 = -sunRadius / sd1;
        pairs.clear();
        for (const Caster &c : casters) {
            if (c.body == body) {
                for (auto *m : body->nearComponents) {
                    if (m != c.module && (m->getTraits() & BMT_RECEIVE_SHADOW)) {
                        pairs.push_back({&c, 0.f, FLT_MAX});
                        break;
                    }
                }
                continue;
            }
            const Vec3f v2 = c.body->getObservedPosition() - L;
            const float d = v1.dot(v2);
            if (d > 0 && d < sd1) {
                const float corridor = cst1 + d * cst2 + c.info.radius;
                if (((v2 - v1 * (d / sd1)) / corridor).lengthSquared() < 1) {
                    // Penumbra growth radius (solarsystem_display.cpp:189-194):
                    // sunRadius * axialDist / |v1| == sunCoef * distToMainBody.
                    const float smooth = sunRadius * (sd1 - d) / sd1;
                    // Peak-occlusion >= 1/16 gate [vixy]: penumbra within 4x
                    // caster radius (the old `smoothRadius < bounding*4`).
                    if (smooth < c.info.radius * 4)
                        pairs.push_back({&c, smooth, c.info.radius / smooth});
                }
            }
        }
        if (pairs.empty())
            continue;
        // Occlusion-ordered: what the pool/shader caps drop is least visible.
        std::sort(pairs.begin(), pairs.end(), [](const Pair &a, const Pair &b) {
            return a.rank > b.rank;
        });
        const Vec3f z = v1 / sqrtf(sd1);
        Vec3f axis(body->mat.r[8], body->mat.r[9], body->mat.r[10]);
        Vec3f x = axis ^ z;
        if (x.lengthSquared() < 1e-8f) {
            axis = Vec3f(body->mat.r[4], body->mat.r[5], body->mat.r[6]);
            x = axis ^ z;
        }
        x.normalize();
        const Vec3f y = z ^ x;
        const Vec4f row0(x[0], x[1], x[2], -x.dot(rpos));
        const Vec4f row1(y[0], y[1], y[2], -y.dot(rpos));
        for (const Pair &p : pairs) {
            if (body->receivedShadows.entries.size() >= MAX_SHADOW_CASTERS_PER_RECEIVER)
                break; // receiver shader array cap - aligned with the budget
            const Caster &c = *p.caster;
            if ((c.traits & BMT_PROJECT_G8_SHADOW) && g8Remaining == 0) {
                if (!g8Logged) {
                    g8Logged = true;
                    VulkanMgr::instance->putLog("ModularSystem: G8 projection budget (10) exhausted - least significant greyscale shadows dropped this frame", LogType::WARNING);
                }
                continue;
            }
            ModularBody *caster = c.body;
            const Vec3f cpos = caster->getObservedPosition();
            const Vec3f toLight = L - cpos;
            const Vec3f lightDirLocal(
                caster->mat.r[0] * toLight[0] + caster->mat.r[1] * toLight[1] + caster->mat.r[2] * toLight[2],
                caster->mat.r[4] * toLight[0] + caster->mat.r[5] * toLight[1] + caster->mat.r[6] * toLight[2],
                caster->mat.r[8] * toLight[0] + caster->mat.r[9] * toLight[1] + caster->mat.r[10] * toLight[2]);
            const float size = c.info.radius + p.smooth;
            // Blur radius in shadow-map pixels (body.cpp:1227 + the
            // drawShadower *halfShadowRes fold).
            const float radiusPx = p.smooth / size * Context::instance->shadowRes * 0.5f;
            bool reused = false;
            const int idx = service.acquire(caster, c.module, radiusPx, lightDirLocal, &reused);
            if (idx < 0)
                continue; // pool exhausted or blur bank still building (both logged/transient)
            if (!reused) {
                Mat4f frame = Mat4f::identity(); // off-cells MUST be zero
                frame.r[0] = x[0]; frame.r[4] = x[1]; frame.r[8] = x[2];
                frame.r[1] = y[0]; frame.r[5] = y[1]; frame.r[9] = y[2];
                frame.r[2] = z[0]; frame.r[6] = z[1]; frame.r[10] = z[2];
                Mat4f rot = caster->mat;
                rot.r[12] = rot.r[13] = rot.r[14] = 0;
                const float s = c.info.radius / size;
                const Mat4f sil = frame * rot * Mat4f::scaling(Vec3f(s, s, s * caster->getOneMinusOblateness()));
                c.module->drawShadow(renderer, caster, sil, idx); // ONLY this module - one layer per (body, module)
            }
            if (c.traits & BMT_PROJECT_G8_SHADOW)
                --g8Remaining;
            const bool graded = (c.traits & BMT_PROJECT_G8_SHADOW) != 0;
            const Vec3f aT = graded ? c.info.absorbtion : Vec3f(1.f, 1.f, 1.f);
            const Vec3f gR = graded ? Vec3f(0.f, 0.f, 0.f)
                                    : Vec3f(1.f - c.info.absorbtion[0],
                                            1.f - c.info.absorbtion[1],
                                            1.f - c.info.absorbtion[2]);
            Vec4f clip = c.info.clip;
            if (caster == body && clip[0] == 0 && clip[1] == 0 && clip[2] == 0) {
                clip = Vec4f(-z[0], -z[1], -z[2], z.dot(rpos));
            }
            body->receivedShadows.entries.push_back({caster, c.module,
                {x.dot(cpos - rpos), y.dot(cpos - rpos)},
                size, static_cast<uint8_t>(idx), aT, gR, clip,
                row0, row1});
        }
    }
}

void ModularSystem::drawSystemBodies(Renderer &renderer)
{
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // Skip bodies not evaluated on update
        if (body.isSystem()) {
            // A nested system draws its content (or its star point) at its
            // own sort position - G2's systems-as-bodies at draw time.
            static_cast<ModularSystem &>(body).drawNested(renderer);
        } else {
            body.draw(renderer);
        }
    }
}

void ModularSystem::drawOrbits(Renderer &renderer)
{
    // Skip the whole pass (trace sweep + depth clear + line sweep) when no
    // orbit is shown or fading - the default (orbits off) pays nothing.
    if (!OrbitModule::anyActive())
        return;
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    renderer.beginOrbitTrace();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // unevaluated tail (drawSystem rule)
        if (!body)
            continue; // operator bool = on-screen (old isVisibleOnScreen)
        const Mat4f traceMat = body.getMat().multiplyFast(body.computeBodyToSurface());
        for (auto *m : body.nearComponents)
            if (m->getTraits() & BMT_DEPTH_TRACE)
                m->drawTrace(renderer, &body, traceMat);
    }
    renderer.beginOrbitLines();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break;
        if (body.orbitComponents.empty())
            continue;
        if (!body.getParent())
            continue; // a parentless body has no orbit to draw around
        Mat4f parentFrame = body.getMatLocalToBodyPos();
        parentFrame.multiplyTranslation(-body.getDisplayEclipticPos());
        for (auto *m : body.orbitComponents) {
            m->update(&body, body.getScaledRadius());
            m->draw(renderer, &body, parentFrame);
        }
    }
}

void ModularSystem::drawTrails(Renderer &renderer)
{
    if (!TrailModule::anyActive())
        return;
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    renderer.beginTrailDraw();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // unevaluated tail (drawSystem rule; a G4-culled subsystem)
        if (body.trailComponents.empty())
            continue;
        if (!body.getParent())
            continue; // parentless: no parent frame to draw the trail in
        Mat4f parentFrame = body.getMatLocalToBodyPos();
        parentFrame.multiplyTranslation(-body.getDisplayEclipticPos());
        for (auto *m : body.trailComponents) {
            m->update(&body, body.getScaledRadius());
            m->draw(renderer, &body, parentFrame);
        }
    }
}

void ModularSystem::drawTails(Renderer &renderer)
{
    if (!TailModule::anyActive())
        return;
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    renderer.beginTailDraw();
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // unevaluated tail (drawSystem rule)
        if (body.tailComponents.empty())
            continue;
        if (!body.getParent())
            continue; // parentless: no parent frame to place the tail in
        Mat4f parentFrame = body.getMatLocalToBodyPos();
        parentFrame.multiplyTranslation(-body.getDisplayEclipticPos());
        for (auto *m : body.tailComponents) {
            m->update(&body, body.getScaledRadius());
            m->draw(renderer, &body, parentFrame);
        }
    }
    // One instanced draw of every submitted tail (old Tail::endDraw + drawBatch).
    renderer.flushTails();
}

void ModularSystem::drawNested(Renderer &renderer)
{
    if (!isVisible)
        return;
    const float px = (distance > subsystemRadius)
        ? (atanf(subsystemRadius / sqrtf(distance*distance - subsystemRadius*subsystemRadius))
           / halfFov) * 2.f * getViewportRadius()
        : 2.f * getViewportRadius();
    if (px >= SYSTEM_VISIBILITY_SUBSYSTEM_SIZE) {
        const float savedAlpha = drawAlpha;
        const bool inBand = px < (SYSTEM_VISIBILITY_SUBSYSTEM_SIZE + SYSTEM_COLLAPSE_CROSSFADE_BAND);
        const float t = inBand
            ? (px - SYSTEM_VISIBILITY_SUBSYSTEM_SIZE) / SYSTEM_COLLAPSE_CROSSFADE_BAND
            : 1.f; // above the band: full resolved, drawAlpha stays savedAlpha
        drawAlpha = savedAlpha * t; // fade the interior IN (1.0 outside the band)
        {
            const Vec3f savedLightPos = lightPosition;
            const float savedLightDist = lightDistance;
            const float savedLightSize = lightSize;
            updateSystem(); // sort OUR list + set OUR star as light source
            computeShadows(renderer);
            drawSystemBodies(renderer);
            lightPosition = savedLightPos;
            lightDistance = savedLightDist;
            lightSize = savedLightSize;
        }
        if (inBand && isBodyVisible) {
            drawAlpha = savedAlpha * (1.f - t); // fade the proxy dot OUT
            drawStarProxy(renderer);
        }
        drawAlpha = savedAlpha;
    } else if (isBodyVisible) {
        drawStarProxy(renderer);
    }
}

void ModularSystem::drawStarProxy(Renderer &renderer)
{
    ModularBody *s = getSystemStar();
    if (!s)
        return;
    const float mag = -26.73f + 2.5f * log10f(distance * distance);
    const float starScreenR = (s->getScaledRadius() / (distance * halfFov)) * 2.f * viewportRadius;
    drawHaloCore(renderer, mag, starScreenR, s->getHaloColor(), false);
}

void ModularSystem::drawSystem(Renderer &renderer)
{
    computeShadows(renderer);
    for (auto &module : inComponents)
        module->draw(renderer, this, mat);
    renderer.beginBodyDraw();
    drawSystemBodies(renderer);
    drawTails(renderer);
    drawTrails(renderer);
    drawOrbits(renderer);
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

namespace {
enum class RotFrame { PARENT_RELATIVE, ABSOLUTE_POLE };

RotFrame resolveRotationFrame(std::map<std::string, std::string> &param,
                              const std::string &englishName,
                              float &rot_obliquity, float &rot_asc_node,
                              float &rot_offset)
{
    rot_obliquity = Utility::strToFloat(param["rot_obliquity"], 0.) * M_PI / 180.;
    rot_asc_node  = Utility::strToFloat(param["rot_equator_ascending_node"], 0.) * M_PI / 180.;
    rot_offset = Utility::strToFloat(param["rot_rotation_offset"], 0.);

    const std::string &decl = param["rot_frame"];
    const bool hasPole = (param["rot_pole_ra"] != "" || param["rot_pole_de"] != "");
    const RotFrame derived = hasPole ? RotFrame::ABSOLUTE_POLE : RotFrame::PARENT_RELATIVE;
    const char *derivedName = (derived == RotFrame::ABSOLUTE_POLE) ? "absolute_pole" : "parent_relative";

    RotFrame frame;
    if (decl.empty()) {
        frame = derived;
        cLog::get()->write("Body '" + englishName + "': rot_frame not declared, using derived default '"
            + derivedName + "' (in memory; not written - file write-back is B31).", LOG_TYPE::L_DEBUG);
    } else if (decl == "absolute_pole") {
        frame = RotFrame::ABSOLUTE_POLE;
    } else if (decl == "parent_relative") {
        frame = RotFrame::PARENT_RELATIVE;
    } else {
        frame = derived;
        cLog::get()->write("Body '" + englishName + "': invalid rot_frame = '" + decl
            + "'. Valid values are 'absolute_pole' (an absolute J2000-equatorial north pole in "
            "rot_pole_ra/rot_pole_de) or 'parent_relative' (rot_obliquity/rot_equator_ascending_node "
            "relative to the parent's equator). Falling back to the derived default '" + derivedName
            + "' (from the rotation keys present). To fix: set rot_frame to one of the valid values, "
            "or remove it to keep the derived default.", LOG_TYPE::L_ERROR);
    }

    if (frame == RotFrame::ABSOLUTE_POLE) {
        // J2000 equatorial pole -> ecliptic (VSOP87) obliquity/ascending node.
        // NB: north pole needs to be defined by right hand rotation rule.
        float J2000_npole_ra = Utility::strToFloat(param["rot_pole_ra"], 0.) * M_PI / 180.;
        float J2000_npole_de = Utility::strToFloat(param["rot_pole_de"], 0.) * M_PI / 180.;
        Vec3f J2000_npole;
        Utility::spheToRect(J2000_npole_ra, J2000_npole_de, J2000_npole);
        Vec3f vsop87_pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(J2000_npole));
        float ra, de;
        Utility::rectToSphe(&ra, &de, vsop87_pole);
        rot_obliquity = (M_PI_2 - de);
        rot_asc_node = (ra + M_PI_2);

        if (!param["rot_pole_w0"].empty()) {
            const float W0 = Utility::strToFloat(param["rot_pole_w0"], 0.) * M_PI / 180.;
            const Vec3f node(-std::sin(J2000_npole_ra), std::cos(J2000_npole_ra), 0.f);
            const Vec3f perp(J2000_npole ^ node);
            const Vec3f pm_icrf(node * std::cos(W0) + perp * std::sin(W0));
            const Vec3f pm(mat_j2000_to_vsop87.multiplyWithoutTranslation(pm_icrf));
            const Mat4f eqframe(Mat4f::xzrotation(rot_obliquity, rot_asc_node));
            const Vec3f ex(eqframe.multiplyWithoutTranslation(Vec3f(1, 0, 0)));
            const Vec3f ey(eqframe.multiplyWithoutTranslation(Vec3f(0, 1, 0)));
            rot_offset = std::atan2((pm * ey), (pm * ex)) * (180. / M_PI);
            if (rot_offset < 0.f)
                rot_offset += 360.f;
            cLog::get()->write("Body '" + englishName + "': rot_pole_w0 (IAU W0="
                + param["rot_pole_w0"] + " deg, from the ICRF-equator node) converted to "
                "rot_rotation_offset " + std::to_string(rot_offset)
                + " deg (ecliptic node; the azimuth the texture's centre column "
                "is drawn at).", LOG_TYPE::L_DEBUG);
        }
    } else if (!param["rot_pole_w0"].empty()) {
        cLog::get()->write("Body '" + englishName + "': rot_pole_w0 is set but the "
            "rotation frame is 'parent_relative' (no absolute pole). rot_pole_w0 is the "
            "IAU prime meridian, measured from the ICRF-equator node, and needs an "
            "absolute pole. Ignoring rot_pole_w0 and using rot_rotation_offset. To fix: "
            "add rot_pole_ra/rot_pole_de (+ rot_frame = absolute_pole), or remove "
            "rot_pole_w0.", LOG_TYPE::L_ERROR);
    }
    return frame;
}
} // namespace

void ModularSystem::loadBody(std::map<std::string, std::string> &param,
                             ModularSystemFormat::Section *origin,
                             bool supplemental)
{
    stringHash_t declared = param;
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

	float rot_obliquity, rot_asc_node, rot_offset;
	const bool absoluteTiltFrame =
		(resolveRotationFrame(param, englishName, rot_obliquity, rot_asc_node, rot_offset)
			== RotFrame::ABSOLUTE_POLE);

    const std::string &bodyTypeString = param["type"];
    // A6 - surface-lighting lineage (`surface_model`), consumed by
    // LayeredMeshLoader. Legacy source: type = Moon.
    SurfaceModel surfaceModel = SurfaceModel::PLANET;
    if (const std::string *v = authored(param, "surface_model")) {
        surfaceModel = parseSurfaceModel(*v, englishName);
    } else if (bodyTypeString == "Moon") {
        if (composedFile)
            logRetiredTypeCapability(englishName, bodyTypeString, "surface_model", "lunar", "planet", origin);
        else
            surfaceModel = SurfaceModel::LUNAR;
    }
    int trailLength = TRAIL_LENGTH_DEFAULT;
    if (const std::string *v = authored(param, "trail_length")) {
        trailLength = Utility::strToInt(*v, TRAIL_LENGTH_DEFAULT);
    } else {
        const int legacyTrailLength =
            (bodyTypeString == "Planet" || bodyTypeString == "Dwarf") ? 1460
            : (bodyTypeString == "Comet") ? 2920
            : TRAIL_LENGTH_DEFAULT;
        if (legacyTrailLength != TRAIL_LENGTH_DEFAULT) {
            if (composedFile)
                logRetiredTypeCapability(englishName, bodyTypeString, "trail_length",
                    std::to_string(legacyTrailLength), std::to_string(TRAIL_LENGTH_DEFAULT), origin);
            else
                trailLength = legacyTrailLength;
        }
    }
    BodyType bodyType;
    bool primary;
    const BodyType legacyBodyType = strToBodyType(bodyTypeString);
    const bool legacyStar = (legacyBodyType & BodyType::STAR) == BodyType::STAR;
    if (composedFile) {
        const std::string *lightSourceKey = authored(param, "light_source");
        const std::string *primaryKey = authored(param, "primary");
        const std::string *shadowExemptKey = authored(param, "shadow_exempt");
        const bool lightSource = lightSourceKey && parseCapabilityFlag(*lightSourceKey, englishName, "light_source");
        const bool shadowExempt = shadowExemptKey && parseCapabilityFlag(*shadowExemptKey, englishName, "shadow_exempt");
        primary = primaryKey && parseCapabilityFlag(*primaryKey, englishName, "primary");
        if (lightSource && shadowExempt) {
            diagnose(origin, "shadow_exempt", "capability-conflict",
                "Body '" + englishName + "': light_source and shadow_exempt are both "
                "declared, but a body cannot be both a light source and a shadow-exempt minor body "
                "in this engine (one BodyType tag carries both). light_source wins; shadow_exempt is "
                "ignored. To fix: remove one of the two keys.", LOG_TYPE::L_ERROR);
        }
        bodyType = lightSource ? BodyType::STAR
                 : shadowExempt ? BodyType::MINOR_BODY
                 : BodyType::CUSTOM_BODY;
        if (legacyStar) {
            if (!lightSourceKey)
                logRetiredTypeCapability(englishName, bodyTypeString, "light_source", "true", "false", origin);
            if (!primaryKey)
                logRetiredTypeCapability(englishName, bodyTypeString, "primary", "true", "false", origin);
        } else if (legacyBodyType == BodyType::MINOR_BODY && !shadowExemptKey) {
            logRetiredTypeCapability(englishName, bodyTypeString, "shadow_exempt", "true", "false", origin);
        }
    } else {
        bodyType = legacyBodyType;
        primary = legacyStar;
    }

    ModularBodyCreateInfo createInfo {
        .orbit=ModuleLoaderMgr::instance.loadOrbit(param),
        .englishName=englishName,
        .re={
            .period=Utility::strToFloat(param["rot_periode"], Utility::strToFloat(param["orbit_period"], 24.f))/24.f,
            .offset=rot_offset,   // raw rot_rotation_offset, or the rot_pole_w0 conversion (B28/S11.67 class)
            .epoch=Utility::strToDouble(param["rot_epoch"], J2000),
            .obliquity=rot_obliquity,
            .ascendingNode=rot_asc_node,
            .precessionRate=Utility::strToFloat(param["rot_precession_rate"],0.)*static_cast<float>(M_PI/(180*36525)),
            .sidereal_period=Utility::strToDouble(param["orbit_visualization_period"],0.),
            .axialTilt=Utility::strToFloat(param["axial_tilt"], 0.),
            .absoluteTiltFrame=absoluteTiltFrame
        },
        .haloColor=param["color"].empty() ? defaultHaloColor : Utility::strToVec3f(param["color"]),
        .albedo=Utility::strToFloat(param["albedo"]),
        .radius=radius/static_cast<float>(AU),
        .datumRadius=Utility::strToFloat(param["datum_radius"], radius)/static_cast<float>(AU),
        .groundRadius=Utility::strToFloat(param["ground_radius"], radius)/static_cast<float>(AU),
        .oblateness=Utility::strToFloat(param["oblateness"], 0.0),
        .solLocalDay=Utility::strToFloat(param["sol_local_day"],1.0),

        .shadowAbsorbtion=(param.find("shadow_color") != param.end()) ? Utility::strToVec3f(param["shadow_color"]) : Vec3f{1, 1, 1},
        .brightness=Utility::strToFloat(param["brightness"], 0.0),
        .siderealTimeModel=parseSiderealTimeModel(param["sidereal_time"], englishName),
        // B27 tail (A6/A7 + D14 format scope) - resolved above, one site.
        .surfaceModel=surfaceModel,
        .trailLength=trailLength,
        .composedDeclaration=composedFile,
        // B27 Tier B, the D27 split (S11.113(f)) - the structural half of what
        // the STAR bit used to carry, resolved above with everything else.
        .primary=primary,

        .bodyType=bodyType,
        .isHaloEnabled=Utility::isTrue(param["halo"]),
    };
	if (!createInfo.orbit) {
		cLog::get()->write("Invalid orbit '" + param["coord_func"] + "' for body '" + englishName + "', skip loading this body.", LOG_TYPE::L_ERROR);
		return;
	}

    {
        const auto hardcodedIt = param.find("hardcoded");
        if (!composedFile && (hardcodedIt == param.end() || Utility::isTrue(hardcodedIt->second)))
            applyHardcodedContent(createInfo, param);
    }
    BodyRelation rel;
    {
        const std::string &relDecl = param["relation"];
        const BodyRelation legacyRel = Utility::isTrue(param["bound_to_surface"])
            ? BodyRelation::GROUNDED : BodyRelation::ORBITING;
        if (relDecl.empty()) {
            rel = legacyRel;
        } else if (relDecl == "orbiting") {
            rel = BodyRelation::ORBITING;
        } else if (relDecl == "grounded") {
            rel = BodyRelation::GROUNDED;
        } else if (relDecl == "inner") {
            rel = BodyRelation::INNER;
        } else {
            rel = legacyRel;
            diagnose(origin, "relation", "invalid-value",
                "Body '" + englishName + "': invalid relation = '" + relDecl
                + "'. Valid values are 'orbiting' (standard satellite), 'grounded' (bound to the "
                "parent's surface - rover class) or 'inner' (inside the parent's volume, shown "
                "while the camera is inside the parent's AoI). Falling back to '"
                + (legacyRel == BodyRelation::GROUNDED ? "grounded" : "orbiting")
                + "' (from bound_to_surface). To fix: set relation to one of the valid values, "
                "or remove it to keep the bound_to_surface-derived default.", LOG_TYPE::L_ERROR);
        }
        if (!relDecl.empty() && !param["bound_to_surface"].empty()
                && (legacyRel == BodyRelation::GROUNDED) != (rel == BodyRelation::GROUNDED)) {
            diagnose(origin, "bound_to_surface", "contradicted-by-relation",
                "Body '" + englishName + "': relation = '" + relDecl
                + "' disagrees with bound_to_surface = '" + param["bound_to_surface"]
                + "'. The explicit relation wins; remove bound_to_surface to silence this "
                "(the two keys declare the same thing - keep one).", LOG_TYPE::L_ERROR);
        }
    }
    if (ModularBody *replaced = ModularBody::findBodyOnce(englishName))
        supplemental = replaced->supplemental;
    ModularBody *body = parent->createChild(createInfo, rel);
    body->supplemental = supplemental;
    body->declaredParams = std::move(declared);
    {
        const bool authoredSpin = !param["rot_periode"].empty();
        if (rel == BodyRelation::GROUNDED) {
            if (!authoredSpin)
                body->surfaceLockedAttitude = true;
        } else if (!authoredSpin && param["orbit_period"].empty()) {
            diagnose(origin, "rot_periode", "default-applied",
                "Body '" + englishName + "': no rotation period in "
                "the data (neither rot_periode nor orbit_period) - applying the "
                "legacy default rot_periode = 24 h, so this body rotates once every "
                "24 h. To author its rotation explicitly, set rot_periode = <hours> "
                "(or orbit_period for synchronous rotation).", LOG_TYPE::L_WARNING);
        }
    }
    if (auto *binary = dynamic_cast<BinaryOrbit *>(parent->orbit.get())) {
        if (!binary->hasSecondaryOrbit() && binary->getSecondaryName() == englishName) {
            cLog::get()->write("Adding " + englishName + " to " + parent->getEnglishName() + " binary orbit.", LOG_TYPE::L_INFO);
            binary->setSecondaryOrbit(body->orbit.get());
        }
    }
    {
        const std::string &declaredScale = param["display_scale"];
        if (!declaredScale.empty()) {
            const float s = Utility::strToFloat(declaredScale, 1.f);
            if (s > 0.f && std::isfinite(s)) {
                body->restoreScaling(s);
            } else {
                diagnose(origin, "display_scale", "invalid-value",
                    "Body '" + englishName + "': invalid display_scale = '" + declaredScale
                    + "'. It is a DISPLAY multiplier on this body's drawn size - a positive,"
                    " finite number, 1 meaning drawn at its true size (it changes nothing"
                    " physical: orbit, shadows and the model position stay as authored)."
                    " Falling back to 1. To fix: set display_scale to a positive number, or"
                    " remove it.", LOG_TYPE::L_ERROR);
            }
        }
    }
    if (Utility::isTrue(param["hidden"]))
        body->hide();
    body->captureAuthoredState();
    if (Utility::isTrue(param["system_star"]) || (star == this && body->isStar() && parent == this))
        star = body;
    if (!param["has_atmosphere"].empty() || !param["atmosphere_lim_landscape"].empty()) {
        body->envParams.hasAtmosphere = Utility::strToBool(param["has_atmosphere"], false);
        body->envParams.model = parseAtmosphereModel(param["atmosphere_model"]);
        body->envParams.limInf = Utility::strToFloat(param["atmosphere_lim_inf"], 40000.f);
        body->envParams.limSup = Utility::strToFloat(param["atmosphere_lim_sup"], 80000.f);
        body->envParams.limLandscape = Utility::strToFloat(param["atmosphere_lim_landscape"], 10000.f);
    }
    if (radius > 0)
        body->addEnvironment(std::make_unique<LandscapeEnv>(), true);
    if (body->envParams.hasAtmosphere)
        body->addEnvironment(std::make_unique<AtmosphereEnv>(), false);
    bool deduce = true;
    {
        const std::string &compose = param["compose"];
        if (compose == "explicit") {
            deduce = false;
        } else if (!compose.empty() && compose != "deduced") {
            diagnose(origin, "compose", "invalid-value",
                "Body '" + englishName + "': invalid compose = '" + compose
                + "'. Valid values are 'deduced' (modules deduced from this body's keys, the "
                "default) or 'explicit' (modules come only from BodyModule declarations). "
                "Falling back to 'deduced'. To fix: set compose to one of the valid values, "
                "or remove it.", LOG_TYPE::L_ERROR);
        }
    }
    if (deduce) {
        for (auto moduleType : body->deduceBodyModuleList(param))
            ModuleLoaderMgr::instance.loadModule(moduleType, body, param);
        if (Utility::isTrue(param["planet_grid"]))
            ModuleLoaderMgr::instance.loadModule(BodyModuleType::CUSTOM, body, param, "GRID");
    }
    body->updateCache(); // Ensure bounding radius are properly set
}

ModularBody *ModularSystem::findBodyAt(const std::pair<float, float> &searchPos) const
{
    const float tol = 30.f / ModularBody::viewportRadius;
    const float tol2 = tol * tol;
    ModularBody *inTolerance = nullptr;
    float biggest = -1.f;
    ModularBody *underDisc = nullptr;
    float nearest = 0.f;
    for (auto child : sortedSystemBodies) {
        // Entries are nulled by removeBody until the next cleanUp().
        if (!child || !*child)
            continue;
        const float dx = child->screenPos.first - searchPos.first;
        const float dy = child->screenPos.second - searchPos.second;
        const float squaredDistance = dx * dx + dy * dy;
        if (squaredDistance <= tol2) {
            if (child->screenSize > biggest) {
                biggest = child->screenSize;
                inTolerance = child;
            }
        } else if (squaredDistance <= child->screenSize * child->screenSize) {
            if (!underDisc || child->distance < nearest) {
                nearest = child->distance;
                underDisc = child;
            }
        }
    }
    return inTolerance ? inTolerance : underDisc;
}

void ModularSystem::loadSystem(const std::string &filename)
{
    std::ifstream file(filename);
    if (file) {
        systemFilename = filename;
        loadedSections.clear();
        stringHash_t bodyParams;
        std::string line, key, value;
        while (getline(file, line)) {
            switch (IniLine::read(line, key, value)) {
                case IniLine::Kind::SECTION:
                    if (!bodyParams.empty()) {
                        loadBody(bodyParams);
                        bodyParams.clear();
                    }
                    break;
                case IniLine::Kind::ENTRY:
                    bodyParams[key] = value;
                    break;
                case IniLine::Kind::MALFORMED:
                    cLog::get()->write("Ignoring line without '=' in " + filename + ": '"
                        + key + "' - a key/value line needs 'key = value'; write '#' first "
                        "to make it a comment.", LOG_TYPE::L_WARNING);
                    break;
                case IniLine::Kind::EMPTY:
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

void ModularSystem::loadComposedSystem(const std::string &filename)
{
    std::vector<ModularSystemFormat::Section> sections;
    if (!ModularSystemFormat::parse(filename, sections)) {
        cLog::get()->write("Unable to open file " + filename, LOG_TYPE::L_ERROR);
        return;
    }
    systemFilename = filename;
    composedFile = true;
    loadedSections = std::move(sections);
    std::map<std::string, stringHash_t> nodeParams;
    for (auto &section : loadedSections) {
        if (section.isPreamble())
            continue;
        stringHash_t params = section.params();
        bool isFamily;
        const BodyModuleType famType =
            ModuleLoaderMgr::moduleTypeFromName(params["type"], isFamily);
        if (isFamily) {
            loadDeclaredModule(params, section.getHeader(), nodeParams, famType, &section);
        } else if (params.count("body")) {
            const std::string &badType = params["type"];
            diagnose(&section, "type", "invalid-module-family",
                "Section '[" + section.getHeader() + "]' of " + filename + ": "
                + (badType.empty() ? std::string("missing the type key")
                                   : "invalid type = '" + badType + "'")
                + " for a module (it binds a body with body = '" + params["body"]
                + "'). Valid module families are: " + ModuleLoaderMgr::moduleTypeNames()
                + ". Declaration skipped. To fix: set type to the family to instantiate, or "
                "remove body= to declare a node instead.", LOG_TYPE::L_ERROR);
        } else {
            loadBody(params, &section);
            const std::string &name = params["name"];
            if (!name.empty())
                nodeParams[name] = params;
        }
    }
    cLog::get()->write("(system " + englishName + " loaded, composed format)", LOG_TYPE::L_INFO);
    cLog::get()->mark();
}

void ModularSystem::loadDeclaredModule(std::map<std::string, std::string> &params, const std::string &header,
                                       const std::map<std::string, std::map<std::string, std::string>> &nodeParams,
                                       BodyModuleType type, ModularSystemFormat::Section *origin)
{
    const std::string &bodyName = params["body"];
    ModularBody *body = bodyName.empty() ? nullptr : findBodyOnce(bodyName);
    if (!body) {
        diagnose(origin, "body", "unresolved-body",
            "BodyModule declaration '[" + header + "]': body = '" + bodyName
            + "' names no loaded body. A module's body must be declared EARLIER in the same file "
            "(or already exist). Declaration skipped. To fix: check the name, or move this "
            "section below its body's section.", LOG_TYPE::L_ERROR);
        return;
    }
    stringHash_t effective;
    {
        const auto it = nodeParams.find(bodyName);
        if (it != nodeParams.end())
            effective = it->second;
    }
    for (const auto &kv : params) {
        if (kv.first == "type" || kv.first == "body"
         || kv.first == "slot" || kv.first == "relation" || kv.first == "compose")
            continue;
        effective[kv.first] = kv.second;
    }
    const std::string &slotName = params["slot"];
    ModuleLoaderMgr::instance.loadModule(type, body, effective, slotName);
    const std::string &relation = params["relation"];
    if (!relation.empty()) {
        BodyModule *module = body->slot(ModularBody::slotID[
            slotName.empty() ? std::string(ModuleLoaderMgr::moduleTypeName(type)) : slotName]);
        // loadModule's no-capable-loader case already logged; only re-route
        // what was actually installed.
        if (module && !ModuleLoader::reroute(body, module, relation)) {
            diagnose(origin, "relation", "invalid-value",
                "BodyModule declaration '[" + header + "]': invalid relation = '"
                + relation + "'. Valid values are far, near, grounded, in, orbit, trail, tail "
                "(the routing lists - see ModuleLoader.hpp). The loader's own routing is kept. "
                "To fix: set relation to one of the valid values, or remove it.",
                LOG_TYPE::L_ERROR);
        }
    }
    body->updateCache(); // module set changed - bounding radius may have too
}

void ModularSystem::generateComposedTwin(const std::string &legacyFilename, const std::string &outPath)
{
    std::vector<ModularSystemFormat::Section> sections;
    if (!ModularSystemFormat::parse(legacyFilename, sections))
        return; // the legacy load already logged the unreadable file
    std::vector<ModularSystemFormat::Section> out;
    for (auto &section : sections) {
        if (section.isPreamble())
            continue; // the legacy file's own banner declares no body
        stringHash_t legacy = section.params();
        const std::string name = legacy["name"];
        if (name.empty())
            continue; // the legacy load skipped it too ("Can't load unnamed body")
        ModularBody *body = ModularBody::findBodyOnce(name);
        if (!body)
            continue; // not loaded (orphan/invalid orbit) - not part of the semantic content
        if (!body->isInSubtreeOf(this)) {
            cLog::get()->write("Composed twin of " + legacyFilename + ": section '" + name
                + "' is NOT declared, because a body named '" + name + "' was already loaded by "
                "another system and this system's section was skipped at load time. The twin "
                "describes what THIS system contains. To fix: rename the body, or add "
                "'replace = true' to the section that should win.", LOG_TYPE::L_WARNING);
            continue;
        }
        appendWholeDeclaration(body, legacy, out);
    }
    const std::vector<std::string> banner{
        "Generated by spacecrafter from " + legacyFilename + " - semantically equivalent (B24/B25).",
        "MACHINE-OWNED: regenerated at every legacy load of this system; edits HERE are lost.",
        "To adopt the composed format: copy this file dropping the .disabled extension",
        "(same directory). The composed file then wins over the legacy one, which stops",
        "being read; your copy is user-owned and never touched by the generator.",
    };
    if (ModularSystemFormat::write(outPath, out, banner))
        cLog::get()->write("Composed twin of " + legacyFilename + " generated at " + outPath,
            LOG_TYPE::L_INFO);
}

stringHash_t ModularSystem::composedNodeParams(const ModularBody *body, const stringHash_t &declared)
{
    stringHash_t node = declared;
    {
        const auto it = declared.find("bound_to_surface");
        if (it != declared.end() && Utility::isTrue(it->second)) {
            node.erase("bound_to_surface");   // translated, not duplicated -
            node["relation"] = "grounded";    // one relation authority per generated file
        }
    }
    if (body->getSiderealTimeModel() == SiderealTimeModel::EARTH_APPARENT
            && !node.count("sidereal_time"))
        node["sidereal_time"] = "earth_apparent";
    if (!node.count("shadow_color")
            && body->getShadowAbsorbtion() != Vec3f{1, 1, 1})
        node["shadow_color"] = Utility::vec3fToStr(body->getShadowAbsorbtion());
    if (body->getSurfaceModel() == SurfaceModel::LUNAR
            && !node.count("surface_model"))
        node["surface_model"] = "lunar";
    if (body->getTrailLength() != TRAIL_LENGTH_DEFAULT
            && !node.count("trail_length"))
        node["trail_length"] = std::to_string(body->getTrailLength());
    if (body->isStar() && !node.count("light_source"))
        node["light_source"] = "true";
    if (body->isPrimary() && !node.count("primary"))
        node["primary"] = "true";
    if (body->isMinorBody() && !node.count("shadow_exempt"))
        node["shadow_exempt"] = "true";
    if (body->getScalingTarget() != 1.f && !node.count("display_scale"))
        node["display_scale"] = fileValue(body->getScalingTarget());
    return node;
}

void ModularSystem::appendWholeDeclaration(ModularBody *body, const stringHash_t &declared,
                                           std::vector<ModularSystemFormat::Section> &out)
{
    const auto nameIt = declared.find("name");
    const std::string name = (nameIt == declared.end() || nameIt->second.empty())
        ? body->getEnglishName() : nameIt->second;
    stringHash_t node = composedNodeParams(body, declared);
    node["compose"] = "explicit";
    out.push_back(ModularSystemFormat::Section::fromParams(name, node));
    const auto composeIt = declared.find("compose");
    if (composeIt != declared.end() && composeIt->second == "explicit") {
        for (uint32_t i = 0; i < body->components.size(); ++i) {
            if (!body->components[i])
                continue;
            const std::string slotName{ModularBody::slotID.nameOf(i)};
            const std::string typeName{ModuleLoaderMgr::moduleTypeName(body->components[i]->getType())};
            stringHash_t mod{{"type", typeName}, {"body", name}};
            if (slotName != typeName)
                mod["slot"] = slotName;
            out.push_back(ModularSystemFormat::Section::fromParams(name + ":" + slotName, mod));
        }
        return;
    }
    stringHash_t deduceFrom = declared; // deduceBodyModuleList reads through map[]
    for (BodyModuleType type : body->deduceBodyModuleList(deduceFrom)) {
        const std::string typeName{ModuleLoaderMgr::moduleTypeName(type)};
        const stringHash_t mod{{"type", typeName}, {"body", name}};
        out.push_back(ModularSystemFormat::Section::fromParams(name + ":" + typeName, mod));
    }
    const auto gridIt = declared.find("planet_grid");
    if (gridIt != declared.end() && Utility::isTrue(gridIt->second)) {
        const stringHash_t mod{{"type", "CUSTOM"}, {"body", name}, {"slot", "GRID"}};
        out.push_back(ModularSystemFormat::Section::fromParams(name + ":GRID", mod));
    }
}

void ModularSystem::collectContentBodies(ModularBody *node, std::vector<ModularBody *> &out)
{
    const auto walk = [&out](ModularBody *body, const auto &self) -> void {
        if (body->isSystem())
            return;
        out.push_back(body);
        for (auto &c : body->groundedBodies) self(c.get(), self);
        for (auto &c : body->orbitingBodies) self(c.get(), self);
        for (auto &c : body->innerBodies)    self(c.get(), self);
        for (auto &c : body->hiddenBodies)   self(c.get(), self);
    };
    for (auto &c : node->groundedBodies) walk(c.get(), walk);
    for (auto &c : node->orbitingBodies) walk(c.get(), walk);
    for (auto &c : node->innerBodies)    walk(c.get(), walk);
    for (auto &c : node->hiddenBodies)   walk(c.get(), walk);
}

// Contract + rationale: ModularSystem.hpp (removeSupplementalBodies).
bool ModularSystem::removeSupplementalBodies()
{
    std::vector<ModularBody *> content;
    collectContentBodies(this, content);
    std::vector<ModularBody *> targets;
    for (ModularBody *b : content) {
        if (!b->supplemental)
            continue;
        bool covered = false;
        for (ModularBody *t : targets) {
            for (ModularBody *p = b->parent; p && !covered; p = p->parent)
                covered = (p == t);
            if (covered)
                break;
        }
        if (!covered)
            targets.push_back(b);
    }
    for (ModularBody *t : targets)
        t->remove(true);
    return !targets.empty();
}

// Contract + rationale: ModularSystem.hpp (startTrails).
void ModularSystem::startTrails(bool record)
{
    std::vector<ModularBody *> content;
    collectContentBodies(this, content);
    for (ModularBody *b : content)
        b->startTrail(record);
}

bool ModularSystem::saveSystem(const std::string &outPath)
{
    std::vector<ModularSystemFormat::Section> out;
    std::vector<std::string> banner;
    bool wholeFile = false;
    if (composedFile && outPath == systemFilename && !loadedSections.empty()) {
        out = loadedSections;
    } else if (ModularSystemFormat::parse(outPath, out)) {
        cLog::get()->write("Saving system '" + englishName + "' into the EXISTING file " + outPath
            + ": its content is kept as it is (comments, layout and every key included) and only "
            "what it does not declare yet is added. To write a fresh file instead, save under a "
            "name that does not exist.", LOG_TYPE::L_INFO);
    } else {
        out.clear();
        wholeFile = true;
        banner = {
            "Composed system file written by spacecrafter on request (system '" + englishName + "').",
            "USER-OWNED: nothing regenerates this file - it is yours to edit, and the",
            "engine gives back every line of it when you save again.",
            "It is read INSTEAD of the legacy source of this system as long as it is here.",
        };
    }
    std::set<std::string> alreadyDeclared;
    for (const auto &section : out) {
        if (section.isPreamble() || section.find("body"))
            continue;
        if (const std::string *name = section.find("name"))
            alreadyDeclared.insert(*name);
    }
    std::vector<ModularBody *> bodies;
    collectContentBodies(this, bodies);
    std::size_t added = 0, skipped = 0;
    for (ModularBody *body : bodies) {
        if (body->declaredParams.empty()) {
            ++skipped;
            continue;
        }
        if (alreadyDeclared.count(body->getEnglishName()))
            continue; // the file says it already; its author's words stand
        appendWholeDeclaration(body, body->declaredParams, out);
        ++added;
    }
    if (!ModularSystemFormat::write(outPath, out, banner))
        return false; // the writer said why, and left any previous content alone
    cLog::get()->write("System '" + englishName + "' saved to " + outPath + " ("
        + std::to_string(added) + " declaration" + (added == 1 ? "" : "s") + " added, "
        + std::to_string(bodies.size() - skipped - added) + " already declared, "
        + std::to_string(skipped) + " engine-owned bod" + (skipped == 1 ? "y" : "ies")
        + " skipped)" + (wholeFile ? " - a new file" : "")
        + ". It is read instead of this system's source at the next launch.", LOG_TYPE::L_INFO);
    return true;
}

void ModularSystem::applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param)
{
    // LEGACY-FORMAT ONLY (gated on !composedFile at the call site, D14 S11.79(h)):
    // the composed format declares these capabilities as keys (B25-emit, S11.73).
    if (createInfo.englishName == "Earth") {
        const auto siderealIt = param.find("sidereal_time");
        if (siderealIt == param.end() || siderealIt->second.empty())
            createInfo.siderealTimeModel = SiderealTimeModel::EARTH_APPARENT;
        if (param.find("shadow_color") == param.end())
            createInfo.shadowAbsorbtion = Vec3f(0.6f, 0.88f, 1.0f);
    }
}

// Contract + rationale: ModularSystem.hpp (reloadSystem).
bool ModularSystem::reloadSystem()
{
    if (systemFilename.empty())
        return false;
    if (Context::instance)
        Context::instance->quiesceFrames();
    clearChildren();
    if (composedFile)
        loadComposedSystem(systemFilename);
    else
        loadSystem(systemFilename);
    return true;
}
