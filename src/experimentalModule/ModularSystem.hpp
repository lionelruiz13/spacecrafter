#ifndef MODULAR_SYSTEM_HPP_
#define MODULAR_SYSTEM_HPP_

#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"

class ModularSystem : public ModularBody {
public:
    ModularSystem(ModularBody *parent, ModularBodyCreateInfo &info);

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
private:
    // Clean the list when it is dirty
    void cleanUp();
    // Tell that the list is dirty
    std::vector<ModularBody *> sortedSystemBodies;
    ModularBodyPtr star; // Star of the system
    bool needCleanUp = false;
};

#endif /* end of include guard: MODULAR_SYSTEM_HPP_ */
