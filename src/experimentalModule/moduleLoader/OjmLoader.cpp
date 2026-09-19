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
    target->setRadius(target->getRadius() * model->getRadius());
    // Old Artificial suppressed its halo entirely (drawHalo override).
    target->setHaloEnabled(false);
    auto module = std::make_unique<OjmModule>(std::move(model));
    addNearComponent(target, module.get());
    return module;
}
