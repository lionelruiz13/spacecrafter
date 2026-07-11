#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"

// Shadow orchestration (SKETCH - contract only, implementation second-pass,
// G7): the system level decides WHICH bodies shadow which - per-module hooks
// (drawShadow/drawSelfShadow) only say HOW. Successor of the old
// SolarSystemDisplay::computePreDraw shadowingBody ranking + bindShadows
// plumbing (ShadowRenderData/UShadowingBody). Selection inputs: distance to
// the light source vs the shadow receiver, angular overlap, and the shadow
// traits of the candidates' modules. MINOR_BODY bodies never participate
// (D3 border case). The center of interest gets the high-resolution
// self-shadow buffer (MAIN_SELF_SHADOWING_RESOLUTION), others the small one -
// buffer ownership is the Renderer's (Renderer.hpp), bucket selection is
// this system's.
class ModularSystem : public ModularBody {
public:
    ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info);

    // Reload a system
    void reloadSystem() {
        childs.clear();
        loadSystem(systemFilename);
    }
    // Load a system
    void loadSystem(const std::string &filename);
    // Load a body
    void loadBody(std::map<std::string, std::string> &param);
    // Update this system
    void updateSystem();
    // Draw this system
    void drawSystem(Renderer &renderer);
    // Internally used by ModularBody to inform the creation of body in this system
    inline void addBody(ModularBody *body) {
        sortedSystemBodies.push_back(body);
        needCleanUp = true;
    }
    // Internally used by ModularBody to inform the destruction of body in this system
    inline void removeBody(ModularBody *body) {
        auto ptr = sortedSystemBodies.data();
        while (*ptr != body)
            ++ptr;
        *ptr = nullptr;
        needCleanUp = true;
    }
    inline std::vector<ModularBody *>::const_iterator begin() const {
        return sortedSystemBodies.begin();
    }
    inline std::vector<ModularBody *>::const_iterator end() const {
        return sortedSystemBodies.end();
    }
    // Find the body at the given normalized screen position (in range [-1, 1])
    ModularBody *findBodyAt(const std::pair<float, float> &screenPos) const;
    // Return the star of this system
    inline ModularBody *getSystemStar() const {
        return star;
    }
    // Find the system in which the given body is
    static inline ModularSystem *systemOf(ModularBody *body) {
        while (body->isNotIsolated)
            body = body->parent;
        return static_cast<ModularSystem *>(body);
    }
private:
    // Apply some hardcoded content
    void applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param);
    // Clean the list when it is dirty
    void cleanUp();
    // Tell that the list is dirty
    std::vector<ModularBody *> sortedSystemBodies;
    ModularBodyPtr star; // Star of the system
    std::string systemFilename;
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
