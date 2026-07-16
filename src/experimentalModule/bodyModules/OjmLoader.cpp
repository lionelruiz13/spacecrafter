#include "OjmLoader.hpp"
#include "OjmModule.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "ojmModule/ojm.hpp"
#include "tools/app_settings.hpp"
#include "tools/log.hpp"
#include <cstring>

uint8_t OjmLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    // The compound deduction key (ModularBody::deduceBodyModuleList): the
    // 4-byte "Arti" prefix = the old parse's exact type discriminator.
    if (params["model_name"].empty())
        return 0;
    const std::string &type = params["type"];
    if (type.size() < 4 || std::memcmp(type.data(), "Arti", 4) != 0)
        return 0;
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
