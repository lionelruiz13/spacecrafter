#include"core.hpp"

Core::Core() {
    obj1a = new Objet("obj1a");
    obj1b = new Objet("obj1b");
    obj2_universe = new Objet("obj2_universe");
    obj3_galaxy = new Objet("obj3_galaxy");
    camera = new Camera(0,0);
}
Core::~Core() {
    delete obj1a;
    delete obj1b;
    delete obj2_universe;
    delete obj3_galaxy;
    delete camera;
}

void Core::init() {
    std::cout << "initialisation core" << std::endl;
}

void Core::update() {
    std::cout << "update core" << std::endl;
}

void Core::draw() {
    std::cout << "draw core" << std::endl;
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