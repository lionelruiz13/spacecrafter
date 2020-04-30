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
    void onEnter(Objet* _obj1a, Objet* _obj1b, Camera* &cam);
    void onExit(Camera * &cam);

protected:
    void maCamera();
    
private:
    Objet* obj1a;
    Objet* obj1b;

    Camera* maCamera;
};

#endif