#include "RingLoader.hpp"
#include "experimentalModule/bodyModules/RingModule.hpp"
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
    // ring_shadow_color: the D8-resolved ADDITIVE data key [vixy 2026-07-17]
    // - a color key parameterises (expands), the dead ring_shadow gate key
    // stays retired (a gate forecloses: a non-casting ring cannot cast on
    // the planet's moons). Default = the derived old-parity constant
    // ({0.7} == mix(1.0, 0.3, alpha) under the S5 change of variable).
    const Vec3f shadowColor = params["ring_shadow_color"].empty()
        ? Vec3f(0.7f, 0.7f, 0.7f)
        : Utility::strToVec3f(params["ring_shadow_color"]);
    auto ring = std::make_unique<RingModule>(
        std::make_unique<s_texture>(params["tex_ring"], TEX_LOAD_TYPE_PNG_ALPHA, true),
        inner, outer, shadowColor);
    // NEAR-ONLY routing (2026-07-18 correction; RingModule.hpp header block):
    // the earlier near+far routing double-drew in the 0.008-0.2 screenSize
    // band (ModularBody::draw runs BOTH lists there) and far modules never
    // receive update(). Near membership covers the drawNoDepth band down to
    // ~3 px < the old 5-px cutoff (body.cpp:1132).
    addNearComponent(target, ring.get());
    return ring;
}
