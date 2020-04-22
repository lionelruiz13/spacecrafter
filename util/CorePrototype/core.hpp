#ifndef CORE_HPP
#define CORE_HPP

#include"objet.hpp"
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
    Objet* obj1;
    Objet* obj2;
    Objet* obj3_universe;
    Objet* obj4_galaxy;
    Objet* camera;
};

#endif