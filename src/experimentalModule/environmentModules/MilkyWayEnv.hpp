#ifndef MILKYWAY_ENV_HPP_
#define MILKYWAY_ENV_HPP_

#include "../EnvironmentModule.hpp"

class MilkyWay;
class ToneReproductor;

// Milkyway 2D backdrop (+ zodiacal light) - InAoI environment member of the
// galaxy root body. Migration form: wraps the app's MilkyWay engine (shared
// with the old path - textures, faders, pipelines, iris selection state);
// this member supplies the NEW-path matrix and draw position in the frame.
//
// Matrix derivation (parity with old draw = J2000ToEye * modelMilkyway):
// the received `mat` is a root-aligned chain frame -> eye (the manager
// passes the current system's mat - flat-chain contract, all root-aligned
// frames share one orientation), i.e. VSOP87(ecliptic J2000) -> eye once the
// translation is stripped. J2000 -> eye = that rotation * mat_j2000_to_vsop87
// (the navigator's constant). The engine composes modelMilkyway /
// modelZodiacal internally (single authority on the texture alignment).
//
// The iris/standard texture selection is DRIVEN by the manager's engine
// block (broadcast consumer of EnvironmentState.atmosphereActive), not here:
// it is a write on shared state, owned by whoever owns phase gating.
class MilkyWayEnv : public EnvironmentModule {
public:
    MilkyWayEnv(MilkyWay *engine, ToneReproductor *eye) : engine(engine), eye(eye) {}
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override {
        return false; // continuous: draw reads live fader/intensity state
    }
    void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
private:
    MilkyWay *engine; // app-lifetime (Core shared_ptr member) - outlives this
    ToneReproductor *eye;
};

#endif
