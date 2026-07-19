#include "TrailModule.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include <cmath>
#include <cstdlib>

bool TrailModule::show = false;           // config flag_object_trails default (setFlagTrails seam)
uint32_t TrailModule::flagGeneration = 0; // bumped by every global toggle
int TrailModule::activeCount = 0;         // modules with a live fader (phase gate)
Vec3f TrailModule::defaultColor{1.f, 0.5f, 0.f}; // config object_trails_color (checkConfig default)

// TRAIL line family - third line-class family (AXIS §11.31, ORBIT §11.39 were
// the first two). body_trail.{vert,geom,frag} REUSED VERBATIM (parity by
// construction). Its own push-constant contract (INTENT §10.1 "orbit/trail/
// axis/grid as separate families where push contracts differ"): a FRAGMENT
// color at 0 (old uColor) and the VERTEX {int nbPoints, mat4 ModelViewMatrix,
// float fader} at 12 (old layoutTrail, trail.cpp:192-193). The shaders read
// main_clipping_fov from cam_block (context.uboSet) - the family binds the
// global UBO set (the SAME camera-block authority the new-path body shaders
// use, I2), NOT a pushed clipping_fov (unlike AXIS/ORBIT, whose shaders were
// converted to push in the 2023-master merge; body_trail was not). LINE_STRIP
// fed to the geometry shader (segment wrap-cull + subdivision), BLEND_SRC_ALPHA
// carries the per-vertex fade alpha, NO depth (old setDepthStencilMode() =
// test+write off). Spec-const 8 registry-injected (§11.33).
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
    : BodyModule(BodyModuleType::TRAIL), color(color), maxTrail(maxTrail), deltaTrail(deltaTrail) {}

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

void TrailModule::startTrail(bool record)
{
    // Old Trail::startTrail (trail.cpp:165-174): enable => fresh restart;
    // disable => stop (recording mirrors the dead trail_on, not a gate).
    recording = record;
    if (record)
        firstPoint = true;
}

void TrailModule::setShown(bool b)
{
    // Old Body::setFlagTrail -> Trail::setFlagTrail (fader target + startTrail):
    // set the override AND reset on enable (first_point).
    nameOverride = b ? 1 : 0;
    overrideGen = flagGeneration;
    if (b)
        firstPoint = true;
    // Kick the system-level trail phase so this module's update() runs and its
    // fader can rise even when the global master is off (the phase is gated on
    // anyActive() - OrbitModule precedent).
    if (b && !live) { live = true; ++activeCount; }
}

void TrailModule::accumulate(ModularBody *body)
{
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
    // Accumulation gate = the display fader (old updateTrail's ONLY gate,
    // trail.cpp:122; trail_on was DEAD). The display/recording separation (old
    // TODO, header intent) is SUSPENDED for Vixy (§11.41). Ticks every frame
    // for every EVALUATED body (drawTrails sweeps all), so accumulation
    // continues while the body is invisible - the row-9 invisible-tick contract.
    if (interstate >= 0.001f) {
        if (!recording) { recording = true; firstPoint = true; } // rising edge = old startTrail(true)
        accumulate(body);
    } else {
        recording = false; // fader down: accumulation stops (points retained)
    }
    // TRAIL never inflates the body's boundingRadius (it is not in a regime
    // list): this return is consumed by nobody - kept for the interface. The
    // trail needs the per-frame tick, so it never self-deregisters (returns false).
    boundingRadius = scaledRadius;
    return false;
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
