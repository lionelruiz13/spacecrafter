#ifndef OJM_MODULE_HPP_
#define OJM_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp" // VariantKey (drawInternal)
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>

class Ojm;
class Set;
struct ojmVert;
struct ojmGeom;
struct ojmLight;
struct ojmShadowBlock;

// Draw a body from a 3D model (type=artificial + model_name)
class OjmModule : public BodyModule {
public:
    // A model which failed to load is never drawn
    OjmModule(std::shared_ptr<Ojm> model);
    ~OjmModule();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_BASIC_SELF_SHADOW
             | BMT_PROJECT_G1_SHADOW | BMT_RECEIVE_SHADOW;
    }
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawSelfShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool isLoaded() override;
protected:
    void drawInternal(Renderer &renderer, ModularBody *body, const Mat4f &mat, VariantKey base);
    std::shared_ptr<Ojm> model;
    std::unique_ptr<Set> setPlain;
    std::unique_ptr<Set> setShadow;
    SharedBuffer<ojmVert> uVert;
    SharedBuffer<ojmGeom> uGeom;
    SharedBuffer<ojmLight> uLight;        // plain rows, binding 2
    SharedBuffer<ojmShadowBlock> uShadow; // shadowed rows, binding 2
    Mat4f selfShadowMat;
    bool selfShadowActive = false; // frame-scoped
    bool bound = false;         // sets bound at first successful load
};

#endif /* end of include guard: OJM_MODULE_HPP_ */
