#include "SkinnableColorMap.hpp"
#include "tools/file_path.hpp"

void SkinnableColorMap::preload()
{
    int tmp = s_texture::setBigTextureLifetime(100);
    mapTexture.prioritize(LoadPriority::PRELOAD);
    mapTexture.getBigTexture();
    s_texture::setBigTextureLifetime(tmp);
}

void SkinnableColorMap::createSkin(const std::string &texName)
{
    // Old parity (Body::createTexSkin): creating or replacing a skin resets the
    // drawn texture to the map; activation is switchSkin's job. Load
    // type/flags mirror old exactly (PNG_SOLID_REPEAT, mipmap, resolution).
    skinUse = false;
    skinTexture = std::make_unique<s_texture>(FilePath(texName, FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true, true);
}

void SkinnableColorMap::switchSkin(bool use)
{
    if (use && !skinTexture)
        return; // old parity: switchMapSkin(true) without a skin is a no-op
    skinUse = use;
}

s_texture *SkinnableColorMap::activeColorTex()
{
    return (skinUse && skinTexture && !skinTexture->isLoading()) ? skinTexture.get() : &mapTexture;
}

Texture *SkinnableColorMap::resolve(bool allowBigTexture)
{
    // One compare per transition (skin on/off, skin load completion, big
    // texture appear/drop) - the exact branch structure BasicMesh carried
    // before this type existed, moved unchanged.
    if (s_texture *color = activeColorTex(); color != &mapTexture) {
        if (texBinding != BIND_SKIN) {
            texBinding = BIND_SKIN;
            return &color->getTexture();
        }
    } else if (allowBigTexture) {
        TEXMAP1(mapTexture);
        if (texBinding != texmap) {
            texBinding = texmap;
            return &TEX(0, mapTexture);
        }
    } else if (texBinding) {
        texBinding = 0;
        return &mapTexture.getTexture();
    }
    return nullptr;
}
