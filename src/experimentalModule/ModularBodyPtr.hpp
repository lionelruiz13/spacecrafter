#ifndef MODULAR_BODY_PTR_HPP_
#define MODULAR_BODY_PTR_HPP_

#include <vector>
#include <cstddef>

class ModularBody;

// Safe long-lived ModularBody pointer
// Prefer raw ModularBody pointer when short-lived
class ModularBodyPtr {
public:
    inline ModularBodyPtr() : ptr(nullptr)
    {
        ref.push_back(this);
    }
    ModularBodyPtr(ModularBody *body);
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

    void operator=(nullptr_t);
    void operator=(ModularBody *body);

    static void redirect(ModularBody *from, ModularBody *to);

    // Allow ModularBody to keep track of their parent body while hidden (thus not considered as a child)
    static void rawTrack(ModularBody *&toTrack);
    static void rawUntrack(ModularBody *&toTrack);
protected:
    ModularBody *ptr;
    static std::vector<ModularBodyPtr *> ref;
};

class ModularBodySelector : public ModularBodyPtr {
public:
    ~ModularBodySelector();
    // NOT inline: defined in ModularBodyPtr.cpp - the previous inline
    // specifiers on out-of-line definitions were an ODR trap that stayed
    // latent only because no TU assigned a selector until the SSystemFactory
    // selection seam (first user, 2026-07-12).
    void operator=(nullptr_t);
    void operator=(ModularBody *body);
};

#endif /* end of include guard: MODULAR_BODY_PTR_HPP_ */
