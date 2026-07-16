#ifndef LANDSCAPE_ENV_HPP_
#define LANDSCAPE_ENV_HPP_

#include "../EnvironmentModule.hpp"

class Landscape;

// Grounded landscape (+ fog, which rides the Landscape engine) - grounded
// environment member, present on every landable body (radius > 0), matching
// the old behavior: BodyDecor::bodyAssign showed a landscape on ANY body
// below limLandscape (default 10000 m, Body::defaultAtmosphereParams).
//
// Migration form: Core still owns WHICH landscape is current (the
// setLandscapeToBody auto-rules: Earth->initial, planet->name,
// satellite->moon, sun->sun) and swaps the engine object on setLandscape;
// the current engine pointer reaches this member through the manager at
// every swap (SSystemFactory::setEnvironmentLandscape - I5: the owner
// re-seats the reference at replacement). This member owns the gate
// (drawLandscape state) and the NEW-path drawing (camera-derived matrix).
//
// Matrix derivation: the landscape mesh lives in the OLD local frame
// (x=South, y=East, z=Up - observer getRotLocalToEquatorialFixed). The
// camera's zenith frame is x=East, y=North, z=Up; same components in both
// frames = Z(-90 deg) conversion, the loadCamera init_view_pos seam
// (ssystem_factory.cpp) applied to a matrix instead of a vector:
// MV = viewRotation() * Z(-pi/2), then the engine composes its own
// Z(-rotate_z) (landscape data convention, single authority in drawEnv).
// viewRotation() maps zenith->eye under BOTH mounts (the equatorial fold is
// parametrization, not a frame change - Camera.hpp view composition).
class LandscapeEnv : public EnvironmentModule {
public:
    LandscapeEnv() = default;
    bool update(ModularBody *body, const Vec3f &cameraLocalPos,
                float deltaTime, EnvironmentState &state) override;
    void drawSky(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Current engine (Core-owned, swapped on setLandscape). Static: ONE
    // current landscape exists app-wide (old Core::landscape), every
    // grounded member draws it - per-body landscape identity is the
    // post-parity association data (EnvironmentModule.hpp convergence note).
    static Landscape *engine;
};

#endif
