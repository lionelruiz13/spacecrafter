#include "bodyModule/planet_grid.hpp"
#include "bodyModule/body.hpp"
#include "coreModule/projector.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/utility.hpp"
#include <cmath>

std::unique_ptr<VertexArray> PlanetGrid::vertexModel;
std::unique_ptr<Pipeline> PlanetGrid::pipeline;
std::unique_ptr<PipelineLayout> PlanetGrid::layout;
std::unique_ptr<VertexBuffer> PlanetGrid::staticGridBuffer;
SubBuffer PlanetGrid::staticIndexSubBuffer;
std::vector<PlanetGrid::GridVertex> PlanetGrid::staticGridVertices;
std::vector<uint16_t> PlanetGrid::staticGridIndices;
bool PlanetGrid::staticMeshInitialized = false;
unsigned int PlanetGrid::staticTotalVertices = 0;

PlanetGrid::PlanetGrid(Body *_body)
    : body(_body)
{
    // Generate and use static mesh if not already initialized
    if (!staticMeshInitialized) {
        computeStaticGridVertices();
        staticMeshInitialized = true;
    }
}

PlanetGrid::~PlanetGrid()
{
    // Vulkan resources are automatically cleaned up by unique_ptrs
}

void PlanetGrid::computeStaticGridVertices()
{
    staticGridVertices.clear();
    staticGridIndices.clear();

    Vec3f meridianColor(0.0f, 1.0f, 0.0f); // Green for meridians
    Vec3f parallelColor(0.0f, 0.0f, 1.0f); // Blue for parallels

    // STEP 1: Generate all unique vertices for MERIDIANS
    std::vector<std::vector<uint16_t>> meridianVertexIndices(DEFAULT_NB_MERIDIAN);

    for (unsigned int m = 0; m < DEFAULT_NB_MERIDIAN; m++) {
        double longitude = 2.0 * M_PI * m / DEFAULT_NB_MERIDIAN;
        meridianVertexIndices[m].reserve(SEGMENTS_PER_LINE + 1);

        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            // Go to the poles to cover the entire sphere
            double latitude_factor = double(i) / SEGMENTS_PER_LINE - 0.5;
            double latitude = M_PI * latitude_factor;

            GridVertex vertex;
            // Normalized position (radius = 1.0) for the static mesh
            vertex.position[0] = cos(latitude) * cos(longitude);
            vertex.position[1] = cos(latitude) * sin(longitude);
            vertex.position[2] = sin(latitude);
            vertex.color = meridianColor;

            uint16_t vertexIndex = staticGridVertices.size();
            if (vertexIndex >= 65535) {
                printf("ERROR: Vertex index overflow! %d vertices\n", vertexIndex);
                return; // Avoid overflow
            }
            staticGridVertices.push_back(vertex);
            meridianVertexIndices[m].push_back(vertexIndex);
        }
    }

    // STEP 2: Generate indices to connect meridians with LINE_LIST
    for (unsigned int m = 0; m < DEFAULT_NB_MERIDIAN; m++) {
        for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
            // Each segment: point i -> point i+1
            staticGridIndices.push_back(meridianVertexIndices[m][i]);
            staticGridIndices.push_back(meridianVertexIndices[m][i + 1]);
        }
    }

    // STEP 3: Generate all unique vertices for PARALLELS
    std::vector<std::vector<uint16_t>> parallelVertexIndices(DEFAULT_NB_PARALLEL);

    for (unsigned int p = 0; p < DEFAULT_NB_PARALLEL; p++) {
        // Ignore first and last parallels (exact poles, not visible)
        if (p == 0 || p == DEFAULT_NB_PARALLEL - 1) continue;

        // Uniform distribution including the poles
        double latitude = M_PI * ((double(p) / (DEFAULT_NB_PARALLEL - 1)) - 0.5);
        parallelVertexIndices[p].reserve(SEGMENTS_PER_LINE + 1);

        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;

            GridVertex vertex;
            // Normalized position (radius = 1.0) for the static mesh
            vertex.position[0] = cos(latitude) * cos(longitude);
            vertex.position[1] = cos(latitude) * sin(longitude);
            vertex.position[2] = sin(latitude);
            vertex.color = parallelColor;

            uint16_t vertexIndex = staticGridVertices.size();
            if (vertexIndex >= 65535) {
                printf("ERROR: Vertex index overflow in parallels! %d vertices\n", vertexIndex);
                return; // Avoid overflow
            }
            staticGridVertices.push_back(vertex);
            parallelVertexIndices[p].push_back(vertexIndex);
        }
    }

    // STEP 4: Generate indices to connect parallels with LINE_LIST
    for (unsigned int p = 0; p < DEFAULT_NB_PARALLEL; p++) {
        if (p == 0 || p == DEFAULT_NB_PARALLEL - 1) continue;

        for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
            // Each segment: point i -> point i+1 (with wraparound)
            staticGridIndices.push_back(parallelVertexIndices[p][i]);
            staticGridIndices.push_back(parallelVertexIndices[p][i + 1]);
        }
    }

    staticTotalVertices = staticGridVertices.size();
}

