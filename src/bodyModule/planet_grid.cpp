#include "bodyModule/planet_grid.hpp"
#include "bodyModule/body.hpp"
#include "coreModule/projector.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include <cmath>

// Static shared resources (common to all instances)
std::unique_ptr<VertexArray> PlanetGrid::vertexModel;
std::unique_ptr<Pipeline> PlanetGrid::pipeline;
std::unique_ptr<PipelineLayout> PlanetGrid::layout;

PlanetGrid::PlanetGrid(Body *_body)
    : body(_body)
{
    // Mesh will be generated lazily on first draw
    // to ensure body's axialTilt is properly initialized
}

PlanetGrid::~PlanetGrid()
{
    // Vulkan resources are automatically cleaned up by unique_ptrs
    // Instance-specific buffers will be cleaned up automatically
}

void PlanetGrid::computeGridVertices()
{
    // Clear all vertex and index buffers
    meridianVertices.clear();
    meridianIndices.clear();
    equatorVertices.clear();
    equatorIndices.clear();
    tropicsVertices.clear();
    tropicsIndices.clear();
    polarCirclesVertices.clear();
    polarCirclesIndices.clear();

    // Use white color for all vertices - actual color will be updated dynamically in updateVertexColors()
    Vec3f whiteColor(1.0f, 1.0f, 1.0f);

    // Get body's axial tilt (in degrees) and convert to radians
    double axialTilt = body->getAxialTilt() * M_PI / 180.0;

    // ========== MERIDIANS ==========
    std::vector<std::vector<uint16_t>> meridianVertexIndices(DEFAULT_NB_MERIDIAN);

    for (unsigned int m = 0; m < DEFAULT_NB_MERIDIAN; m++) {
        double longitude = 2.0 * M_PI * m / DEFAULT_NB_MERIDIAN;
        meridianVertexIndices[m].reserve(SEGMENTS_PER_LINE + 1);

        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            // Go to the poles to cover the entire sphere
            double latitude_factor = double(i) / SEGMENTS_PER_LINE - 0.5;
            double latitude = M_PI * latitude_factor;

            GridVertex vertex;
            vertex.position[0] = cos(latitude) * cos(longitude);
            vertex.position[1] = cos(latitude) * sin(longitude);
            vertex.position[2] = sin(latitude);
            vertex.color = whiteColor;

            uint16_t vertexIndex = meridianVertices.size();
            if (vertexIndex >= 65535) {
                printf("ERROR: Meridian vertex index overflow! %d vertices\n", vertexIndex);
                return;
            }
            meridianVertices.push_back(vertex);
            meridianVertexIndices[m].push_back(vertexIndex);
        }
    }

    // Generate indices for meridians
    for (unsigned int m = 0; m < DEFAULT_NB_MERIDIAN; m++) {
        for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
            meridianIndices.push_back(meridianVertexIndices[m][i]);
            meridianIndices.push_back(meridianVertexIndices[m][i + 1]);
        }
    }

    // ========== EQUATOR ==========
    double equatorLatitudeFactor = 0.0;
    for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
        double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;
        double latitude = equatorLatitudeFactor;

        GridVertex vertex;
        vertex.position[0] = cos(latitude) * cos(longitude);
        vertex.position[1] = cos(latitude) * sin(longitude);
        vertex.position[2] = sin(latitude);
        vertex.color = whiteColor;

        equatorVertices.push_back(vertex);
    }

    // Generate indices for equator
    for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
        equatorIndices.push_back(i);
        equatorIndices.push_back(i + 1);
    }

    // ========== TROPICS ==========
    if (!body->isSatellite() && body->getEnglishName() != "Sun") {
        // Only draw tropics for planets (not satellites or the Sun)
        double tropicFactor = axialTilt;

        // Tropic of Cancer (North) - first half of vertices
        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;
            double latitude = tropicFactor;

            GridVertex vertex;
            vertex.position[0] = cos(latitude) * cos(longitude);
            vertex.position[1] = cos(latitude) * sin(longitude);
            vertex.position[2] = sin(latitude);
            vertex.color = whiteColor;

            tropicsVertices.push_back(vertex);
        }

        // Tropic of Capricorn (South) - second half of vertices
        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;
            double latitude = -tropicFactor;

            GridVertex vertex;
            vertex.position[0] = cos(latitude) * cos(longitude);
            vertex.position[1] = cos(latitude) * sin(longitude);
            vertex.position[2] = sin(latitude);
            vertex.color = whiteColor;

            tropicsVertices.push_back(vertex);
        }

        // Generate indices for tropics (both circles)
        // Tropic of Cancer
        for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
            tropicsIndices.push_back(i);
            tropicsIndices.push_back(i + 1);
        }
        // Tropic of Capricorn
        uint16_t offset = SEGMENTS_PER_LINE + 1;
        for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
            tropicsIndices.push_back(offset + i);
            tropicsIndices.push_back(offset + i + 1);
        }
    }

    // ========== POLAR CIRCLES ==========
    double polarCircleLat = M_PI / 2.0 - axialTilt;

    // Arctic Circle (North) - first half of vertices
    for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
        double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;
        double latitude = polarCircleLat;

        GridVertex vertex;
        vertex.position[0] = cos(latitude) * cos(longitude);
        vertex.position[1] = cos(latitude) * sin(longitude);
        vertex.position[2] = sin(latitude);
        vertex.color = whiteColor;

        polarCirclesVertices.push_back(vertex);
    }

    // Antarctic Circle (South) - second half of vertices
    for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
        double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;
        double latitude = -polarCircleLat;

        GridVertex vertex;
        vertex.position[0] = cos(latitude) * cos(longitude);
        vertex.position[1] = cos(latitude) * sin(longitude);
        vertex.position[2] = sin(latitude);
        vertex.color = whiteColor;

        polarCirclesVertices.push_back(vertex);
    }

    // Generate indices for polar circles (both circles)
    // Arctic Circle
    for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
        polarCirclesIndices.push_back(i);
        polarCirclesIndices.push_back(i + 1);
    }
    // Antarctic Circle
    uint16_t offset = SEGMENTS_PER_LINE + 1;
    for (unsigned int i = 0; i < SEGMENTS_PER_LINE; i++) {
        polarCirclesIndices.push_back(offset + i);
        polarCirclesIndices.push_back(offset + i + 1);
    }
}

