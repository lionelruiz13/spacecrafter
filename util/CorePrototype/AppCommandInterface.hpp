#ifndef APPCOMMANDINTERFACE_HPP
#define APPCOMMANDINTERFACE_HPP

class Core;
class App;
class Ui;

#include<iostream>

class AppCommandInterface {

public:
	AppCommandInterface(Core * _core,  App * _app, Ui* _ui ) {
		core = _core;
		app = _app;
		ui = _ui;
	}
	~AppCommandInterface() {}

    void init() {
		std::cout << "initialisation AppCommandInterface" << std::endl;
	}

    void update() {}

	void draw() {}

private:

	Core * core = nullptr;
	App * app = nullptr;
	Ui* ui = nullptr;
};

#endif