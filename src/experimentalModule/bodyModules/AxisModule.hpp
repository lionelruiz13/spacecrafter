#ifndef AXIS_MODULE_HPP_
#define AXIS_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

// Rotation-axis line of a body (near component), depth-tested in the body's slice so its own disc hides it
class AxisModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (the inline ctor's exception path instantiates the member destructor).
    AxisModule();
    ~AxisModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show; // Global axis visibility (setFlagAxis)
protected:
    // Lazy 2-vertex line buffer, persistently mapped at pPos
    std::unique_ptr<VertexBuffer> line;
    Vec3f *pPos = nullptr;
};

#endif /* end of include guard: AXIS_MODULE_HPP_ */
