#include"AppCommandInterface.hpp"

AppCommandInterface::AppCommandInterface(Core * _core,  App * _app, Ui* _ui ) {
		core = _core;
		app = _app;
		ui = _ui;
	}
AppCommandInterface::~AppCommandInterface() {}

void AppCommandInterface::init() {
    std::cout << "initialisation AppCommandInterface" << std::endl;
}

void AppCommandInterface::update() {
    core->setFlagSelectedObjectPointer();
    core->setFlagTracking();
    core->unSelect();
}

void AppCommandInterface::draw() {}