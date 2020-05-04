#ifndef CORE_HPP
#define CORE_HPP

#include"objet.hpp"
#include"camera.hpp"
#include<iostream>

class Core {

public:
    friend class CoreLink;
    
	Core();
	~Core();

    void init();

    void update();

	void draw();

    void setFlagSelectedObjectPointer();
    void setFlagTracking();
    void dragView();
    void unSelect();

private:
    //core 1
    Objet* obj1a;
    Objet* obj1b;
    //core 2
    Objet* obj2_universe;
    //core3
    Objet* obj3_galaxy;
    //partout
    Camera* camera;
};

#endif