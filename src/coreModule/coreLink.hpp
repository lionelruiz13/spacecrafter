#ifndef CORELINK_HPP
#define CORELINK_HPP

#include "coreModule/core.hpp"


class CoreLink {
public: 

    void skyLineMgrSetColor(SKYLINE_TYPE name, Vec3f a);

    CoreLink(Core* _core);
    ~CoreLink();

private: 
    Core *core=nullptr;

};

#endif