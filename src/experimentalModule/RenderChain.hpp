#ifndef RENDER_CHAIN_HPP_
#define RENDER_CHAIN_HPP_

#include "EntityCore/Executor/Taskable.hpp"
#include <memory>
#include <vector>

class ModularBody;

class RenderChain : public Taskable {
public:
    void park(std::unique_ptr<ModularBody> body);

    void onPinDrained(ModularBody *body);

    static RenderChain instance;
private:
    std::vector<std::unique_ptr<ModularBody>> parked;
};

#endif /* end of include guard: RENDER_CHAIN_HPP_ */
