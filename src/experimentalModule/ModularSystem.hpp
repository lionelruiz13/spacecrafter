#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"
#include "ModularSystemFormat.hpp"

class ModularSystem : public ModularBody {
public:
    ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info);
    ~ModularSystem() override {
        clearChildren();
    }

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
    void loadComposedSystem(const std::string &filename);
    void generateComposedTwin(const std::string &legacyFilename, const std::string &outPath);
    bool saveSystem(const std::string &outPath);
    void loadBody(std::map<std::string, std::string> &param,
                  ModularSystemFormat::Section *origin = nullptr,
                  bool supplemental = false);
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
    void drawNested(Renderer &renderer);
    void drawStarProxy(Renderer &renderer);
    // Internally used by ModularBody to inform the creation of body in this system
    inline void addBody(ModularBody *body) {
        sortedSystemBodies.push_back(body);
        needCleanUp = true;
    }
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
    std::vector<ModularSystemFormat::Section> loadedSections;
    // Which reader systemFilename belongs to (reloadSystem dispatch):
    // false = legacy loadSystem, true = composed loadComposedSystem.
    bool composedFile = false;
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
