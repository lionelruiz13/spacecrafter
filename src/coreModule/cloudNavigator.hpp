#ifndef CLOUD_NAVIGATOR_HPP
#define CLOUD_NAVIGATOR_HPP

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "vulkanModule/Context.hpp"
#include <memory>

class Projector;
class Navigator;
class VertexArray;
class Pipeline;
class PipelineLayout;
class Set;
class Uniform;
class Buffer;
class Texture;

class CloudNavigator: public NoCopy {
public:
    CloudNavigator(ThreadContext *_context);
    ~CloudNavigator();
    void computePosition(Vec3f posI);
    void draw(const Navigator * nav, const Projector* prj);
    void draw(const Mat4f &mat, const Projector* prj);
    void insert(const Vec4f &color, const Mat4f &model);
private:
    void build(int nbClouds);
    ThreadContext *context;

    int commandIndex;
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertex;
    std::unique_ptr<Texture> texture;
    struct cloud {
        Vec4f color;
        Mat4f model;
        Mat4f invmodel;
    } *pInstance;
    std::vector<cloud> cloudData;
    std::vector<Vec3f> cloudPos;

    int instanceCount = 0;

    std::unique_ptr<Set> set;
    std::unique_ptr<Uniform> uModelViewMatrix, uclipping_fov, uCamRotToLocal;
    Mat4f *pModelViewMatrix;
    Vec3f *pclipping_fov;
    Mat4f *pCamRotToLocal; // represent a Mat3f
};

#endif /* end of include guard: CLOUD_NAVIGATOR_HPP */
