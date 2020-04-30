#include"CoreMode1.hpp"

CoreMode1::CoreMode1() {}
CoreMode1::~CoreMode1() {}

void CoreMode1::draw() {
    std::cout << "draw CoreMode1" << std::endl;
    obj1a->draw();
    obj1b->draw();
}

void CoreMode1::update() {
    std::cout << "update CoreMode1" << std::endl;
    obj1a->update();
    obj1b->update();
}

void CoreMode1::onEnter(Objet* _obj1a, Objet* _obj1b, Camera * &cam) {
    obj1a = _obj1a;
    obj1b = _obj1b;
    Camera* camTemp = cam;
    cam = maCamera;
    maCamera = camTemp;
}

void CoreMode1::onExit(Camera * &cam) {
    cam = maCamera;
}