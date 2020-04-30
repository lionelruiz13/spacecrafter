#ifndef COREMODE3_HPP
#define COREMODE3_HPP

#include"objet.hpp"
#include"CoreMode.hpp"
#include"camera.hpp"
#include<iostream>

class CoreMode3 : public CoreMode {

public:
    
	CoreMode3();
	~CoreMode3();

    void draw();
    void update();
    void onEnter(Objet* _obj3_galaxy, Camera* &cam);
    void onExit(Camera * &cam);
    
protected:
    void maCamera();

private:
    Objet* obj3_galaxy;

    Camera* maCamera;
};

#endif