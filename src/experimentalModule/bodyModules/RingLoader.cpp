#include "RingLoader.hpp"
#include "RingModule.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include "tools/sc_const.hpp"
#include "tools/log.hpp"

uint8_t RingLoader::isLikely(ModularBody *target, std::map<std::string, std::string> &params) const
{
    if (params["tex_ring"].empty())
        return 0;
    return 16; // slot-uncontested convention (AtmExtLoader/BasicMeshLoader)
}

bool RingLoader::isLoaderOf(BodyModule *module) const
{
    return dynamic_cast<RingModule*>(module);
}

std::unique_ptr<BodyModule> RingLoader::load(ModularBody *target, std::map<std::string, std::string> &params)
{
    // Old parse parity (protosystem.cpp ring block): radii in km -> AU (the
    // loadBody radius convention), texture PNG_ALPHA + mipmaps (ring.cpp:67).
    const float inner = Utility::strToFloat(params["ring_inner_size"]) / static_cast<float>(AU);
    const float outer = Utility::strToFloat(params["ring_outer_size"]) / static_cast<float>(AU);
    if (!(inner > 0 && outer > inner)) {
        cLog::get()->write("RingLoader: invalid ring radii for body '" + target->getEnglishName() + "' (ring_inner_size/ring_outer_size)", LOG_TYPE::L_WARNING);
        return nullptr;
    }
    auto ring = std::make_unique<RingModule>(
        std::make_unique<s_texture>(params["tex_ring"], TEX_LOAD_TYPE_PNG_ALPHA, true),
        inner, outer);
    // near+far routing (12 row 4): shadow selection scans nearComponents;
    // the far entry is where the row-4 color draw will ride (rings stay
    // visible when the disc regime drops).
    addNearComponent(target, ring.get());
    addFarComponent(target, ring.get());
    return ring;
}
