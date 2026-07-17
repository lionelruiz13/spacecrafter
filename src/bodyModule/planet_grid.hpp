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

    void drawGrid(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat, double observerAltitude, 
                  bool showMeridians, bool showEquator, bool showTropics, bool showPolarCircles,
                  const Vec3f& meridianColor, const Vec3f& equatorColor, const Vec3f& tropicColor, const Vec3f& polarCircleColor);

    static void createSC_context();
    static void destroySC_context();

private:
    void computeGridVertices();
    void updateVertexColors(const Vec3f& meridianColor, const Vec3f& equatorColor, const Vec3f& tropicColor, const Vec3f& polarCircleColor);

    Body *body;

    // Shared Vulkan resources (common to all instances)
    static std::unique_ptr<VertexArray> vertexModel;
    static std::unique_ptr<Pipeline> pipeline;
    static std::unique_ptr<PipelineLayout> layout;

    // Structure for vertex with color
    struct GridVertex {
        Vec3f position;
        Vec3f color;
    };

    // Per-instance mesh data - separate buffers for each type
    // Meridians
    std::vector<GridVertex> meridianVertices;
    std::vector<uint16_t> meridianIndices;
    std::unique_ptr<VertexBuffer> meridianBuffer;
    SubBuffer meridianIndexSubBuffer;

    // Equator
    std::vector<GridVertex> equatorVertices;
    std::vector<uint16_t> equatorIndices;
    std::unique_ptr<VertexBuffer> equatorBuffer;
    SubBuffer equatorIndexSubBuffer;

    // Tropics
    std::vector<GridVertex> tropicsVertices;
    std::vector<uint16_t> tropicsIndices;
    std::unique_ptr<VertexBuffer> tropicsBuffer;
    SubBuffer tropicsIndexSubBuffer;

    // Polar Circles
    std::vector<GridVertex> polarCirclesVertices;
    std::vector<uint16_t> polarCirclesIndices;
    std::unique_ptr<VertexBuffer> polarCirclesBuffer;
    SubBuffer polarCirclesIndexSubBuffer;

    // Lazy initialization flag
    bool initialized = false;

    // Cached colors to detect changes
    Vec3f cachedMeridianColor = Vec3f(1.0f, 1.0f, 1.0f);
    Vec3f cachedEquatorColor = Vec3f(1.0f, 1.0f, 1.0f);
    Vec3f cachedTropicColor = Vec3f(1.0f, 1.0f, 1.0f);
    Vec3f cachedPolarCircleColor = Vec3f(1.0f, 1.0f, 1.0f);

    static const unsigned int SEGMENTS_PER_LINE = 64; // Segments per grid line
    static const unsigned int DEFAULT_NB_MERIDIAN = 24;
};

#endif // _PLANET_GRID_HPP_
