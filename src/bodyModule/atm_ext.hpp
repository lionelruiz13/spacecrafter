#ifndef ATM_EXT_HPP_
#define ATM_EXT_HPP_

#include "tools/vecmath.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

class Body;
class Projector;
class Navigator;
class ObjL;

class AtmExt {
public:
    AtmExt(Body *parent, ObjL *obj, const std::string &model);
    ~AtmExt();

    void draw(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav, const Mat4f &mat, const Vec3f &sunPos, const Vec3f &bodyPos, float planetOneMinusOblateness, const Vec2i &TesParam, float radius, float screen_sz, bool depthTest);

    struct _uniform {
        Mat4f ModelViewMatrix;
        Vec3f sunPos;
        float planetRadius;
        Vec3f bodyPos;
        float planetOneMinusOblateness;
        Vec3f clipping_fov;
        float atmRadius;
        Vec2i TesParam;
        float atmAlpha; // this value is a scale for atmosphere transparency
    };
private:
    class _dataSet;
    static std::weak_ptr<_dataSet> _shared;

    SharedBuffer<_uniform> uniform;
    std::shared_ptr<_dataSet> shared;
    Body &parent;
    ObjL *obj;
    std::unique_ptr<Set> set;
    s_texture texture;
    bool enabled = false;
};

#endif /* end of include guard: ATM_EXT_HPP_ */
