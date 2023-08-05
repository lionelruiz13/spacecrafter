#include "ModularBodyPtr.hpp"

std::vector<ModularBodyPtr *> ModularBodyPtr::ref;

ModularBodyPtr::~ModularBodyPtr()
{
    if (ptr)
        --(ptr->pointerCount);
    int i = -1;
    while (ref[++i] != this);
    for (const int end = ref.size() - 1; i < end; ++i) {
        ref[i] = ref[i+1];
    }
    ref.pop_back();
}

ModularBodySelector::~ModularBodySelector()
{
    if (ptr)
        ptr->deselect();
}

void ModularBodyPtr::redirect(ModularBody *from, ModularBody *to)
{
    if (from->isSelected) {
        from->deselect();
        to->select();
    }
    for (auto &r : ref) {
        if (r->ptr == from) {
            r->ptr = to;
            ++(to->pointerCount);
        }
    }
}
