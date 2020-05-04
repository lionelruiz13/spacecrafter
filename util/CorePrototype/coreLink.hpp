#ifndef CORELINK_HPP
#define CORELINK_HPP

#include "core.hpp"

class CoreLink {
public: 
    CoreLink(Core* _core);
    ~CoreLink();

    void getObj1(){
        core->obj1a->getObjet();
    }

    void getObj2(){
        core->obj1b->getObjet();
    }

    void getObj3_universe(){
        core->obj2_universe->getObjet();
    }

    void getObj4_galaxy(){
        core->obj3_galaxy->getObjet();
    }

    void getCamera(){
        core->camera->getCamera();
    }

    void setObj1(std::string n){
        core->obj1a->setObjet(n);
    }

    void setObj2(std::string n){
        core->obj1b->setObjet(n);
    }

    void setObj3_universe(std::string n){
        core->obj2_universe->setObjet(n);
    }

    void setObj4_galaxy(std::string n){
        core->obj3_galaxy->setObjet(n);
    }

    void setCamera(std::string n){
        core->camera->setCamera(n);
    }

private: 
    Core *core = nullptr;
};

#endif