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

PlanetGrid::PlanetGrid(Body *_body, unsigned int _nb_meridian, unsigned int _nb_parallel)
    : body(_body), nb_meridian(_nb_meridian), nb_parallel(_nb_parallel)
{
    // Compute the total number of vertices needed
    // nb_meridian meridians, each with SEGMENTS_PER_LINE+1 points
    // nb_parallel parallels, each with SEGMENTS_PER_LINE+1 points
    total_vertices = (nb_meridian * (SEGMENTS_PER_LINE + 1)) + (nb_parallel * (SEGMENTS_PER_LINE + 1));

    computeGridVertices();
}

PlanetGrid::~PlanetGrid()
{
    // Vulkan resources are automatically cleaned up by unique_ptrs
}

void PlanetGrid::computeGridVertices()
{
    meridian_vertices.clear();
    parallel_vertices.clear();

    // Generate meridians (lines of constant longitude)
    for (unsigned int m = 0; m < nb_meridian; m++) {
        double longitude = 2.0 * M_PI * m / nb_meridian;

        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            // Go to the poles to cover the entire sphere
            double latitude_factor = double(i) / SEGMENTS_PER_LINE - 0.5;
            double latitude = M_PI * latitude_factor;

            Vec3f point;
            point[0] = grid_radius * cos(latitude) * cos(longitude);
            point[1] = grid_radius * cos(latitude) * sin(longitude);
            point[2] = grid_radius * sin(latitude);

            meridian_vertices.push_back(point);
        }
    }

    // Generate parallels (lines of constant latitude)
    // Uniform distribution from -π/2 to +π/2 (south pole to north pole)
    for (unsigned int p = 0; p < nb_parallel; p++) {
        // Uniform distribution including the poles
        double latitude = M_PI * ((double(p) / (nb_parallel - 1)) - 0.5);

        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            double longitude = 2.0 * M_PI * i / SEGMENTS_PER_LINE;

            Vec3f point;
            point[0] = grid_radius * cos(latitude) * cos(longitude);
            point[1] = grid_radius * cos(latitude) * sin(longitude);
            point[2] = grid_radius * sin(latitude);

            parallel_vertices.push_back(point);
        }
    }
}

void PlanetGrid::drawGrid(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat)
{
    if (!m_GridGL) {
        m_GridGL = vertexModel->createBuffer(0, total_vertices, Context::instance->tinyMgr.get());
        pGridVertices = static_cast<Vec3f *>(Context::instance->tinyMgr->getPtr(m_GridGL->get()));
    }

    pipeline->bind(cmd);

    // Pass the color for meridians (push constant 0)
    layout->pushConstant(cmd, 0, &meridian_color);

    // Pass the transformation matrix with planetary rotation and clipping (push constant 1)
    struct {
        Mat4f mat;
        Vec3f clipping_fov;
    } matData;

    // Create the planet rotation matrix (rotation around the Z-axis)
    // Convert degrees to radians since Mat4d::zrotation expects radians
    Mat4d planetRotation = Mat4d::zrotation(body->getAxisRotation() * M_PI / 180.0);

    // Combine the view/projection matrix with the planet rotation
    Mat4d combinedMatrix = mat * planetRotation;

    matData.mat = combinedMatrix.convert();
    matData.clipping_fov = prj->getClippingFov();
    layout->pushConstant(cmd, 1, &matData);

    VertexArray::bind(cmd, m_GridGL->get());

    updateVertexBuffer(prj, mat);

    // Draw the meridians - each meridian is a separate line
    for (unsigned int m = 0; m < nb_meridian; m++) {
        vkCmdDraw(cmd, SEGMENTS_PER_LINE + 1, 1, m * (SEGMENTS_PER_LINE + 1), 0);
    }

    // Pass the color for parallels (push constant 0)
    layout->pushConstant(cmd, 0, &parallel_color);

    // Draw the parallels - each parallel is a separate line
    unsigned int parallel_offset = nb_meridian * (SEGMENTS_PER_LINE + 1);
    for (unsigned int p = 0; p < nb_parallel; p++) {
        vkCmdDraw(cmd, SEGMENTS_PER_LINE + 1, 1, parallel_offset + p * (SEGMENTS_PER_LINE + 1), 0);
    }
}

void PlanetGrid::updateVertexBuffer(const Projector* prj, const Mat4d& mat)
{
    unsigned int vertex_index = 0;

    // Treat the meridians - pass the 3D coordinates directly to the shader
    unsigned int m_base = 0;
    for (unsigned int m = 0; m < nb_meridian; m++) {
        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            Vec3f pos3d = meridian_vertices[m_base + i] * body->getRadius();
            pGridVertices[vertex_index] = pos3d;
            vertex_index++;
        }
        m_base += (SEGMENTS_PER_LINE + 1);
    }

    // Treat the parallels - pass the 3D coordinates directly to the shader
    unsigned int p_base = 0;
    for (unsigned int p = 0; p < nb_parallel; p++) {
        for (unsigned int i = 0; i <= SEGMENTS_PER_LINE; i++) {
            Vec3f pos3d = parallel_vertices[p_base + i] * body->getRadius();
            pGridVertices[vertex_index] = pos3d;
            vertex_index++;
        }
        p_base += (SEGMENTS_PER_LINE + 1);
    }
}

void PlanetGrid::createSC_context()
{
    VulkanMgr &vkmgr = *VulkanMgr::instance;
    Context &context = *Context::instance;
    assert(!vertexModel);

    vertexModel = std::make_unique<VertexArray>(vkmgr, 3*sizeof(float));
    vertexModel->createBindingEntry(3*sizeof(float));
    vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT); // 3D position

    layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setPushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vec4f)); // Push constant 0: color
    layout->setPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(Vec4f), sizeof(Mat4f) + sizeof(Vec3f)); // Push constant 1: matrix + clipping
    layout->buildLayout();
    layout->build();

    pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
    pipeline->setLineWidth(1.5);
    pipeline->bindShader("body_orbit3d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline->bindShader("body_orbit3d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline->bindVertex(*vertexModel);
    pipeline->build();
}

void PlanetGrid::destroySC_context()
{
    pipeline.reset();
    layout.reset();
    vertexModel.reset();
}