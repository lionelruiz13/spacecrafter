#ifndef ASYNC_HUB_HPP_
#define ASYNC_HUB_HPP_

#include "EntityCore/Executor/TickMgr.hpp"

// Used to reduce constraints. The new-path tick manager: the registry, the
// tick and the mid-transition teardown all come from TickMgr<AsyncHub>.
class AsyncHub : public TickMgr<AsyncHub> {
public:
    AsyncHub();
    ~AsyncHub();

    static AsyncHub *instance;
};

#endif /* end of include guard: ASYNC_HUB_HPP_ */
