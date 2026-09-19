#ifndef AXIS_MODULE_HPP_
#define AXIS_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

class AxisModule : public BodyModule {
public:
    // Ctor/dtor out-of-line: unique_ptr<VertexBuffer> needs the complete type
    // (the inline ctor's exception path instantiates the member destructor).
    AxisModule();
    ~AxisModule();
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show; // old setFlagAxis / actualdrawaxis
protected:
    // Lazy 2-vertex line buffer (old m_AxisGL pattern), tinyMgr persistent map.
    std::unique_ptr<VertexBuffer> line;
    Vec3f *pPos = nullptr;
};

#endif /* end of include guard: AXIS_MODULE_HPP_ */
