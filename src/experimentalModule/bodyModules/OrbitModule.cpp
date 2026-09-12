#include "OrbitModule.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "bodyModule/orbit.hpp"
#include <cmath>

// Old Orbit3D constants (orbit_3d.hpp): 180 sample points over one
// visualization period + 64 smoothing points threaded through the body center.
static constexpr int ORBIT_POINTS = 180;
static constexpr int ORBIT_ADDITIONNAL_POINTS = 64;
static constexpr int ORBIT_TOTAL = ORBIT_POINTS + ORBIT_ADDITIONNAL_POINTS;

bool OrbitModule::showPlanets = false;    // config flag_planets_orbits default
bool OrbitModule::showSatellites = false; // config flag_satellites default
uint32_t OrbitModule::flagGeneration = 0; // bumped by every global toggle
int OrbitModule::activeCount = 0;         // modules with a live fader (phase gate)
Vec3f OrbitModule::defaultColor{0.f, 0.f, 0.f}; // set by the seam (config planet_orbits_color)

// ORBIT line family - the second line-class family (AXIS was the first, S11.31).
// body_orbit3d.{vert,geom,frag} REUSED VERBATIM (parity by construction). Its
// own push-constant contract (INTENT S10.1 "orbit/trail/axis/grid as separate
// families where push-constant contracts differ"): a FRAGMENT color at 0 and
// the VERTEX|GEOMETRY {mat, clipping_fov} at 16 (old layoutOrbit3d,
// orbit_plot.cpp:81-85). LINE_STRIP fed to the geometry shader, which
// subdivides each segment; BLEND_SRC_ALPHA carries the fader alpha (the
// EntityCore Pipeline default the old code relied on - registry default is
// BLEND_NONE). Spec-const 8 registry-injected (S11.33).
namespace {
struct OrbitFamilyData {
    std::unique_ptr<VertexArray> vertexModel; // 1 binding, vec3 pos (m_Orbit parity)
    PipelineFamily family;
};
OrbitFamilyData &orbitFamily()
{
    static OrbitFamilyData data = []() -> OrbitFamilyData {
        Renderer &renderer = Context::instance->renderer;
        OrbitFamilyData d;
        d.vertexModel = std::make_unique<VertexArray>(*VulkanMgr::instance, 3*sizeof(float));
        d.vertexModel->createBindingEntry(3*sizeof(float));
        d.vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);
        PipelineFamilyDesc desc;
        desc.name = "ORBIT";
        desc.vertex = d.vertexModel.get();
        // Two push ranges, order = the pushConstant() index used at draw
        // (old orbit_plot.cpp:83-84): 0 = frag color, 1 = vert/geom mat+fov.
        desc.pushConstants = {
            {VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint16_t>(sizeof(Vec4f))},
            {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
             static_cast<uint16_t>(sizeof(Vec4f)),
             static_cast<uint16_t>(sizeof(Mat4f) + sizeof(Vec3f))},
        };
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "body_orbit3d.vert.spv",
                                  .geom = "body_orbit3d.geom.spv",
                                  .frag = "body_orbit3d.frag.spv"}}};
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        color.state.blend = BLEND_SRC_ALPHA; // fader alpha (old default)
        color.state.cull = false;            // lines: no face culling (old default)
        desc.passes.push_back(std::move(color));
        d.family = renderer.allocateFamily(std::move(desc));
        return d;
    }();
    return data;
}
} // namespace

OrbitModule::OrbitModule(const Vec3f &color, bool closeOrbit)
    : BodyModule(BodyModuleType::ORBIT), color(color), authoredColor(color), closeOrbit(closeOrbit),
      orbitPoint(std::make_unique<Vec3d[]>(ORBIT_POINTS)) {}

OrbitModule::~OrbitModule()
{
    if (live) // keep the phase-gate counter balanced on removal (body drop)
        --activeCount;
}

