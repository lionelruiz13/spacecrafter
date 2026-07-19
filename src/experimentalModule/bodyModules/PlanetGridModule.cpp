#include "PlanetGridModule.hpp"
#include "tools/context.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include <cmath>

// GRID line family - port of the old PlanetGrid friend (planet_grid.cpp) onto
// the registry. planet_grid.{vert,frag} REUSED VERBATIM (parity by
// construction): the vert declares push_constant {mat4 ModelViewMatrix; vec3
// clipping_fov} + custom_project spec-8 (the AXIS/ORBIT push contract, NOT
// TRAIL's UBO), and per-vertex color (location 1) - so the family carries an
// interleaved pos+color vertex and NO descriptor set. Spec-const 8 is
// registry-injected (§11.33). Fourth line-class family (AXIS §11.31, ORBIT
// §11.39, TRAIL §11.41 were the first three).

// Grid resolution (old planet_grid.cpp DEFAULT_NB_MERIDIAN / SEGMENTS_PER_LINE).
static constexpr int NB_MERIDIAN = 24;
static constexpr int SEGMENTS = 64;
// Tilt-independent parallels (the clean spec drops old's tilt-derived tropic/
// polar circles - ModularBody exposes no axial-tilt scalar; INTENT §11.42).
// The equator (0) overlays old's equator exactly; the rest are the generic
// lat/lon grid the header names ("latitude/longitude grid").
static constexpr float PARALLEL_LAT_DEG[] = {-60.f, -30.f, 0.f, 30.f, 60.f};
static constexpr int NB_PARALLEL = sizeof(PARALLEL_LAT_DEG) / sizeof(float);
// Non-indexed LINE_LIST: 2 verts per segment.
static constexpr uint32_t MERIDIAN_VERTS = NB_MERIDIAN * SEGMENTS * 2;
static constexpr uint32_t PARALLEL_VERTS = NB_PARALLEL * SEGMENTS * 2;
static constexpr uint32_t TOTAL_VERTS = MERIDIAN_VERTS + PARALLEL_VERTS;

bool PlanetGridModule::show = false; // old flag_planet_grid init (body.cpp:163)

namespace {
// Global grid colors (old observable: sky-manager colors shared by every body -
// body.cpp:1261-1264 reads them from CoreLink each draw). Default white matches
// old computeGridVertices' pre-color base; the seam feeds the real values.
Vec3f g_meridianColor{1.f, 1.f, 1.f};
Vec3f g_parallelColor{1.f, 1.f, 1.f};
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
        // Old fixed state (planet_grid.cpp:382-390): LINE_LIST, line width 1.5,
        // default depth (test+write on - the grid hides behind its own disc
        // through the shared bucket depth mapping) + default blend NONE (the
        // frag outputs opaque vec4(color,1)). Lines: no face culling.
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

// Write the whole line buffer: unit-sphere positions (body/radius-independent -
// the scale is folded into the pushed matrix) + baked per-vertex colors.
void fillGrid(GridVertex *v, const Vec3f &meridian, const Vec3f &parallel)
{
    auto onSphere = [](double lat, double lon) {
        return Vec3f(std::cos(lat) * std::cos(lon),
                     std::cos(lat) * std::sin(lon),
                     std::sin(lat));
    };
    // Meridians: NB_MERIDIAN longitude lines, pole to pole (old
    // computeGridVertices, planet_grid.cpp:49-71). LINE_LIST: each of SEGMENTS
    // segments emits both endpoints.
    for (int m = 0; m < NB_MERIDIAN; ++m) {
        const double lon = 2.0 * M_PI * m / NB_MERIDIAN;
        for (int i = 0; i < SEGMENTS; ++i) {
            const double lat0 = M_PI * (double(i) / SEGMENTS - 0.5);
            const double lat1 = M_PI * (double(i + 1) / SEGMENTS - 0.5);
            *v++ = {onSphere(lat0, lon), meridian};
            *v++ = {onSphere(lat1, lon), meridian};
        }
    }
    // Parallels: latitude circles (old equator + generic lat/lon parallels).
    for (int p = 0; p < NB_PARALLEL; ++p) {
        const double lat = PARALLEL_LAT_DEG[p] * M_PI / 180.0;
        for (int i = 0; i < SEGMENTS; ++i) {
            const double lon0 = 2.0 * M_PI * i / SEGMENTS;
            const double lon1 = 2.0 * M_PI * (i + 1) / SEGMENTS;
            *v++ = {onSphere(lat, lon0), parallel};
            *v++ = {onSphere(lat, lon1), parallel};
        }
    }
}
} // namespace

PlanetGridModule::PlanetGridModule()
    : BodyModule(BodyModuleType::CUSTOM),
      meridianColor(g_meridianColor), parallelColor(g_parallelColor) {}
PlanetGridModule::~PlanetGridModule() = default; // VertexBuffer complete here

void PlanetGridModule::setColors(const Vec3f &meridian, const Vec3f &parallel)
{
    g_meridianColor = meridian;
    g_parallelColor = parallel;
    ++g_colorGeneration; // every live instance re-bakes on its next draw
}

bool PlanetGridModule::update(ModularBody *, float scaledRadius)
{
    // Grid sits at 1.05 * scaledRadius: the traced bounding radius must enclose
    // it (BodyModule::boundingRadius contract) so the grid's near cap fits this
    // body's depth slice (else the front grid is znear-clipped by the bucket).
    // Only inflate when the grid actually draws (show): an installed-but-hidden
    // grid must not perturb the body's AoI / depth partitioning.
    boundingRadius = show ? scaledRadius * 1.05f : scaledRadius;
    return true;
}

void PlanetGridModule::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (!show)
        return;
    const FamilyBound bound = renderer.bind(gridFamily().family);
    if (!bound.layout)
        return; // pass unavailable (shader not deployed) - C3 degrade
    if (!built) {
        buffer = gridFamily().vertexModel->createBuffer(0, TOTAL_VERTS,
                                                        Context::instance->globalBuffer.get());
        vertexCount = TOTAL_VERTS;
        meridianVertexCount = MERIDIAN_VERTS;
    }
    // (Re)bake positions + colors on first build and on any global color change
    // (rare: a color command). Static geometry otherwise persists GPU-side.
    if (!built || syncedColorGen != g_colorGeneration) {
        meridianColor = g_meridianColor;
        parallelColor = g_parallelColor;
        GridVertex *v = static_cast<GridVertex *>(
            Context::instance->transfer->planCopy(buffer->get()));
        fillGrid(v, meridianColor, parallelColor);
        syncedColorGen = g_colorGeneration;
        built = true;
    }
    // The passed mat is the surface-spin-folded body frame (ModularBody::draw:
    // mat.multiplyFast(computeBodyToSurface)); the grid is rotationally
    // symmetric so the exact spin offset is visually invisible. Fold the unit
    // -> 1.05*scaledRadius scale into the pushed ModelViewMatrix (old
    // planet_grid.cpp:273-281 used radius*1.03; spec = scaledRadius*1.05, the
    // moon_scale AXIS parity). clipping_fov = the CURRENT depth slice (the same
    // bucket range the disc wrote, so grid-vs-disc occlusion stays exact).
    const float s = body->getScaledRadius() * 1.05f;
    struct {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
    } pushData;
    pushData.ModelViewMatrix = mat.multiplyFast(Mat4f::scaling(Vec3f(s, s, s)));
    pushData.clipping_fov = renderer.getClippingFov();
    bound.layout->pushConstant(renderer, 0, &pushData);
    VertexArray::bind(renderer, buffer->get());
    vkCmdDraw(renderer, vertexCount, 1, 0, 0);
}
