#include"core.hpp"

Core::Core() {}
Core::~Core() {}

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