#ifndef APPCOMMANDINTERFACE_HPP
#define APPCOMMANDINTERFACE_HPP

#include"core.hpp"
#include"coreLink.hpp"
class App;
class Ui;

#include<iostream>

class AppCommandInterface {

public:
	AppCommandInterface(Core * _core, CoreLink *_coreLink, App * _app, Ui* _ui );
	~AppCommandInterface();

    void init();

    void update();

	void draw();

private:

	Core * core = nullptr;
	CoreLink* coreLink = nullptr;
	App * app = nullptr;
	Ui* ui = nullptr;
};

#endif