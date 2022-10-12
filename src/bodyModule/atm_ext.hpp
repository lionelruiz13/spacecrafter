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
    AtmExt(Body *parent, ObjL *obj);
    ~AtmExt();

    void draw(VkCommandBuffer cmd, const Projector *prj, const Navigator *nav, const Mat4f &mat, float radius, float screen_sz, bool depthTest);

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
    SharedBuffer<_uniform> uniform;
private:
    class _dataSet;
    static std::weak_ptr<_dataSet> _shared;
    std::shared_ptr<_dataSet> shared;
    Body &parent;
    ObjL *obj;
    std::unique_ptr<Set> set;
};

#endif /* end of include guard: ATM_EXT_HPP_ */
