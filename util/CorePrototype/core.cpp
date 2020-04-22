#include"core.hpp"

Core::Core() {
    obj1 = new Objet("obj1");
    obj2 = new Objet("obj2");
    obj3_universe = new Objet("obj3_universe");
    obj4_galaxy = new Objet("obj4_galaxy");
    camera = new Objet("camera");
}
Core::~Core() {
    delete obj1;
    delete obj2;
    delete obj3_universe;
    delete obj4_galaxy;
    delete camera;
}

void Core::init() {
    std::cout << "initialisation core" << std::endl;
}

void Core::update() {
    std::cout << "update core" << std::endl;
    obj1->update();
    obj2->update();
    obj3_universe->update();
    obj4_galaxy->update();
    camera->update();
}

void Core::draw() {
    std::cout << "draw core" << std::endl;
    obj1->draw();
    obj2->draw();
    obj3_universe->draw();
    obj4_galaxy->draw();
    camera->draw();
}

void Core::setFlagSelectedObjectPointer() {
    std::cout << "core->setFlagSelectedObjectPointer" << std::endl;
}
void Core::setFlagTracking() {
    std::cout << "core->setFlagTracking" << std::endl;
}
void Core::dragView() {
    std::cout << "core->dragView" << std::endl;
}
void Core::unSelect() {
    std::cout << "core->unSelect" << std::endl;
}