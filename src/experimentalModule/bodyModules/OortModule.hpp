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

// Oort-cloud point cloud, a body at the SolarSystem floor (near component, depth-less, alpha-blended)
// Low edge gated by the body's scaledRadius regime, far edge by the collapse of its node
class OortModule : public BodyModule {
public:
    OortModule(unsigned int nbr, const Vec3f &color);
    ~OortModule();
    // Reports the cloud extent as boundingRadius: the body must stay visible from inside the cloud
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Same render as draw(): the cloud must not blink out at the far edge of the regime band
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show;
    // Static because a single oort exists; seeded by the ctor, then written by Core::setColorScheme
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
