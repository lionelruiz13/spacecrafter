#ifndef SKINNABLE_COLOR_MAP_HPP_
#define SKINNABLE_COLOR_MAP_HPP_

#include "tools/s_texture.hpp"
#include <memory>
#include <string>

class Texture;

class SkinnableColorMap {
public:
    // Same construction as the old BasicMesh member: solid PNG, mipmapped,
    // resolution-managed (the big-texture ladder).
    SkinnableColorMap(const std::string &texturePath)
        : mapTexture(texturePath, TEX_LOAD_TYPE_PNG_SOLID, true, true) {}

    inline bool isLoading() {
        return mapTexture.isLoading();
    }
    inline s_texture &map() {
        return mapTexture;
    }
    void preload(int keepFrames);
    // Old parity (Body::createTexSkin): create/replace resets the drawn texture
    // to the map; activation is switchSkin's job.
    void createSkin(const std::string &texName);
    // Old parity (Body::switchMapSkin): switch(true) without a skin is a no-op.
    void switchSkin(bool use);
    //! Is the skin the one being drawn? The read half of switchSkin
    //! (b31-design S2 row D7; INTENT S11.129).
    bool isSkinUsed() const { return skinUse; }
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
