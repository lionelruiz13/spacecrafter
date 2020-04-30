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
    void onEnter(Objet* _obj2_universe, Camera* &cam);
    void onExit(Camera * &cam);

protected:
    void maCamera();

private:
    Objet* obj2_universe;

    Camera* maCamera2;
};

#endif