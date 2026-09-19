#ifndef RENDER_CHAIN_HPP_
#define RENDER_CHAIN_HPP_

#include "EntityCore/Executor/Taskable.hpp"
#include <memory>
#include <vector>

class ModularBody;

// All draw-visible state (body tree, slots, regime lists, notableBody) is only touched by tasks of this chain:
// ordering comes from the chain, not from thread identity. The frame task never blocks on loading
// Work tasks may block but never touch that state; they end by scheduling a small publish task here
class RenderChain : public Taskable {
public:
    // Hold a body whose destruction waits for its pins to drain. From a task of this chain, body already detached
    void park(std::unique_ptr<ModularBody> body);

    // Destroy the parked body whose pin count reached zero; called from its unpin path, inside the chain
    void onPinDrained(ModularBody *body);

    static RenderChain instance;
private:
    std::vector<std::unique_ptr<ModularBody>> parked;
};

#endif /* end of include guard: RENDER_CHAIN_HPP_ */
