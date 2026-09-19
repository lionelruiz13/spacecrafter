#include "SkinnableColorMap.hpp"
#include "tools/file_path.hpp"

void SkinnableColorMap::preload(int keepFrames)
{
    int tmp = s_texture::setBigTextureLifetime(keepFrames);
    mapTexture.prioritize(LoadPriority::PRELOAD);
    mapTexture.getBigTexture();
    s_texture::setBigTextureLifetime(tmp);
}

void SkinnableColorMap::createSkin(const std::string &texName)
{
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
