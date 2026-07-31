#include "ModularSystem.hpp"
#include "ModularSystemFormat.hpp"
#include "ModuleLoader.hpp" // reroute (composed relation= override)
#include "ModuleLoaderMgr.hpp"
#include "environmentModules/LandscapeEnv.hpp"
#include "environmentModules/AtmosphereEnv.hpp"
#include "tools/ini_line.hpp" // the ONE .ini line grammar (INTENT §5.39/D29)
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
#include <cmath>  // std::sin/cos/atan2 (rot_pole_w0 -> offset conversion)
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
		// CASE("Moon", ...) DELETED (B25-emit, §11.73 A4, 2026-07-23): it mapped
		// to CUSTOM_BODY, which IS the default (below) - a dead case. "Moon"
		// falls through to the default and still resolves to CUSTOM_BODY, so this
		// is bit-identical for legacy and composed loads [re-verified at delete].
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

// Parse the `sidereal_time` capability key (B27 A1; D10key ratified spelling
// §11.79(e)): the analytic spin-phase model, data-selected (not identity-keyed).
// Absent / "generic" -> GENERIC (the default (jd-epoch)/period spin, every
// body); "earth_apparent" -> EARTH_APPARENT (apparent sidereal time). §2(f):
// an unknown value names the valid values, the fallback (generic), and the fix,
// and never guesses (D12 - the fallback ACTS, so it is logged).
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

// --- Capability-key reading, D14 format boundary (B27 tail, §11.73/§11.79(h)) ---
//
// THE operator[] TRAP, once, structurally [§11.103(b), F0 2026-07-25]: loadBody
// reads its keys through std::map::operator[], which INSERTS an empty entry for
// an absent key - so `param.find(k) != param.end()` is TRUE for keys nobody
// authored, and a naive key-absent guard silently inverts. "The data authored
// nothing" is therefore ABSENT-OR-EMPTY, and this is the only place that test is
// written: every capability key below goes through it, so the trap cannot be
// re-stepped in by writing one more guard by hand (I6 - fix the class).
// Returns nullptr when nothing was authored, the value otherwise.
static const std::string *authored(std::map<std::string, std::string> &param, const char *key)
{
    const auto it = param.find(key);
    return (it == param.end() || it->second.empty()) ? nullptr : &it->second;
}

// SAY IT ONCE, IN BOTH CHANNELS (I2, b31-design §5.3). A loader diagnosis has
// always had one destination - the log, which the operator reads at launch and
// nobody reads a month later, when the file is opened in a text editor and the
// datum is right there with no trace of what the engine thought of it. So the
// same sentence now also travels WITH the datum: `origin` is the section the
// datum was read from, and the annotation lands ABOVE its line the next time an
// explicit save writes that file (never at load - that is decided against, D33
// §11.113(l)).
// `origin` is null wherever there is no writable datum to annotate - a legacy
// file (READ-ONLY forever, D35), a script's parameter map - and the log line is
// then the whole channel, exactly as before.
// `reason` is the diagnosis' stable machine key: the same verdict reached twice
// is ONE annotation, which is what makes a re-save byte-identical (T9).
// D12: this is for defaults that ACTED. A default that merely did nothing is
// forbidden from annotating, and none of the callers below is one.
static void diagnose(ModularSystemFormat::Section *origin, const std::string &key,
    const char *reason, const std::string &text, LOG_TYPE severity)
{
    cLog::get()->write(text, severity);
    if (origin)
        origin->annotate(key, reason, text);
}

// D14 (§11.79(h), [vixy 2026-07-23]) — "Yes for the new star system format, no
// for the legacy star system format". A capability whose LEGACY source is the
// `type` data string keeps that source FOREVER in a legacy file (D9: the field is
// frozen); in the composed format `type` grants nothing and the capability KEY is
// the only source. This log is what makes the boundary non-silent (D12: the
// default ACTS): it fires only when a composed body's `type` WOULD have granted
// something and no key is present - i.e. exactly on a composed file hand-written
// from a legacy one without translating its `type`. It never fires on a generated
// twin (the generator emits the keys), which makes its absence a co-delivery gate.
// The datum it is about is `type`, so that is where the annotation sits.
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

// Parse the `surface_model` capability key (B27 A6; D10key ratified spelling
// §11.79(e)). §2(f): an unknown value names the valid values, the fallback and
// the fix, and never guesses.
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

// Parse a boolean capability key (B27 Tier B: `light_source`, `shadow_exempt`).
// §2(f) on a non-boolean value; the caller has already established that the key
// was authored (authored() above).
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
    // Nav-radius CLASS DEFAULT (B10-datum0, §11.75(a) [vixy 2026-07-22]): a
    // system node is something you navigate INTO, so an UNSET datum/ground
    // defaults to 0 - its centre: free-mode altitude measured from the centre,
    // free descent reaching the centre - instead of the plain-body `radius`.
    // Keyed off system NATURE, not name: this ctor runs for EVERY ModularSystem
    // whatever its bodyType/name (Universe, MilkyWay/GALAXY, every per-system
    // node) - I4, never a per-name list. The base ModularBody ctor already
    // resolved the sentinel to `radius`; the system default overrides it to 0.
    // USER-OVERRIDABLE: an explicit datum_radius/ground_radius arrives as a
    // non-sentinel (>= 0) value - via a data key, or the §11.84 runtime command
    // which writes the member AFTER construction - and neither ctor's
    // sentinel branch touches it, so it wins (and reverses trivially).
    const bool datumDefaulted = (info.datumRadius < 0.f);
    const bool groundDefaulted = (info.groundRadius < 0.f);
    if (datumDefaulted)
        datumRadius = 0.f;
    if (groundDefaulted)
        groundRadius = 0.f;
    // D12 (§2.0 - acting defaults must be logged): the system class default
    // DEVIATES from the universal `radius` default and, on a node with a
    // non-zero render radius, causes navigation behaviour the node's author did
    // not write - sharpest at a MilkyWay reference, where free-mode `moveto
    // altitude X` now lands at X instead of radius + X (a 3.2e9 AU shift,
    // §11.80). That is an ACTION -> logged (load-time, once per such node).
    // When radius == 0 (Universe, per-system nodes) the default coincides with
    // `radius` (centre == surface) -> no distinct behaviour -> inaction -> silent.
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
    // Starless systems (galaxy/universe level) leave the light state alone -
    // lightPosition is CURRENT-SYSTEM-scoped (nested draws save/restore it,
    // drawSystem dispatch), and no body of a starless system consumes it
    // outside the delegation path.
    // THE FRAME'S NON-RENDER USES, behind the D8 barrier (B39 §11.117). Both of
    // these read a body's cached position every frame from OUTSIDE the draw
    // walks, so for a body that no longer ticks they are exactly the "use" D8
    // names - and both are reachable with a hidden body (a hidden star still
    // illuminates, recorded §11.117; `S10.sts` SELECTS a hidden body).
    if (ModularBody *s = getSystemStar()) {
        s->useNow();
        s->updateAsLightSource();
    }
    // Being the SELECTION is a use: the ModularObject readouts (RA/DE, alt/az,
    // distance, magnitude, on-screen size) and Camera's selection distance all
    // poll it per frame. Refreshed here, once, rather than at each of the dozen
    // reader sites (I2).
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

