#ifndef DSO_NAVIGATOR_HPP
#define DSO_NAVIGATOR_HPP

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
class Texture;
class s_texture;

class DsoNavigator: public NoCopy {
public:
    DsoNavigator(ThreadContext *_context, const std::string& tex_file);
    ~DsoNavigator();
    void computePosition(Vec3f posI);
    void draw(const Navigator * nav, const Projector* prj);
    void insert(const Mat4f &model, int textureID, float unscale);
private:
    void build(int nbDso);
    ThreadContext *context;

    int commandIndex;
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertex;
    std::unique_ptr<Texture> texture;
    std::unique_ptr<s_texture> colorTexture;
    struct dso {
        Mat4f model;
        Mat4f invmodel;
        Vec2f data; // texOffset, coefScale
    } *pInstance;
    std::vector<dso> dsoData;
    std::vector<Vec3f> dsoPos;

    int instanceCount = 0;

    std::unique_ptr<Set> set;
    std::unique_ptr<Uniform> uModelViewMatrix, uclipping_fov, uCamRotToLocal;
    Mat4f *pModelViewMatrix;
    Vec3f *pclipping_fov;
    Mat4f *pCamRotToLocal; // represent a Mat3f
    float texScale;
};

#endif /* end of include guard: DSO_NAVIGATOR_HPP */
