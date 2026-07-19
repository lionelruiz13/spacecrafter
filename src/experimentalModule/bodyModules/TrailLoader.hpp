#ifndef TRAIL_LOADER_HPP_
#define TRAIL_LOADER_HPP_

#include "experimentalModule/ModuleLoader.hpp"

// Loader of the TRAIL slot (row 9 - the historical path polyline). Deduction
// lives in deduceBodyModuleList (non-still orbit, non-satellite, non-Artificial
// - the old BigBody+SmallBody trail set). Routes into the dedicated
// trailComponents list (the trail pass is system-driven, ModularSystem::
// drawTrails, not a screen-size regime). MaxTrail follows the old per-class
// value: Planet/Dwarf 1460 (BigBody), Comet 2920, Asteroid/KBO/unknown 60
// (SmallBody). DeltaTrail is always 1 sim-day (old never data-set it).
class TrailLoader : public ModuleLoader {
public:
    virtual uint8_t isLikely(ModularBody *target, std::map<std::string, std::string> &params) const override;
    virtual bool isLoaderOf(BodyModule *module) const override;
    virtual std::unique_ptr<BodyModule> load(ModularBody *target, std::map<std::string, std::string> &params) override;
};

#endif /* end of include guard: TRAIL_LOADER_HPP_ */