bool OrbitModule::wantShown(ModularBody *body) const
{
    // A per-name override wins only until the next global toggle (old parity:
    // a global setFlag*Orbits clobbered every per-body fader).
    if (nameOverride >= 0 && overrideGen == flagGeneration)
        return nameOverride != 0;
    // Old classification: satellites ride setFlagSatellitesOrbits, everyone
    // else setFlagPlanetsOrbits (SolarSystemSelected::setFlag*Orbits).
    return body->isSatellite() ? showSatellites : showPlanets;
}

void OrbitModule::sampleOrbit(ModularBody *body)
{
    const Orbit *orbit = body->getOrbit();
    if (!orbit)
        return;
    const double date = body->getLastJD();
    const double period = body->getSiderealPeriod();
    const double increment = period / ORBIT_POINTS;
    // A PLOT IS NOT A USE (INTENT S5.150, S11.229(h2); fixed here, S11.239).
    // These 180 points used to go through positionAtTimevInVSOP87Coordinates,
    // i.e. through the POSITION solver's own Newton seed (orbit.hpp:120), and
    // they left it at the LAST sample's date - half a visualization period from
    // the body's own - so the next frame's evaluation started from there.
    // MEASURED in the engine at a time rate where every frame resamples:
    // 18 of 31 walked iterating records perturbed, worst Neso 0.16623 AU =
    // 1172 arcsec, against a flag-off floor of 5.2e-05 AU (S11.239).
    // The OLD path's plot never did this: it calls the BATCH pair, whose seed
    // is a SEPARATE member (batchLastE, orbit.hpp:122; orbit.cpp:442 resets it,
    // :453-458 runs ten warm-up calls then one per point), precisely so that
    // drawing an orbit cannot disturb the position it is drawn around
    // (orbit_plot.cpp:142/:174 for the prepair, :153/:167/:182 for the points).
    // This is that same pair, called the same way - the generality the port
    // dropped, restored rather than re-invented (S11.52(b)).
    //
    // The prepair is UNCONDITIONAL because the old site's is: orbit_plot.cpp:142
    // sits above the osc/plain split, not inside it.  It is a no-op for every
    // orbit type but EllipticalOrbit, which is also the only type in this tree
    // that overrides either half of the pair (orbit.hpp:100-101); all the others
    // fall back to the base defaults (orbit.hpp:40-47), which is exactly what
    // the old path gets on them.
    //
    // ITS RETURN VALUE IS IGNORED, and that is a decision with a reason. The
    // pair is the old plot's own incremental-cache window (consumed at
    // orbit_plot.cpp:107-111, :120 and :124-127). The new path owns that window
    // itself, in update(): `|date - lastSampleJD| >= period/ORBIT_POINTS` (:132)
    // IS the symmetric window {-increment, +increment} that EVERY implementation
    // of prepair in this tree returns (orbit.hpp:41, orbit.cpp:443). Consuming
    // it here would put a second authority on one fact (I2) and change no
    // behaviour; an orbit type that returned an ASYMMETRIC pair would have to be
    // learned by that gate, not by this loop. Recorded at S11.239.
    //
    // The const_cast is well defined and is NOT hiding a mutation of a const
    // object: the orbit is owned by a non-const `std::unique_ptr<Orbit>`
    // (ModularBody.hpp:119) and only the accessor is const (`getOrbit`,
    // :1134). prepairFast is non-const merely because the fast half is const
    // and needed somewhere to cache (orbit.hpp:39) - batchLastE is already
    // `mutable`. The two clean spellings (const-qualify prepairFast, or add a
    // non-const getOrbit) both change files this task may not touch; the choice
    // is recorded at S11.239, not taken here.
    const_cast<Orbit *>(orbit)
        ->prepairFastPositionAtTimevInVSOP87Coordinates(date, increment);
    // Osculating orbits (comets) sample through their own function; the rest
    // through the batch call (old computeOrbit's osc-vs-plain split).
    OsculatingFunctionType *osc = orbit->getOsculatingFunction();
    for (int d = 0; d < ORBIT_POINTS; ++d) {
        const double calc_date = date + (d - ORBIT_POINTS/2) * increment;
        if (osc)
            (*osc)(date, calc_date, orbitPoint[d]);
        else
            orbit->fastPositionAtTimevInVSOP87Coordinates(date, calc_date, orbitPoint[d]);
    }
    lastSampleJD = date;
    sampled = true;
}

