#ifndef MILKYWAY_ENV_HPP_
#define MILKYWAY_ENV_HPP_

#include "../EnvironmentModule.hpp"

class MilkyWay;
class ToneReproductor;

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
