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

void Core::setFlagSelectedObjectPointer() {}
void Core::setFlagTracking() {}
void Core::dragView() {}
void Core::unSelect() {}