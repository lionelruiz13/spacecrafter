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

// The layered disc module (row 2): bodies with tex_night / tex_normal /
// tex_heightmap - the old my_earth / my_moon / body_night / body_bump /
// my_earth_shadow / my_moon_shadow shader grid, on TWO families + a
// close-range substitution:
//
// - MID regime: MESH_TES (tessellated + heightmap displacement) or
//   MESH_LAYERED (flat), row picked at load from the texture set + body type
//   (mirrors the old selectShader ORDER, incl. the Moon/Planet split:
//   Moon-type tests heightmap first, Planet-type tests night then norm).
// - CLOSE regime: MESH_RAYMARCH (per-pixel heightmap relief + terrain
//   self-shadow), substituting the mid family when the observer is near -
//   the old CoI switch (importance-ranked, shadow-flag-gated) becomes a
//   module-local gate (G4/D4 regime philosophy, I4):
//     rayMarchCapable && distance < scaledRadius*64 && screenSize > 0.025
//   Derivation: proximity is the actual CoI semantic (validated against the
//   4 reference scenes; K=64 sits in the dead zone [60, 221] between
//   engage/stay-mid cases); 0.025 = altimetry displacement >= 1px @2048.
//   NOT gated on ShadowService::enabled (old CoI was) - accepted-divergence
//   proposal, C1 class [convergence point].
//
// Shadows: all rows receive through the S5 generalized block
// (meshShadowFill.hpp); this module also CASTS via the service silhouette
// (BMT_PROJECT_G1_SHADOW), like BasicMesh.
class LayeredMesh : public BodyModule {
public:
    struct Config {
        std::string day;        // resolved paths; empty = absent
        std::string night;
        std::string specular;
        std::string normal;
        std::string heightmap;
        uint16_t midVariant;    // MeshFamilies::VARIANT_* bits of the mid row
        bool tessellated;       // mid family: MESH_TES vs MESH_LAYERED
        bool moonClass;         // altimetry level selector (ini type == Moon)
        bool rayCapable;        // normal && heightmap
        uint16_t rayVariant;    // VARIANT_NIGHT when night && specular (Earth class)
        Vec3f atmColor;         // atmosphere_ambient_r/g/b (0,0,0 default)
        float sunDeviation;     // sin(atmosphere_sun_deviation deg), 0 default
        float atmDeviation;     // sin(atmosphere_ambient_deviation deg), 0 default
    };
    LayeredMesh(ObjL *mesh, Config &&cfg);
    virtual ~LayeredMesh();
    virtual uint32_t getTraits() const override {
        return BMT_USE_DEPTH | BMT_DEPTH_TRACE | BMT_PROJECT_G1_SHADOW | BMT_RECEIVE_SHADOW;
    }
    virtual bool isLoaded() override;
    virtual void preload(ModularBody *body) override;
    // boundingRadius includes the heightmap displacement headroom
    // (scaledRadius * (1 + 0.01*altimetryLevel)) for tessellated/ray bodies:
    // the new path has no depth margin (the old 1.1 bucket margin absorbed
    // the old path's non-inclusive radius) - the inclusive contract
    // (BodyModule.hpp) carries the safety. Level is Scalable (live commands):
    // re-evaluated every update.
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    virtual void drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx) override;
    // Row-8 TRACE prepass: the layered disc cuts its orbit-hole exactly like
    // BasicMesh (shared sphere-trace family, TraceFamily.hpp).
    virtual void drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
private:
    float altimetryLevel() const; // moonClass? moon : planet level (shared BodyTesselation seam)
    void fillVert(Renderer &renderer, ModularBody *body, const Mat4f &mat);
    // Rebind a whole Set (descriptor generation change / initial bind).
    // slots = texture bindings in contract order; big = per-slot big texture
    // (null entries fall back to the s_texture's base level).
    void rebind(bool ray, Texture *const *big);
    void drawMid(Renderer &renderer, ModularBody *body, const Mat4f &mat, uint16_t wanted, bool low);
    void drawRay(Renderer &renderer, ModularBody *body, const Mat4f &mat);

    bool loaded = false;
    uint16_t midBigMapping = 0;
    uint16_t rayBigMapping = 0;
    ObjL *mesh;
    Config cfg;
    s_texture day;
    std::unique_ptr<s_texture> night, specular, normal, heightmap;
    // Texture pointers in CONTRACT ORDER per family (null = slot absent ->
    // day bound as the valid never-sampled placeholder). Mid: TES bindings
    // 3..7 = {day, night, specular, normal, heightmap}; LAYERED bindings
    // 2..4 = {day, night, normal}. Ray: bindings 2..6 =
    // {heightmap, normal, day, night, specular}.
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
