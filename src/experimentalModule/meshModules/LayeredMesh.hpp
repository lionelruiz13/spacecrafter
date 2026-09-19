#ifndef LAYERED_MESH_HPP_
#define LAYERED_MESH_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/meshModules/MeshFamilies.hpp"
#include "experimentalModule/meshModules/bodyShaderInterface.hpp"
#include "tools/s_texture.hpp"
#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>
class ObjL;

// Draw a layered disc, ray-marched when close and rayCapable
class LayeredMesh : public BodyModule {
public:
    struct Config {
        std::string day;        // resolved paths, empty if absent
        std::string night;
        std::string specular;
        std::string normal;
        std::string heightmap;
        uint16_t midVariant;    // MeshFamilies::VARIANT_* bits
        bool tessellated;
        bool moonClass;         // type == Moon, select the altimetry level
        bool rayCapable;        // normal && heightmap
        uint16_t rayVariant;    // VARIANT_NIGHT when night && specular
        Vec3f atmColor;         // atmosphere_ambient_r/g/b
        float sunDeviation;     // sin(atmosphere_sun_deviation)
        float atmDeviation;     // sin(atmosphere_ambient_deviation)
    };
    LayeredMesh(ObjL *mesh, Config &&cfg);
    virtual ~LayeredMesh();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_PROJECT_G1_SHADOW | BMT_RECEIVE_SHADOW;
    }
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body, int keepFrames) override;
    // Include the heightmap displacement headroom in boundingRadius
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Replace the day layer only
    virtual void createTexSkin(const std::string &texName) override;
    virtual void switchTexSkin(bool use) override;
    bool getSkinUse(bool &out) const override { out = skinUse; return true; }
private:
    float altimetryLevel() const; // moon level if moonClass, else planet level
    s_texture &dayTex() { return skinBound ? *skinTexture : day; }
    // To call at every draw entry, rebind on any skin transition
    void refreshSkinState();
    void fillVert(Renderer &renderer, ModularBody *body, const Mat4f &mat);
    // Rebind a whole Set, big is per slot, null for the base level
    void rebind(bool ray, Texture *const *big);
    void drawMid(Renderer &renderer, ModularBody *body, const Mat4f &mat, uint16_t wanted, bool low);
    void drawRay(Renderer &renderer, ModularBody *body, const Mat4f &mat);

    bool loaded = false;
    bool skinUse = false;   // commanded state
    bool skinBound = false; // drawn state, skinUse and skin resident
    uint16_t midBigMapping = 0;
    uint16_t rayBigMapping = 0;
    ObjL *mesh;
    Config cfg;
    s_texture day;
    std::unique_ptr<s_texture> skinTexture;
    std::unique_ptr<s_texture> night, specular, normal, heightmap;
    // In binding order, null if the slot is absent
    s_texture *midSlots[5];
    uint8_t midSlotCount;
    uint8_t midFirstBinding;
    uint8_t midShadowBinding;
    s_texture *raySlots[5];
    PipelineFamily midFamily;
    PipelineFamily rayFamily; // null handle when !rayCapable
    std::unique_ptr<Set> midSet;
    std::unique_ptr<Set> raySet;
    SharedBuffer<globalVertProj> vert;
    SharedBuffer<meshFrag> frag;
    std::unique_ptr<SharedBuffer<meshTescGeom>> tescGeom; // tessellated only
    std::unique_ptr<SharedBuffer<rayMarchVert>> rayVert;  // rayCapable only
    std::unique_ptr<SharedBuffer<rayMarchFrag>> rayFrag;
};

#endif /* end of include guard: LAYERED_MESH_HPP_ */
