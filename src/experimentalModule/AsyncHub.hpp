#ifndef ASYNC_HUB_HPP_
#define ASYNC_HUB_HPP_

#include "EntityCore/Executor/Tickable.hpp"
#include <list>

// Used to reduce constraints
class AsyncHub {
public:
    AsyncHub();
    ~AsyncHub();

    void update(float deltaTime) {
        updateList.remove_if([deltaTime](auto *obj){return obj->update(deltaTime);});
    }
    void startTicking(Tickable<AsyncHub> *arg) {
        updateList.push_back(arg);
    }
    void stopTicking(Tickable<AsyncHub> *arg) {
        updateList.remove(arg);
    }
    static AsyncHub *instance;
private:
    std::list<Tickable<AsyncHub> *> updateList;
};

#endif /* end of include guard: ASYNC_HUB_HPP_ */