// Shadow orchestration - the WHICH half of G7 (class comment + shadow-paths.md
// B2; every ported formula carries its old-path line reference).
// MODULE-GRANULAR since the composition-typed rework (2026-07-16): a caster
// candidate is a (body, projecting module) pair and each pair gets its own
// layer + receiver entry. Per-entry application multiplies transmissions and
// products commute, so per-module layers compose exactly like one combined
// caster map (ShadowProjection.hpp) - while making per-module absorbtion and
// WITHIN-BODY pairs (ring<->planet on one ModularBody) expressible, which
// body-granular selection excluded by construction (caster == body skip).
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
    // ---- Self-shadow nomination (OJM wave, 2026-07-16) --------------------
    // Old CoI parity: ProtoSystem::computeDraw nominated the single
    // highest-importance rendered body (importance = screen_sz/distance,
    // body.hpp:484-486), gated by the same experimental_shadows flag this
    // function already gates on. ONE nomination per frame = the single MAIN
    // depth target (ShadowService.hpp header; the SECONDARY ladder rides S3).
    // Placed BEFORE the caster scan: a lone artificial body with no caster
    // in the system still self-shadows (the empty-casters early return below
    // must not skip it).
    {
        ModularBody *best = nullptr;
        BodyModule *bestModule = nullptr;
        float bestImportance = 0;
        for (ModularBody *body : sortedSystemBodies) {
            if (!body || body->distance == 0)
                break; // sorted: unevaluated tail
            if (!(*body && body->screenSize > 0.0015f) || body->isStar() || body->isMinorBody())
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
            // model -> sun-frame-NDC matrix, rotation-only (unit geometry
            // covers NDC; shadow_trace.vert maps z*0.5+0.5). Third row TOWARD
            // the sun - the old lookAt(sun->body) convention (its -f row):
            // depth GREATER + clear 0 keeps the most-sunward surface, and
            // computeEnlightment's step() compares against that map.
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
            // The DRAWN orientation: near-component matrix (mat x
            // bodyToSurface zrot) - production and consumption project the
            // same raw model vertices, so the same composition must be used.
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
    // G8 budget [vixy: 2026-07-12]: up to 10 simultaneous greyscale
    // projections per frame is the sizing budget (shadow-paths.md B5).
    // Enforcement lives here (selection owns significance ordering); overflow
    // drops the least significant G8 entries, logged once per frame.
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
        // Receiver gate: drawn this frame (the ModularBody::draw entry test),
        // not MINOR/light-source. NOT gated by screen size beyond drawing:
        // the occlusion criterion below is the shadow-relevance gate [vixy].
        if (!(*body && body->screenSize > 0.0015f) || body->isStar() || body->isMinorBody())
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
                // WITHIN-BODY pair (ring->planet / planet->ring). No corridor
                // test - the caster is AT the receiver, always in its own
                // light cylinder; penumbra growth over the body's own extent
                // is negligible (the old analytic ring shadows were sharp -
                // parity), so smooth = 0. Rank FLT_MAX: the closest possible
                // shadow is never the one to drop. Emitted only when ANOTHER
                // module of this body can sample it - the projecting module
                // never samples its own layer (meshShadowFill self-filter),
                // so without a second receiver the layer feeds nobody.
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
        // Entry rows: per-(receiver, LIGHT) fold - stored per ENTRY (self-
        // contained; multi-light readiness [vixy: 2026-07-18], see
        // ShadowProjection.hpp). Single light today: one fold, every entry.
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
            // Cache key: CASTER-LOCAL light direction (columns of the
            // orthonormal rotation part dot the vector = transpose apply) -
            // the old heliocentric getLocalSunDirection equivalent,
            // camera-independent by construction.
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
                // Silhouette matrix: rows(x,y,z) . rot3(caster->mat) .
                // diag(r, r, r*(1-oblateness)) / size - maps the module's
                // silhouette geometry into shadow-map NDC (old:
                // lookAt*model*scaling mat3, body.cpp:1222-1230, same algebra
                // in the eye frame; r = the MODULE's silhouette radius -
                // outer ring radius for an annulus).
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
            // Physical-sharp role split (2026-07-18 [vixy] - the derivation
            // lives at ShadowProjection.hpp): the module's declared absorbtion
            // maps per WORD SEMANTICS. Graded casters (G8 rings): material
            // TRANSMISSION - aT = declared, no refraction glow. Solid casters:
            // opaque to direct light (aT = 1) and the declared absorbtion's
            // complement is the atmospheric REFRACTION glow lighting the true
            // umbra (Earth {0.6,0.88,1} -> gR {0.4,0.12,0}; airless default
            // {1,1,1} -> gR 0, black umbra, formula reduces exactly).
            const bool graded = (c.traits & BMT_PROJECT_G8_SHADOW) != 0;
            const Vec3f aT = graded ? c.info.absorbtion : Vec3f(1.f, 1.f, 1.f);
            const Vec3f gR = graded ? Vec3f(0.f, 0.f, 0.f)
                                    : Vec3f(1.f - c.info.absorbtion[0],
                                            1.f - c.info.absorbtion[1],
                                            1.f - c.info.absorbtion[2]);
            Vec4f clip = c.info.clip;
            if (caster == body && clip[0] == 0 && clip[1] == 0 && clip[2] == 0) {
                // WITHIN-BODY solid caster (planet -> its own rings): the
                // degenerate clip is only valid where the corridor test
                // carries the z-order - and within-body pairs skip it while
                // their ANNULUS receiver spans BOTH sides of the caster.
                // Without a gate the z-less layer falsely shadows the
                // SUNWARD ring half (found live at the row-4 port: the lit
                // ring strip went black in the Iapetus A/B). Gate = the
                // plane through the caster center perpendicular to the
                // light axis, normal toward the sun (the ring-clip
                // convention): only the anti-sun half shadows - exactly the
                // z-order the old analytic SeparationAngle test carried
                // (ring_planet.frag).
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
    // Trace sweep: every ON-SCREEN body with a DEPTH_TRACE module writes its
    // disc into the orbit depth buffer (old drawOrbit -> cmdBodyDepth, gated on
    // isVisibleOnScreen). ALL traces precede ALL lines so a nearer body's disc
    // can hide a farther body's orbit (the two-buffer reason old split them).
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // unevaluated tail (drawSystem rule)
        if (!body)
            continue; // operator bool = on-screen (old isVisibleOnScreen)
        // Same matrix the COLOR draw hands its near-components
        // (drawLoaded: mat . computeBodyToSurface()) so the depth silhouette
        // matches the drawn body EXACTLY. computeBodyToSurface is the pole
        // spin (zrotation(axisRotation)): a sphere/ring is invariant under it
        // (azimuthally symmetric - BasicMesh/LayeredMesh/RING output
        // bit-identical to the raw-mat form), but a non-spherical OJM model's
        // silhouette depends on the spin, so it MUST ride the surface matrix.
        const Mat4f traceMat = body.getMat().multiplyFast(body.computeBodyToSurface());
        for (auto *m : body.nearComponents)
            if (m->getTraits() & BMT_DEPTH_TRACE)
                m->drawTrace(renderer, &body, traceMat);
    }
    renderer.beginOrbitLines();
    // Line sweep: each orbit module draws in its parent's POSITION frame (the
    // frame the parent-relative orbit points live in - old parent_mat). That
    // frame is parent->mat with the parent's own accumulated tilt undone
    // (matLocalToBodyPos = mat . accumulatedBodyPosToBody^-1).
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break;
        if (body.orbitComponents.empty())
            continue;
        if (!body.getParent())
            continue; // a parentless body has no orbit to draw around
        // Parent POSITION frame = the body's own cached position frame
        // (matLocalToBodyPos = P_frame . translation(B.ecl)) with B's own
        // offset undone. The cached frame is correct for EVERY body, visible or
        // not (unlike `mat`, whose rotation goes stale out of the view cone -
        // the halo-only planets whose orbits were misplaced pre-fix).
        Mat4f parentFrame = body.getMatLocalToBodyPos();
        parentFrame.multiplyTranslation(-body.getEclipticPos());
        for (auto *m : body.orbitComponents) {
            m->update(&body, body.getScaledRadius());
            m->draw(renderer, &body, parentFrame);
        }
    }
}

