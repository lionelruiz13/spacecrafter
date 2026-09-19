#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"
#include "ModularSystemFormat.hpp"

class ModularSystem : public ModularBody {
public:
    ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info);
    // Children deregister from sortedSystemBodies on destruction: destroy them while the members still exist
    ~ModularSystem() override {
        clearChildren();
    }

    // Destroy and rebuild every content body from the source file, at the current date; false if there is no file
    // Waits for the frames in flight. Holders of the old bodies re-seat them by name (see SSystemFactory)
    bool reloadSystem();
    // Whether this system has a data file behind it (see reloadSystem).
    inline bool hasSystemFile() const {
        return !systemFilename.empty();
    }
    inline const std::string &getSystemFilename() const {
        return systemFilename;
    }
    inline bool isComposedFile() const {
        return composedFile;
    }
    // Load a system
    void loadSystem(const std::string &filename);
    // Composed format: a section with type=<module family> is a module of the earlier node named by body=, else a node
    void loadComposedSystem(const std::string &filename);
    // Write the composed equivalent of a loaded legacy file from its live bodies; loading it must reproduce that load
    void generateComposedTwin(const std::string &legacyFilename, const std::string &outPath);
    // Write the declared bodies of this system (hidden included, nested systems excluded) to a composed file
    // An existing file is edited, never rebuilt: only missing declarations and loader annotations are added
    // Which files may be written is the caller's decision (SSystemFactory::saveCurrentSystem). false = nothing written
    bool saveSystem(const std::string &outPath);
    // origin = the declaring section when its file is writable (composed), annotated in place with the diagnostics
    // supplemental = pushed at runtime rather than loaded from a file
    void loadBody(std::map<std::string, std::string> &param,
                  ModularSystemFormat::Section *origin = nullptr,
                  bool supplemental = false);
    // Remove the supplemental bodies of this system's own content, hidden included; true if anything was removed
    bool removeSupplementalBodies();
    void startTrails(bool record);
    // Update this system
    void updateSystem();
    void drawSystem(Renderer &renderer);
    // The sorted body loop alone (no begin/end, no pointer) - shared by the
    // frame entry and nested draws.
    void drawSystemBodies(Renderer &renderer);
    void drawOrbits(Renderer &renderer);
    void drawTrails(Renderer &renderer);
    void drawTails(Renderer &renderer);
    // Draw this system as an entry of an enclosing system (camera outside): its content when large enough on screen,
    // else the halo of its star. No shadows inside
    void drawNested(Renderer &renderer);
    void drawStarProxy(Renderer &renderer);
    // Internally used by ModularBody to inform the creation of body in this system
    inline void addBody(ModularBody *body) {
        sortedSystemBodies.push_back(body);
        needCleanUp = true;
    }
    // Internally used by ModularBody to inform the destruction of body in this system; absent is legal (hidden body)
    inline void removeBody(ModularBody *body) {
        auto ptr = sortedSystemBodies.data();
        auto const end = ptr + sortedSystemBodies.size();
        while (ptr != end) {
            if (*ptr == body) {
                *ptr = nullptr; // compacted by the next cleanUp()
                needCleanUp = true;
                return;
            }
            ++ptr;
        }
    }
    // Take a hidden body out of the drawn and pickable list without destroying it; no-op if absent
    inline void unregisterBody(ModularBody *body) {
        for (auto it = sortedSystemBodies.begin(); it != sortedSystemBodies.end(); ++it) {
            if (*it == body) {
                sortedSystemBodies.erase(it);
                return;
            }
        }
    }
    inline std::vector<ModularBody *>::const_iterator begin() const {
        return sortedSystemBodies.begin();
    }
    inline std::vector<ModularBody *>::const_iterator end() const {
        return sortedSystemBodies.end();
    }
    // Find the body at the given normalized screen position (in range [-1, 1])
    ModularBody *findBodyAt(const std::pair<float, float> &screenPos) const;
    // nullptr for a starless system (star == this is the internal "unassigned" and must not leak)
    inline ModularBody *getSystemStar() const {
        return (star == this) ? nullptr : static_cast<ModularBody *>(star);
    }
    // Find the system in which the given body is
    static inline ModularSystem *systemOf(ModularBody *body) {
        while (body->isNotIsolated)
            body = body->parent;
        return static_cast<ModularSystem *>(body);
    }
private:
    // Decide which bodies shadow which and fill the receivedShadows of each drawn receiver; runs at drawSystem start
    void computeShadows(Renderer &renderer);
    void loadDeclaredModule(std::map<std::string, std::string> &params, const std::string &header,
                            const std::map<std::string, std::map<std::string, std::string>> &nodeParams,
                            BodyModuleType type, ModularSystemFormat::Section *origin);
    static stringHash_t composedNodeParams(const ModularBody *body, const stringHash_t &declared);
    static void appendWholeDeclaration(ModularBody *body, const stringHash_t &declared,
                                       std::vector<ModularSystemFormat::Section> &out);
    static void collectContentBodies(ModularBody *node, std::vector<ModularBody *> &out);
    // Apply some hardcoded content
    void applyHardcodedContent(ModularBodyCreateInfo &createInfo, std::map<std::string, std::string> &param);
    // Clean the list when it is dirty
    void cleanUp();
    // Tell that the list is dirty
    std::vector<ModularBody *> sortedSystemBodies;
    ModularBodyPtr star; // Star of the system
    std::string systemFilename;
    // The composed file as parsed, every line in order: the base saveSystem edits. Empty after a legacy load
    std::vector<ModularSystemFormat::Section> loadedSections;
    // Which reader systemFilename belongs to (reloadSystem dispatch):
    // false = legacy loadSystem, true = composed loadComposedSystem.
    bool composedFile = false;
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
