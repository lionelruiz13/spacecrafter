#ifndef COREMODULE_HPP
#define COREMODULE_HPP

#include"core.hpp"
#include"camera.hpp"
#include<iostream>

class CoreMode : public Core {

public:
    
	CoreMode();
	~CoreMode();

    void draw();
    void update();
    void onEnter();
    void onExit();

protected:
    void maCamera();
    
private:
    Camera* maCamera;
};

#endif