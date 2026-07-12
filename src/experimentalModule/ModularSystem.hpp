#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"

// Shadow orchestration (LIVE, S5/G7 - design: shadow-paths.md B2): the
// system level decides WHICH bodies shadow which - per-module hooks
// (drawShadow/drawSelfShadow) only say HOW. Successor of the old
// SolarSystemDisplay::computePreDraw shadowingBody ranking + bindShadows
// plumbing, generalized past the center-of-interest restriction: EVERY drawn
// body is a receiver candidate; casters are bodies whose modules declare
// BMT_PROJECT_* traits. Selection = the ported light-cylinder test + the
// peak-occlusion >= 1/16 gate [vixy: 2026-07-12] (the old "4%" comment's
// exact form); ranking by occlusion, so pool exhaustion drops the least
// significant shadows. MINOR_BODY bodies never participate (D3 border case).
// Self-shadow bucket selection (MAIN vs SECONDARY resolution) remains
// second-pass: no self-shadow client exists before the OJM/terrain ports
// (shadow-paths.md C3).
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
    // Shadow orchestration (see class comment): fills each drawn receiver's
    // receivedShadows and drives ShadowService production. Runs at drawSystem
    // start - positions updated, renderer frame begun (frameIdx known).
    void computeShadows(Renderer &renderer);
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
