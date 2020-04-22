#include"ui.hpp"

Ui::Ui(Core* _core, App *_app) {
        core = _core;
        app = _app;
    }
Ui::~Ui() {}

void Ui::init() {
    std::cout << "initialisation ui" << std::endl;
}

void Ui::update() {
	std::cout << "ui update" << std::endl;
    core->setFlagSelectedObjectPointer();
    core->setFlagTracking();
    core->dragView();
}

void Ui::draw() {
    std::cout << "draw ui" << std::endl;
}
