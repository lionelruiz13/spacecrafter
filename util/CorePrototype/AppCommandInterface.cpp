#include"AppCommandInterface.hpp"

AppCommandInterface::AppCommandInterface(Core * _core, CoreLink *_coreLink, App * _app, Ui* _ui ) {
		core = _core;
		coreLink = _coreLink;
        app = _app;
		ui = _ui;
	}
AppCommandInterface::~AppCommandInterface() {}

void AppCommandInterface::init() {
    std::cout << "initialisation AppCommandInterface" << std::endl;
}

void AppCommandInterface::update() {
	std::cout << "AppCommandInterface update" << std::endl;
    core->setFlagSelectedObjectPointer();
    core->setFlagTracking();
    core->unSelect();
    coreLink->getObj1();
    coreLink->getObj2();
    coreLink->getObj3_universe();
    coreLink->getObj4_galaxy();
    coreLink->getCamera();
}

void AppCommandInterface::draw() {
	std::cout << "AppCommandInterface draw" << std::endl;
    coreLink->getObj1();
    coreLink->getObj2();
    coreLink->getObj3_universe();
    coreLink->getObj4_galaxy();
    coreLink->getCamera();
}