void ModularSystem::drawTrails(Renderer &renderer)
{
    // Skip the whole pass (accumulation + draw) when no trail is shown or
    // fading - the default (trails off) pays nothing. The always-run sweep hung
    // scene E (the drawOrbits precedent) - the gate is mandatory.
    if (!TrailModule::anyActive())
        return;
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    renderer.beginTrailDraw();
    // ONE sweep: for every EVALUATED body (distance != 0), update() its trail
    // (accumulate the current parent-relative position at sim time + advance the
    // fader) then draw the polyline. update() ticks regardless of the body's
    // visibility, so accumulation continues while it is off-screen - the row-9
    // invisible-tick contract (old drew AND accumulated the trail in both the
    // visible and off-screen branch, body.cpp:1131/1163 + updateTrail on every
    // body every frame). No `!body` on-screen skip: an off-screen body's trail
    // is drawn (its on-screen segments show; the geom shader wrap-culls the rest)
    // exactly like the old off-screen branch.
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // unevaluated tail (drawSystem rule; a G4-culled subsystem)
        if (body.trailComponents.empty())
            continue;
        if (!body.getParent())
            continue; // parentless: no parent frame to draw the trail in
        // Parent POSITION frame (matLocalToBodyPos . translation(-ecl)) - the
        // frame the parent-relative trail points live in, identical to the
        // ORBIT pass (§11.39: the cached flat frame carries the correct rotation
        // for a body that is at least halo-visible; a fully off-screen body's
        // cache is stale, same shared limitation as ORBIT - the accumulation is
        // unaffected, and the frame refreshes the instant the body returns).
        Mat4f parentFrame = body.getMatLocalToBodyPos();
        parentFrame.multiplyTranslation(-body.getEclipticPos());
        for (auto *m : body.trailComponents) {
            m->update(&body, body.getScaledRadius());
            m->draw(renderer, &body, parentFrame);
        }
    }
}