bool OrbitModule::update(ModularBody *body, float scaledRadius)
{
    fader = wantShown(body);
    fader.update(static_cast<int>(ModularBody::deltaTime));
    // Maintain the phase-gate counter: a module counts while its fader is live.
    const bool nowLive = fader.getInterstate() > 1e-6f;
    if (nowLive != live) {
        live = nowLive;
        activeCount += nowLive ? 1 : -1;
    }
    if (fader.getInterstate() > 1e-6f) {
        const double period = body->getSiderealPeriod();
        if (period > 0) {
            // Resample only when the body has advanced ~one point (the old
            // incremental cache's granularity; full recompute is the S9 item).
            const double date = body->getLastJD();
            if (!sampled || std::abs(date - lastSampleJD) >= period / ORBIT_POINTS)
                sampleOrbit(body);
        } else {
            sampled = false; // still orbit: nothing to draw
        }
    }
    // ORBIT never inflates the body's boundingRadius (the orbit is far larger
    // than the depth slice): it is not in a regime list, so this return is
    // consumed by nobody - kept for the interface contract.
    boundingRadius = scaledRadius;
    return false;
}

void OrbitModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // mat = the PARENT position frame (ModularSystem::drawOrbits). Orbit points
    // are parent-relative root-aligned vectors (old parent_mat frame).
    const float alpha = fader.getInterstate();
    if (alpha <= 1e-6f || !sampled)
        return;
    // Depth-free fallback when no body reserved an orbit slice this frame
    // ({0,0} bucket, old backup-plane path): the orbit draws over everything.
    const auto &bucket = renderer.getOrbitDepthBucket();
    const VariantKey variant = (bucket.znear == 0.f && bucket.zfar == 0.f)
                                   ? VARIANT_NO_DEPTH : 0;
    const FamilyBound bound = renderer.bind(orbitFamily().family, variant);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - C3 degrade
    if (!line)
        line = orbitFamily().vertexModel->createBuffer(0, ORBIT_TOTAL,
                                                        Context::instance->globalBuffer.get());
    // Fill the line strip (old Orbit3D::computeShader, orbit_3d.cpp:63-112):
    // the first/last halves are the sampled points; the middle threads a
    // notch through the body center so the line passes cleanly across the disc.
    float *v = static_cast<float *>(Context::instance->transfer->planCopy(line->get()));
    auto put = [&v](const Vec3d &p) { *v++ = p[0]; *v++ = p[1]; *v++ = p[2]; };
    for (int n = 0; n < ORBIT_POINTS/2 - 1; ++n)
        put(orbitPoint[n]);
    const Vec3f ecl = body->getEclipticPos();
    const float r10 = body->getRadius() / 10.f;
    const Vec3d center(ecl[0] - r10, ecl[1] - r10, ecl[2]);
    float coef = 1.f;
    for (int n = 1; n < ORBIT_ADDITIONNAL_POINTS/2 + 1; ++n) {
        coef /= 1.3f;
        put(orbitPoint[ORBIT_POINTS/2-1] * coef + center * (1. - coef));
    }
    put(center);
    for (int n = 1; n < ORBIT_ADDITIONNAL_POINTS/2 + 1; ++n) {
        put(orbitPoint[ORBIT_POINTS/2+1] * coef + center * (1. - coef));
        coef *= 1.3f;
    }
    for (int n = ORBIT_POINTS/2 + 1; n < ORBIT_POINTS; ++n)
        put(orbitPoint[n]);
    put(closeOrbit ? orbitPoint[0] : orbitPoint[ORBIT_POINTS-1]);

    const Vec4f fragColor(color[0], color[1], color[2], alpha);
    struct {
        Mat4f mat;
        Vec3f clipping_fov;
    } pushGeom {mat, renderer.getClippingFov()};
    bound.layout->pushConstant(renderer, 0, &fragColor);
    bound.layout->pushConstant(renderer, 1, &pushGeom);
    VertexArray::bind(renderer, line->get());
    vkCmdDraw(renderer, ORBIT_TOTAL, 1, 0, 0);
}
