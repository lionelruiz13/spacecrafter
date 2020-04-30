#include"CoreMode2.hpp"

CoreMode2::CoreMode2() {}
CoreMode2::~CoreMode2() {}

void CoreMode2::draw() {
    std::cout << "draw CoreMode2" << std::endl;
    obj2_universe->draw();
}

void CoreMode2::update() {
    std::cout << "update CoreMode2" << std::endl;
    obj2_universe->update();
}

void CoreMode2::onEnter(Objet* _obj2_universe, Camera * &cam) {
     obj2_universe = _obj2_universe;
     Camera* camTemp = cam;
    cam = maCamera;
    maCamera = camTemp;
}

void CoreMode2::onExit(Camera * &cam) {
    cam = maCamera;
}