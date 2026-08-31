#include "TrailModule.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "tools/log.hpp"
#include <cmath>
#include <cstdlib>
#include <ostream>
#include <iomanip>

bool TrailModule::show = false;           // config flag_object_trails default (setFlagTrails seam)
uint32_t TrailModule::flagGeneration = 0; // bumped by every global toggle
int TrailModule::activeCount = 0;         // modules with a live fader (phase gate)
Vec3f TrailModule::defaultColor{1.f, 0.5f, 0.f}; // config object_trails_color (checkConfig default)

// TRAIL line family - third line-class family (AXIS S11.31, ORBIT S11.39 were
// the first two). body_trail.{vert,geom,frag} REUSED VERBATIM (parity by
// construction). Its own push-constant contract (INTENT S10.1 "orbit/trail/
// axis/grid as separate families where push contracts differ"): a FRAGMENT
// color at 0 (old uColor) and the VERTEX {int nbPoints, mat4 ModelViewMatrix,
// float fader} at 12 (old layoutTrail, trail.cpp:192-193). The shaders read
// main_clipping_fov from cam_block (context.uboSet) - the family binds the
// global UBO set (the SAME camera-block authority the new-path body shaders
// use, I2), NOT a pushed clipping_fov (unlike AXIS/ORBIT, whose shaders were
// converted to push in the 2023-master merge; body_trail was not). LINE_STRIP
// fed to the geometry shader (segment wrap-cull + subdivision), BLEND_SRC_ALPHA
// carries the per-vertex fade alpha, NO depth (old setDepthStencilMode() =
// test+write off). Spec-const 8 registry-injected (S11.33).
namespace {
struct TrailFamilyData {
    std::unique_ptr<VertexArray> vertexModel; // 1 binding, vec3 pos (m_dataGL parity)
    PipelineFamily family;
};
TrailFamilyData &trailFamily()
{
    static TrailFamilyData data = []() -> TrailFamilyData {
        Renderer &renderer = Context::instance->renderer;
        TrailFamilyData d;
        d.vertexModel = std::make_unique<VertexArray>(*VulkanMgr::instance, 3*sizeof(float));
        d.vertexModel->createBindingEntry(3*sizeof(float));
        d.vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);
        PipelineFamilyDesc desc;
        desc.name = "TRAIL";
        desc.vertex = d.vertexModel.get();
        // cam_block at set 0 (body_trail reads main_clipping_fov from it).
        desc.sets.push_back(renderer.globalUboContract());
        // Two push ranges, index = the pushConstant() index used at draw (old
        // trail.cpp:107/113): 0 = frag color, 1 = vert {nbPoints, mat, fader}.
        desc.pushConstants = {
            {VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint16_t>(sizeof(Vec3f))},
            {VK_SHADER_STAGE_VERTEX_BIT, static_cast<uint16_t>(sizeof(Vec3f)),
             static_cast<uint16_t>(sizeof(int) + sizeof(Mat4f) + sizeof(float))},
        };
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "body_trail.vert.spv",
                                  .geom = "body_trail.geom.spv",
                                  .frag = "body_trail.frag.spv"}}};
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        color.state.blend = BLEND_SRC_ALPHA; // per-vertex fade alpha (old default)
        color.state.cull = false;            // lines: no face culling
        color.state.depthTest = false;       // old setDepthStencilMode(): no depth
        color.state.depthWrite = false;
        desc.passes.push_back(std::move(color));
        d.family = renderer.allocateFamily(std::move(desc));
        return d;
    }();
    return data;
}
} // namespace

TrailModule::TrailModule(const Vec3f &color, int maxTrail, double deltaTrail)
    : BodyModule(BodyModuleType::TRAIL), color(color), authoredColor(color), maxTrail(maxTrail), deltaTrail(deltaTrail) {}

TrailModule::~TrailModule()
{
    if (live) // keep the phase-gate counter balanced on removal (body drop)
        --activeCount;
}

bool TrailModule::wantShown(ModularBody *body) const
{
    // A per-name override wins only until the next global toggle (old parity:
    // a global setFlagTrails clobbered every per-body fader).
    if (nameOverride >= 0 && overrideGen == flagGeneration)
        return nameOverride != 0;
    return show;
}

void TrailModule::resetTrail()
{
    // THE fresh start, single authority (I2) for every path that re-enables
    // recording. Vixy Q14 [2026-07-21, INTENT 11.48(a) A1 -> 11.56] is explicit
    // that re-enabling starts FRESH, so a fresh start DISCARDS the recorded
    // history outright: the buffer is emptied (not merely marked stale), which
    // makes it impossible for pre-off history to be revealed later and frees
    // the memory the gate stopped paying CPU for.
    points.clear();
    firstPoint = true;
}

