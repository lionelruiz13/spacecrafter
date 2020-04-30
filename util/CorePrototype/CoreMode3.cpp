#include"CoreMode3.hpp"

CoreMode3::CoreMode3() {
    maCamera3 = new Camera("CoreMode3");
}
CoreMode3::~CoreMode3() {}

void CoreMode3::draw() {
    std::cout << "draw CoreMode3" << std::endl;
    obj3_galaxy->draw();
}

void CoreMode3::update() {
    std::cout << "update CoreMode3" << std::endl;
    obj3_galaxy->update();
}

void CoreMode3::onEnter(Objet* _obj3_galaxy, Camera * &cam) {
    obj3_galaxy = _obj3_galaxy;
    Camera* camTemp = cam;
    cam = maCamera3;
    maCamera3 = camTemp;
}

void CoreMode3::onExit(Camera * &cam) {
    cam = maCamera3;
}