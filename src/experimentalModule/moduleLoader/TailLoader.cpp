#include "TailLoader.hpp"
#include "experimentalModule/bodyModules/TailModule.hpp"
#include "tools/utility.hpp"

uint8_t TailLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    return 16; // Requested explicitly by deduceBodyModuleList; no competition
}

bool TailLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<TailModule *>(module);
}

std::unique_ptr<BodyModule> TailLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Faithful port of the SmallBody comet-tail construction (protosystem.cpp:
    // 802-851): the gas tail, the dust tail, and an optional extra tail, each
    // with the same defaults. The Tail ctor arg order was (deltaTraceJD,
    // ejectionForce, ejectionLinearity, coefRadius{xx,x,base}, color{r,g,b}).
    std::vector<TailModule::SubTail> subTails;
    // Gas tail (blue, straight - high ejection force, short trace window).
    subTails.push_back({
        Utility::strToFloat(params["gaz_tail_trace_jd"], 1),
        Utility::strToFloat(params["gaz_tail_ejection_force"], 30),
        Utility::strToFloat(params["gaz_tail_ejection_linearity"], 1),
        Vec3f{
            Utility::strToFloat(params["gaz_tail_radius_xx_coef"], -1),
            Utility::strToFloat(params["gaz_tail_radius_x_coef"], 0.5),
            Utility::strToFloat(params["gaz_tail_radius_base_coef"], 2),
        },
        Vec3f{
            Utility::strToFloat(params["gaz_tail_color_red"], 0.3),
            Utility::strToFloat(params["gaz_tail_color_green"], 0.3),
            Utility::strToFloat(params["gaz_tail_color_blue"], 0.7),
        },
    });
    // Dust tail (grey, curved - low ejection force, long trace window).
    subTails.push_back({
        Utility::strToFloat(params["dust_tail_trace_jd"], 30),
        Utility::strToFloat(params["dust_tail_ejection_force"], 0.5),
        Utility::strToFloat(params["dust_tail_ejection_linearity"], 1),
        Vec3f{
            Utility::strToFloat(params["dust_tail_radius_xx_coef"], -2),
            Utility::strToFloat(params["dust_tail_radius_x_coef"], 1),
            Utility::strToFloat(params["dust_tail_radius_base_coef"], 2),
        },
        Vec3f{
            Utility::strToFloat(params["dust_tail_color_red"], 0.5),
            Utility::strToFloat(params["dust_tail_color_green"], 0.5),
            Utility::strToFloat(params["dust_tail_color_blue"], 0.5),
        },
    });
    // Optional extra tail (only when the block is present, protosystem.cpp:835).
    if (!params["extra_tail_trace_jd"].empty()) {
        subTails.push_back({
            Utility::strToFloat(params["extra_tail_trace_jd"], 30),
            Utility::strToFloat(params["extra_tail_ejection_force"], 0.5),
            Utility::strToFloat(params["extra_tail_ejection_linearity"], 1),
            Vec3f{
                Utility::strToFloat(params["extra_tail_radius_xx_coef"], -2),
                Utility::strToFloat(params["extra_tail_radius_x_coef"], 1),
                Utility::strToFloat(params["extra_tail_radius_base_coef"], 2),
            },
            Vec3f{
                Utility::strToFloat(params["extra_tail_color_red"], 0.5),
                Utility::strToFloat(params["extra_tail_color_green"], 0.5),
                Utility::strToFloat(params["extra_tail_color_blue"], 0.5),
            },
        });
    }
    // Comet photometry: apparent_magnitude is the absolute magnitude H fed to
    // the coma/tail-size formula (misnamed in the old data), slope is G. Old
    // SmallBody defaults were -99 / -10 (setAbsoluteMagnitudeAndSlope never
    // called), but the deduce gate guarantees apparent_magnitude is present;
    // keep the old defaults for the degenerate case.
    auto module = std::make_unique<TailModule>(
        std::move(subTails),
        Utility::strToFloat(params["apparent_magnitude"], -99.f),
        Utility::strToFloat(params["slope"], -10.f));
    addTailComponent(target, module.get());
    return module;
}
