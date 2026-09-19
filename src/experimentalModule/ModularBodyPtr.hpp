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

    // Redirect every holder of from; to == nullptr only at teardown of the root
    static void redirect(ModularBody *from, ModularBody *to);
protected:
    ModularBody *ptr;
    static std::vector<ModularBodyPtr *> ref; // Every live holder
};

// Select the body it points to, deselect the one it leaves
class ModularBodySelector : public ModularBodyPtr {
public:
    ~ModularBodySelector();
    void operator=(nullptr_t);
    void operator=(ModularBody *body);
};

#endif /* end of include guard: MODULAR_BODY_PTR_HPP_ */
