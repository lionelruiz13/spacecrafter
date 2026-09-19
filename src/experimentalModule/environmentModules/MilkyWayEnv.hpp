#ifndef MILKYWAY_ENV_HPP_
#define MILKYWAY_ENV_HPP_

#include "../EnvironmentModule.hpp"

class MilkyWay;
class ToneReproductor;

// Draw the milky way and the zodiacal light
class MilkyWayEnv : public EnvironmentModule {
public:
    MilkyWayEnv(MilkyWay *engine, ToneReproductor *eye) : engine(engine), eye(eye) {}
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override {
        return false; // draw reads the live fader/intensity state
    }
    // Only use the rotation of mat
    void drawBackdrop(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
private:
    MilkyWay *engine; // not owned, outlives this
    ToneReproductor *eye;
};

#endif
