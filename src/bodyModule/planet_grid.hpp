#ifndef _PLANET_GRID_HPP_
#define _PLANET_GRID_HPP_

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
    PlanetGrid(Body *body, unsigned int nb_meridian = 24, unsigned int nb_parallel = 7);
    ~PlanetGrid();

    void drawGrid(VkCommandBuffer &cmd, const Projector* prj, const Mat4d& mat);

    static void createSC_context();
    static void destroySC_context();

private:
    void computeGridVertices();
    void updateVertexBuffer(const Projector* prj, const Mat4d& mat);

    Body *body;
    unsigned int nb_meridian;  // Nombre de méridiens (longitude)
    unsigned int nb_parallel;  // Nombre de parallèles (latitude)

    // Données des vertices pour les méridiens et parallèles
    std::vector<Vec3f> meridian_vertices;
    std::vector<Vec3f> parallel_vertices;

    Vec3f *pGridVertices = nullptr;
    std::unique_ptr<VertexBuffer> m_GridGL;

    // Ressources partagées Vulkan
    static std::unique_ptr<VertexArray> vertexModel;
    static std::unique_ptr<Pipeline> pipeline;
    static std::unique_ptr<PipelineLayout> layout;

    static inline float grid_radius = 1.05f; // Rayon max pour afficher la grille (multiples of body radius)
    static inline Vec4f meridian_color = Vec4f(0.0f, 1.0f, 0.0f, 1.0f); // Vert
    static inline Vec4f parallel_color = Vec4f(0.0f, 0.0f, 1.0f, 1.0f); // Bleu
    static const unsigned int SEGMENTS_PER_LINE = 64; // Segments par ligne de grille
    unsigned int total_vertices;
};

#endif // _PLANET_GRID_HPP_