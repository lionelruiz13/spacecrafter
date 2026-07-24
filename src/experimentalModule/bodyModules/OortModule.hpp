#ifndef OORT_MODULE_HPP_
#define OORT_MODULE_HPP_

#include "experimentalModule/BodyModule.hpp"
#include "experimentalModule/PipelineFamily.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class VertexArray;
class VertexBuffer;
class Set;
template<typename> class SharedBuffer;

// OORT slot - the solar-system oort-cloud point cloud, as a modular body at the
// SolarSystem floor (B5, INTENT §6.9 content-migration PILOT; the [vixy] mapping
// rule "mode content becomes ModularBody instances at their floor level - oort
// at/around the SolarSystem node"). The old altitude-gated draw (coreModule/
// oort.cpp, solarSystemModule.cpp:196) is reproduced through the NEW path's
// distance/visibility REGIME machinery (G4 floor gating), NOT a hardcoded
// altitude test: attached as a NEAR component, so the body's own scaledRadius
// gates the low edge (hidden while the observer sits inside scaledRadius·2 -
// the in/grounded regime - shown once outside it, ModularBody::draw) and the
// node's resolve/collapse gates the far edge (a collapsed SolarSystem node
// draws no children).
//
// FAMILY CHOICE IS PROVISIONAL (B5 carve-out): this rides the CUSTOM loader
// family with an explicit "OORT" slot (the GRID precedent), the SIMPLEST family
// that renders a point cloud today. The reserved VOLUMETRIC slot vs a
// MINOR_BODY/cluster-traits carrier is the module-FAMILY generalization the
// pilot explicitly does NOT decide - flagged for Vixy.
//
// Shaders oort.vert/frag are used VERBATIM (shared with the old path - the I2
// single authority for the projection+color law); the point geometry shares
// oortSamplePoint() (coreModule/oort.hpp). Depth-less COLOR, alpha-blended
// (the cloud is a diffuse background element, old setDepthStencilMode() off).
class OortModule : public BodyModule {
public:
    // nbr = point count (config oort_elements, shared with the old cloud);
    // color = the cloud RGB (config oort_color). Builds the vertex buffer +
    // pipeline family at construction (core-init registration domain, the same
    // context the old Oort::populate runs in).
    OortModule(unsigned int nbr, const Vec3f &color);
    ~OortModule();
    // Static geometry: report the cloud extent as the bounding radius so the
    // body is visible whenever the observer is inside the cloud (all-direction
    // visibility - a small bounding sphere would cull the surrounding cloud when
    // the view points away from centre). NB (B5 finding): this extent feeds the
    // parent's subsystemRadius/AoI - see the INTENT §6.9 coupling note.
    virtual bool update(ModularBody *body, float scaledRadius) override;
    virtual void draw(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;
    // Far edge of the regime band (0.0015 < screenSize < 0.008): keep drawing so
    // the cloud does not blink out one regime early. Same render as draw().
    virtual void drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat) override;

    // One show flag shared with the old cloud's fader (the `flag oort` command /
    // config flag_oort route through CoreLink to BOTH). I2: one concept, "show
    // the oort", one operator surface; the pilot's A/B toggles it to isolate the
    // cloud's pixel contribution in each render phase.
    static bool show;
private:
    void render(Renderer &renderer, const Mat4f &mat);
    PipelineFamily family;
    std::unique_ptr<VertexArray> vertexModel;
    std::unique_ptr<VertexBuffer> vertex;
    Set *set = nullptr; // renderer-pool owned (family set 1: uMat + uFrag)
    struct Frag { Vec3f color; float fader; };
    std::unique_ptr<SharedBuffer<Mat4f>> uMat;
    std::unique_ptr<SharedBuffer<Frag>> uFrag;
    unsigned int nbPoints;
    Vec3f cloudColor;
    float cloudExtent = 0.f; // max |point|, the reported bounding radius (AU)
};

#endif /* end of include guard: OORT_MODULE_HPP_ */