void PlanetGrid::updateVertexColors(const Vec3f& meridianColor, const Vec3f& equatorColor, const Vec3f& tropicColor, const Vec3f& polarCircleColor)
{
    // Update meridian vertices colors
    for (auto& vertex : meridianVertices) {
        vertex.color = meridianColor;
    }
    if (meridianBuffer) {
        GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(meridianBuffer->get()));
        memcpy(pVertices, meridianVertices.data(), meridianVertices.size() * sizeof(*pVertices));
    }

    // Update equator vertices colors
    for (auto& vertex : equatorVertices) {
        vertex.color = equatorColor;
    }
    if (equatorBuffer) {
        GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(equatorBuffer->get()));
        memcpy(pVertices, equatorVertices.data(), equatorVertices.size() * sizeof(*pVertices));
    }

    // Update tropics vertices colors
    for (auto& vertex : tropicsVertices) {
        vertex.color = tropicColor;
    }
    if (tropicsBuffer) {
        GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(tropicsBuffer->get()));
        memcpy(pVertices, tropicsVertices.data(), tropicsVertices.size() * sizeof(*pVertices));
    }

    // Update polar circles vertices colors
    for (auto& vertex : polarCirclesVertices) {
        vertex.color = polarCircleColor;
    }
    if (polarCirclesBuffer) {
        GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(polarCirclesBuffer->get()));
        memcpy(pVertices, polarCirclesVertices.data(), polarCirclesVertices.size() * sizeof(*pVertices));
    }
}

