#include "RenderChain.hpp"
#include "ModularBody.hpp"
#include <algorithm>

RenderChain RenderChain::instance;

void RenderChain::park(std::unique_ptr<ModularBody> body)
{
    parked.push_back(std::move(body));
}

void RenderChain::onPinDrained(ModularBody *body)
{
    auto it = std::find_if(parked.begin(), parked.end(), [body](auto &b) {
        return b.get() == body;
    });
    if (it != parked.end())
        parked.erase(it); // unique_ptr destruction = the deferred destruction
}
