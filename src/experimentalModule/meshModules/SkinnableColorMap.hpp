#ifndef SKINNABLE_COLOR_MAP_HPP_
#define SKINNABLE_COLOR_MAP_HPP_

#include "tools/s_texture.hpp"
#include <memory>
#include <string>

class Texture;

// THE colour-map binding authority of every textured-sphere body module (I2).
//
// A body's drawn colour texture is not one texture but a small state machine
// over three: the authored map, an optional runtime SKIN (old
// Body::createTexSkin/switchMapSkin - commands `body name X skin_tex <file>` /
// `skin_use on|off`), and the map's BIG-TEXTURE level, which only engages when
// the body is large on screen. The rules are old-path parity and identical for
// every sphere module:
//   - creating or REPLACING a skin never activates it (old resets tex_current
//     to the map on replace); activating without a skin is a no-op;
//   - a skin that is still LOADING keeps the map bound (binding an s_texture
//     before its upload gives an uninitialized descriptor - the §11.44(d)
//     black-texture class);
//   - the big texture is only asked for above the caller's size gate, and
//     asking is part of the mechanism (getBigTexture() is what keeps a big
//     level resident - s_texture::update drops levels not asked for 2 frames).
//
// Lifted here (B12) when the star photosphere became the second module needing
// exactly these rules: a second copy of the state machine is a pending silent
// desync (I2), and the skin seam on the SUN is a demonstrated operator use -
// the shipped data carries `#tex_skin = bodies/sun304A-sdo.jpg` on the Sun.
//
// Split of responsibility (I1): this type owns WHICH texture should be bound
// and WHETHER that changed; the module owns its own descriptor set, so it does
// the binding. resolve() returns the texture to bind, or nullptr when the
// current binding is already right - so a module never rebinds a set per frame.
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
    // Preload hint (old BasicMesh::preload): pull the big level in at PRELOAD
    // priority with a lifetime long enough to survive the approach.
    void preload();
    // Old parity (Body::createTexSkin): create/replace resets the drawn texture
    // to the map; activation is switchSkin's job.
    void createSkin(const std::string &texName);
    // Old parity (Body::switchMapSkin): switch(true) without a skin is a no-op.
    void switchSkin(bool use);
    //! Is the skin the one being drawn? The read half of switchSkin
    //! (b31-design §2 row D7; INTENT §11.129).
    bool isSkinUsed() const { return skinUse; }
    // THE state machine. allowBigTexture = the caller's size gate (the near
    // regime's BODY_BIG_TEXTURE_BOUNDING_SIZE; the depth-less mid band never engages big
    // textures). Returns the texture to (re)bind, or nullptr if the binding
    // this object last reported is still the right one.
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