void TrailModule::startTrail(bool record)
{
    // Old Trail::startTrail (trail.cpp:165-174): enable => fresh restart;
    // disable => stop. Old-path seam only (startTrails, INTENT 11.41(d)); the
    // new path drives the gate from the display flag through update().
    recording = record;
    if (record)
        resetTrail();
}

void TrailModule::setShown(bool b)
{
    // Old Body::setFlagTrail -> Trail::setFlagTrail (fader target + startTrail):
    // set the override AND reset on enable (first_point). Same Q14 rule as the
    // global flag - a per-name enable is a re-enable, so it starts fresh too.
    nameOverride = b ? 1 : 0;
    overrideGen = flagGeneration;
    if (b)
        resetTrail();
    // Kick the system-level trail phase so this module's update() runs and its
    // fader can rise even when the global master is off (the phase is gated on
    // anyActive() - OrbitModule precedent).
    if (b && !live) { live = true; ++activeCount; }
}

void TrailModule::accumulate(ModularBody *body)
{
    ++accumulateCount; // instrument (INTENT 11.56): "the work actually ran"
    // Faithful port of Trail::updateTrail (trail.cpp:120-163). Sampled at the
    // body's SIM time (getLastJD - fresh even when invisible), parent-relative
    // position (getEclipticPos - old get_heliocentric_ecliptic_pos for the trail
    // set, whose parent is the ~fixed system root). points NEWEST FIRST.
    const double date = body->getLastJD();
    int dt = 0;
    // First point, or a time jump bigger than the whole window: clear + restart.
    // (Answers "time jump: clear or interpolate?" - old CLEARS, no interpolation.)
    if (firstPoint || (dt = std::abs(static_cast<int>((date - lastJD) / deltaTrail))) > maxTrail) {
        dt = 1;
        points.clear();
        firstPoint = false;
    }
    // Add ONE point at the current position when >= DeltaTrail has elapsed
    // (old "add only one point at a time" - detail lost on big jumps, by design).
    if (dt) {
        lastJD = date;
        points.insert(points.begin(), {body->getEclipticPos(), date}); // push_front
        if (static_cast<int>(points.size()) > maxTrail)
            points.pop_back(); // drop the oldest
    }
    // Prune points beyond the time window (old trail.cpp:157-162; |.|/DeltaTrail
    // handles retrograde time + jumps in either direction). points newest-first,
    // so the stale ones are at the tail.
    for (size_t i = 0; i < points.size(); ++i) {
        if (std::fabs(points[i].jd - date) / deltaTrail > maxTrail) {
            points.erase(points.begin() + i, points.end());
            break;
        }
    }
}

