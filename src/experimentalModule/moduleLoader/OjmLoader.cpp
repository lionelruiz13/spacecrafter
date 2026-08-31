#include "OjmLoader.hpp"
#include "experimentalModule/bodyModules/OjmModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "ojmModule/ojm.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include <cstring>

uint8_t OjmLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    // What this loader NEEDS is a model to load: `model_name`.
    if (params["model_name"].empty())
        return 0;
    // The 4-byte "Arti" prefix (= the old parse's exact type discriminator) is
    // the DEDUCTION key, owned by ModularBody::deduceBodyModuleList - repeating
    // it here duplicated that authority (I2), and the duplicate VETOES an OJM
    // module the composed format DECLARED (`type = OJM` section): loadModule
    // runs isLikely for declared modules too, so a composed node that does not
    // carry the legacy `type = Artificial` silently lost its model. That is the
    // S11.89(c) blocker for body-type-less composed nodes. B27 Tier B / D14: the
    // composed format does not read `type` here; the legacy format keeps the
    // duplicate, because a legacy deduction never requests OJM without it and
    // dropping the test would let a NON-artificial legacy body with a model_name
    // (the Phobos-class named-ObjL case, drawn by the MESH family) be served an
    // OJM module - a legacy behaviour change, which D9 forbids.
    if (!target->isComposedDeclared()) {
        const std::string &type = params["type"];
        if (type.size() < 4 || std::memcmp(type.data(), "Arti", 4) != 0)
            return 0;
    }
    return 16; // slot-uncontested convention (RingLoader/AtmExtLoader)
}

bool OjmLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<OjmModule*>(module);
}

std::unique_ptr<BodyModule> OjmLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Old Artificial ctor parity (body_artificial.cpp:86-93).
    const std::string &modelName = params["model_name"];
    const std::string dir = AppSettings::Instance()->getModel3DDir() + modelName + "/";
    std::shared_ptr<Ojm> model = Ojm::load(dir + modelName + ".ojm", dir);
    if (!model || !model->getOk()) {
        // Old: cout error + radius zeroed (never drawn). The body stays (orbit
        // etc. remain valid) - only the model is undrawable.
        cLog::get()->write("OjmLoader: failed to load model '" + modelName + "' for body '" + target->getEnglishName() + "'", LOG_TYPE::L_ERROR);
        target->setRadius(0);
        return nullptr;
    }
    // radius param is a scale factor on the model's own radius
    // (old: initialRadius *= obj3D->getRadius(); vertices are unit-normalized
    // at load, the model radius survives in getRadius()).
    target->setRadius(target->getRadius() * model->getRadius());
    // Old Artificial suppressed its halo entirely (drawHalo override).
    target->setHaloEnabled(false);
    auto module = std::make_unique<OjmModule>(std::move(model));
    // near-only routing: an artificial body's drawn regime is near (old
    // parity: Artificial drew as a body, no far-specific content; the halo -
    // the usual far carrier - is suppressed).
    addNearComponent(target, module.get());
    return module;
}
