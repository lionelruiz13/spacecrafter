#ifndef _PLANET_GRID_HPP_
#define _PLANET_GRID_HPP_

#include "EntityCore/SubBuffer.hpp"
#include "tools/vecmath.hpp"
#include "tools/context.hpp"
#include <vector>
#include <memory>

#include "EntityCore/Resource/SharedBuffer.hpp"

class Body;
class Projector;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;

class PlanetGrid {
public:
    PlanetGrid() = delete;
    PlanetGrid(const PlanetGrid&) = delete;
    PlanetGrid(Body *body);
    ~PlanetGrid();

    void drawGrid(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat);

    static void createSC_context();
    static void destroySC_context();

private:
    static void computeStaticGridVertices();

    Body *body;

    // Shared Vulkan resources and static mesh
    static std::unique_ptr<VertexArray> vertexModel;
    static std::unique_ptr<Pipeline> pipeline;
    static std::unique_ptr<PipelineLayout> layout;
    static std::unique_ptr<VertexBuffer> staticGridBuffer;
    static SubBuffer staticIndexSubBuffer;
    // Structure for vertex with color
    struct GridVertex {
        Vec3f position;
        Vec3f color;
    };
    static std::vector<GridVertex> staticGridVertices;
    static std::vector<uint16_t> staticGridIndices;  // Index buffer for separate lines
    static bool staticMeshInitialized;

    static inline float grid_radius = 1.05f; // Radius for displaying the grid (multiples of body radius)
    static inline Vec4f meridian_color = Vec4f(0.0f, 1.0f, 0.0f, 1.0f); // Green
    static inline Vec4f parallel_color = Vec4f(0.0f, 0.0f, 1.0f, 1.0f); // Blue
    static const unsigned int SEGMENTS_PER_LINE = 64; // Segments per grid line
    static const unsigned int DEFAULT_NB_MERIDIAN = 24;
    static const unsigned int DEFAULT_NB_PARALLEL = 7;
    static unsigned int staticTotalVertices;
};

#endif // _PLANET_GRID_HPP_
