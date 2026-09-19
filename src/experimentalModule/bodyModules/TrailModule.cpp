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
    points.clear();
    firstPoint = true;
}

void TrailModule::startTrail(bool record)
{
    recording = record;
    if (record)
        resetTrail();
}

void TrailModule::setShown(bool b)
{
    nameOverride = b ? 1 : 0;
    overrideGen = flagGeneration;
    if (b)
        resetTrail();
    if (b && !live) { live = true; ++activeCount; }
}

void TrailModule::accumulate(ModularBody *body)
{
    ++accumulateCount; // instrument (INTENT 11.56): "the work actually ran"
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
        resetTrail();
        return;
    }
    const Orbit *orbit = body->getOrbit();
    if (!orbit) {
        cLog::get()->write("Trail of '" + body->getEnglishName() + "': the "
            + std::to_string(missed) + " sample(s) missed while the body was hidden "
            "cannot be reconstructed, because this body has no orbit to evaluate at a "
            "past date. The trail restarts from the current position instead of "
            "resuming. To keep a continuous trail across a hide, give the body a "
            "time-parametrized orbit, or leave it shown.", LOG_TYPE::L_WARNING);
        resetTrail();
        return;
    }
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
    if (want) {
        if (!recording) { // rising edge of the flag = fresh restart
            recording = true;
            resetTrail();
        }
        accumulate(body);
    } else {
        recording = false; // falling edge: the work stops on this very frame
        if (!live)
            resetTrail();
    }
    boundingRadius = scaledRadius;
    return false;
}

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
        << ",\"color\":[" << color[0] << ',' << color[1] << ',' << color[2] << "]"
        << ",\"head\":";
    if (points.empty()) {
        out << "null,\"headJD\":null,\"tailJD\":null,\"pathLength\":0";
    } else {
        out << '[' << points.front().pos[0] << ',' << points.front().pos[1]
            << ',' << points.front().pos[2] << "],\"headJD\":"
            << std::setprecision(17) << points.front().jd
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
