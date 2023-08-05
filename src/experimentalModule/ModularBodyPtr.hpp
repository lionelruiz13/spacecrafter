#ifndef MODULAR_BODY_PTR_HPP_
#define MODULAR_BODY_PTR_HPP_

#include "ModularBody.hpp"

// Safe long-lived ModularBody pointer
// Prefer raw ModularBody pointer when short-lived
class ModularBodyPtr {
public:
    inline ModularBodyPtr() : ptr(nullptr)
    {
        ref.push_back(this);
    }
    inline ModularBodyPtr(ModularBody *body) : ptr(body)
    {
        if (body)
            ++(body->pointerCount);
        ref.push_back(this);
    }
    ~ModularBodyPtr();

    inline ModularBody &operator*() const {
        return *ptr;
    }

    inline ModularBody *operator->() const {
        return ptr;
    }

    inline operator ModularBody*() const {
        return ptr;
    }

    inline operator ModularBody&() const {
        return *ptr;
    }

    inline void operator=(nullptr_t) {
        if (ptr)
            --(ptr->pointerCount);
        ptr = nullptr;
    }
    inline ModularBody *operator=(ModularBody *body) {
        if (ptr)
            --(ptr->pointerCount);
        if (ptr = body)
            ++(ptr->pointerCount);
    }

    static void redirect(ModularBody *from, ModularBody *to);
private:
    ModularBody *ptr;
    static std::vector<ModularBodyPtr *> ref;

    // Allow ModularBody to keep track of their parent body while hidden (thus not considered as a child)
    friend bool ModularBody::show();
    friend bool ModularBody::hide();
    friend bool ModularBody::remove(bool);
    static inline void rawTrack(ModularBody *&toTrack) {
        ++toTrack->pointerCount;
        auto self = reinterpret_cast<ModularBodyPtr *>(&toTrack);
        ref.push_back(self);
    }
    static inline void rawUntrack(ModularBody *&toTrack) {
        reinterpret_cast<ModularBodyPtr&>(toTrack).~ModularBodyPtr();
    }
};

class ModularBodySelector {
public:
    ~ModularBodySelector();
    inline void operator=(nullptr_t) {
        if (ptr) {
            --(ptr->pointerCount);
            ptr->deselect();
        }
        ptr = nullptr;
    }
    inline ModularBody *operator=(ModularBody *body) {
        if (ptr) {
            --(ptr->pointerCount);
            ptr->deselect();
        }
        if (ptr = body) {
            ++(ptr->pointerCount);
            ptr->select();
        }
    }
};

#endif /* end of include guard: MODULAR_BODY_PTR_HPP_ */
