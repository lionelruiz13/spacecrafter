#include "PlanetGridModule.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include <cmath>

// Grid resolution (old planet_grid.cpp DEFAULT_NB_MERIDIAN / SEGMENTS_PER_LINE).
static constexpr int NB_MERIDIAN = 24;
static constexpr int SEGMENTS = 64;
static constexpr float PARALLEL_LAT_DEG[] = {-60.f, -30.f, 0.f, 30.f, 60.f};
static constexpr int NB_PARALLEL = sizeof(PARALLEL_LAT_DEG) / sizeof(float);
// Non-indexed LINE_LIST: 2 verts per segment.
static constexpr uint32_t MERIDIAN_VERTS = NB_MERIDIAN * SEGMENTS * 2;
static constexpr uint32_t PARALLEL_VERTS = NB_PARALLEL * SEGMENTS * 2;
static constexpr uint32_t MAIN_VERTS = MERIDIAN_VERTS + PARALLEL_VERTS;
// One latitude circle = SEGMENTS segments = SEGMENTS*2 verts. Tropics and polar
// circles are each a north+south pair (old planet_grid.cpp:103-192).
static constexpr uint32_t CIRCLE_VERTS = SEGMENTS * 2;
static constexpr uint32_t PAIR_VERTS = CIRCLE_VERTS * 2;

bool PlanetGridModule::show = false; // old flag_planet_grid init (body.cpp:163)
bool PlanetGridModule::showTropics = false;       // old LINE_TROPIC show init
bool PlanetGridModule::showPolarCircles = false;  // old LINE_CIRCLE_POLAR show init

namespace {
Vec3f g_meridianColor{1.f, 1.f, 1.f};
Vec3f g_parallelColor{1.f, 1.f, 1.f};
Vec3f g_tropicColor{1.f, 1.f, 1.f};   // old tropicColor = LINE_TROPIC color
Vec3f g_polarColor{1.f, 1.f, 1.f};    // old polarCircleColor = LINE_CIRCLE_POLAR
uint32_t g_colorGeneration = 1; // >0 so a fresh instance (syncedColorGen=0) bakes

struct GridFamilyData {
    std::unique_ptr<VertexArray> vertexModel; // 1 binding, vec3 pos + vec3 color
    PipelineFamily family;
};
GridFamilyData &gridFamily()
{
    static GridFamilyData data = []() -> GridFamilyData {
        Renderer &renderer = Context::instance->renderer;
        GridFamilyData d;
        // Interleaved pos+color (old planet_grid.cpp:371-374: 6 floats, 1
        // binding, two vec3 inputs - the deployed shader's vertex layout).
        d.vertexModel = std::make_unique<VertexArray>(*VulkanMgr::instance, 6*sizeof(float));
        d.vertexModel->createBindingEntry(6*sizeof(float));
        d.vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT); // position (unit sphere)
        d.vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT); // color (meridian/parallel)
        PipelineFamilyDesc desc;
        desc.name = "GRID";
        desc.vertex = d.vertexModel.get();
        // Single VERTEX push range {ModelViewMatrix, clipping_fov} - the
        // planet_grid.vert push_constant block (old planet_grid.cpp:377-378).
        desc.pushConstants = {{VK_SHADER_STAGE_VERTEX_BIT, 0,
                               static_cast<uint16_t>(sizeof(Mat4f) + sizeof(Vec3f))}};
        PassDesc color;
        color.pass = PassKind::COLOR;
        color.shaderTable = {{0, {.vert = "planet_grid.vert.spv",
                                  .frag = "planet_grid.frag.spv"}}};
        color.state.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        color.state.lineWidth = 1.5f;
        color.state.cull = false;
        desc.passes.push_back(std::move(color));
        d.family = renderer.allocateFamily(std::move(desc));
        return d;
    }();
    return data;
}

// One interleaved pos+color vertex.
struct GridVertex { Vec3f pos; Vec3f color; };

Vec3f onSphere(double lat, double lon)
{
    return Vec3f(std::cos(lat) * std::cos(lon),
                 std::cos(lat) * std::sin(lon),
                 std::sin(lat));
}
// Emit one latitude circle (SEGMENTS segments, LINE_LIST) at `lat` radians.
GridVertex *fillLatCircle(GridVertex *v, double lat, const Vec3f &color)
{
    for (int i = 0; i < SEGMENTS; ++i) {
        const double lon0 = 2.0 * M_PI * i / SEGMENTS;
        const double lon1 = 2.0 * M_PI * (i + 1) / SEGMENTS;
        *v++ = {onSphere(lat, lon0), color};
        *v++ = {onSphere(lat, lon1), color};
    }
    return v;
}

void fillGrid(GridVertex *v, double axialTiltRad, bool hasTropics,
              const Vec3f &meridian, const Vec3f &parallel,
              const Vec3f &tropic, const Vec3f &polar)
{
    for (int m = 0; m < NB_MERIDIAN; ++m) {
        const double lon = 2.0 * M_PI * m / NB_MERIDIAN;
        for (int i = 0; i < SEGMENTS; ++i) {
            const double lat0 = M_PI * (double(i) / SEGMENTS - 0.5);
            const double lat1 = M_PI * (double(i + 1) / SEGMENTS - 0.5);
            *v++ = {onSphere(lat0, lon), meridian};
            *v++ = {onSphere(lat1, lon), meridian};
        }
    }
    // Parallels: equator + generic lat/lon parallels.
    for (int p = 0; p < NB_PARALLEL; ++p)
        v = fillLatCircle(v, PARALLEL_LAT_DEG[p] * M_PI / 180.0, parallel);
    // Tropics: +/-axialTilt (old planet_grid.cpp:104-134). Present only for
    // planets (hasTropics); the astronomically-direct obliquity readout.
    if (hasTropics) {
        v = fillLatCircle(v,  axialTiltRad, tropic);
        v = fillLatCircle(v, -axialTiltRad, tropic);
    }
    // Polar circles: +/-(90 - axialTilt) (old planet_grid.cpp:150-179), drawn
    // for every body (no satellite/name gate in the old path).
    const double polarLat = M_PI / 2.0 - axialTiltRad;
    v = fillLatCircle(v,  polarLat, polar);
    v = fillLatCircle(v, -polarLat, polar);
}
} // namespace

