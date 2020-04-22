#ifndef CORELINK_HPP
#define CORELINK_HPP

#include "core.hpp"

class CoreLink {
public: 
    CoreLink(Core* _core);
    ~CoreLink();

    void getObj1(){
        core->obj1->getObjet();
    }

    void getObj2(){
        core->obj2->getObjet();
    }

    void getObj3_universe(){
        core->obj3_universe->getObjet();
    }

    void getObj4_galaxy(){
        core->obj4_galaxy->getObjet();
    }

    void getCamera(){
        core->camera->getObjet();
    }

    void setObj1(std::string n){
        core->obj1->setObjet(n);
    }

    void setObj2(std::string n){
        core->obj2->setObjet(n);
    }

    void setObj3_universe(std::string n){
        core->obj3_universe->setObjet(n);
    }

    void setObj4_galaxy(std::string n){
        core->obj4_galaxy->setObjet(n);
    }

    void setCamera(std::string n){
        core->camera->setObjet(n);
    }

private: 
    Core *core = nullptr;
};

#endif