void PlanetGrid::drawGrid(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat, double observerAltitude,
                          bool showMeridians, bool showEquator, bool showTropics, bool showPolarCircles,
                          const Vec3f& meridianColor, const Vec3f& equatorColor, const Vec3f& tropicColor, const Vec3f& polarCircleColor)
{
    // Lazy initialization: compute vertices on first draw
    if (!initialized) {
        computeGridVertices();
        initialized = true;
    }

    // Check if colors have changed and update vertex colors if needed
    if (cachedMeridianColor != meridianColor ||
        cachedEquatorColor != equatorColor ||
        cachedTropicColor != tropicColor ||
        cachedPolarCircleColor != polarCircleColor) {

        updateVertexColors(meridianColor, equatorColor, tropicColor, polarCircleColor);

        // Update cached colors
        cachedMeridianColor = meridianColor;
        cachedEquatorColor = equatorColor;
        cachedTropicColor = tropicColor;
        cachedPolarCircleColor = polarCircleColor;
    }

    // Only draw if altitude > 10km
    if (observerAltitude <= 10000.0) {
        return;
    }

    // Common transformation setup
    pipeline->bind(cmd);

    struct {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
    } matData;

    // Pre-compute all transformations on CPU
    float totalScale = body->radius * 1.05f;
    Mat4d scaleMatrix = Mat4d::scaling(Vec3d(totalScale, totalScale, totalScale));

    double axisRotationRad = body->getAxisRotation() * M_PI / 180.0;
    Mat4d rotationMatrix = Mat4d::zrotation(axisRotationRad);

    Mat4d completeTransform = mat * rotationMatrix * scaleMatrix;

    matData.ModelViewMatrix = completeTransform.convert();
    matData.clipping_fov = prj->getClippingFov();

    layout->pushConstant(cmd, 0, &matData);

    // ========== DRAW MERIDIANS ==========
    if (showMeridians && !meridianVertices.empty() && !meridianIndices.empty()) {
        // Create buffers if not already created
        if (!meridianBuffer || !meridianIndexSubBuffer.buffer) {
            meridianBuffer = vertexModel->createBuffer(0, meridianVertices.size(), Context::instance->globalBuffer.get());

            GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(meridianBuffer->get()));
            memcpy(pVertices, meridianVertices.data(), meridianVertices.size() * sizeof(*pVertices));

            meridianIndexSubBuffer = Context::instance->indexBufferMgr->acquireBuffer(meridianIndices.size() * sizeof(uint16_t));

            uint16_t *pIndices = static_cast<uint16_t *>(Context::instance->transfer->planCopy(meridianIndexSubBuffer));
            memcpy(pIndices, meridianIndices.data(), meridianIndices.size() * sizeof(uint16_t));
        }

        VertexArray::bind(cmd, meridianBuffer->get());
        vkCmdBindIndexBuffer(cmd, meridianIndexSubBuffer.buffer, meridianIndexSubBuffer.offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, meridianIndices.size(), 1, 0, 0, 0);
    }

    // ========== DRAW EQUATOR ==========
    if (showEquator && !equatorVertices.empty() && !equatorIndices.empty()) {
        if (!equatorBuffer || !equatorIndexSubBuffer.buffer) {
            equatorBuffer = vertexModel->createBuffer(0, equatorVertices.size(), Context::instance->globalBuffer.get());

            GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(equatorBuffer->get()));
            memcpy(pVertices, equatorVertices.data(), equatorVertices.size() * sizeof(*pVertices));

            equatorIndexSubBuffer = Context::instance->indexBufferMgr->acquireBuffer(equatorIndices.size() * sizeof(uint16_t));

            uint16_t *pIndices = static_cast<uint16_t *>(Context::instance->transfer->planCopy(equatorIndexSubBuffer));
            memcpy(pIndices, equatorIndices.data(), equatorIndices.size() * sizeof(uint16_t));
        }

        VertexArray::bind(cmd, equatorBuffer->get());
        vkCmdBindIndexBuffer(cmd, equatorIndexSubBuffer.buffer, equatorIndexSubBuffer.offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, equatorIndices.size(), 1, 0, 0, 0);
    }

    // ========== DRAW TROPICS ==========
    if (showTropics && !tropicsVertices.empty() && !tropicsIndices.empty()) {
        if (!tropicsBuffer || !tropicsIndexSubBuffer.buffer) {
            tropicsBuffer = vertexModel->createBuffer(0, tropicsVertices.size(), Context::instance->globalBuffer.get());

            GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(tropicsBuffer->get()));
            memcpy(pVertices, tropicsVertices.data(), tropicsVertices.size() * sizeof(*pVertices));

            tropicsIndexSubBuffer = Context::instance->indexBufferMgr->acquireBuffer(tropicsIndices.size() * sizeof(uint16_t));

            uint16_t *pIndices = static_cast<uint16_t *>(Context::instance->transfer->planCopy(tropicsIndexSubBuffer));
            memcpy(pIndices, tropicsIndices.data(), tropicsIndices.size() * sizeof(uint16_t));
        }

        VertexArray::bind(cmd, tropicsBuffer->get());
        vkCmdBindIndexBuffer(cmd, tropicsIndexSubBuffer.buffer, tropicsIndexSubBuffer.offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, tropicsIndices.size(), 1, 0, 0, 0);
    }

    // ========== DRAW POLAR CIRCLES ==========
    if (showPolarCircles && !polarCirclesVertices.empty() && !polarCirclesIndices.empty()) {
        if (!polarCirclesBuffer || !polarCirclesIndexSubBuffer.buffer) {
            polarCirclesBuffer = vertexModel->createBuffer(0, polarCirclesVertices.size(), Context::instance->globalBuffer.get());

            GridVertex *pVertices = static_cast<GridVertex *>(Context::instance->transfer->planCopy(polarCirclesBuffer->get()));
            memcpy(pVertices, polarCirclesVertices.data(), polarCirclesVertices.size() * sizeof(*pVertices));

            polarCirclesIndexSubBuffer = Context::instance->indexBufferMgr->acquireBuffer(polarCirclesIndices.size() * sizeof(uint16_t));

            uint16_t *pIndices = static_cast<uint16_t *>(Context::instance->transfer->planCopy(polarCirclesIndexSubBuffer));
            memcpy(pIndices, polarCirclesIndices.data(), polarCirclesIndices.size() * sizeof(uint16_t));
        }

        VertexArray::bind(cmd, polarCirclesBuffer->get());
        vkCmdBindIndexBuffer(cmd, polarCirclesIndexSubBuffer.buffer, polarCirclesIndexSubBuffer.offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, polarCirclesIndices.size(), 1, 0, 0, 0);
    }
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
                           sizeof(Mat4f) + sizeof(Vec3f)); // Push constant: matrix + clipping_fov
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST); // Independent lines for index buffer
    pipeline->setLineWidth(1.5);
    pipeline->bindShader("planet_grid.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    // Set specialization constant for projection type (constant_id = 8)
    pipeline->setSpecializedConstant(8, Context::projectionType);
    pipeline->bindShader("planet_grid.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline->bindVertex(*vertexModel);
    pipeline->build();
}

void PlanetGrid::destroySC_context()
{
    pipeline.reset();
    layout.reset();
    vertexModel.reset();
}
