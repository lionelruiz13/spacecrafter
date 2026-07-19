#include "ModularSystem.hpp"
#include "ModuleLoaderMgr.hpp"
#include "environmentModules/LandscapeEnv.hpp"
#include "environmentModules/AtmosphereEnv.hpp"
#include "tools/log.hpp"
#include "tools/sc_const.hpp"
#include "tools/context.hpp"
#include "meshModules/bodyShaderInterface.hpp" // MAX_SHADOW_CASTERS_PER_RECEIVER
#include "bodyModules/OrbitModule.hpp" // orbit-pass activity gate (row 8)
#include "bodyModules/TrailModule.hpp" // trail-pass activity gate (row 9)
#include "bodyModules/TailModule.hpp"  // tail-pass activity gate (row 12)
#include <algorithm>
#include <cfloat> // FLT_MAX (within-body pair rank)
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
    // Starless systems (galaxy/universe level) leave the light state alone -
    // lightPosition is CURRENT-SYSTEM-scoped (nested draws save/restore it,
    // drawSystem dispatch), and no body of a starless system consumes it
    // outside the delegation path.
    if (ModularBody *s = getSystemStar())
        s->updateAsLightSource();
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
            if (!(*body && body->screenSize > 0.0015f) || body->isStar() || body->bodyType == BodyType::MINOR_BODY)
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
        if (!body || body->distance == 0 || body->isStar() || body->bodyType == BodyType::MINOR_BODY)
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
        if (!(*body && body->screenSize > 0.0015f) || body->isStar() || body->bodyType == BodyType::MINOR_BODY)
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
    if (!(isVisible & isBodyVisible))
        return;
    // px full diameter of the subsystem on screen - the drawHalo/pointer px
    // idiom. At/above the constant: the interior is content (per-child
    // visibility gating does the rest - D3); below: one point of light.
    if (screenSize * 2.f * viewportRadius >= SYSTEM_VISIBILITY_SUBSYSTEM_SIZE) {
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
    } else {
        drawStarProxy(renderer);
    }
}

void ModularSystem::drawStarProxy(Renderer &renderer)
{
    ModularBody *s = getSystemStar();
    if (!s || !s->isHaloEnabled)
        return;
    // Star branch of computeMagnitude (factor = distance^2) at the NODE's
    // fresh distance - the star's own cached distance is stale while its
    // system isn't current. Disc floor = the STAR's disc at that distance
    // (small angle), never the subsystem extent (which would inflate the
    // halo floor up to the nested-draw threshold).
    const float mag = -26.73f - 2.5f * log10f(distance * distance);
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
    // Relation from data BEFORE creation - relation is the ownership
    // authority (boundToSurface is its cache, written by createChild only).
    ModularBody *body = parent->createChild(createInfo,
        Utility::isTrue(param["bound_to_surface"]) ? BodyRelation::GROUNDED
                                                   : BodyRelation::ORBITING);
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
    for (auto moduleType : body->deduceBodyModuleList(param))
        ModuleLoaderMgr::instance.loadModule(moduleType, body, param);
    // Explicit-slot declaration (§6.7 declaration half, INTENT §11.42): the GRID
    // slot is NOT deduced - deduceBodyModuleList returns a bare BodyModuleType
    // and cannot name a slot, and CUSTOM's default slot name ("CUSTOM") would
    // collide. A body opts in with planet_grid=true, installed through
    // loadModule's explicit `slot` argument into the named "GRID" slot. This is
    // the first and only user of that argument - the concrete, minimal form of
    // the D2 explicit-declaration mechanism (the general param syntax is still
    // under-specified, suspended for Vixy).
    if (Utility::isTrue(param["planet_grid"]))
        ModuleLoaderMgr::instance.loadModule(BodyModuleType::CUSTOM, body, param, "GRID");
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
