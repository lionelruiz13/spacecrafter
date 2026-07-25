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
    // Copying MUST re-register (B4, §11.111 / §5.43): the implicitly generated
    // copy constructor duplicated `ptr` without pushing the new object into
    // `ref` and without incrementing pointerCount, so the copy's destructor ran
    // `while (ref[++i] != this)` past the end of `ref` - a SIGSEGV, and before
    // that a pointerCount too low to protect a pinned body. Latent until a
    // ModularBodyPtr was stored in a value that a container RELOCATES (the B4
    // anchor registry: measured, crash at ModularBodyPtr.cpp:19 on the first
    // std::vector growth). Delegating to the raw-pointer constructor is the
    // single registration authority (I2), so a copy is exactly a second holder.
    inline ModularBodyPtr(const ModularBodyPtr &other) : ModularBodyPtr(other.ptr) {}
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

    // Redirect every tracked pointer from `from` to `to`. `to == nullptr` is
    // legal only at final teardown (the parentless universe root): pointers
    // are nulled, selection just drops. (rawTrack/rawUntrack removed
    // 2026-07-17: they existed solely to keep a hidden body's parent alive
    // while the body sat in the retired global hidden list - hidden bodies
    // are parent-owned now, INTENT 11.36.)
    static void redirect(ModularBody *from, ModularBody *to);
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