// THE UNHIDE EDGE (B39 S11.117 / D23 clause iv: "behave as if they never were
// hidden when unhidden"). See the header for the two behind-state halves.
void TrailModule::resumeAfterHidden(ModularBody *body)
{
    const bool want = wantShown(body);
    // (1) The DISPLAY fader advances in WALL time and stopped with the sweep.
    // Snapping it to its target is the as-if answer: a fade lasts under a second
    // and the body was gone for at least a frame, so by the time it is back the
    // ramp is over. Without this, a trail switched OFF while the body was hidden
    // fades out AFTER the body reappears - visible, and visibly wrong.
    fader.reset(want);
    // Keep the phase-gate counter consistent with the snapped fader: update()
    // maintains this pairing, and it did not run while the body was parked.
    const bool nowLive = fader.getInterstate() > 1e-6f;
    if (nowLive != live) {
        live = nowLive;
        activeCount += nowLive ? 1 : -1;
    }
    if (!want || !recording || firstPoint || points.empty())
        return; // nothing was being recorded: nothing to reconstruct
    // (2) The recorded HISTORY. `date` is the body's own sim time, already
    // brought to the current frame by the D8 barrier before this call.
    const double date = body->getLastJD();
    const int missed = static_cast<int>((date - lastJD) / deltaTrail);
    if (missed <= 0)
        return; // less than one sampling period was missed
    if (missed > maxTrail) {
        // The hidden span is longer than the whole time window: every surviving
        // sample would have been pruned anyway, so the honest state is a fresh
        // start - and it is accumulate()'s OWN answer to the same situation
        // (time jump bigger than the window), reused rather than re-decided.
        resetTrail();
        return;
    }
    const Orbit *orbit = body->getOrbit();
    if (!orbit) {
        // THE ONE NAMED RESIDUAL of D23 (S11.113(b)(iv)): a past that is not a
        // function of time cannot be reconstructed. Degrade to a fresh start and
        // SAY SO (S2.0 D12 - a behaviour the author did not write must be
        // visible; S2(f) shape: what happened, why, what was done, what to do).
        cLog::get()->write("Trail of '" + body->getEnglishName() + "': the "
            + std::to_string(missed) + " sample(s) missed while the body was hidden "
            "cannot be reconstructed, because this body has no orbit to evaluate at a "
            "past date. The trail restarts from the current position instead of "
            "resuming. To keep a continuous trail across a hide, give the body a "
            "time-parametrized orbit, or leave it shown.", LOG_TYPE::L_WARNING);
        resetTrail();
        return;
    }
    // Re-evaluate the missed samples at the module's OWN declared cadence
    // (deltaTrail), oldest first, inserting at the front so `points` stays
    // newest-first. Each sample is evaluated 1 + RESUME_EXTRA_ITERATIONS times at
    // its own date: EllipticalOrbit/IterativeEll advance ONE Newton step per
    // call from the previous call's seed, so a single call at a jumped-to date
    // would not be the position at that date - the same reason the S11.76(b)
    // barrier exists, applied per reconstructed sample.
    OsculatingFunctionType *osc = orbit->getOsculatingFunction();
    Vec3d tmp;
    double sampleJD = lastJD;
    for (int k = 1; k <= missed; ++k) {
        sampleJD = lastJD + k * deltaTrail;
        for (int i = 0; i <= RESUME_EXTRA_ITERATIONS; ++i) {
            if (osc)
                (*osc)(date, sampleJD, tmp);
            else
                orbit->positionAtTimevInVSOP87Coordinates(date, sampleJD, tmp);
        }
        points.insert(points.begin(),
                      {Vec3f(tmp[0], tmp[1], tmp[2]), sampleJD});
    }
    lastJD = sampleJD;
    if (static_cast<int>(points.size()) > maxTrail)
        points.resize(maxTrail); // drop the oldest (newest-first buffer)
    // Same time-window prune as accumulate(), against the same `date`.
    for (size_t i = 0; i < points.size(); ++i) {
        if (std::fabs(points[i].jd - date) / deltaTrail > maxTrail) {
            points.erase(points.begin() + i, points.end());
            break;
        }
    }
    cLog::get()->write("Trail of '" + body->getEnglishName() + "': reconstructed "
        + std::to_string(missed) + " sample(s) missed while hidden, from the body's "
        "orbit at their own dates.", LOG_TYPE::L_DEBUG);
}

bool TrailModule::update(ModularBody *body, float scaledRadius)
{
    const bool want = wantShown(body);
    fader = want;
    fader.update(static_cast<int>(ModularBody::deltaTime));
    const float interstate = fader.getInterstate();
    // Maintain the phase-gate counter: a module counts while its fader is live.
    const bool nowLive = interstate > 1e-6f;
    if (nowLive != live) {
        live = nowLive;
        activeCount += nowLive ? 1 : -1;
    }
    // ---- THE RECORDING GATE (INTENT 11.56; closes the 11.41 suspension) ----
    // Vixy Q14 [2026-07-21, INTENT 11.48(a) A1]: the DISPLAY FLAG gates
    // RECORDING - `flag object_trails off` STOPS the accumulation, and
    // re-enabling starts FRESH. The stated reason is cost: nobody should pay
    // for accumulating a trail nobody sees.
    // The gate is `want` (the flag / per-name override), NOT the fader
    // interstate: a toggle inside the ~1 s fade window is still a re-enable and
    // must still start fresh, and an `off` must stop the work AT ONCE rather
    // than one fade-length later. (The fader remains the DISPLAY gate - draw()
    // reads it - so the fade-out is unchanged.)
    // This gate is INDEPENDENT of the body's visibility. A HIDDEN body keeps
    // recording [vixy Q13 / A10, INTENT 11.54]: drawTrails sweeps every
    // EVALUATED body, hidden ones included (their eclipticPos/lastJD/distance
    // ride recursiveTranslationUpdate), so `want` is the only thing that can
    // stop accumulation. Two conditions, two observables - reading them as one
    // gate produces a wrong implementation (S13.B B11).
    if (want) {
        if (!recording) { // rising edge of the flag = fresh restart
            recording = true;
            resetTrail();
        }
        accumulate(body);
    } else {
        recording = false; // falling edge: the work stops on this very frame
        // The points are kept while the fader is still up so the fade-out
        // draws the trail it was showing (old fade parity), then DISCARDED the
        // moment nothing can display them any more - after which this module
        // holds no history at all until the next re-enable.
        if (!live)
            resetTrail();
    }
    // TRAIL never inflates the body's boundingRadius (it is not in a regime
    // list): this return is consumed by nobody - kept for the interface. The
    // trail needs the per-frame tick, so it never self-deregisters (returns false).
    boundingRadius = scaledRadius;
    return false;
}

