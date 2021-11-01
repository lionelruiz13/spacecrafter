#include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class Projector;
class Navigator;
class Pipeline;
class PipelineLayout;
class Set;
class VertexArray;
class VertexBuffer;
class ObjL;

class StarViewer {
public:
    StarViewer(const Vec3f &pos, const Vec3f &color, const float radius);
    ~StarViewer();
    void draw(const Navigator * nav, const Projector* prj, const Mat4f &mat);
    static void createSC_context();
private:
    void createLocalContext();
    float getOnScreenSize(const Projector* prj, const Vec3f &pos);

    static PipelineLayout *layout;
    static Pipeline *pipeline, *pipelineCorona;
    static std::unique_ptr<VertexArray> modelHalo;
    Mat4f model;
    int cmds[3];
    std::unique_ptr<Set> set;
    struct s_vert {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
        float radius;
    };
    struct s_frag {
        Vec3f color;
        float radius;
        Vec3f cam_view;
    };
    std::unique_ptr<SharedBuffer<s_vert>> uVert;
    std::unique_ptr<SharedBuffer<s_frag>> uFrag;
    std::unique_ptr<VertexBuffer> vertexHalo;
    float *pVertexHalo;
    float radius;
    std::unique_ptr<ObjL> objl;
};
