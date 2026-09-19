#ifndef RENDER_CHAIN_HPP_
#define RENDER_CHAIN_HPP_

#include "EntityCore/Executor/Taskable.hpp"
#include <memory>
#include <vector>

class ModularBody;

// Draw-visible state is only touched by tasks of this chain, work tasks publish through it
class RenderChain : public Taskable {
public:
    // Hold a detached body until its pins drain. Call from a task of this chain
    void park(std::unique_ptr<ModularBody> body);

    void onPinDrained(ModularBody *body);

    static RenderChain instance;
private:
    std::vector<std::unique_ptr<ModularBody>> parked;
};

#endif /* end of include guard: RENDER_CHAIN_HPP_ */