// Harness instrument (INTENT 11.56) - the recording gate's observable.
// `accumulateCount` is what separates "the gate stopped the WORK" from "the
// gate only stopped the DRAWING": it counts entries into accumulate(), so a
// frozen counter over an interval in which simulated time advanced is direct
// evidence from the running process that the accumulation code did not run.
// `points`/`head` carry the discard evidence (0 while off, and the first point
// after a re-enable sits at the body's CURRENT position, not at pre-off
// history). `fader` is the DISPLAY state, deliberately dumped next to
// `recording` so the two gates can be read apart.
void TrailModule::dumpState(std::ostream &out) const
{
    out << std::setprecision(9)
        << "{\"points\":" << points.size()
        << ",\"recording\":" << (recording ? "true" : "false")
        << ",\"firstPoint\":" << (firstPoint ? "true" : "false")
        << ",\"fader\":" << fader.getInterstate()
        << ",\"accumulateCount\":" << accumulateCount
        << ",\"maxTrail\":" << maxTrail
        << ",\"deltaTrail\":" << deltaTrail
        // Trail color (B29 runtime-color instrument, INTENT S11.65): the
        // per-instance TRAIL channel drawn by this module (old BodyColor::trail).
        // Lets the harness read the runtime recolor + reload behaviour.
        << ",\"color\":[" << color[0] << ',' << color[1] << ',' << color[2] << "]"
        << ",\"head\":";
    if (points.empty()) {
        out << "null,\"headJD\":null,\"tailJD\":null,\"pathLength\":0";
    } else {
        out << '[' << points.front().pos[0] << ',' << points.front().pos[1]
            << ',' << points.front().pos[2] << "],\"headJD\":"
            << std::setprecision(17) << points.front().jd
            // Oldest sample's date + the POLYLINE LENGTH (B39 S11.117): together
            // with `points` they make the recorded history's GEOMETRY observable,
            // not just its size. That is what separates "n samples appeared" from
            // "n samples that trace this body's actual orbit": length/span is the
            // body's mean orbital speed, so a reconstruction placed anywhere else
            // fails by orders of magnitude, and a reconstruction that left a GAP
            // shows up as a chord shortcut. AU, parent-relative (the frame the
            // samples live in).
            << ",\"tailJD\":" << points.back().jd << std::setprecision(9);
        double len = 0;
        for (size_t i = 1; i < points.size(); ++i)
            len += (points[i].pos - points[i - 1].pos).length();
        out << ",\"pathLength\":" << std::setprecision(12) << len << std::setprecision(9);
    }
    out << '}';
}

void TrailModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // mat = the PARENT position frame (ModularSystem::drawTrails) - the frame the
    // parent-relative points live in (same as the ORBIT pass).
    const float alpha = fader.getInterstate();
    const int n = static_cast<int>(points.size());
    if (alpha <= 1e-6f || n < 2) // old doDraw: fader && trail.size() >= 2
        return;
    const FamilyBound bound = renderer.bind(trailFamily().family);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - C3 degrade
    if (!line)
        line = trailFamily().vertexModel->createBuffer(0, maxTrail,
                                                       Context::instance->globalBuffer.get());
    // Upload the current points (newest first, index 0 = brightest - the vert
    // shader's indice = (1 - 0.9*idx/nbPoints)*fader).
    Vec3f *v = static_cast<Vec3f *>(
        Context::instance->transfer->planCopy(line->get(), 0, n * static_cast<int>(sizeof(Vec3f))));
    if (!v)
        return; // transfer staging full this frame - skip (C3, no stall)
    for (int i = 0; i < n; ++i)
        v[i] = points[i].pos;

    bound.layout->pushConstant(renderer, 0, &color);
    struct {
        int nbPoints;
        Mat4f modelViewMatrix;
        float fader;
    } cst {n, mat, alpha};
    bound.layout->pushConstant(renderer, 1, &cst);
    bound.layout->bindSet(renderer, *Context::instance->uboSet); // cam_block (main_clipping_fov)
    VertexArray::bind(renderer, line->get());
    vkCmdDraw(renderer, n, 1, 0, 0);
}
