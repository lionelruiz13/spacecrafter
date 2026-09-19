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

class OjmModule : public BodyModule {
public:
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
    // Shared draw body of draw/drawNoDepth (variant bit is the only delta).
    void drawInternal(Renderer &renderer, ModularBody *body, const Mat4f &mat, VariantKey base);
    std::shared_ptr<Ojm> model; // shared resource (Ojm::load recycler, D6)
    std::unique_ptr<Set> setPlain;
    std::unique_ptr<Set> setShadow;
    SharedBuffer<ojmVert> uVert;
    SharedBuffer<ojmGeom> uGeom;
    SharedBuffer<ojmLight> uLight;        // plain rows, binding 2
    SharedBuffer<ojmShadowBlock> uShadow; // shadowed rows, binding 2
    Mat4f selfShadowMat;        // the nomination's matrix (consumption copy)
    bool selfShadowActive = false; // frame-scoped (BodyModule.hpp contract)
    bool bound = false;         // set bindings done at first successful load
};

#endif /* end of include guard: OJM_MODULE_HPP_ */
