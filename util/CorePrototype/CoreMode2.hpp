#ifndef COREMODE2_HPP
#define COREMODE2_HPP

#include"objet.hpp"
#include"CoreMode.hpp"
#include"camera.hpp"
#include<iostream>

class CoreMode2 : public CoreMode {

public:
    
	CoreMode2();
	~CoreMode2();

    void draw();
    void update();
    void onEnter();
    void onExit();

private:
    Objet* obj2_universe;
    
    Camera* maCamera;
};

#endif