void ModularSystem::drawTails(Renderer &renderer)
{
    // Skip the whole pass when no comet with a tail is loaded - the default (no
    // comets) pays nothing. The always-run sweep hung scene E (the drawOrbits/
    // drawTrails precedent) - the gate is mandatory.
    if (!TailModule::anyActive())
        return;
    ModularBody ** const end = sortedSystemBodies.data() + sortedSystemBodies.size();
    renderer.beginTailDraw();
    // ONE sweep: for every EVALUATED body (distance != 0) with a TAIL module,
    // update() (coma/tail size + parent-frame expansion vectors) then draw()
    // (submit the eye-space instance to the Renderer batch). Handed the PARENT
    // POSITION frame (matLocalToBodyPos . translation(-ecl)), identical to the
    // ORBIT/TRAIL passes: the root-aligned VSOP87 -> eye rotation the tail's
    // parent-frame expansion vectors need (the old nav->getHelioToEyeMat()). No
    // on-screen skip: an off-screen comet's tail projects outside the viewport
    // and is clipped (like TRAIL), matching the old draw-whenever-evaluated.
    for (ModularBody **pos = sortedSystemBodies.data(); pos < end; ++pos) {
        ModularBody &body = **pos;
        if (body.distance == 0)
            break; // unevaluated tail (drawSystem rule)
        if (body.tailComponents.empty())
            continue;
        if (!body.getParent())
            continue; // parentless: no parent frame to place the tail in
        Mat4f parentFrame = body.getMatLocalToBodyPos();
        parentFrame.multiplyTranslation(-body.getEclipticPos());
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
    // px full diameter of the subsystem on screen - the drawHalo/pointer px
    // idiom. NOT the node's own screenSize: preUpdate derives that from
    // boundingRadius = the node's OWN body extent, which is 0 for a bare
    // system node - its child-visibility branch computes exactly this angle
    // from subsystemRadius and then overwrites it (found at the first live
    // draw, INTENT 11.80: every nested system classified as 0 px, the
    // collapse test could never resolve). Same geometric form; inside the
    // subsystem the interior is resolved by construction.
    // At/above the collapse threshold T the interior is content
    // (per-child visibility gating does the rest - D3); below T, one point of
    // light (the star-proxy dot, guarded by isBodyVisible: the dot sits at
    // the node's centre, whose on-screen test is the own-body one).
    //
    // B22 cross-fade (INTENT 11.64): the hard switch at T
    // (SYSTEM_VISIBILITY_SUBSYSTEM_SIZE) POPPED - the whole interior appeared,
    // and the proxy dot vanished, in one frame. Softened over a band [T, T+B):
    //   - RESOLVED interior runs for px >= T EXACTLY as before (its expensive
    //     draw region is UNCHANGED, so the cross-fade adds NO resolved cost),
    //     but its halos are scaled by t = (px-T)/B in the band ⇒ they fade IN.
    //   - The DOT also runs across the band, scaled by (1-t) ⇒ it fades OUT.
    //     This is the ONLY added cost: one drawStarProxy (one drawHaloCore /
    //     halo instance) per frame, and only while px is inside the band.
    // Endpoints match by construction: at px=T, t=0 ⇒ interior invisible + dot
    // full (== pure dot); at px=T+B, t=1 ⇒ interior full + no dot (== pure
    // resolved). drawAlpha carries the ramp into every halo via drawHaloCore;
    // it is saved/restored here (nested-in-band compounds multiplicatively).
    const float px = (distance > subsystemRadius)
        ? (atanf(subsystemRadius / sqrtf(distance*distance - subsystemRadius*subsystemRadius))
           / halfFov) * 2.f * viewportRadius
        : 2.f * viewportRadius;
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
            // Shadow selection under OUR light (2026-07-18, closing the 11.36
            // "nested-draw shadows absent" suspension): without this call a
            // visibly-resolved nested system drew shadowless - drawSystem's
            // computeShadows only serves the CURRENT system. Rides the same
            // light save/restore; jobs land in the same frame's pre-color
            // recording window (the helper records at frame assembly, after all
            // queueing). Runtime-unexercised BY CONSTRUCTION until the executor
            // dissolution (6.9) gives drawNested its first live surface - the
            // same status as drawNested itself (11.36 named limitation).
            // Cross-SYSTEM shadows (a body of system A onto a body of B) stay
            // excluded as a documented model precondition: physically negligible
            // at inter-system distances.
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
    // NOT gated on s->isHaloEnabled: that flag selects the star's CLOSE-RANGE
    // halo channel (the shipped Sun authors halo=false because its self-draw
    // is the big-halo texture), and gating the SYSTEM's collapsed
    // representation on it left the shipped solar system with no far dot at
    // all (found at the first live collapse, INTENT 11.80 - a 2(a2)-class
    // foreclosure: an authoring flag for one regime gating another).
    if (!s)
        return;
    // Star apparent magnitude at the NODE's fresh distance - the star's own
    // cached distance is stale while its system isn't current. Dims with
    // distance: old-path parity body_sun.cpp:86 (-26.73 + 2.5*log10(d^2)).
    // Disc floor = the STAR's disc at that distance
    // (small angle), never the subsystem extent (which would inflate the
    // halo floor up to the nested-draw threshold).
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
    // Tail pass (row 12): the comet gas/dust tails, right after the body draw -
    // the old path batched them WITH the halos in the body pass (halo.cpp:55).
    // Depth-less instanced overlay in its own command buffer (beginTailDraw),
    // gated on anyActive(). Placed before the trails/orbits so the comet's tail
    // sits closest to the body/halo pass it belonged to (the exact z-order among
    // these depth-less overlays is a documented, immaterial divergence, §11.43).
    drawTails(renderer);
    // Trail pass (row 9): accumulate + draw the fading path polylines, after the
    // body draw (old drew each trail in its body's command buffer, body.cpp:
    // 1131/1163) and BEFORE the orbit lines so a crossing orbit draws on top
    // (old order: bodies+trails in the body pass, then the orbit phase). No-depth
    // COLOR; its own command buffer (beginTrailDraw), gated on anyActive().
    drawTrails(renderer);
    // Orbit pass (row 8): trace holes + orbit lines, after the body draw and
    // before the pointer (old solarsystem_display.cpp order: bodies, halos,
    // orbits, then the pointer on top). Nested systems' orbits are deliberately
    // absent (drawNested is runtime-unexercised, INTENT 11.36; the orbit phase
    // owns a command buffer, which drawNested cannot open mid-parent-loop).
    drawOrbits(renderer);
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

namespace {
// Rotation-frame declaration (B28, INTENT §11.67). The data declares which
// coordinate system its axial orientation is authored in; the loader converts
// through this ONE authority (grep: the mat_j2000_to_vsop87 pole conversion
// lives nowhere else in the new path).
enum class RotFrame { PARENT_RELATIVE, ABSOLUTE_POLE };

// Resolve a body's rotation frame and produce the (obliquity, ascendingNode)
// the rotation model consumes:
//   PARENT_RELATIVE  rot_obliquity / rot_equator_ascending_node, relative to
//                    the parent's equatorial frame (accumulated by
//                    ModularBody's ancestor-tilt walk).
//   ABSOLUTE_POLE    rot_pole_ra / rot_pole_de, a J2000-equatorial north pole,
//                    converted to the ecliptic (VSOP87) ROOT frame here; the
//                    result is root-aligned, so the caller marks the body
//                    absoluteTiltFrame and the accumulation skips ancestors
//                    (§11.49(e): a parent-relative slot filled with an absolute
//                    published pole is an invalid orientation that looks right
//                    in the file - making the frame explicit closes it, and
//                    lets B14 declare the 28 moon poles in the absolute frame).
// Frame source: the explicit `rot_frame` key wins; when absent it is DERIVED
// from which rotation keys are present (pole keys => absolute, else
// parent-relative), reproducing every legacy file bit-for-bit (the 7 existing
// rot_pole_ra planets derive to absolute_pole, and the pole conversion below is
// byte-for-byte the legacy arithmetic). A DEFAULT is applied in memory only and
// NEVER written back (write-back is B31, §11.66(c)). An invalid `rot_frame`
// value gets a §2(f) actionable diagnostic (valid states + error trace +
// fallback + fix action) and falls back to the derived default.
RotFrame resolveRotationFrame(std::map<std::string, std::string> &param,
                              const std::string &englishName,
                              float &rot_obliquity, float &rot_asc_node,
                              float &rot_offset)
{
    rot_obliquity = Utility::strToFloat(param["rot_obliquity"], 0.) * M_PI / 180.;
    rot_asc_node  = Utility::strToFloat(param["rot_equator_ascending_node"], 0.) * M_PI / 180.;
    // rot_rotation_offset (the prime-meridian offset, DEGREES) is consumed RAW by
    // default - bit-identical for every legacy body. An absolute-pole body may
    // instead declare the IAU prime meridian rot_pole_w0 (measured from the node
    // of the body equator on the ICRF/J2000 equator), which the loader CONVERTS
    // to this ecliptic-node referential below (D4 §11.79(a), the meridian twin of
    // the pole conversion - ONE authority, B28/§11.67 class).
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

        // W0 (IAU prime-meridian) conversion - the meridian twin of the pole
        // conversion above (D4 §11.79(a); ONE authority, B28/§11.67 class). The
        // IAU W0 (rot_pole_w0) is measured from the ascending node of the body
        // equator on the ICRF (J2000) equator; rot_rotation_offset is measured
        // from that node on the ECLIPTIC (ascendingNode = ra_ecliptic+90°, the
        // §11.69(e) referential mismatch) - a per-body node-difference. Convert
        // ONLY when the body declares rot_pole_w0 (a fetched IAU value); absent
        // => rot_rotation_offset raw (bit-identical; backward compat, D9/Q26).
        if (!param["rot_pole_w0"].empty()) {
            const float W0 = Utility::strToFloat(param["rot_pole_w0"], 0.) * M_PI / 180.;
            // IAU prime-meridian direction at W0, in the ecliptic root frame.
            // node = ascending node of the body equator on the ICRF equator
            // (RA = pole_ra + 90°); perp = 90° east of it about the right-hand
            // pole; the meridian is (cos W0)·node + (sin W0)·perp.
            const Vec3f node(-std::sin(J2000_npole_ra), std::cos(J2000_npole_ra), 0.f);
            const Vec3f perp(J2000_npole ^ node);
            const Vec3f pm_icrf(node * std::cos(W0) + perp * std::sin(W0));
            const Vec3f pm(mat_j2000_to_vsop87.multiplyWithoutTranslation(pm_icrf));
            // Loader equatorial-frame x/y axes (in the ecliptic) at this tilt.
            // WHERE THE MERIDIAN MUST LAND: on the texture's CENTRE column
            // (u = 0.5), not on the mesh's x̂. The sphere is textured
            // u = θ/360 − 0.25 [ojmModule/SphereObjL.cpp:153] and the draw spins
            // it by getAxisRotation() = axisRotation + π/2 [ModularBody.hpp:487],
            // so the two compose to: texture column u is drawn at azimuth
            // axisRotation + 180° + 360°·u. The IMAGE CENTRE therefore draws at
            // zrotation(axisRotation)·x̂ exactly - the +π/2 fudge and the −0.25
            // texcoord cancel each other - while mesh x̂ is column u = 0.75.
            // rot_rotation_offset is thus the azimuth of the image centre, which
            // is the convention the shipped corpus is registered to: the four
            // longitudinally registered planets' file offsets place the IAU
            // meridian at u 0.5000 / 0.4998 / 0.4978 / 0.4923 (Saturn, Mercury,
            // Mars, Neptune - §11.101(b2)), and Earth's Greenwich-centred map
            // agrees independently (its spin bypasses the offset through
            // apparent sidereal time, so GAST = 0 puts the image centre on the
            // vernal equinox - the definition of sidereal time).
            // Solve xzrotation(obliquity,ascNode) · zrotation(offset) · x̂ = pm:
            //     offset = atan2(pm·ey, pm·ex).
            // Targeting mesh x̂ instead put every rot_pole_w0 body 90° out
            // (§5.28, decided in the CONVERSION by D22 §11.113(a) - never in the
            // 20 bodies' data, and never by retiring the +π/2, which would move
            // every body's meridian instead of the three that are wrong). The
            // correction is exactly +90° for every body, whatever its pole,
            // because atan2(pm·ey, pm·ex) ≡ atan2(−(pm·ex), pm·ey) + 90°.
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
        // rot_pole_w0 is meaningful only for an absolute pole (it is the IAU
        // prime meridian measured in the ICRF-equatorial frame); a parent_relative
        // body has no absolute pole to reference it to. §2(f) actionable log.
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
                             ModularSystemFormat::Section *origin)
{
    // WHAT THE DATA SAID, taken HERE and not one line later: every read below
    // goes through operator[], which inserts an empty entry for every absent key
    // it touches (§11.103(b)), so a snapshot taken after the load would carry a
    // dozen keys nobody wrote - and a save built on it would author them into
    // the user's file. This copy is the body's declaration record
    // (ModularBody::declaredParams): the only source a runtime-pushed body will
    // ever have for what it was asked to be (b31-design §4.1).
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

    // Rotation-frame declaration + conversion (B28, INTENT §11.67). The frame in
    // which the axial orientation is authored is now DECLARED, not inferred:
    // resolveRotationFrame() is the ONE authority that reads the declaration,
    // validates it (§2(f) actionable log), converts an absolute J2000 pole into
    // the internal obliquity/ascendingNode, and reports whether the tilt is
    // root-aligned (absolute) so the orientation accumulation does not re-apply
    // ancestor tilts to it. Bit-identical for the 7 rot_pole_ra planets (frame
    // derives to absolute_pole, byte-for-byte the legacy pole arithmetic, and
    // their Sun parent is system-centered so the accumulation was already inert).
	float rot_obliquity, rot_asc_node, rot_offset;
	const bool absoluteTiltFrame =
		(resolveRotationFrame(param, englishName, rot_obliquity, rot_asc_node, rot_offset)
			== RotFrame::ABSOLUTE_POLE);

    // --- B27 tail: the capabilities the `type` data string used to carry ------
    // ONE resolution site for all of them (I2): every consumer downstream asks
    // the BODY for the capability, none re-reads `type`. Each follows the same
    // three-legged rule, which is D9 and D14 written out:
    //   key authored          -> the key drives, in BOTH formats (a legacy file
    //                            has never carried these keys, so a legacy load
    //                            is bit-identical unless its author adds one);
    //   key absent, LEGACY    -> the `type`-derived value, frozen forever (D9);
    //   key absent, COMPOSED  -> the neutral default; `type` grants nothing
    //                            (D14 [vixy §11.79(h)]), and the divergence is
    //                            logged when it would change something (D12).
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
    // A7 - trail sample count (`trail_length`), consumed by TrailLoader. Legacy
    // source: the old per-class dispatch (protosystem.cpp:645-801 + trail.cpp
    // defaults) BigBody Planet/Dwarf 1460, SmallBody Comet 2920, everything else
    // (Asteroid/KBO/unknown) TRAIL_LENGTH_DEFAULT.
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
    // TIER B - the `type` -> CAPABILITY mapping (§11.73(c), D14 answered YES for
    // the new format). THREE capabilities since the D27 split (§11.113(f)):
    // `light_source` (the STAR bit - emits light, isStar()), `primary` (the
    // structural remainder of what the STAR bit used to mean, isPrimary(), its
    // own member - see ModularBody.hpp for why it is not a second enum bit), and
    // `shadow_exempt` (MINOR_BODY - exempt from inter-body shadowing, D3,
    // isMinorBody(), 3 consumers). ANCHOR (type =
    // Observer/Anchor/Center) has ZERO consumers [re-verified whole-src at edit
    // time, 2026-07-25 - the A3/EARTH_MOON pattern], so a composed body that
    // resolves to CUSTOM_BODY instead of ANCHOR is behaviourally identical and
    // needs no key and no log. Every other legacy value already maps to
    // CUSTOM_BODY, which IS the composed default - so the composed resolution
    // below reproduces strToBodyType EXACTLY, value for value, on everything but
    // Sun/Star and Asteroid/KBO/Comet, which is precisely §11.73(c)'s Tier-B set.
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
        // Same enum VALUES strToBodyType produces (STAR = 0x40 alone, not
        // CUSTOM_BODY|STAR): the two BodyType capabilities are not composable in
        // this enum's shape, so declaring both is reported rather than silently
        // half-applied (§2(f)); no shipped body is both. `primary` is NOT in that
        // enum and therefore composes freely with either.
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
        // D12/D14: name every capability this body's `type` would have granted
        // and does not. PER CAPABILITY since the D27 split - `type = Sun` now
        // grants TWO of them, so a single "some key is missing" gate would leave
        // one of the two silently unmentioned, which is the co-delivery hole
        // §11.73(g) exists to prevent. (It also stops an unrelated key - a
        // declared `shadow_exempt = false` - from suppressing a warning about
        // `light_source`, which the previous single gate did.)
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
        // D9/D14: in a legacy file the `type` string keeps granting the whole
        // bundle it always granted, split or not - `isStar()` and `isPrimary()`
        // are both true for `type = Sun|Star` and both false otherwise, which is
        // exactly what the un-split code did through the single STAR bit.
        primary = legacyStar;
    }

    ModularBodyCreateInfo createInfo {
        .orbit=ModuleLoaderMgr::instance.loadOrbit(param),
        .englishName=englishName,
        .re={
            .period=Utility::strToFloat(param["rot_periode"], Utility::strToFloat(param["orbit_period"], 24.f))/24.f,
            .offset=rot_offset,   // raw rot_rotation_offset, or the rot_pole_w0 conversion (B28/§11.67 class)
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
        // Navigation radii (B10 §5.2), both in km in the data (like `radius`),
        // both DEFAULTING TO `radius` ⇒ absent keys reproduce today exactly.
        // datum_radius = altitude/landscape/atmosphere zero-point; ground_radius
        // = free-flight descent floor. Set datum_radius=ground_radius=0 for an
        // enterable / transparent body (the two-body-patch replacement, R4);
        // set ground_radius=radius*1.002 for terrain clearance.
        .datumRadius=Utility::strToFloat(param["datum_radius"], radius)/static_cast<float>(AU),
        .groundRadius=Utility::strToFloat(param["ground_radius"], radius)/static_cast<float>(AU),
        .oblateness=Utility::strToFloat(param["oblateness"], 0.0),
        .solLocalDay=Utility::strToFloat(param["sol_local_day"],1.0),

        .shadowAbsorbtion=(param.find("shadow_color") != param.end()) ? Utility::strToVec3f(param["shadow_color"]) : Vec3f{1, 1, 1},
        .brightness=Utility::strToFloat(param["brightness"], 0.0),
        // Spin-phase model (B27 A1, the `sidereal_time` key). Absent -> GENERIC
        // (bit-identical to today for every legacy body); the legacy Earth reaches
        // EARTH_APPARENT through applyHardcodedContent below (legacy format only,
        // D14), the composed Earth through the key the twin emits (§11.73 A1).
        .siderealTimeModel=parseSiderealTimeModel(param["sidereal_time"], englishName),
        // B27 tail (A6/A7 + D14 format scope) - resolved above, one site.
        .surfaceModel=surfaceModel,
        .trailLength=trailLength,
        .composedDeclaration=composedFile,
        // B27 Tier B, the D27 split (§11.113(f)) - the structural half of what
        // the STAR bit used to carry, resolved above with everything else.
        .primary=primary,

        .bodyType=bodyType,
        .isHaloEnabled=Utility::isTrue(param["halo"]),
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
    // D14 FORMAT SCOPE (§11.79(h), B25-emit): the name/type-identity sniff is
    // LEGACY-FORMAT ONLY. The composed format expresses these capabilities as
    // explicit keys (sidereal_time A1, shadow_color A2) that the twin generator
    // materializes at the format boundary; running the sniff on a composed load
    // would re-introduce the identity dependency D14 retires AND mask a missing
    // key (the co-delivery counterfactual would stop discriminating - §11.73(g)).
    // The composed Earth therefore gets apparent sidereal time and its shadow
    // default from the KEYS, never from englishName.
    {
        const auto hardcodedIt = param.find("hardcoded");
        if (!composedFile && (hardcodedIt == param.end() || Utility::isTrue(hardcodedIt->second)))
            applyHardcodedContent(createInfo, param);
    }
    // Relation from data BEFORE creation - relation is the ownership
    // authority (boundToSurface is its cache, written by createChild only).
    // B24 (INTENT §11.78(d)): `relation = orbiting|grounded|inner` is the
    // declared form and the ONLY data route to INNER; the legacy
    // `bound_to_surface` boolean stays as an alias. One resolution authority,
    // both formats (legacy files simply never carry `relation`).
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
    ModularBody *body = parent->createChild(createInfo, rel);
    // The body keeps what declared it (B31 slice 2, b31-design §4.1): this is
    // the record a save writes back, and for a script-pushed body it is the only
    // one that will ever exist. Handed over AFTER creation, so a load that
    // refused (unnamed, duplicate name, invalid orbit - each returns above)
    // leaves no declaration behind for a body that is not there.
    body->declaredParams = std::move(declared);
    // --- Attitude default resolution (B24-att; D18 §11.79(l) + D12 §2.0) ------
    // The default's home is the loader - the one site that owns rotation-key
    // resolution (I2/I4); no per-draw sniffing. `authoredSpin` = the author
    // wrote rot_periode (the spin-rate key the default supplies): its presence
    // retires every default below and wins as-authored on both relations.
    //  (1) GROUNDED + no authored spin: the body is STATIC on the terrain it
    //      stands on (a rover sits still, locked to the surface) -> surface-locked
    //      attitude (computeAxisRotation drops the own spin). This is INACTION
    //      (no rotation relative to the surface - D18: "inaction is no rotation")
    //      -> SILENT (D12). The parent-surface FOLD (§5.23) is unaffected: it uses
    //      the PARENT's spin on this body's POSITION, a different observable.
    //  (2) NON-grounded + neither rot_periode NOR its orbit_period fallback: the
    //      legacy default rot_periode = 24 h ACTS - the body spins once/24 h though
    //      the author wrote no rotation. Kept for backward compat (D9, "likely the
    //      legacy behavior") and now LOGGED (D12; §2(f): names what fired, why,
    //      and the override). The orbit_period synchronous fallback is a distinct
    //      default (out of this row's scope - a §11.79(l) general-D12 follow-up).
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
    if (Utility::isTrue(param["hidden"]))
        body->hide();
    // star is initialized to the SYSTEM itself (valid light-position default
    // for starless systems), so "unassigned" is star == this, NOT !star - the
    // old !star test was dead and the Sun never became the star (its radius
    // stayed 0 -> zero penumbra, found by the S5 fidelity probe).
    if (Utility::isTrue(param["system_star"]) || (star == this && body->isStar() && parent == this))
        star = body;
    // Environment layer (S8, 2026-07-16): the old AtmosphereParams block
    // becomes per-body data + environment members. Parse gate and defaults
    // are protosystem.cpp:865-882 verbatim; absent block = the old
    // Body::defaultAtmosphereParams (limLandscape 10000, no atmosphere).
    if (!param["has_atmosphere"].empty() || !param["atmosphere_lim_landscape"].empty()) {
        body->envParams.hasAtmosphere = Utility::strToBool(param["has_atmosphere"], false);
        body->envParams.model = parseAtmosphereModel(param["atmosphere_model"]);
        body->envParams.limInf = Utility::strToFloat(param["atmosphere_lim_inf"], 40000.f);
        body->envParams.limSup = Utility::strToFloat(param["atmosphere_lim_sup"], 80000.f);
        body->envParams.limLandscape = Utility::strToFloat(param["atmosphere_lim_landscape"], 10000.f);
    }
    // Grounded landscape on every landable body - the old path showed a
    // landscape on ANY body below limLandscape (default params); which
    // landscape stays Core's association rule during migration.
    if (radius > 0)
        body->addEnvironment(std::make_unique<LandscapeEnv>(), true);
    // From-ground atmosphere iff the data declares one (bodies without it
    // fall to the EnvironmentState defaults = bodyAssign's !hasAtmosphere
    // branch).
    if (body->envParams.hasAtmosphere)
        body->addEnvironment(std::make_unique<AtmosphereEnv>(), false);
    // B24 compose gate (INTENT §11.78(d)): `compose = explicit` turns
    // deduction OFF for this body - its module list comes ONLY from the
    // file's BodyModule declarations (the generated twins emit this, so the
    // corpus equivalence test exercises the declaration path per slot).
    // Default `deduced` = legacy behavior; declarations then add/replace
    // per slot on top of deduction.
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
        // Explicit-slot declaration (§6.7 declaration half, INTENT §11.42): the GRID
        // slot is NOT deduced - deduceBodyModuleList returns a bare BodyModuleType
        // and cannot name a slot, and CUSTOM's default slot name ("CUSTOM") would
        // collide. A body opts in with planet_grid=true, installed through
        // loadModule's explicit `slot` argument into the named "GRID" slot. This
        // was the first user of that argument; the general declaration mechanism
        // is now the composed format's BodyModule sections (loadComposedSystem,
        // B24 - under compose=explicit a grid is one of those declarations).
        if (Utility::isTrue(param["planet_grid"]))
            ModuleLoaderMgr::instance.loadModule(BodyModuleType::CUSTOM, body, param, "GRID");
    }
    body->updateCache(); // Ensure bounding radius are properly set
}

ModularBody *ModularSystem::findBodyAt(const std::pair<float, float> &searchPos) const
{
    // Pick tolerance = the OLD path's own, expressed in this frame. cleverFind
    // takes candidates inside a 30-pixel circle (core.cpp:1049: fov per pixel
    // x 30), and screenPos is the angle over halfFov, so 30 px is
    // 30/viewportRadius here - ONE tolerance for both picking channels
    // (§11.52(b): old's observable is the spec).
    const float tol = 30.f / ModularBody::viewportRadius;
    const float tol2 = tol * tol;
    // TWO TIERS, both from recorded resolutions, in the old path's own shape.
    //
    //  (1) candidates whose CENTRE is inside the pick tolerance - A17's
    //      "candidates cluster inside the pick tolerance". BIGGEST WINS
    //      [R5, vixy §11.70(d): "the biggest should win because it'll be the
    //      brightest in 99% of cases due to surface magnitude"]. The size is
    //      the APPARENT one (screenSize = halfAngularSize/halfFov): surface
    //      magnitude is an apparent-area argument, and screenSize is what
    //      this selection surface itself exposes (ModularObject::
    //      getOnScreenSize is screenSize x viewportRadius, the pixels the
    //      pointer draws). boundingRadius would rank a distant giant above
    //      the moon filling the screen - the opposite of what R5 argues.
    //
    //  (2) nobody's centre in tolerance, but a body's DISC covers the pick:
    //      NEAREST TO THE OBSERVER wins - old's own rule for exactly this
    //      tier (searchAround stops at the first disc hit walking closest-
    //      first: "do not want any planets behind this one!",
    //      protosystem.cpp:344-355), and the body actually seen there.
    //      Keeping (2) separate from (1) is what keeps A17's own criterion
    //      ("if a body can be seen, we must be able to select it") true for
    //      a small body in front of a large disc: one merged biggest-wins set
    //      would make it permanently unselectable behind its parent.
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
        // A legacy file is not a write base: it is READ-ONLY forever (D35,
        // §2.0 D13), and its composed expression is the machine-owned twin,
        // built whole. Cleared rather than left, because reloadSystem re-enters
        // here and a stale record of a PREVIOUS composed load would then be
        // written back as if it described this content.
        loadedSections.clear();
        stringHash_t bodyParams;
        // ONE line grammar for the whole .ini family (tools/ini_line.hpp,
        // INTENT §5.39/D29): this reader used to do its own substr arithmetic,
        // which required exactly "key = value" - `radius  = 100` bound the key
        // "radius " and the body silently got no radius, and the twin generator
        // reading the SAME file through ModularSystemFormat::parse disagreed
        // with it on seven shipped keys.
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
                    // §2(f): what fired, where, and what to do. Reachable on the
                    // SHIPPED corpus - `[Sedna] orbit_LongOfPericenter 95.58754`
                    // has no '=' and has been silently turning into a garbage key
                    // (never read by anything) for as long as it has shipped.
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
    // The file's own record, KEPT (b31-design §5.3): every line of it, in order.
    // Two things need it - a save gives the file back whole instead of
    // rebuilding it, and the diagnoses the loaders produce below annotate the
    // very sections they came from. Nothing is written here: a load NEVER
    // rewrites the user's file (D33, §11.113(l) - decided against, not
    // undecided). Iterating the MEMBER is what makes the section addresses
    // handed to the loaders outlive the load.
    loadedSections = std::move(sections);
    // This file's node sections by body name - the overlay base for module
    // declarations (a module's effective params = its node's params overlaid
    // by the module section's own keys).
    std::map<std::string, stringHash_t> nodeParams;
    for (auto &section : loadedSections) {
        // The lines before the file's first '[' - a banner, a note - declare
        // nothing. The parse keeps them so a rewrite gives them back
        // (§11.66(b)); a loader has nothing to do with them.
        if (section.isPreamble())
            continue;
        // What this section says, as the capability layer wants it. The section
        // itself stays as the file wrote it: this is a copy, and the loaders
        // below fill defaults into their copy (loadBody does), which is exactly
        // why they may not be handed the file's own record of what it contains.
        stringHash_t params = section.params();
        // D16 (INTENT §11.79(j)): ONE `type=` key carries the declaration kind.
        // A value in the module-family vocabulary (ModuleLoaderMgr's own enum,
        // I2) declares a BodyModule OF that family; anything else declares a
        // ModularBody node - the value is then the body-type (`BODY`, a legacy
        // Planet/Moon/Sun carried transitionally on the node until B27/B25-emit
        // materializes capability keys, or absent), read by loadBody exactly as
        // the legacy loader reads it. The two `type=` roles never collide: a
        // module section names its node with `body=`, a node never does - the
        // presence of that binding disambiguates a mistyped family (§2(f)),
        // never a value guess across the composed/legacy namespaces (kept
        // distinct by which loader runs, §11.79(j)).
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
    // Effective params: the node's params overlaid by this section's own keys
    // (module key wins); the grammar's own keys are not data. `type` is skipped
    // deliberately - it names the module FAMILY here, and must NOT overwrite the
    // node's own `type=` (its body-type, which loaders like OjmLoader read from
    // the overlaid params, D16 §11.79(j)).
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
        // The legacy section's content. The TWIN is machine-owned and built
        // whole, section by section, from what the live body IS - it is not an
        // edit of the legacy file and never carries its layout (the legacy file
        // is READ-ONLY forever, D35 §11.113(n)).
        stringHash_t legacy = section.params();
        const std::string name = legacy["name"];
        if (name.empty())
            continue; // the legacy load skipped it too ("Can't load unnamed body")
        ModularBody *body = ModularBody::findBodyOnce(name);
        if (!body)
            continue; // not loaded (orphan/invalid orbit) - not part of the semantic content
        // ... and it must be OUR body. findBodyOnce reads the GLOBAL name
        // registry, which answers "does a body with this name exist anywhere",
        // not "did THIS system load it" - and loadBody SKIPS a section whose
        // name is already taken ("already exists and replace param isn't
        // true"), so for a name another system loaded first the lookup succeeds
        // and returns a FOREIGN body. Without this test the twin of system S
        // declares bodies of system H, with H's live capabilities, and stops
        // being the composed equivalent of S's own legacy load (§11.78(f)'s
        // contract). Reachable on the SHIPPED galactic.ini, which points five
        // entries (HelixDwarf/M57Dwarf/M1Pulsar/M27Dwarf/NGC2392Dwarf) at one
        // stellar_systems file: measured 5 byte-identical twins for 4 empty
        // systems + 1 real one (INTENT §11.109(c)). The solar twin is
        // unaffected - every body of ssystem.ini is in SolarSystem's subtree.
        if (!body->isInSubtreeOf(this)) {
            cLog::get()->write("Composed twin of " + legacyFilename + ": section '" + name
                + "' is NOT declared, because a body named '" + name + "' was already loaded by "
                "another system and this system's section was skipped at load time. The twin "
                "describes what THIS system contains. To fix: rename the body, or add "
                "'replace = true' to the section that should win.", LOG_TYPE::L_WARNING);
            continue;
        }
        // ONE emitter (I2): the twin and the live-tree save write the same
        // declaration for the same body, from the same rules - the twin's
        // declaration record is the legacy section it just read, the save's is
        // the body's own (b31-design §4.1).
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
    // What the data said, verbatim (D16 §11.79(j)): the node's own `type=` (its
    // body-type, e.g. Planet/Moon/Sun) is a NON-family value, which is exactly
    // what marks the section as a node - no separate declaration key is emitted.
    stringHash_t node = declared;
    {
        const auto it = declared.find("bound_to_surface");
        if (it != declared.end() && Utility::isTrue(it->second)) {
            node.erase("bound_to_surface");   // translated, not duplicated -
            node["relation"] = "grounded";    // one relation authority per generated file
        }
    }
    // B25-emit / §11.73 A1+A2: materialize the capabilities the legacy name
    // sniff (applyHardcodedContent) granted this LIVE body as explicit keys,
    // so the composed load - which does NOT run that sniff (D14 §11.79(h)) -
    // reproduces them. Read the body's actual capability, not its name (I4):
    // the generator emits whatever the body IS. Only ADD when the declaration
    // did not already carry the key (explicit data is preserved verbatim
    // above); the sniff only ever sets these for Earth, so this is the ONE node
    // that gains them on the shipped corpus.
    if (body->getSiderealTimeModel() == SiderealTimeModel::EARTH_APPARENT
            && !node.count("sidereal_time"))
        node["sidereal_time"] = "earth_apparent";
    if (!node.count("shadow_color")
            && body->getShadowAbsorbtion() != Vec3f{1, 1, 1})
        node["shadow_color"] = Utility::vec3fToStr(body->getShadowAbsorbtion());
    // B27 tail / D14 (§11.79(h)): the capabilities the legacy `type` string
    // carried are materialized as KEYS here - the format boundary is exactly
    // where they must become explicit, because the composed load no longer
    // reads `type` for any of them. Same rule as above: read what the body
    // IS (I4), and only ADD where the declaration did not already declare
    // it. Emitted only when the value differs from the composed default, so
    // a file carries a key exactly where its absence would change something
    // (the co-delivery contract, §11.73(g): every key consumed is emitted).
    if (body->getSurfaceModel() == SurfaceModel::LUNAR
            && !node.count("surface_model"))
        node["surface_model"] = "lunar";
    if (body->getTrailLength() != TRAIL_LENGTH_DEFAULT
            && !node.count("trail_length"))
        node["trail_length"] = std::to_string(body->getTrailLength());
    // D27's own requirement (§11.113(f)): a legacy star's `type` grants BOTH
    // halves of the split, so both keys are emitted, value for value -
    // emit one and the composed load stops reproducing strToBodyType.
    if (body->isStar() && !node.count("light_source"))
        node["light_source"] = "true";
    if (body->isPrimary() && !node.count("primary"))
        node["primary"] = "true";
    if (body->isMinorBody() && !node.count("shadow_exempt"))
        node["shadow_exempt"] = "true";
    return node;
}

void ModularSystem::appendWholeDeclaration(ModularBody *body, const stringHash_t &declared,
                                           std::vector<ModularSystemFormat::Section> &out)
{
    // The section header is decorative; the name key is the identity. Use what
    // the declaration says when it says anything, so a machine-built file reads
    // exactly like the declaration it came from.
    const auto nameIt = declared.find("name");
    const std::string name = (nameIt == declared.end() || nameIt->second.empty())
        ? body->getEnglishName() : nameIt->second;
    stringHash_t node = composedNodeParams(body, declared);
    // `compose = explicit` turns deduction OFF, so it and the declarations below
    // are ONE decision and are emitted together - a file carrying the key
    // without them would load a body with no modules at all.
    node["compose"] = "explicit";
    out.push_back(ModularSystemFormat::Section::fromParams(name, node));
    // One BodyModule declaration per family - the decomposition the twin exists
    // to make visible [vixy, §11.50(b)]. `type=<family>` is the one declaration
    // key (was declare=BodyModule + module=<family>, both retired by D16
    // §11.79(j)).
    // WHICH LIST, and why it is not always the live one: the modules of a body
    // whose declaration DEDUCES them are what deduction makes of that
    // declaration, and re-deriving them keeps their ORDER - which is semantic
    // (a routing list holds modules in record order, and that is the draw order:
    // the atmosphere shell draws after the disc, §11.19). The live slot list is
    // ordered by global slot id and would silently re-order them. A body whose
    // modules did NOT come from deduction (`compose = explicit`) has no such
    // derivation, and its live set is then the only honest answer.
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
    // Parents first: a declaration may only name a body declared before it (the
    // findBody forward-reference rule the format inherits from the legacy
    // loader), so the walk order IS a correctness condition, not a preference.
    // Hidden children are walked like any other: a hidden body is declared data
    // ([Goldilocks_Zone] ships hidden = true) and its `hidden` key travels in
    // its own declaration, so it comes back hidden.
    // A nested system node ENDS the walk: its content is declared by its own
    // file, and the node itself by whoever created the system (galactic.ini,
    // SSystemFactory) - neither is this file's to write.
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

bool ModularSystem::saveSystem(const std::string &outPath)
{
    // THE BASE: what the target file already contains, whenever it contains
    // anything. Three sources, in this order of authority:
    //  (1) the sections this system was LOADED from, when the save targets that
    //      same file - they are that file's content AND they carry the loader's
    //      annotations, which is what makes an explicit save the moment the
    //      diagnoses reach the file (b31-design §5.3, D33);
    //  (2) a parse of the target, when it exists but is not our source - its
    //      author's content is preserved exactly as (1)'s is;
    //  (3) nothing: a file that does not exist yet is built whole, like the twin.
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
    // Every body of this system that the file does not declare yet. The DECLARED
    // set is read from the file itself (a node section = one that carries a
    // `name` and binds no body), so a file that already describes a body is
    // never given a second, conflicting section for it.
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
        // No declaration, nothing to write: an engine-minted body (a camera
        // anchor, the B5 pilot oort) was never asked for by data, and turning
        // one into authored content would make the next launch load it twice -
        // once from the file, once from the code that mints it.
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
    // LEGACY-FORMAT ONLY (gated on !composedFile at the call site, D14 §11.79(h)):
    // the composed format declares these capabilities as keys (B25-emit, §11.73).
    if (createInfo.englishName == "Earth") {
        // A1 (§11.73): apparent sidereal time is now the sidereal_time capability
        // (SiderealTimeModel::EARTH_APPARENT), set here for the legacy Earth (the
        // twin emits sidereal_time=earth_apparent for the composed Earth). The old
        // BodyType::EARTH tag is RETIRED - its two consumers (computeAxisRotation,
        // getSiderealTime) now read this selector.
        // PRECEDENCE (§11.102(c) -> §11.103): explicit data WINS over the sniff,
        // the rule this whole gate block states for itself above ("key present ->
        // explicit data wins") and the shadow_color branch below already honors.
        // Without the guard an authored `sidereal_time = generic` was silently
        // overridden here, applyHardcodedContent being the LAST writer (createInfo
        // is built first, this runs after).
        // The EMPTINESS test is load-bearing, not defensive: the createInfo build
        // above reads param["sidereal_time"] through std::map::operator[], which
        // INSERTS an empty entry when the key is absent - so `find() == end()`
        // alone is never true here and would retire the sniff outright (the legacy
        // Earth would lose apparent sidereal time). Absent and present-but-empty
        // are one case for parseSiderealTimeModel (both -> GENERIC), so testing
        // both is testing "the data authored nothing".
        const auto siderealIt = param.find("sidereal_time");
        if (siderealIt == param.end() || siderealIt->second.empty())
            createInfo.siderealTimeModel = SiderealTimeModel::EARTH_APPARENT;
        // A2 (§11.73): Earth's shadow absorbs G/B more than R - red light
        // diffracted by the atmosphere reaches the umbra (lunar-eclipse color;
        // replaces the old my_moon UmbraColor hardcode). VALUE IS DERIVED, not
        // tuned [visual-fidelity mandate, vixy 2026-07-12]: old composition
        // diffuse*(s + U*(1-s)) with U = UmbraColor(0.4, 0.12, 0) equals the
        // new diffuse*(1 - cov*a) EXACTLY under a = 1-U, cov = 1-s - so
        // {0.6, 0.88, 1.0} reproduces the old model across the whole
        // penumbra; the only residual is coverage-profile shape
        // (shadow-paths.md D2). Explicit shadow_color in the data still wins
        // (loadBody reads it after this call). The twin emits this as the
        // shadow_color key so the composed Earth reproduces it without the sniff.
        if (param.find("shadow_color") == param.end())
            createInfo.shadowAbsorbtion = Vec3f(0.6f, 0.88f, 1.0f);
    }
    // The "Moon" -> BodyType::EARTH_MOON branch is RETIRED (B25-emit, §11.73 A3):
    // EARTH_MOON had ZERO consumers, so it granted no behavior and needs no
    // capability key. The Moon's surface-shader lineage rides its `type=Moon`
    // (LayeredMeshLoader, A6 - a later Tier-B step, not this row).
}

// Contract + rationale: ModularSystem.hpp (reloadSystem).
bool ModularSystem::reloadSystem()
{
    if (systemFilename.empty())
        return false;
    // Every body about to be destroyed may still be referenced by a frame in
    // flight: the drawing thread records shadow geometry straight from
    // module-owned Ojm buffers, and the GPU is executing the frames before it.
    // Releasing under them is INTENT 5.58 - the SIGSEGV in Ojm::drawShadow on
    // the helper thread. Paid once per commanded reload, against a rebuild
    // that re-reads the whole data file.
    if (Context::instance)
        Context::instance->quiesceFrames();
    clearChildren();
    if (composedFile)
        loadComposedSystem(systemFilename);
    else
        loadSystem(systemFilename);
    return true;
}
