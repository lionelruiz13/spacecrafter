#ifndef TAIL_MODULE_HPP_
#define TAIL_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"

// TAIL slot - comet gas/dust tail (landing zone of the old Tail on
// SmallBody). Color pass, no depth (tails are translucent overlays).
// BATCHING CONTRACT: the old path batched all tails through a global
// singleton (Tail::global beginDraw/drawBatch/endDraw). Globals-as-batchers
// are exactly what G9 dissolves: batching is a Renderer service - modules
// submit, the Renderer coalesces per pipeline family (BMT_REPLICATED is the
// declared hint). No module-owned global state.
// Data: ejection/coma parameters (from body params), light position and
// observer direction (ModularBody statics/public interface), body magnitude
// inputs (albedo, radius - public).
// Regime: far components (a tail is visible when the nucleus is a dot).
// Deduction rule: comet-typed bodies (param type=Comet) or explicit.
class TailModule : public BodyModule {
public:
    TailModule() : BodyModule(BodyModuleType::TAIL) {}
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual bool update(ModularBody *body, float scaledRadius) override;
protected:
    float ejectionForce;
    float coefRadius;
    Vec3f color;
};

#endif /* end of include guard: TAIL_MODULE_HPP_ */
