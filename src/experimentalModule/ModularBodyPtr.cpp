#include "ModularBodyPtr.hpp"
#include "ModularBody.hpp"

std::vector<ModularBodyPtr *> ModularBodyPtr::ref;

ModularBodyPtr::ModularBodyPtr(ModularBody *body) :
    ptr(body)
{
    if (body)
        ++(body->pointerCount);
    ref.push_back(this);
}

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

void ModularBodyPtr::operator=(nullptr_t)
{
    if (ptr)
        --(ptr->pointerCount);
    ptr = nullptr;
}

void ModularBodyPtr::operator=(ModularBody *body)
{
    if (ptr)
        --(ptr->pointerCount);
    if ((ptr = body))
        ++(ptr->pointerCount);
}

void ModularBodyPtr::rawTrack(ModularBody *&toTrack)
{
    ++toTrack->pointerCount;
    auto self = reinterpret_cast<ModularBodyPtr *>(&toTrack);
    ref.push_back(self);
}

void ModularBodyPtr::rawUntrack(ModularBody *&toTrack)
{
    reinterpret_cast<ModularBodyPtr&>(toTrack).~ModularBodyPtr();
}

void ModularBodySelector::operator=(nullptr_t)
{
    if (ptr) {
        --(ptr->pointerCount);
        ptr->deselect();
    }
    ptr = nullptr;
}

void ModularBodySelector::operator=(ModularBody *body) {
    if (ptr) {
        --(ptr->pointerCount);
        ptr->deselect();
    }
    if ((ptr = body)) {
        ++(ptr->pointerCount);
        ptr->select();
    }
}
