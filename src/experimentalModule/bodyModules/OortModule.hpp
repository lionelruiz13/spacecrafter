#ifndef OORT_MODULE_HPP_
#define OORT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class VertexArray;
class VertexBuffer;
class Set;
template<typename> class SharedBuffer;

class OortModule : public BodyModule {
public:
    OortModule(unsigned int nbr, const Vec3f &color);
    ~OortModule();
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show;
    static Vec3f cloudColor;
private:
    void render(Renderer &renderer, const Mat4f &mat);
    PipelineFamily family;
    std::unique_ptr<VertexArray> vertexModel;
    std::unique_ptr<VertexBuffer> vertex;
    Set *set = nullptr; // renderer-pool owned (family set 1: uMat + uFrag)
    struct Frag { Vec3f color; float fader; };
    std::unique_ptr<SharedBuffer<Mat4f>> uMat;
    std::unique_ptr<SharedBuffer<Frag>> uFrag;
    unsigned int nbPoints;
    float cloudExtent = 0.f; // max |point|, the reported bounding radius (AU)
};

#endif /* end of include guard: OORT_MODULE_HPP_ */
