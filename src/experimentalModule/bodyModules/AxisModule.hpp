#ifndef AXIS_MODULE_HPP_
#define AXIS_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include <memory>

class VertexBuffer;

// AXIS slot - the rotation-axis line of a body (port of the old Axis friend,
// axis.cpp). Color pass, depth-tested: endpoints at +-1.4 x scaled radius on
// the body z axis, CPU-fisheye-projected with the CURRENT depth-slice range
// (the same mapping the disc wrote - hidden by its own disc exactly).
// Regime: near components (only meaningful when the body is resolved).
// Deduction rule: default for every body with a mesh (tex_map); global
// toggle via the seam (setFlagAxis -> show, the old static actualdrawaxis).
// Color: ONE shared red uniform for all bodies (old-parity: Axis::uColor was
// a single static set once to {1,0,0}); the family and Set live at family
// scope in AxisModule.cpp, not per-module.
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
