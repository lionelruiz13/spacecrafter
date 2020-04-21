#ifndef APPCOMMANDINTERFACE_HPP
#define APPCOMMANDINTERFACE_HPP

#include"core.hpp"
#include"app.hpp"
#include"ui.hpp"


class AppCommandInterface {

public:
	AppCommandInterface(Core * _core,  App * _app, UI* _ui ) {
		core = _core;
		app = _app;
		ui = _ui;
	}
	~AppCommandInterface() {}

    void init() {}

    void update() {}

	void draw() {}

private:

	Core * core = nullptr;
	App * app = nullptr;
	Ui* ui = nullptr;
};

#endif