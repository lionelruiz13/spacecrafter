#ifndef SKINNABLE_COLOR_MAP_HPP_
#define SKINNABLE_COLOR_MAP_HPP_

#include "tools/s_texture.hpp"
#include <memory>
#include <string>

class Texture;

// Select the color texture to bind: map, big level or loaded skin
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
    void preload(int keepFrames);
    // Create or replace the skin, without activating it
    void createSkin(const std::string &texName);
    void switchSkin(bool use);
    bool isSkinUsed() const { return skinUse; }
    // To call every drawn frame, return nullptr if the binding still holds
    Texture *resolve(bool allowBigTexture);
private:
    s_texture *activeColorTex();
    static constexpr uint16_t BIND_SKIN = 0x8000;
    s_texture mapTexture;
    std::unique_ptr<s_texture> skinTexture;
    bool skinUse = false;
    uint16_t texBinding = 0; // 0 = map, TEXMAP mapping bits, or BIND_SKIN
};

#endif /* end of include guard: SKINNABLE_COLOR_MAP_HPP_ */
