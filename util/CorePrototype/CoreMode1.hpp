#ifndef COREMODE1_HPP
#define COREMODE1_HPP

#include"objet.hpp"
#include"CoreMode.hpp"
#include"camera.hpp"
#include<iostream>

class CoreMode1 : public CoreMode {

public:
    
	CoreMode1();
	~CoreMode1();

    void draw();
    void update();
    void onEnter();
    void onExit();

private:
    Objet* obj1a;
    Objet* obj1b;
    Camera* maCamera;
};

#endif