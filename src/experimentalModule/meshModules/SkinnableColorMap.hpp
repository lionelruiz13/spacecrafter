#ifndef SKINNABLE_COLOR_MAP_HPP_
#define SKINNABLE_COLOR_MAP_HPP_

#include "tools/s_texture.hpp"
#include <memory>
#include <string>

class Texture;

// Which color texture a textured-sphere module must bind: the authored map, its big-texture level or a runtime skin
// Owns WHICH texture and WHETHER it changed; the module owns its descriptor set and does the binding
// A skin still loading keeps the map bound (an s_texture bound before its upload is an uninitialized descriptor)
class SkinnableColorMap {
public:
    SkinnableColorMap(const std::string &texturePath)
        : mapTexture(texturePath, TEX_LOAD_TYPE_PNG_SOLID, true, true) {}

    inline bool isLoading() {
        return mapTexture.isLoading();
    }
    inline s_texture &map() {
        return mapTexture;
    }
    // Pull the big level in at PRELOAD priority, kept resident keepFrames frames if nothing uses it meanwhile
    void preload(int keepFrames);
    // Create or replace the skin: resets the drawn texture to the map, activation is switchSkin's job
    void createSkin(const std::string &texName);
    // switchSkin(true) without a skin is a no-op
    void switchSkin(bool use);
    bool isSkinUsed() const { return skinUse; }
    // allowBigTexture = the caller's size gate. Returns the texture to (re)bind, or nullptr if the last one still holds
    // To call every drawn frame: asking is what keeps the big level resident
    Texture *resolve(bool allowBigTexture);
private:
    s_texture *activeColorTex();
    // Binding-state key: 0 = plain map, big-texture bits = TEXMAP mapping,
    // BIND_SKIN = the skin texture.
    static constexpr uint16_t BIND_SKIN = 0x8000;
    s_texture mapTexture;
    std::unique_ptr<s_texture> skinTexture;
    bool skinUse = false;
    uint16_t texBinding = 0;
};

#endif /* end of include guard: SKINNABLE_COLOR_MAP_HPP_ */