void PlanetGrid::drawGrid(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat)
{
    // Initialize static vertices (unit sphere)
    if (staticGridVertices.empty()) {
        computeStaticGridVertices();
        if (staticGridVertices.empty() || staticGridIndices.empty()) {
            return;
        }
    }

    // Static buffers: vertices + indices, created only once
    if (!staticGridBuffer || !staticIndexSubBuffer.buffer) {
        // Vertex buffer - use globalBuffer with transfer for large allocations
        staticGridBuffer = vertexModel->createBuffer(0, staticTotalVertices, Context::instance->globalBuffer.get());

        // Copy vertices (position + color) into the buffer with transfer
        GridVertex *pStaticVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(staticGridBuffer->get()));
        memcpy(pStaticVertices, staticGridVertices.data(), staticGridVertices.size() * sizeof(GridVertex));

        // Index buffer - use indexBufferMgr like the others
        staticIndexSubBuffer = Context::instance->indexBufferMgr->acquireBuffer(staticGridIndices.size() * sizeof(uint16_t));

        // Copy indices into the buffer
        uint16_t *pStaticIndices = static_cast<uint16_t *>(Context::instance->transfer->planCopy(staticIndexSubBuffer));
        memcpy(pStaticIndices, staticGridIndices.data(), staticGridIndices.size() * sizeof(uint16_t));
    }

    pipeline->bind(cmd);

    struct {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
        float bodyRadius;
        float gridRadius;
        float axisRotation;
    } matData;

    matData.ModelViewMatrix = mat.convert();
    matData.clipping_fov = prj->getClippingFov();
    matData.bodyRadius = body->radius;
    matData.gridRadius = 1.05f;
    matData.axisRotation = body->getAxisRotation() * M_PI / 180.0f;

    layout->pushConstant(cmd, 0, &matData);

    VertexArray::bind(cmd, staticGridBuffer->get());

    // Bind index buffer and draw with indices
    vkCmdBindIndexBuffer(cmd, staticIndexSubBuffer.buffer, staticIndexSubBuffer.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(staticGridIndices.size()), 1, 0, 0, 0);
}

void PlanetGrid::createSC_context()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    assert(!vertexModel);

    // Vertex format: position (3 floats) + color (3 floats) = 6 floats total
    vertexModel = std::make_unique<VertexArray>(vkmgr, 6*sizeof(float));
    vertexModel->createBindingEntry(6*sizeof(float));
    vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT); // 3D position
    vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT); // 3D color

    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(Mat4f) + sizeof(Vec3f) + sizeof(float) + sizeof(float) + sizeof(float)); // Push constant 1: matrix + clipping + rotation + radii
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST); // Independent lines for index buffer
    pipeline->setLineWidth(1.5);
    pipeline->bindShader("planet_grid.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->bindShader("planet_grid.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline->bindVertex(*vertexModel);
    pipeline->build();
}

void PlanetGrid::destroySC_context()
{
    pipeline.reset();
    layout.reset();
    vertexModel.reset();
    staticGridBuffer.reset();
    staticGridVertices.clear();
    staticGridIndices.clear();
    staticMeshInitialized = false;
}
