#ifndef DSO_NAVIGATOR_HPP
#define DSO_NAVIGATOR_HPP

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>

class Projector;
class Navigator;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class s_texture;

class DsoNavigator: public NoCopy {
public:
    DsoNavigator(const std::string& tex_file, const std::string &tex3d_file = "dso3d.png", int depth = 256);
    ~DsoNavigator();
    void computePosition(Vec3f posI, const Projector *prj);
    void draw(const Navigator *nav, const Projector *prj);
    void insert(const Mat4f &model, int textureID, float unscale);
private:
    void build();

    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertex;
    std::unique_ptr<VertexBuffer> instance;
    SubBuffer index;
    std::unique_ptr<s_texture> texture;
    std::unique_ptr<s_texture> colorTexture;
    struct dso {
        Mat4f model;
        Mat4f invmodel;
        Vec3f data; // texOffset, coefScale, lod
    };
    std::vector<dso> dsoData;
    std::vector<Vec3f> dsoPos;

    int instanceCount = 0;

    std::unique_ptr<Set> set;
    std::unique_ptr<SharedBuffer<Mat4f>> uModelViewMatrix;
    std::unique_ptr<SharedBuffer<Vec3f>> uclipping_fov;
    std::unique_ptr<SharedBuffer<Mat4f>> uCamRotToLocal; // represent a Mat3f
    VkCommandBuffer cmds[3];
    float texScale;
    bool needRebuild[3];
};

#endif /* end of include guard: DSO_NAVIGATOR_HPP */