PlanetGridModule::PlanetGridModule()
    : BodyModule(BodyModuleType::CUSTOM),
      meridianColor(g_meridianColor), parallelColor(g_parallelColor),
      tropicColor(g_tropicColor), polarColor(g_polarColor) {}
PlanetGridModule::~PlanetGridModule() = default; // VertexBuffer complete here

void PlanetGridModule::setColors(const Vec3f &meridian, const Vec3f &parallel)
{
    g_meridianColor = meridian;
    g_parallelColor = parallel;
    ++g_colorGeneration; // every live instance re-bakes on its next draw
}

void PlanetGridModule::setTropicPolar(bool showT, bool showP,
                                      const Vec3f &tropic, const Vec3f &polarCircle)
{
    // Flags gate the draw ranges - no re-bake needed.
    showTropics = showT;
    showPolarCircles = showP;
    // Colors are baked in: re-bake only on an actual change (this runs every
    // frame from the seam poll; sky colors are otherwise static).
    if (tropic != g_tropicColor || polarCircle != g_polarColor) {
        g_tropicColor = tropic;
        g_polarColor = polarCircle;
        ++g_colorGeneration;
    }
}

bool PlanetGridModule::update(ModularBody *, float scaledRadius)
{
    boundingRadius = show ? scaledRadius * 1.05f : scaledRadius;
    return true;
}

void PlanetGridModule::dumpState(std::ostream &out) const
{
    out << "{\"axialTilt\":" << bodyAxialTilt
        << ",\"hasTropics\":" << (hasTropics ? "true" : "false")
        << ",\"tropicLat\":" << (hasTropics ? bodyAxialTilt : 0.f)
        << ",\"polarLat\":" << (90.f - bodyAxialTilt)
        << ",\"show\":" << (show ? "true" : "false")
        << ",\"showTropics\":" << (showTropics ? "true" : "false")
        << ",\"showPolarCircles\":" << (showPolarCircles ? "true" : "false")
        << ",\"built\":" << (built ? "true" : "false")
        << ",\"vertexCount\":" << vertexCount
        << ",\"tropicCount\":" << tropicCount
        << ",\"polarCount\":" << polarCount << '}';
}

void PlanetGridModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!show)
        return;
    const FamilyBound bound = renderer.bind(gridFamily().family);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - C3 degrade
    hasTropics = !body->isSatellite() && !body->isStar();
    bodyAxialTilt = body->getAxialTilt();
    const double axialTiltRad = bodyAxialTilt * M_PI / 180.0;
    if (!built) {
        // Buffer size is body-dependent: the tropic pair is absent on
        // satellites / stars. Ranges: [meridians+parallels][tropics?][polar].
        tropicFirst = MAIN_VERTS;
        tropicCount = hasTropics ? PAIR_VERTS : 0;
        polarFirst = MAIN_VERTS + tropicCount;
        polarCount = PAIR_VERTS;
        mainCount = MAIN_VERTS;
        vertexCount = polarFirst + polarCount;
        buffer = gridFamily().vertexModel->createBuffer(0, vertexCount,
                                                        Context::instance->globalBuffer.get());
    }
    if (!built || syncedColorGen != g_colorGeneration) {
        meridianColor = g_meridianColor;
        parallelColor = g_parallelColor;
        tropicColor = g_tropicColor;
        polarColor = g_polarColor;
        GridVertex *v = static_cast<GridVertex *>(
            Context::instance->transfer->planCopy(buffer->get()));
        fillGrid(v, axialTiltRad, hasTropics,
                 meridianColor, parallelColor, tropicColor, polarColor);
        syncedColorGen = g_colorGeneration;
        built = true;
    }
    const float s = body->getScaledRadius() * 1.05f;
    struct {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
    } pushData;
    pushData.ModelViewMatrix = mat.multiplyFast(Mat4f::scaling(Vec3f(s, s, s)));
    pushData.clipping_fov = renderer.getClippingFov();
    bound.layout->pushConstant(renderer, 0, &pushData);
    VertexArray::bind(renderer, buffer->get());
    // Meridians + generic parallels: ride the grid (axis) flag - always drawn.
    vkCmdDraw(renderer, mainCount, 1, 0, 0);
    // Tropics: gated by LINE_TROPIC (old body.cpp:1257). Absent on satellites/
    // stars (tropicCount 0).
    if (showTropics && tropicCount)
        vkCmdDraw(renderer, tropicCount, 1, tropicFirst, 0);
    // Polar circles: gated by LINE_CIRCLE_POLAR (old body.cpp:1258).
    if (showPolarCircles)
        vkCmdDraw(renderer, polarCount, 1, polarFirst, 0);
}
