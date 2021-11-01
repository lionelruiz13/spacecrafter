#ifndef CLOUD_NAVIGATOR_HPP
#define CLOUD_NAVIGATOR_HPP

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include <memory>

#include "EntityCore/SubBuffer.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

class Projector;
class Navigator;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class s_texture;

class CloudNavigator: public NoCopy {
public:
    CloudNavigator();
    ~CloudNavigator();
    void computePosition(Vec3f posI);
    void draw(const Navigator * nav, const Projector* prj);
    void draw(const Mat4f &mat, const Projector* prj);
    void insert(const Vec4f &color, const Mat4f &model);
private:
    void build(int nbClouds);

    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertex;
    std::unique_ptr<VertexBuffer> instance;
    SubBuffer index;
    std::unique_ptr<s_texture> texture;
    struct cloud {
        Vec4f color;
        Mat4f model;
        Mat4f invmodel;
    };
    std::vector<cloud> cloudData;
    std::vector<Vec3f> cloudPos;

    int instanceCount = 0;
    int cmds[3] = {-1, -1, -1};

    std::unique_ptr<Set> set;
    std::unique_ptr<SharedBuffer<Mat4f>> uModelViewMatrix;
    std::unique_ptr<SharedBuffer<Vec3f>> uclipping_fov;
    std::unique_ptr<SharedBuffer<Mat4f>> uCamRotToLocal; // represent a Mat3f
};

#endif /* end of include guard: CLOUD_NAVIGATOR_HPP */
