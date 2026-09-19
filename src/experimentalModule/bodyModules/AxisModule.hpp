#ifndef AXIS_MODULE_HPP_
#define AXIS_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

class AxisModule : public BodyModule {
public:
    // Out-of-line, unique_ptr<VertexBuffer> needs the complete type
    AxisModule();
    ~AxisModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show;
protected:
    std::unique_ptr<VertexBuffer> line; // created lazily, mapped at pPos
    Vec3f *pPos = nullptr;
};

#endif /* end of include guard: AXIS_MODULE_HPP_ */
