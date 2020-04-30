#include"CoreMode.hpp"

CoreMode::CoreMode() {}
CoreMode::~CoreMode() {}

void CoreMode::draw() {
    std::cout << "draw CoreMode" << std::endl;
}
void CoreMode::update() {
    std::cout << "update CoreMode" << std::endl;
}
void CoreMode::onEnter() {}
void CoreMode::onExit() {}