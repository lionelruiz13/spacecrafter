#ifndef AXIS_MODULE_HPP_
#define AXIS_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// AXIS slot - the rotation-axis line of a body (landing zone of the old Axis
// friend). Color pass, depth-tested (drawn with the body, hidden by it).
// Data: the body transform (the mat parameter carries body-to-observer), the
// body radius for axis length; axis color.
// Regime: near components (only meaningful when the body is resolved).
// Deduction rule: default for every body with a mesh; global toggle via the
// seam (setFlagAxis - the old static actualdrawaxis).
class AxisModule : public BodyModule {
public:
    AxisModule() : BodyModule(BodyModuleType::AXIS) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    static bool show; // old setFlagAxis
protected:
    Vec3f color;
};

#endif /* end of include guard: AXIS_MODULE_HPP_ */
